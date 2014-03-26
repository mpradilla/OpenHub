#!/usr/bin/env python
import pika
import logging
import importlib
import glob
import shutil
import os
import json
import datetime
import traceback
import requests
import zipfile
import StringIO
import signal
from openhub_exceptions import TimeoutException
from pymongo import MongoClient
from contextlib import contextmanager
import linecache
import sys
from xml.etree import ElementTree
import github3
from bson.objectid import ObjectId
import pymongo
import collections
import time
import logging

DATA_PATH = '../data'
BASE_DIR = ''
REPO_DOWNLOAD_DIR = ''

RABBIT_HOST = ''
RABBIT_USER = ''
RABBIT_PWD = ''
RABBIT_KEY = ''
RABBIT_QUEUE = ''

MONGO_HOST = ''
MONGO_PORT = 0
MONGO_USER = ''
MONGO_PWD = ''
MONGO_DB = ''
MONGO_COLL = ''
MONGO_COLL_VERSION = 'cversion'
MONGO_COLL_BLACKLIST = 'blacklist'
MONGO_COLL_REPO_VERSIONS = 'repoVersions'


GH_USERS = []
GH_CUR_USR = 0
API_LIMITS_EX= 0

dirs = []
collection = ''
collectionVersion =''
collectionRepoVersions =''
collectionBlacklist = ''
#evolution = {"dsm_packages_clustering_cost":[], "dsm_packages_propagation_cost":[], "dsm_packages_size":[],"dsm_classes_clustering_cost":[], "dsm_classes_propagation_cost":[], "dsm_classes_size":[], "dsm_process_time":[], "project_size":[], "dates":[]}
evolution = collections.defaultdict(list)
versions =[]

def main():
    global collection
    global collectionVersion
    global collectionRepoVersions
    global collectionBlacklist

    load_config()
    
    logging.basicConfig(filename='manager2.log', level=logging.INFO)
    logging.info('Started at %s', str(time.time()))

    # Database connection
    client = MongoClient(MONGO_HOST, MONGO_PORT)
    db = client[MONGO_DB]
    db.authenticate(MONGO_USER, MONGO_PWD)
    collection = db[MONGO_COLL]

    collectionVersion = db[MONGO_COLL_VERSION]
    collectionRepoVersions = db[MONGO_COLL_REPO_VERSIONS]
    collectionBlacklist = db[MONGO_COLL_BLACKLIST]

    # Necesary logging
    logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.CRITICAL)

    # RabbitMQ connection
    server_credentials = pika.PlainCredentials(RABBIT_USER, RABBIT_PWD)
    connection = pika.BlockingConnection(pika.ConnectionParameters(host=RABBIT_HOST, credentials=server_credentials))
    # connection = pika.BlockingConnection(pika.ConnectionParameters(RABBIT_HOST))

    # Queue and Exchange declaration
    channel = connection.channel()
    channel.exchange_declare(exchange='repo_classifier', type='direct')
    result = channel.queue_declare(queue=RABBIT_QUEUE, durable=True)
    queue_name = result.method.queue

    # Queue Binding
    channel.queue_bind(exchange='repo_classifier', queue=queue_name, routing_key=RABBIT_KEY)

    print ' [*] Waiting for messages. To exit press CTRL+C'

    # Channel Options and Execution
    channel.basic_qos(prefetch_count=1)
    channel.basic_consume(callback, queue=queue_name)
    channel.start_consuming()


def getCleanCommitList(repo_id, html_url, name):
    """
    This method get all the commits with pom.xml file changes for a specific repository.
    Then clean all commits, and leave only one for each day.
    Then download and analyze each pom.xml file searching for dependencie additions or suppresions
    Then leave only 2 commits with no changes between commits with changes, trying to hace the biggest distance between them.
    
    :params: Github repository id
    :return: json_data with the selected commits for a repository
    
    """
    global GH_CUR_USR
    global versions
    global API_LIMITS_EX
    
    try:
        os.chdir(REPO_DOWNLOAD_DIR)
        
        #GET ALL COMMITS FOR A REPO
        r = requests.get("https://api.github.com/repositories/"+str(repo_id)+"/commits?path=pom.xml&per_page=100", auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
        json_data = json.loads(r.text)
        delete_repo(REPO_DOWNLOAD_DIR+"pom.xml")
        
        if 'documentation_url' in json_data and json_data['documentation_url'] == "http://developer.github.com/v3/#rate-limiting":
            print "limit API exceded"
            API_LIMITS_EX+=1
            logging.warning('API limit exceeded at %s, this is exceeded number: %s', str(time.time()), str(API_LIMITS_EX))
            GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)
            return getCleanCommitList(repo_id, html_url, name)
    
        
        end=False
        #Initial commit in first page, avoid while
        if len(json_data)==0 or json_data[-1]['parents'] is None or len(json_data[-1]['parents'])==0:
            end=True
        else:
            #Last commit sha from page
            sha = json_data[-1]['sha']
        
        while not end:
        
            r = requests.get("https://api.github.com/repositories/"+str(repo_id)+"/commits?path=pom.xml&per_page=100&last_sha="+str(sha), auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
            next_data = json.loads(r.text)
            delete_repo(REPO_DOWNLOAD_DIR+"pom.xml")

            if 'documentation_url' in next_data and next_data['documentation_url'] == "http://developer.github.com/v3/#rate-limiting":
                print "limit API exceded"
                API_LIMITS_EX+=1
                logging.warning('API limit exceeded at %s, this is exceeded number: %s', str(time.time()),str(API_LIMITS_EX))
                GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)

            elif next_data is None or len(next_data)==0:
                end=True
            else:
                #Last commit sha from page
                sha = next_data[-1]['sha']
                json_data = json_data + next_data


        #json_data contains now all commits for repo
        # Remove commits for the same day, let only one -  TODO - UPDATE: let the las commit from day, not the first one!
        version= {"repo_id":"","sha":"","date":"", "html_url":"", "full_name":"", "external_dependencies":{"use": {}, "used_by": {}, "new":{}, "removed":{}, "compare_to":""}, "state":"pending", "analyzed_at":"", "next_sha":"", "last_sha":"", "stable":"", "analyze":"", "dsm":{}}
        
        last_day=-1
        last_sha=0
        last_deps=None
        for commit in json_data:
        
            day = getDateAsNumber(commit['commit']['author']['date'])
            sh = commit['sha']
            print "COMMIT SHA %s FROM DAY %s" % (sh,commit['commit']['author']['date'])
            
            if last_day==day:
                #json_data.remove(commit)
                print "COMMIT From same date DELETED - not deleted! "
            if day:
                last_day=day
                pomContent = downloadRepoPomFile(repo_id, name, sh)
                pomContent.encode('ascii', 'ignore')
                tree = ElementTree.fromstring(pomContent)
                mappings = None
                try:
                    mappings = getMappings(tree)
                except:
                    print "CANNOT GET DEPENDENCIES FROM POM... deleting commit from list"
                    json_data.remove(commit)
                    continue
                
                if mappings is not None and len(mappings)!=0:
                    
                    version= {"repo_id":"","sha":"","date":"", "html_url":"", "full_name":"", "external_dependencies":{"use": {}, "used_by": {}, "new":{}, "removed":{}, "compare_to":""}, "state":"pending", "analyzed_at":"", "next_sha":"", "last_sha":"", "stable":"", "analyze":"", "dsm":{}}
                    version['repo_id']= repo_id
                    version['sha'] = sh
                    version['date'] = commit['commit']['author']['date']
                    version['date_number'] = day
                    version['html_url'] = html_url
                    version['full_name'] = name
                    version['last_sha'] = last_sha
                    
                    version['external_dependencies']['use'] = mappings
                    removed = []
                    new= []
                    if last_deps is None:
                        version['external_dependencies']['compare_to'] = 0
                    else:
                        removed, new = compareCommits(last_deps,mappings)
                        version['external_dependencies']['new'] = new
                        version['external_dependencies']['removed'] = removed
                        version['external_dependencies']['compare_to'] = last_sha
                        last_deps = mappings
                    
                    if len(removed)>0 or len(new)>0:
                        print "CHANGE IN POM DEPENDENCIES!!!     num removed: " + str(len(removed)) + "  num new: "+str(len(new))
                        version['analyze']= 1
                        version['stable']= 0
                        stables=0
                    else:
                        version['stable']= 1
                        #Stable version to analyze will be define later, based on a more intelligent decision
                        version['analyze']= 0
        
                    last_deps = mappings
                    last_sha=sh
                else:
                    json_data.remove(commit)
                    print "COMMIT DELETED because of no depedencies in pom.xml where found"
                
                
                
                if len(versions)==0:
                    versions = [version]
                else:
                    versions.append(version)
        
                print version['sha']
                print "|||||||||||||||"
                print versions
        

        return versions
            
    except:
        PrintException()
    finally:
        os.chdir(BASE_DIR)

def getDateAsNumber(string_date):
    """
    param: string_date in format : 2010-09-05T20:01:30Z
    return: number in format 20100905
    """
    x = string_date.split('T',1);
    y = x[0].split('-')
    day = str(y[0])+str(y[1])+ str(y[2])
    return day

def downloadRepoPomFile(repo_id, name, sha):
    
    global GH_CUR_USR
    r = requests.get("https://raw.github.com/"+str(name)+"/"+str(sha)+"/pom.xml", auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
    #json_data = json.loads(r.text)
    #delete_repo(REPO_DOWNLOAD_DIR+"pom.xml")
    
    if 'Max retries exceeded' in r.text:
        print "limit API exceded"
        print r.text
        API_LIMITS_EX+=1
        logging.warning('API limit exceeded at %s, this is exceeded number: %s', str(time.time()),str(API_LIMITS_EX))
        GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)
        return downloadRepoPomFile(repo_id,name,sha)
    
    else:
        return r.text



    #if 'documentation_url' in r and json_data['documentation_url'] == "http://developer.github.com/v3/#rate-limiting":
    #    print "limit API exceded"
    #    GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)
    #    return downloadRepoPomFile(repo_id,name,sha)
    #else:
        #return r.content

def downloadRepo(html_url, sha, repo_id):

    global GH_CUR_USR
    os.chdir(REPO_DOWNLOAD_DIR)
    
    try:
    
         with time_limit(1600):
    
             downUrl= html_url+'/archive/'+sha+'.zip'
             r = requests.get(downUrl, auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
        #if 'documentation_url' in json_data and json_data['documentation_url'] == "http://developer.github.com/v3/#rate-limiting":
        #GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)
        #r = requests.get(downUrl, auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
    
             z = zipfile.ZipFile(StringIO.StringIO(r.content))
             z.extractall()
             os.chdir(BASE_DIR)
             return True
    except:
        
        PrintException()
        logging.error('DOWNLOADING THE REPO %s', str(time.time()), str(PrintException()))
        collectionBlacklist.save({"_id": repo_id, "value":"DOWNLOAD_TIME"})
        os.chdir(BASE_DIR)
        print "TIME LIMIT FOR DOWNLOAD EXCCEEDED"
        return False


def analyzeVersion(repo_id, path, version, complete):
    '''
        
    :param: path of project already EXTRACTED and ready for analysis
    '''
    os.chdir(BASE_DIR)
    #ANALYZE the project
    
    ## Get test available
    for d in dirs:
        print "Analyzing %s..." % d
        tests = [p.replace('/', '.') for p in glob.glob("%s/*" % d) if os.path.isdir(p)]  # Test list in the directory
    
    print "Current tests for %s: %s" % (d, tests)
    completed = False
    for test in tests:
        
        if complete or test=="dsm":
            m = importlib.import_module(test + ".main")
            test_name = test.split('.')[1]
            print "TEST: " + str(test_name)
        
            try:
                # 1 hour time limit for one project release
                with time_limit(3600):
                    res = m.run_test(repo_id, path, version)
                    print "ANALYZER REPONSE: " + str(res)
                    print repo_id
                        #if d is not None and d!=0:
                        #version[d][test_name] = res
                        
                    version[test_name] = res
                        
                    if "error(100):" in res:
                        #ERROR COMPILING THE CODE WITH MAVEN
                        logging.error('Compiling with maven the project %s', res)
                        return False
            
            
                completed = True
        
            except Exception as e:
                print 'Test error: %s %s' % (test, str(e))
                # data = {'name': test_name, 'value': "Error:" + str(e)}
                #version[d][test_name] = {'error': "Error:" + str(e), 'stack_trace': traceback.format_exc()}
                completed = False
                PrintException()
                pass
    
    version['analyzed_at'] = datetime.datetime.now()
    version['state'] = 'completedV5' if completed else 'pending'

    delete_repo(path)

    return version

def saveVersionAnalysisResult(version, repo_id):
    
    print version

    version_json = collectionVersion.find_one({"sha": version["sha"]},{"_id":1})

    print "version_json in database: " + str(version_json)
    
    if version_json is None:
        _id = collectionVersion.save(version)
        print "VERSION ID" + str(_id)
        collectionRepoVersions.insert({"repo":repo_id, "version":_id})
        print "VERSION SAVED"

    else:
        print "Version already existed, updating the json..."
        _id = collectionVersion.update({"_id":version_json["_id"]}, version)
        collectionRepoVersions.insert({"repo":repo_id, "version":_id})
        print "VERSION SAVED"

    #collection.update({"_id": repo_id}, repo_json)


def compareCommits(oldDeps, newDeps):

    new = {}
    removed = {}
    
    if len(newDeps) > len(oldDeps) or len(newDeps) == len(oldDeps):
        for depKey in newDeps:
            if depKey not in oldDeps:
                new[depKey]=newDeps[depKey]

    if len(newDeps) < len(oldDeps):
        for depKey in oldDeps:
            if depKey not in newDeps:
                removed[depKey]=oldDeps[depKey]


    return removed,new

######################################################################################################
##### METTHODS FOR POM.XML DEPENDENCIES EXTRACTION
######################################################################################################

#Auxiliar method to analyze the pom.xml file
def getMappingsNode(node, nodeName):
    if node.findall('*'):
        for n in node.findall('*'):
            if nodeName in n.tag:
                return n
        else:
            return getMappingsNode(n, nodeName)
#Method to extract the dependencies and put ir all togther in a python dictionary #structure
def getMappings(rootNode):
    mappingsNode = getMappingsNode(rootNode, 'dependencies')
    mapping = {}
    
    for prop in mappingsNode.findall('*'):
        key = ''
        val = ''
        
        for child in prop.findall('*'):
            if 'artifactId' in child.tag:
                key = child.text
                #KEYS IN PYHTON DICTIONARY CANNNOT START WITH '$'
                if '$' in key[0]:
                    #Remove first char from string
                    key = key[1:]
            
            if 'version' in child.tag:
                val = child.text
        
        if val and key:
            key= key.replace('.','/')
            mapping[key] = val
    
    return mapping

######################################################################################################
##### METTHODS FOR POM.XML DEPENDENCIES EXTRACTION
######################################################################################################

def delete_repo(path):
    """Delete repo from path."""
    if os.path.exists(path):
        shutil.rmtree(path)


@contextmanager
def time_limit(seconds):
    def signal_handler(signum, frame):
        raise TimeoutException, "Timed out!"
    signal.signal(signal.SIGALRM, signal_handler)
    signal.alarm(seconds)
    try:
        yield
    finally:
        signal.alarm(0)


def callback(ch, method, properties, body):
    """Queue callback function.
    Will be executed when the queue pops this worker
    """
    global evolution
    global versions
   
    blacklistThis = False
    data = body.split("::")
    html_url = data[0]
    repo_id = int(data[1])
    name = data[2]
    full_name = html_url.split("/",3)[3]
    
    print " [x] Received %r" % (data,)
    print full_name
    logging.info('START Analysis for repo %s with id %s',full_name,str(repo_id))

    path = '%s/%s' % (REPO_DOWNLOAD_DIR, name)
    try:
        
        skip=False
        repo_json = collection.find_one({"_id": repo_id})
        
        if collectionBlacklist.find_one({"_id": repo_id}):
            print "PROJECT IN BLACKLIST... skipping analysis"
            logging.error('Repo in blacklist %s with id %s ... skipping ...',full_name,str(repo_id))
            skip=True
        
        elif repo_json is None:
            print "no repo in database"
            logging.error('Repo not found in Database %s with id %s ... skipping ...',full_name,str(repo_id))
            skip=True
        
        else:
            print "Downloading pom file..."
            pomText = downloadRepoPomFile(repo_id, full_name, "master")
            print "Pom download completed."
            
            #CHECK IF THERE IS NO POM FILE IN THE REPOSITORY
            if "<!DOCTYPE html>" in pomText[0:50] or repo_id==24768:
                #404 Github Page, there is no pom there!
                collectionBlacklist.save({"_id": repo_id, "value":"NO_POM"})
                print "REPO HAS NO POM.XML. ADDED TO BLACKLIST"
                logging.error('Repo has no pom.xml file from repo %s with id %s ... skipping ...',full_name,str(repo_id))
                skip=True
            
            #CHECK IF THERE ARE DEPEDENCIES IN POM FILE - !! WARINING
            else:
                
                pomText.encode('ascii', 'ignore')
                tree = ElementTree.fromstring(pomText)
                mappings = None
                try:
                    mappings = getMappings(tree)
                except:
                    print "CANNOT GET DEPENDENCIES FROM MASTER POM... skipping project and adding it to blacklist"
                    collectionBlacklist.save({"_id": repo_id, "value":"NO_DEPENDENCIES_IN_POM"})
                    logging.error('Cannot get dependencies from pom.xml file from repo %s with id %s ... skipping ...',full_name,str(repo_id))
                    skip=True
     
        print "Downloading master source code..."
        #Download master and check viablitiy of analysis for the project. Avoid get all pom.xml commits analysis if is inpossible to analyze
        before = time.time()
        if skip==False and downloadRepo(html_url, "master",repo_id):
            
            repo_json['download_time'] = str(time.time() - before)
            logging.info('Download time for project: %s with id: %s was %s', full_name, str(repo_id), str(time.time() - before) )
            print "Master download completed."
            print time.time() - before
            
            #Download porject Master
            path = '%s/%s' % (REPO_DOWNLOAD_DIR, name+'-master')
            version= {"repo_id":str(repo_id),"sha":"master","date":"", "html_url":"", "full_name":"", "external_dependencies":{"use": {}, "used_by": {}, "new":{}, "removed":{}, "compare_to":""}, "state":"pending", "analyzed_at":"", "next_sha":"", "last_sha":"0", "stable":"", "analyze":"", "tags":{}, "dsm":{}}
            #Try to analyze master
            version = analyzeVersion(repo_id, path, version, True)
            
            #Save result into version
            saveVersionAnalysisResult(version,repo_id)
            
            repo_json["dsm"] = version["dsm"]
            
            if version==False:
            
                repo_json['evolution'] = {"error":"Could compile master version"}
                collectionBlacklist.save({"_id": repo_id, "value":"COULD_NOT_COMPILE"})
                repo_json['state'] = {"state":"blacklistV5'","value":"COULD_NOT_COMPILE"}
                logging.error('Cannot analyze repo %s with id %s',full_name,str(repo_id))
   
   
            elif "error" not in version:
                
                versions = []
                versions = getCleanCommitList(repo_id, repo_json['html_url'], full_name)
                print "***********************************************"
                
                if versions and len(versions)>0:
                    logging.info('Versions found for project: %s with id: %s are in total %s', full_name, str(repo_id), str(len(versions)))
                    print "NUM OF VERSIONS SELECTED: %i" % (len(versions))
                    repo_json['number_versions'] = len(versions)
                    numforAnalysis=0
                    succeedAnalyzed=0
                    
                
                    for ver in versions:
                        if ver['analyze']==1:
                            numforAnalysis+=1
                            print "ANALYZIND REPO VERSION WITH SHA %s" % (ver['sha'])
                            logging.info('Analyzing version with sha %s from project: %s with id: %s', ver['sha'], full_name, str(repo_id))
                        
                            downloadRepo(repo_json['html_url'], ver['sha'], repo_id)
                            pathx = '%s/%s' % (REPO_DOWNLOAD_DIR, name+'-'+ver['sha'])
                            respVersion = analyzeVersion(repo_id, pathx, ver, True)
                            
                            if "dsm" not in respVersion:
                                print "ERROR! - DSM tag not found in analysis response"
                                logging.error('Analyzing version with sha %s from project: %s with id: %s - DSM not in analysis response', ver['sha'], full_name, str(repo_id))
                                respVersion['dsm_extracted'] = 0
                            
                            elif "error" in respVersion["dsm"]:
                                print "ERROR! - Building and getting DSM"
                                logging.error('Analyzing version with sha %s from project: %s with id: %s - Error tag in analysis response', ver['sha'], full_name, str(repo_id))
                                respVersion['dsm_extracted'] = 0
                            else:
                                succeedAnalyzed+=1
                                logging.info('Succed analisis for version with sha:%s from project: %s with id: %s', ver['sha'], full_name, str(repo_id))
                                respVersion['dsm_extracted'] = 1
                            
                            print "&&&&&&&&&&&&&&&&&&&&&"
                            print respVersion
                            print "&&&&&&&&&&&&&&&&&&&&&"
                            
                            saveVersionAnalysisResult(respVersion,repo_id)
                        
                        

                    print "ANALYZED VERSIONS: %i" % numforAnalysis
                    print "REAL ANALYZED VERSIONS: %i" % succeedAnalyzed
                    completed= True
                    repo_json['number_versions_with_changes'] = numforAnalysis
                    repo_json['state'] = 'completedV5'
                    repo_json['succeed_versions_analyzed']= succeedAnalyzed
                else:
                    completed= False
                    repo_json['number_versions_with_changes'] = 0
                    repo_json['state'] = {"error":"Number of versions for analysis equal 0"}
                    logging.info('No changes in pom.xml file for project: %s with id: %s', full_name, str(repo_id))
        
            else:
                print "ERROR in master version analysis, complete evolution analysis skipped"
                repo_json['evolution'] = {"error":"Could not analyze master version"}
                collectionBlacklist.save({"_id": repo_id, "value":"NO_EVOLUTION_CHANGES"})
                repo_json['state'] = {"state":"blacklistV5'","value":"NO_EVOLUTION_CHANGES"}
                logging.error('ERROR in master version analysis for project: %s with id: %s', full_name, str(repo_id))

            repo_json['analyzed_at'] = datetime.datetime.now()

                        

                        
            print "***********************************************"
            print "***********************************************"
            print evolution
            print "***********************************************"
                    
                    

        else:
        
            print "COULD NOT DOWNLOAD MASTER VERSION"
            repo_json['evolution'] = {"error":"Could not analyze master version"}
            repo_json['state'] = 'blacklistV5'
            logging.error('COULD NOT DOWNLOAD MASTER VERSION for project: %s with id: %s', full_name, str(repo_id))


        print "Saving results to databse..."
        collection.update({"_id": repo_id}, repo_json)


        print "Deleting files..."
        delete_repo(path)

        ch.basic_ack(delivery_tag=method.delivery_tag)
        print " [x] Ready, waiting for next repo...\n"
        print "------------------------------------\n"

    except Exception as e:
        print "General error:", str(e)
        
        PrintException()
        logging.error('CRITICAL ERROR in alaysis of project: %s with id: %s   -    %s', full_name, str(repo_id), str(PrintException()))

        collection.update({"_id": repo_id}, {'$set': {'state': 'failed', 'analyzed_at': datetime.datetime.now(), 'error': 'General error:' + str(e), 'stack_trace': traceback.format_exc()}})
        print "Updated repo with failed status"

        print "Deleting files..."
        delete_repo(path)

        ch.basic_ack(delivery_tag=method.delivery_tag)
        print "Will continue. Waiting for next repo...\n"
        print "------------------------------------\n"
        # TODO Terminar de manejar los errores


def load_config():
    global BASE_DIR, REPO_DOWNLOAD_DIR, RABBIT_HOST, RABBIT_USER, RABBIT_PWD, RABBIT_KEY, RABBIT_QUEUE, GH_USERS, MONGO_PWD, MONGO_USER, MONGO_HOST, MONGO_PORT, MONGO_DB, MONGO_COLL, MONGO_COLL_VERSION, MONGO_COLL_REPO_VERSIONS, MONGO_COLL_BLACKLIST, dirs

    paths_cfg = open(DATA_PATH + "/paths.json")
    data = json.load(paths_cfg)
    BASE_DIR = str(data['BASE_DIR'])
    REPO_DOWNLOAD_DIR = str(data['REPO_DOWNLOAD_DIR'])
    paths_cfg.close()

    rabbit_cfg = open(DATA_PATH + "/rabbit.json")
    data = json.load(rabbit_cfg)
    RABBIT_HOST = str(data['RABBIT_HOST'])
    RABBIT_USER = str(data['RABBIT_USER'])
    RABBIT_PWD = str(data['RABBIT_PWD'])
    RABBIT_KEY = str(data['RABBIT_KEY'])
    RABBIT_QUEUE = str(data['RABBIT_QUEUE'])
    rabbit_cfg.close()

    d = open(DATA_PATH + "/analyzers.json")
    dirs = set(json.load(d))
    d.close()
    
    gh_cfg = open(DATA_PATH + "/gh.json")
    GH_USERS = json.load(gh_cfg)
    gh_cfg.close()

    mongo_cfg = open(DATA_PATH + "/mongo.json")
    data = json.load(mongo_cfg)
    MONGO_USER = data['MONGO_USER']
    MONGO_PWD = data['MONGO_PWD']
    MONGO_HOST = data['MONGO_HOST']
    MONGO_PORT = int(data['MONGO_PORT'])
    MONGO_DB = data['MONGO_DB']
    MONGO_COLL = data['MONGO_COLL']
    MONGO_COLL_VERSION = data['MONGO_COLL_VERSION']
    MONGO_COLL_REPO_VERSIONS = data['MONGO_COLL_REPO_VERSIONS']
    MONGO_COLL_BLACKLIST = data['MONGO_COLL_BLACKLIST']
    mongo_cfg.close()

def PrintException():
    exc_type, exc_obj, tb = sys.exc_info()
    f = tb.tb_frame
    lineno = tb.tb_lineno
    filename = f.f_code.co_filename
    linecache.checkcache(filename)
    line = linecache.getline(filename, lineno, f.f_globals)
    print 'EXCEPTION IN ({}, LINE {} "{}"): {}'.format(filename, lineno, line.strip(), exc_obj)


if __name__ == '__main__':
    main()

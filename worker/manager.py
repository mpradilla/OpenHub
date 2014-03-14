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

dirs = []
collection = ''
collectionVersion =''
collectionRepoVersions =''
collectionBlacklist = ''
evolution = {"dsm_packages_clustering_cost":[], "dsm_packages_propagation_cost":[], "dsm_packages_size":[],"dsm_classes_clustering_cost":[], "dsm_classes_propagation_cost":[], "dsm_classes_size":[], "dsm_process_time":[], "project_size":[], "dates":[]}


def main():
    global collection
    global collectionVersion
    global collectionRepoVersions
    global collectionBlacklist

    load_config()

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


def getCleanCommitList(repo_id, html_url, path, name):
    """
    This method get all the commits with pom.xml file changes for a specific repository.
    Then clean all commits, and leave only one for each day.
    Then download and analyze each pom.xml file searching for dependencie additions or suppresions
    Then leave only 2 commits with no changes between commits with changes, trying to hace the biggest distance between them.
    
    :params: Github repository id
    :return: json_data with the selected commits for a repository
    
    """
    global GH_CUR_USR
    
    try:
        os.chdir(REPO_DOWNLOAD_DIR)
        
        #GET ALL COMMITS FOR A REPO
        r = requests.get("https://api.github.com/repositories/"+str(repo_id)+"/commits?path=pom.xml&per_page=100", auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
        json_data = json.loads(r.text)
        delete_repo(REPO_DOWNLOAD_DIR+"pom.xml")
        
        if 'documentation_url' in json_data and json_data['documentation_url'] == "http://developer.github.com/v3/#rate-limiting":
            print "limit API exceded"
            GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)
            getCleanCommitList(repo_id, html_url, path, name)
        
        #Last commit sha from page
        sha = json_data[-1]['sha']
        
        end=false
        #Initial commit in first page, avoid while
        if json_data[-1]['parents'] is None or len(json_data[-1]['parents'])==0:
            end=true
        while not end:
        
            r = requests.get("https://api.github.com/repositories/"+str(repo_id)+"/commits?path=pom.xml&per_page=100&last_sha="+str(sha), auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
            next_data = json.loads(r.text)
            delete_repo(REPO_DOWNLOAD_DIR+"pom.xml")

            if 'documentation_url' in next_data and next_data['documentation_url'] == "http://developer.github.com/v3/#rate-limiting":
                print "limit API exceded"
                GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)

            elif next_data is None or len(next_data)==0:
                end=true
            else:
                #Last commit sha from page
                sha = next_data[-1]['sha']
                json_data = json_data + next_data


        #json_data contains now all commits for repo
        # Remove commits for the same day, let only one
        version= {"repo_id":"","sha":"","date":"","external_dependencies":{}, "html_url":"", "full_name":"", "dependencies":{"use": {}, "used_by": {}, "new":{}, "removed":{}, "compare_to":""}, "state":"pending", "analyzed_at":"", "next_sha":"", "last_sha":"", "stable":"", "analyze":""}
        versions = []
        last_day=-1
        last_sha=0
        last_deps=None
        for commit in json_data:
        
            day = getDateAsNumber(commit['commit']['author']['date'])
            
            if last_day==day:
                json_data.remove(commit)
                print "COMMIT From same date DELETED "
            else:
                last_day=day
                pomContent = downloadRepoPomFile(repo_id,name,commit['sha'])
                tree = ElementTree.fromstring(pomContent)
                mappings = None
                try:
                    mappings = getMappings(tree)
                except:
                    print "CANNOT GET DEPENDENCIES FROM POM... deleting commit from list"
                    json_data.remove(commit)
                    continue
                
                if mappings is not None and len(mappings)!=0:
                    
                    version['repo_id']= repo_id
                    version['sha'] = commit['sha']
                    version['date'] = commit['commit']['author']['date']
                    version['date_number'] = day
                    version['html_url'] = html_url
                    version['full_name'] = name
                    version['last_sha'] = last_sha
                    
                    version['dependencies']['use'] = mappings
                    if last_deps is None:
                        version['dependencies']['compare_to'] = 0
                    else:
                        removed, new = compareCommits(last_deps,mappings)
                        version['dependencies']['new'] = new
                        version['dependencies']['removed'] = removed
                        version['dependencies']['compare_to'] = last_sha
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
                    last_sha=commit['sha']
                else:
                    json_data.remove(commit)
                    print "COMMIT DELETED because of no depedencies in pom.xml where found"
                
                versions.append(version)

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


    r = requests.get("https://raw.github.com/"+str(name)+"/"+str(sha)+"/pom.xml", auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
    json_data = json.loads(r.text)
    delete_repo(REPO_DOWNLOAD_DIR+"pom.xml")
    
    if 'documentation_url' in json_data and json_data['documentation_url'] == "http://developer.github.com/v3/#rate-limiting":
        print "limit API exceded"
        GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)
        return downloadRepoPomFile(repo_id,name,sha)
    else:
        return r.content

def downloadRepo(html_url, sha):

    os.chdir(REPO_DOWNLOAD_DIR)
    
    downUrl= html_url+'/archive/'+sha+'.zip'
    r = requests.get(downUrl, auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
    if 'documentation_url' in json_data and json_data['documentation_url'] == "http://developer.github.com/v3/#rate-limiting":
        GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)
        r = requests.get(downUrl, auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
    
    z = zipfile.ZipFile(StringIO.StringIO(r.content))
    z.extractall()
    return true

    os.chdir(BASE_DIR)


def analyzeVersion(path, version):
    '''
        
    :param: path of project already EXTRACTED and ready for analysis
    '''
    
    os.chdir(BASE_DIR)
    #COMPILE the project
    
    
    #ANALYZE the project
    
    ## Get test available
    for d in dirs:
        print "Analyzing %s..." % d
        tests = [p.replace('/', '.') for p in glob.glob("%s/*" % d) if os.path.isdir(p)]  # Test list in the directory
    
    print "Current tests for %s: %s" % (d, tests)
    completed = True
    for test in tests:
        m = importlib.import_module(test + ".main")
        test_name = test.split('.')[1]
        print "TEST: " + str(test_name)
        
        try:
            # 1 hour time limit for one project release
            with time_limit(3600):
                res = m.run_test(repo_id, path, version)
                print "ANALYZER REPONSE: " + str(res)
                print repo_id
                if d is not None and d!=0:
                    #version[d][test_name] = res
                    version["dsm"] = res
            completed = True
        
        except Exception as e:
            print 'Test error: %s %s' % (test, str(e))
            # data = {'name': test_name, 'value': "Error:" + str(e)}
            #version[d][test_name] = {'error': "Error:" + str(e), 'stack_trace': traceback.format_exc()}
            completed = False
            PrintException()
            pass
    
    version['analyzed_at'] = datetime.datetime.now()
    version['state'] = 'completedV3' if completed else 'pending'
    print "Saving results to databse..."
    

    
    delete_repo(path)

def saveVersionAnalysisResult(version, repo_id):

    _id = collectionVersion.insert(version)
    print "VERSION ID" + str(_id)
    collectionRepoVersions.insert({"repo":repo_id, "version":_id})
    print "VERSION SAVED"


def down_repo(repo_id, html_url, path, name):
    """Download repo from specified url into path."""
    global GH_CUR_USR

    delete_repo(path)
    try:
        os.chdir(REPO_DOWNLOAD_DIR)
        
        
        
        
        
        
        
        #Link to get all commits SHA's where the pom.xml file was modified
        #https://api.github.com/repositories/160985/commits?path=pom.xml&per_page=100
        
        reauth = True
        gh = None
        
        #https://api.github.com/repositories/160985/commits?path=pom.xml&last_sha=9a30d48c276983f57997d93edf2aa0cb6d7d72a1&per_page=100
        r = requests.get("https://api.github.com/repositories/"+str(repo_id)+"/commits?path=pom.xml&per_page=100", auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
        json_data = json.loads(r.text)
        
        if 'documentation_url' in json_data and json_data['documentation_url'] == "http://developer.github.com/v3/#rate-limiting":
            print "limit API exceded"
            GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)
            #gh = github3.login(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd'])
            return "LOGIN response" + str (gh)
            r = requests.get("https://api.github.com/repositories/"+str(repo_id)+"/commits?path=pom.xml", auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
            json_data = json.loads(r.text)
        
        print "Repos ID: " + str(repo_id)
        print json_data
        if json_data:
        
            #shas = [commit['sha'] for commit in json_data]
            #shas = []
            #for commit in json_data:
            #    shas.append(str(commit['sha']))
            
            version= { "modularity":{"external_dependencies":{}, "dsm":{} ,"tags": {} }, "date":"", "html_url":"", "dependencies":{"use": {}, "used_by": {}, "new":{}, "removed":{}}}
            last_deps = None
            stables = 0
            analyze = False
            for commit in json_data:
                
                
    
                if analyze:
                    print "Downloading code..."
                    r = requests.get(downUrl, auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
                    if 'documentation_url' in json_data and json_data['documentation_url'] == "http://developer.github.com/v3/#rate-limiting":
                        GH_CUR_USR = (GH_CUR_USR + 1) % len(GH_USERS)
                        #gh = github3.login(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd'])
                        r = requests.get(downUrl, auth=(GH_USERS[GH_CUR_USR]['login'], GH_USERS[GH_CUR_USR]['pwd']))
                    
                    z = zipfile.ZipFile(StringIO.StringIO(r.content))
                    z.extractall()
    
                    #repo_json = collectionVersions.db.collection.find({"sha":version['sha']}, {"sha": 1}).limit(1)
    
                    os.chdir(BASE_DIR)
    
                    for d in dirs:
                        print "Analyzing %s..." % d
                        tests = [p.replace('/', '.') for p in glob.glob("%s/*" % d) if os.path.isdir(p)]  # Test list in the directory
                    
                    print "Current tests for %s: %s" % (d, tests)
                    # repo_json[d] = []
                    #repo_json[d] = {}
                    completed = True
                    for test in tests:
                        m = importlib.import_module(test + ".main")
                        test_name = test.split('.')[1]
                        print "TEST: " + str(test_name)
                        try:
                            with time_limit(3600):
                                res = m.run_test(repo_id, path, version)
                                print "ANALYZER REPONSE: " + str(res)
                                print repo_id
                                if d is not None and d!=0:
                                    version[d][test_name] = res
                    
                                #evolution
                                if test_name == "dsm":
                                    evolution["dsm_packages_clustering_cost"].append(res["dsm_packages_clustering_cost"])
                                    evolution["dsm_packages_propagation_cost"].append(res["dsm_packages_propagation_cost"])
                                    evolution["dsm_packages_size"].append(res["dsm_packages_size"])
                                    
                                    
                                    evolution["dsm_classes_clustering_cost"].append(res["dsm_classes_clustering_cost"])
                                    evolution["dsm_classes_propagation_cost"].append(res["dsm_classes_propagation_cost"])
                                    evolution["dsm_classes_size"].append(res["dsm_classes_size"])
                                    evolution["dsm_process_time"].append(res["dsm_process_time"])

                                    evolution["project_size"].append(res["project_size"])
                                    evolution["dates"].append(version['date'])
                                    print "EVOLUTION RECORD: "+ str(evolution)
                            completed = True
                        except Exception as e:
                            print 'Test error: %s %s' % (test, str(e))
                            # data = {'name': test_name, 'value': "Error:" + str(e)}
                            #version[d][test_name] = {'error': "Error:" + str(e), 'stack_trace': traceback.format_exc()}
                            completed = False
                            PrintException()
                            pass
                                
                    version['analyzed_at'] = datetime.datetime.now()
                    version['state'] = 'completedV2' if completed else 'pending'
                    print "Saving results to databse..."
                    
                    _id = collectionVersion.insert(version)
                    print "VERSION ID" + str(_id)
                    collectionRepoVersions.insert({"repo":repo_id, "version":_id})
                    
                    delete_repo(path)    
                        
            
            return evolution

        else:
            print "NO POM.XML"
            return None
        # print git.Git().clone(html_url)
        # subprocess.call(['git', 'clone', html_url], close_fds=True)
        #r = requests.get(html_url)
        #z = zipfile.ZipFile(StringIO.StringIO(r.content))
        #z.extractall()

        os.chdir(BASE_DIR)
        print "Done"
    except:
        PrintException()
        raise
    finally:
        os.chdir(BASE_DIR)


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
            
            if 'version' in child.tag:
                val = child.text
        
        if val and key:
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
    data = body.split("::")
    html_url = data[0]
    repo_id = int(data[1])
    name = data[2]
    print " [x] Received %r" % (data,)

    path = '%s/%s' % (REPO_DOWNLOAD_DIR, name)
    try:
        repo_json = collection.find_one({"_id": repo_id})
        
        
        #Download master and check viablitiy of analysis for the project. Avoid get all pom.xml commits analysis if is inpossible to analyze.
        if downloadRepo(repo_json['html_url'], "master"):
        
            #Download porject Master
            downloadRepo(repo_json['html_url'], "master")
            path = '%s/%s' % (REPO_DOWNLOAD_DIR, name+'-master')
            version= {"repo_id":repo_id,"sha":"master","date":"","external_dependencies":{}, "html_url":"", "full_name":"", "dependencies":{"use": {}, "used_by": {}, "new":{}, "removed":{}, "compare_to":""}, "state":"pending", "analyzed_at":"", "next_sha":"", "last_sha":"0", "stable":"", "analyze":""}
            #Try to analyze master
            version = analyzeVersion(path, version)
            #Save result into version
            saveVersionAnalysisResult(version,repo_id)
      
      
        
        else:
        
        
        
        
        
        
        # down_repo(html_url, path)
        #down_repo(repo_id, repo_json['html_url'] + '/archive/master.zip', path , repo_json['full_name'])
        down_repo(repo_id, repo_json['html_url'], path , repo_json['full_name'])
        completed = True
        
        if evolution is None:
            collectionBlacklist.insert({"_id": repo_id})
        
        
        repo_json['evolution'] = evolution

        repo_json['analyzed_at'] = datetime.datetime.now()
        
        if evolution:
            repo_json['state'] = 'completedV2'
        elif completed is None:
            repo_json['state'] = 'NO_POM'
        else:
            repo_json['state'] = 'pending'

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

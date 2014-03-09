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
MONGO_COLL_VERSION = 'versions'
MONGO_COLL_BLACKLIST = 'blacklist'

dirs = []
collection = ''
collectionVersions =''
collectionBlacklist = ''
evolution = []


def main():
    global collection

    load_config()

    # Database connection
    client = MongoClient(MONGO_HOST, MONGO_PORT)
    db = client[MONGO_DB]
    db.authenticate(MONGO_USER, MONGO_PWD)
    collection = db[MONGO_COLL]

    collectionVersions = db[MONGO_COLL_VERSION]
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


def down_repo(repo_id, git_url, path, name):
    """Download repo from specified url into path."""

    delete_repo(path)
    try:
        os.chdir(REPO_DOWNLOAD_DIR)
        
        #Link to get all commits SHA's where the pom.xml file was modified
        #https://api.github.com/repositories/160985/commits?path=pom.xml&per_page=100
        
        r = requests.get("https://api.github.com/repositories/"+str(repo_id)+"/commits?path=pom.xml")
        json_data = json.loads(r.text)
        
        print "Repos ID: " + str(repo_id)
        
        if json_data:
        
            shas = [commit['sha'] for commit in json_data]
            
            version= {}
            last_deps = None
            stables = 0
            analyze = False
            for commit in json_data:
                version['sha']=commit['sha']
                version['date']=commit['commit']['author']['date']
                version['git_url']=git_url
                
                #download specific version of the project
                #https://github.com/apache/hbase/archive/18326945939ce48f8b567482dc3ae732d02debca.zip
                downUrl= git_url+'/archive/'+commit['sha']+'.zip'
                
                print "SHAs for repo: "+ str(shas)
                print "SHAs number: "+ str(len(shas))
    
                #Download specific pom.xml
                #https://raw.github.com/apache/hbase/5722bd679c0416483ab752a3e327f26a4ef8f18d/pom.xml
                
                analyze = False
                r = requests.get("https://raw.github.com/"+str(name)+"/"+str(commit['sha'])+"/pom.xml")
                
                mappings = getMappings(root+"/pom.xml")
                print mappings
                if len(mappings)!=0:
                    version['dependencies']['use'] = mappings
                    if last_deps is None:
                        version['dependencies']['compare_to'] = 1
                        analyze= True
                        downUrl= git_url+'/archive/master.zip'
                    else:
                        removed, new = compareCommits(last_deps,mappings)
                        version['dependencies']['new'] = new
                        version['dependencies']['removed'] = removed
                    
                        if len(removed)>0 or len(new)>0 or stables>5:
                            print "CHANGE IN POM DEPENDENCIES!!!     num removed: " + str(len(removed)) + "  num new: "+str(len(new))
                            analyze= True
                            stables=0
                        else:
                            stables+=1
        
                        last_deps = mappings
                #Delete pom.xml file
                delete_repo(path)
    
                if analyze:
                    print "Downloading code..."
                    r = requests.get(downUrl)
                    z = zipfile.ZipFile(StringIO.StringIO(r.content))
                    z.extractall()
    
                    #repo_json = collectionVersions.db.collection.find({"sha":version['sha']}, {"sha": 1}).limit(1)
    
                    for d in dirs:
                        print "Analyzing %s..." % d
                        tests = [p.replace('/', '.') for p in glob.glob("%s/*" % d) if os.path.isdir(p)]  # Test list in the directory
                    
                    print "Current tests for %s: %s" % (d, tests)
                    # repo_json[d] = []
                    repo_json[d] = {}
                    
                    for test in tests:
                        m = importlib.import_module(test + ".main")
                        test_name = test.split('.')[1]
                        try:
                            with time_limit(3600):
                                res = m.run_test(repo_id, path, version)
                                version[d][test_name] = res
                    
                                #evolution
                                #evolution[test_name].append(res['time_cost'])
                                if test_name == "dsm":
                                    evolution.append(version['date'])
                                    print "EVOLUTION RECORD: "+ str(evolution)
                    
                        except Exception as e:
                            print 'Test error: %s %s' % (test, str(e))
                            # data = {'name': test_name, 'value': "Error:" + str(e)}
                            version[d][test_name] = {'error': "Error:" + str(e), 'stack_trace': traceback.format_exc()}
                            completed = False
                            pass
                                
                    version['analyzed_at'] = datetime.datetime.now()
                    version['state'] = 'completed' if completed else 'pending'
                    print "Saving results to databse..."
                    collectionVersions.update({"sha": version['sha']}, version)
                    delete_repo(path)    
                        
            
            return evolution

        else:
            print "NO POM.XML"
            return None
        # print git.Git().clone(git_url)
        # subprocess.call(['git', 'clone', git_url], close_fds=True)
        #r = requests.get(git_url)
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
    git_url = data[0]
    repo_id = int(data[1])
    name = data[2]
    print " [x] Received %r" % (data,)

    path = '%s/%s' % (REPO_DOWNLOAD_DIR, name + '-master')
    try:
        repo_json = collection.find_one({"_id": repo_id})
        # down_repo(git_url, path)
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
    global BASE_DIR, REPO_DOWNLOAD_DIR, RABBIT_HOST, RABBIT_USER, RABBIT_PWD, RABBIT_KEY, RABBIT_QUEUE, MONGO_PWD, MONGO_USER, MONGO_HOST, MONGO_PORT, MONGO_DB, MONGO_COLL, dirs

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

    mongo_cfg = open(DATA_PATH + "/mongo.json")
    data = json.load(mongo_cfg)
    MONGO_USER = data['MONGO_USER']
    MONGO_PWD = data['MONGO_PWD']
    MONGO_HOST = data['MONGO_HOST']
    MONGO_PORT = int(data['MONGO_PORT'])
    MONGO_DB = data['MONGO_DB']
    MONGO_COLL = data['MONGO_COLL']
    MONGO_COLL_VERSION = data['MONGO_COLL_VERSION']
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

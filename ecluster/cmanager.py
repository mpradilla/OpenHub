import socket
from pymongo import MongoClient
import pymongo
import json
import base64
import zlib


DATA_PATH = '../data'
BASE_DIR = ''


MONGO_HOST = ''
MONGO_PORT = 0
MONGO_USER = ''
MONGO_PWD = ''
MONGO_DB = ''
MONGO_COLL = ''
MONGO_COLL_VERSION = 'cversion'
MONGO_COLL_BLACKLIST = 'blacklist'
MONGO_COLL_REPO_VERSIONS = 'repoVersions'

collection = ''
collectionVersion =''
collectionRepoVersions =''
collectionBlacklist = ''

HOST = '157.253.203.27'    # The remote host
PORT = 5001         # The same port as used by the server


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


# s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
#   s.connect((HOST, PORT))
    
    
    
    for version in collectionVersion.find({"dsm_extracted": 1}):
        
        repo_id = version["_id"]

        if "error" in version["dsm"]:
            break
        elif "dsm_classes" not in version["dsm"]:
            break
        elif "error" in version["dsm"]["dsm_classes"]:
            break
        elif "dsm" in version:
            fix = getBinaryMatrix(version["dsm"]["dsm_classes"])
            if fix and len(fix)>0:
                text = '$:'+ str(repo_id)+':'+str(len(fix))+':'+fix+':$'
            #           s.sendall(text)
                print 'Send', repr(text)
            #data = s.recv(1024)
            #print 'Received', repr(data)


#s.close()

def getBinaryMatrix(dsm):

    #Decompresss Matrix
    ut = base64.b64decode(dsm)
    ut = zlib.decompress(ut)

    matrix = convertDSMTextTomatrix(ut)



def convertDSMTextTomatrix(dsmText):
    #Process the DSM text into a matrix data structure
    lines = dsmText.split('\n');
    dsm=[]
    for line in lines:
        columns = line.split('|');
        for i, colu in enumerate(columns):
            colu2 = colu.replace(" ", "")
            columns[i]= colu2
        
        dsm.append(columns)
    return dsm


def load_config():
    global BASE_DIR, REPO_DOWNLOAD_DIR, MONGO_PWD, MONGO_USER, MONGO_HOST, MONGO_PORT, MONGO_DB, MONGO_COLL, MONGO_COLL_VERSION, MONGO_COLL_REPO_VERSIONS, MONGO_COLL_BLACKLIST, dirs

    
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

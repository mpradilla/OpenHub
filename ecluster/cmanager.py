import socket
from pymongo import MongoClient
import pymongo
import json
import base64
import zlib
import sys
import linecache
import traceback
import signal
from openhub_exceptions import TimeoutException
from contextlib import contextmanager
import csv
#from threading import Thread
import SocketServer
import time
import thread

DATA_PATH = '../data'
BASE_DIR = ''


MONGO_HOST = ''
MONGO_PORT = 0
MONGO_USER = ''
MONGO_PWD =  ''
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

HOST_IN = ''
PORT_IN = 2003

def main():
    print "hey"
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

    #Thread to receive responses
    thread.start_new_thread(receiveResults,())    
    
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #s.connect((HOST, PORT))
    count=0
    total=0

    dsmClassesSizes=[0]
    dsmPackagesSizes=[0]
    
    versionCompileTimes=[0]
    versionSucceedAnalyzed=[0]
    versionDsmExtracted=[0]
    versionDownTimes=[0]
 
    dates=[0]
    stables=[0]
    idss=[0]
    numNOTCompile=0
    numDSMSize=0
    print "hey" 
    try:


    	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    	s.connect((HOST, PORT))


        for version in collectionVersion.find({"dsm_extracted":1}):
            
#	    print version	
            repo_id = version["_id"]
            #print version
            if "error" in version["dsm"]:
		updateVersionDsmExtracted(collectionVersion, repo_id)
        	numNOTCompile+=1
		#print version
		print "error in version"
	    	continue
            elif "dsm_classes" not in version["dsm"] or version["dsm"]["dsm_classes"]==None:
                print "no dsm classes found"
		updateVersionDsmExtracted(collectionVersion, repo_id)
	        continue
            elif "error" in version["dsm"]["dsm_classes"]:
                print "error un classes dsm"
		if "Matrix size > 20" in version["dsm"]["dsm_classes"]["error"]:
		    print "SIZE ERROR"
		    numDSMSize+=1
		    print version["dsm"]["dsm_classes_size"]		
		#print version
		updateVersionDsmExtracted(collectionVersion, repo_id)
	        continue
      	    
            elif "dsm" in version:
	        ''' 
		dsmClassesSizes.append(version["dsm"]["dsm_classes_size"])
		dsmPackagesSizes.append(version["dsm"]["dsm_packages_size"])
 		
		#versionCompileTimes.append(version["compile_time"])
 		#versionSucceedAnalyzed.append(version["succeed_versions_analyzed"])
   		if "dsm_extracted" in version:
		    versionDsmExtracted.append(version["dsm_extracted"])
		else:
		    versionDsmExtracted.append(1)
		idss.append(version["repo_id"])
	        stables.append(version["stable"])
		dates.append(version["date"])

	#	print dsmClassesSizes
	#	print dsmPackagesSizes
	#	print versionDsmExtracted
	    	
            elif "dsm" in version:
	        '''
		print "VERSION:"
	        #print version["dsm"]["dsm_classes"]
	        fix = getBinaryMatrix(version["dsm"]["dsm_classes"])
	        #print fix
	        if fix and len(fix)<1000:
		
		    binM, sizeM = getBinaryStringMatrix(fix)
		    total+=1		
		    if sizeM>25:
			print "DSM OK: %i" % sizeM
			count+=1
			#print binM
			#print testDSMStructure(binM)
			#print binM
			#print zlib.decompress(base64.b64decode(version["dsm"]["dsm_classes"]))	   
		    	text = '$:'+ str(version["sha"])+':'+str(sizeM)+':'+str(binM)+':$'
                    	try:
			     print "SEND version with sha: %s" % version["sha"]
		             sendData(s,text)
			   #  break
		    	except:
			    print "COULD NOT SEND THE DSM"
			    continue;
		    
       	
		    #text = '$:1234:12:1,0,0,1,1;1,0,0,0:$'
		    #s.sendall(text)
		    #text = '$:1234:12:1,0,0,1,1;1,0,0,0:$'
		    #s.sendall(text)
                    #print 'Send', repr(text)
                    #data = s.recv(1024)
                    #priint 'Received', repr(data)
   	    		
	print "TOTAL DSMs: %i" % total 
	print "TOTAL Correct DSMs: %i" % count
	
	'''
	out = csv.writer(open("stats.csv","w"), delimiter=',',quoting=csv.QUOTE_ALL)
	out.writerow(idss)
	out.writerow(dsmClassesSizes)
	out.writerow(dsmPackagesSizes)
	out.writerow(versionDsmExtracted)
	out.writerow(stables)
	out.writerow(dates)
	'''
	s.close()
        print "length: %i" % len(dsmClassesSizes) 
        print "NUM DSM SIZE ERRORs: %i" % numDSMSize 
        print "NUM not compile: %i" % numNOTCompile
    except:
	s.close()
	print "Error:"
        PrintException()

    #s.close()

def testDSMStructure(dsm):

    rows = dsm.split(';')
    cols = rows[0].split(',')
	
    if len(rows)!=len(cols):
	print "DSM not square! %i x %i" % (len(rows), len(cols))
	return False

    cols = rows[len(rows)-1].split(',')
    if len(rows)!=len(cols):
	print "DSM not square in last row!"
	return False

    return True

def sendData(s,data):
    
    #s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    with time_limit(180):
    	#s.connect((HOST, PORT))
    	print "Socket prepared to send...\n"
    	print "lenght of packet: %i" % len(data)
	s.sendall(data);
    	#s.sendall('\x00');
	print "SEND, waiting for ans...\n"
    	resp = s.recv(1024)   
    	#print 'Received', repr(data)
    #s.close()

def getBinaryMatrix(dsm):

    #Decompresss Matrix
    ut = base64.b64decode(dsm)
    ut = zlib.decompress(ut)

    matrix = convertDSMTextToMatrix(ut)
    return matrix

def getBinaryStringMatrix(matrix):

    matrix2 = [row[1:] for row in matrix[1:]]
    string=""	
    for line in matrix2:
	for i,col in enumerate(line):
	    if line[i] == "":
	        string+=str(0)
 	    else:
		string+=str(1)

	    if i!=len(line)-1:
		string+=","
	string+=";"
	
    #Return ans without last ';'
    return string[:-1], len(line)
		    

def convertDSMTextToMatrix(dsmText):
    #Process the DSM text into a matrix data structure
    lines = dsmText.split('\n');
    dsm=[]
    for line in lines:
        columns = line.split('|');
        for i, colu in enumerate(columns):
            colu2 = colu.replace(" ", "")
            columns[i]= colu2
	#Last column need to be supressed, DSM need to be square        
	del columns[-1]
        dsm.append(columns)
    return dsm

def updateVersionDsmExtracted(collection, doc_id):

    collection.update({'_id':doc_id},{'$set':{'dsm_extracted':0}}) 

@contextmanager
def time_limit(seconds):
    def signal_handler(signum, frame):
	raise TimeoutException, "Time out!"
    signal.signal(signal.SIGALRM, signal_handler)
    signal.alarm(seconds)
    try:
	yield
    finally:
	signal.alarm(0)

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
    print 'EXCEPTION IN ({}, LINE {} "{}"): {}'.format(filename,lineno, line.strip(),exc_obj)


def receiveResults():

    print "thread for result receive created"
    ADDR = (HOST_IN,PORT_IN)
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print "socket created"
    s.bind(ADDR)
    if(s<0 or s<0):
	print "WARNING, SOCKET NOT CRREATED CORRECTLY"
    print "waiting for connections..."
    s.listen(1)
    conn, addr = s.accept()

    print 'connected by', addr

    while 1:
    	data = conn.recv(1024)
        if not data:
	    break
	print "result received %s" % data
	conn.send("ok");		
        if "close" ==data.rstrip():
	    break    

    conn.close()
    print "disconnected"

if __name__=='__main__':
    main()




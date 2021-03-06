'''
Created on Feb 12, 2013

@author: Mauricio Pradilla, mpradilla
'''
#from openhub_exceptions import TimeoutException
import os
import subprocess
import time
import commands
import zlib
from propagation_cost import calculatePropagationCost
from clustering_cost import calculateClusteringCost
from os.path import relpath
import linecache
import sys
import bson
import base64
import logging

#===============================================================================
# Extract Project DSM with Dtangler https://support.requeste.com/dtangler/index.php
#===============================================================================
def run_test(id, path, repo_db):
    
    try:
        print "DSM start"
        start_time = time.time()
        response = {}
        os.chdir(path)
    
        #COMPILE THE PROJECT!!
        print "DSM ANALYZER PATH RECEIVED: " +str(path)
    
        print "Compiling project with mvn..."
        logging.info('version received for analysis in dsm.main.py')
        try:
            p = subprocess.Popen(["mvn", "package",  "-DskipTests=true"], stdout=subprocess.PIPE)
            #p = subprocess.Popen(["mvn", "clean", "install" ,"-DskipTests=true"], stdout=subprocess.PIPE)
            out, err = p.communicate()
        except:
            PrintException()
            return {"error": "Could not compile the project with mvn package"}
            logging.error('*** Compiling the project %s with maven - %s',str(id), str(out))
        
        if "BUILD FAILURE" in out:
            print "****[BUILD FAILURE]*****"
            return {"error": "Could not compile the project with mvn package, Build Failure"+ str(out)}
            logging.error('*** MAVEN BUILD FAILURE for project %s  -  %s',str(id), str(out))
        

        print "Project Compiled with mvn"
       
   
        response["project_build_time"] = time.time() - start_time
        logging.info('version compiled with maven in %s seconds', str(time.time() - start_time))
        start_time = time.time()
       
        os.chdir('..')
        os.chdir('..')
        
        
        #os.chdir('/Users/dasein/OpenHub/worker/modularity/dsm')
    
        p = subprocess.Popen(["java", "-Xmx6G", "-jar", "dtangler-core.jar","-input=/"+path], stdout=subprocess.PIPE)
        out, err = p.communicate()
        print "Calculating DSM for packages..."
        matrix = out.split('|', 1 )[1]
        #print matrix.split('\n',1)[0]  get all columns
        matrix = matrix.rsplit('|',1)[0]
        matrix = "|"+matrix+"|"
    
        s_time = time.time()
        dsmStructure = convertDSMTextTomatrix(matrix)
        response["dsm_packages_convert_time"] = time.time() - s_time
        
        
        
        response["dsm_packages_size"] = len(dsmStructure)
        print "DSM packages size: " + str(len(dsmStructure))
        logging.info('DSMText packages with size: %s - converted in Matrix in %s seconds', str(len(dsmStructure)), str(time.time() - s_time))
    
        #ANALYSIS WILL BE DONE IN ENG. CLUSTER
        '''
        try:
        
            prop_cost = calculatePropagationCost(dsmStructure)
            response["dsm_packages_propagation_cost"] = prop_cost

            clus_cost = calculateClusteringCost(dsmStructure)
            response["dsm_packages_clustering_cost"] = clus_cost

        except:
            print "ERROR getting packages prop and clustering cost"
        '''
    
        #Compress DSM before saving
        
        if len(matrix)>3:
            matrix = compressDSMMatrix(matrix)
        else:
            matrix = {"error":"Matrix size < 3"}

        response["dsm_packages"] = matrix
    
        # CLASS DSM
        p = subprocess.Popen(["java","-Xmx6G", "-jar", "dtangler-core.jar","-input=/"+path,"-scope=classes"], stdout=subprocess.PIPE)
        out, err = p.communicate()
    
        #print matrix
        print "Calculating DSM for classes..."
        matrix = out.split('|', 1 )[1]
        matrix = matrix.rsplit('|',1)[0]
        matrix = "|"+matrix+"|"
    

        s_time = time.time()
        dsmStructure = convertDSMTextTomatrix(matrix)
        response["dsm_classes_convert_time"] = time.time() - s_time


        response["dsm_classes_size"] = len(dsmStructure)
        print "DSM classes size: " + str(len(dsmStructure))
        logging.info('DSMText classes with size: %s - converted in Matrix in %s seconds', str(len(dsmStructure)), str(time.time() - s_time))

        #ANALYSIS WILL BE DONE IN ENG. CLUSTER
        '''
        try:

            prop_cost = calculatePropagationCost(dsmStructure)
            response["dsm_classes_propagation_cost"] = prop_cost
   
            clus_cost = calculateClusteringCost(dsmStructure)
            response["dsm_classes_clustering_cost"] = clus_cost

        except:
            print "ERROR getting packages prop and clustering cost"
        '''


        #Compress DSM before saving
        if len(matrix)>20:
            matrix = compressDSMMatrix(matrix)
        else:
            matrix = {"error":"Matrix size > 20"}

        response["dsm_classes"] = matrix
        response["dsm_process_time"] = time.time() - start_time
        print time.time() - start_time
        logging.info('----- Total time needed to extract DSMs: %s', str(time.time() - start_time))
    
        #GET Size of the project in MB
        size = commands.getoutput('du -sh '+path).split()[0]
        met = size[-1:]
        size = size[:-1]
        size = float(size)
        if met == "G":
            size = size*1024
        elif met == "K":
            size = size/1024
    
        response["project_size"] = size
        print str(size) + " MBytes"
        logging.info('Project size in MBytes: %s',str(size))
        print "Done"
        return response
    except:
        PrintException()
        logging.error('IN DSM ANALYSIS, project with id: %s',str(id))

        return {"error": "DSM analysis error"}


#AUX METHODS
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


def compressDSMMatrix(matrix):
    #Compress Matrix string
    try:
        
        test = matrix
        before = len(matrix)
        print "==============================================="
        print "Matrix size before compression: %i" % before
        print "==============================================="
        print matrix
        print "==============================================="
        
            #print "before: " +str(len(matrix))
        matrix = zlib.compress(matrix.encode('utf8'))
        matrix = base64.b64encode(matrix)
      
        print "==============================================="
        print "Matrix size after compression: %i" % len(matrix)
        print "==============================================="
        print matrix
        print "==============================================="
        
       
        
            #print matrix
            #print "after: " + str(len(matrix))
        print "Compressed: " + str((((len(matrix))-before)/before)*100)

        print "==============================================="
        ut = base64.b64decode(matrix)
        ut = zlib.decompress(ut)
        #print ut

        '''
        if test == ut:
            print  "Can get back"
        else:
            print "CANNOT GET BACK"
        '''




        tt = matrix
        
        ##matrix = bson.binary.Binary(matrix)
        
        '''
        try:
            #This code could deal with other encodings, like latin_1
            #but that's not the point here
            matrix.decode('utf-8')
        except UnicodeDecodeError:
        '''
        
        
        # test = matrix.decode('utf-8')
        #print test
        
        #if test==tt:
        #   print "ENCODING OK!!"
        
        return matrix
    except:
        print "[****ERROR****]:Compressing DSM Matrix"
        PrintException()
        logging.error('**** COULD NOT COMPRESS DSM!')
    
def decompressDSMMatrix(matrix):
    
    try:
        return zlib.decompress(matrix)
    except:
        print "[****ERROR****]:Decompressing DSM Matrix"


def PrintException():
    exc_type, exc_obj, tb = sys.exc_info()
    f = tb.tb_frame
    lineno = tb.tb_lineno
    filename = f.f_code.co_filename
    linecache.checkcache(filename)
    line = linecache.getline(filename, lineno, f.f_globals)
    logging.error('EXCEPTION %s', str('EXCEPTION IN ({}, LINE {} "{}"): {}'.format(filename, lineno, line.strip(), exc_obj)))
    print 'EXCEPTION IN ({}, LINE {} "{}"): {}'.format(filename, lineno, line.strip(), exc_obj)

if __name__ == '__main__':
    #run_test(None, '/Users/dasein/Documents/TESIS/test/hbase-0d4fb57590d54c7f4b1b85fe8ec6138d55851f11', None)
    run_test(None, '/Users/dasein/Documents/TESIS/scribe-java-master', None)
    #run_test(None, '/Users/dasein/Documents/TESIS/dtangler-master', None)
    #run_test(None, os.path.dirname(__file__), None)
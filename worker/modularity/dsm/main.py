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
    
        try:
            p = subprocess.Popen(["mvn", "package",  "-DskipTests=true"], stdout=subprocess.PIPE)
            out, err = p.communicate()
        
        except:
            return {"error": "Could not compile the project with mvn package"}
        
        print "Project Compiled with mvn"
    
        response["project_build_time"] = time.time() - start_time
        start_time = time.time()
    
    
        os.chdir('..')
        os.chdir('..')
        #os.chdir('/Users/dasein/OpenHub/worker/modularity/dsm')
    
        p = subprocess.Popen(["java", "-Xmx4G", "-jar", "dtangler-core.jar","-input=/"+path], stdout=subprocess.PIPE)
        out, err = p.communicate()
        print "Calculating DSM for packages..."
        matrix = out.split('|', 1 )[1]
        #print matrix.split('\n',1)[0]  get all columns
        matrix = matrix.rsplit('|',1)[0]
        matrix = "|"+matrix+"|"
    
        s_time = time.time()
        dsmStructure = convertDSMTextTomatrix(matrix)
        response["dsm_packages_convert_time"] = time.time() - s_time
    
        prop_cost = calculatePropagationCost(dsmStructure)
        response["dsm_packages_propagation_cost"] = prop_cost

        clus_cost = calculateClusteringCost(dsmStructure)
        response["dsm_packages_clustering_cost"] = clus_cost
    
        response["dsm_packages_size"] = len(dsmStructure)
    
        #Compress DSM before saving
        matrix = compressDSMMatrix(matrix)
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

        prop_cost = calculatePropagationCost(dsmStructure)
        response["dsm_classes_propagation_cost"] = prop_cost
   
        clus_cost = calculateClusteringCost(dsmStructure)
        response["dsm_classes_clustering_cost"] = clus_cost
    
        response["dsm_classes_size"] = len(dsmStructure)

        #Compress DSM before saving
        matrix = compressDSMMatrix(matrix)
    
        response["dsm_classes"] = matrix
        response["dsm_process_time"] = time.time() - start_time
        print time.time() - start_time
    
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
        print "Done"
        return response
    except:
        PrintException()

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
        before = len(matrix)
            #print "before: " +str(len(matrix))
        matrix = zlib.compress(matrix,1)
            #print matrix
            #print "after: " + str(len(matrix))
        print "Compressed: " + str((((len(matrix))-before)/before)*100)
        return matrix
    except:
        print "[****ERROR****]:Compressing DSM Matrix"
    
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
    print 'EXCEPTION IN ({}, LINE {} "{}"): {}'.format(filename, lineno, line.strip(), exc_obj)

if __name__ == '__main__':
    #run_test(None, '/Users/dasein/Documents/TESIS/test/hbase-0d4fb57590d54c7f4b1b85fe8ec6138d55851f11', None)
    run_test(None, '/Users/dasein/Documents/TESIS/scribe-java-master', None)
    #run_test(None, '/Users/dasein/Documents/TESIS/dtangler-master', None)
    #run_test(None, os.path.dirname(__file__), None)
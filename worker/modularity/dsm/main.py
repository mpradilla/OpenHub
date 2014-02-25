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

#===============================================================================
# Extract Project DSM with Dtangler https://support.requeste.com/dtangler/index.php
#===============================================================================
def run_test(id, path, repo_db):
    
    start_time = time.time()
    print path
    p = subprocess.Popen(["java", "-Xmx4G", "-jar", "dtangler-core.jar","-input=/"+path], stdout=subprocess.PIPE)
    out, err = p.communicate()
    response = {}
    print "Calculating DSM for packages..."
    matrix = out.split('|', 1 )[1]
            #print matrix.split('\n',1)[0]  get all columns
    matrix = matrix.rsplit('|',1)[0]
    matrix = "|"+matrix+"|"
    print matrix
    
    cost = calculatePropagationCost(matrix)
    
    #Compress DSM before saving
    matrix = compressDSMMatrix(matrix)
    response["dsm_packages"] = matrix
    
    # CLASS DSM
    p = subprocess.Popen(["java","-Xmx6G", "-jar", "dtangler-core.jar","-input=/"+path,"-scope=classes"], stdout=subprocess.PIPE)
    out, err = p.communicate()
    response = {}
    print matrix
    print "Calculating DSM for classes..."
    matrix = out.split('|', 1 )[1]
    matrix = matrix.rsplit('|',1)[0]
    matrix = "|"+matrix+"|"
    
   
    
    
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



#AUX METHOD
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



if __name__ == '__main__':
    run_test(None, '/Users/dasein/Documents/TESIS/scribe-java-master', None)
    #run_test(None, os.path.dirname(__file__), None)
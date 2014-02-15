'''
Created on Feb 12, 2013

@author: Mauricio Pradilla, mpradilla
'''
#from openhub_exceptions import TimeoutException
import os
import subprocess

#===============================================================================
# Extract Project DSM with Dtangler https://support.requeste.com/dtangler/index.php
#===============================================================================
def run_test(id, path, repo_db):
    
    p = subprocess.Popen(["java", "-jar", "dtangler-core.jar","-input=/"+path], stdout=subprocess.PIPE)
    out, err = p.communicate()
    response = {}
    print "Calculating DSM for packages..."
    matrix = out.split('|', 1 )[1]
    #print matrix.split('\n',1)[0]  get all columns
    matrix = matrix.rsplit('|',1)[0]
    matrix = "|"+matrix+"|"
    print matrix
    
    response["dsm_packages"] = matrix
    
    
    # CLASS DSM
    p = subprocess.Popen(["java", "-jar", "dtangler-core.jar","-input=/"+path,"-scope=classes"], stdout=subprocess.PIPE)
    out, err = p.communicate()
    response = {}
    print "Calculating DSM for classes..."
    matrix = out.split('|', 1 )[1]
    matrix = matrix.rsplit('|',1)[0]
    matrix = "|"+matrix+"|"
    
    response["dsm_classes"] = matrix
    print "Done"
    return response

if __name__ == '__main__':
    run_test(None, '/Users/dasein/Documents/TESIS/scribe-java-master', None)

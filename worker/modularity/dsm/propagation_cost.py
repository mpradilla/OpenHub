'''
Created on Feb 20, 2013
    
@author: Mauricio Pradilla, mpradilla
'''

import zlib
import time
import numpy as np

#===============================================================================
# Analyze DSMs and calculate the propagation cost. Based on MacCormack et.al 2005
#===============================================================================

#ANALISIS METHOD HEADER

def calculatePropagationCost(dsmText):
    print "::::::::::::::::::::::::::::::"

    start_time = time.time()
    cost = 0

    lines = dsmText.split('\n');
    # print lines
    dsm=[]
    for line in lines:
        columns = line.split('|');
        dsm.append(columns)
    #print columns

    
    

    dsm= [['','A','B','C','D','E','F'],
         ['A','0','1','1','0','0','0'],
         ['B','0','0','0','1','0','0'],
         ['C','0','0','0','0','1','0'],
         ['D','0','0','0','0','0','0'],
         ['E','0','0','0','0','0','1'],
         ['F','0','0','0','0','0','0']
    ]
    print dsm

    for i in range(len(dsm)-1, 0, -1):
        for j in range(len(dsm)-1, 0, -1):
            print dsm[i][j]
            if i==j:
                dsm[i][j]="1"
            elif dsm[i][j]=="1":
                print "oo"
                for k in range(len(dsm)-1,0,-1):
                    print "--" + str(dsm[j][k])
                    if dsm[j][k]=="1":
                        #transitive dep for element in row i
                        print "TRUE"
                        dsm[i][k]="1"
    


    print dsm



    print "::::::::::::::::::::::::::::::"
    return cost


''''
    for lin in dsm:
    for i, colu in enumerate(lin):
    colu2 = colu.replace(" ", "")
    lin[i]= colu2
    
    print dsm'''


    # print dsm
    
    
    #Build response
    #response["pcost_process_time"] = time.time() - start_time



#Decompress the DSM


#Process the DSM text into a matrix data structure


#Calculate Sucesive Powers of the Dpendency Matrix


#Calculate the Fan-Out Visibility for the whole system



#######################################################
if __name__ == '__main__':
    calculatePropagationCost(None)
    #run_test(None, '/Users/dasein/Documents/TESIS/scribe-java-master', None)
#run_test(None, os.path.dirname(__file__), None)
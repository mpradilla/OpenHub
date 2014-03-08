'''
Created on Feb 20, 2013
    
@author: Mauricio Pradilla, mpradilla
'''

import time

#===============================================================================
# Analyze DSMs and calculate the propagation cost. Based on MacCormack et.al 2005
#===============================================================================

#ANALISIS METHOD HEADER

def calculatePropagationCost(dsm):
    print "::::::::::::::::::::::::::::::"

    start_time = time.time()
    cost = 0


#   dsm= [['','A','B','C','D','E','F'],
#      ['A','','1','1','','',''],
#      ['B','','','','1','',''],
#      ['C','','','','','1',''],
#      ['D','','','','','',''],
#      ['E','','','','','','1'],
#      ['F','','','','','','']
#      ]
    
    #Calculate Sucesive Powers of the Dpendency Matrix
    for i in range(len(dsm)-1, 0, -1):
        for j in range(len(dsm)-1, 0, -1):
            if i==j:
                dsm[i][j]=1
            elif dsm[i][j] is not "":
                #Change all weight of deps to 1
                dsm[i][j]=1
                for k in range(len(dsm)-1,0,-1):
                    if dsm[j][k]is not "":
                        #transitive dep for element in row i
                        dsm[i][k]=1

    #Calculate the Fan-Out Visibility for the whole system

    totalSum =0
    for row in dsm:
        Outcount = 0
        for col in row:
            if col == 1 or col =="1":
                Outcount+=1
        totalSum+=Outcount

    totalSum=float(totalSum*100)
    down = float(len(dsm) -1)
    down = down*down
    cost = float(totalSum/down)
    print "FAN-OUT-VISIBILITY: " + str(cost)
    print "TIME NEEDED: " + str(time.time()-start_time)
    print "::::::::::::::::::::::::::::::"

    response ={}
    response["value"]= cost
    response["time"]= time.time() - start_time
    response["version"] = 1.0
    return response


#######################################################
#TEST METHOD
#######################################################

def test_propagation_cost(dsm):

    dsm= [['','A','B','C','D','E','F'],
          ['A','0','1','1','0','0','0'],
          ['B','0','0','0','1','0','0'],
          ['C','0','0','0','0','1','0'],
          ['D','0','0','0','0','0','0'],
          ['E','0','0','0','0','0','1'],
          ['F','0','0','0','0','0','0']
    ]

    ans=calculatePropagationCost(dsm)
    print "TEST" + str(ans)


#######################################################
if __name__ == '__main__':
    calculatePropagationCost(None)
    #run_test(None, '/Users/dasein/Documents/TESIS/scribe-java-master', None)
#run_test(None, os.path.dirname(__file__), None)
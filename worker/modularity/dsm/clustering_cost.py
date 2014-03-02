'''
Created on Feb 25, 2013
    
@author: Mauricio Pradilla, mpradilla
'''
import time
import numpy
print numpy.__version__
from hcluster import pdist, linkage, dendrogram
from numpy.random import rand
import matplotlib.pylab as plt


from pylab import plot,show
from numpy import vstack,array
from scipy.cluster.vq import kmeans,vq
#===============================================================================
# Analyze DSMs and calculate the clustering cost. Based on MacCormack et.al 2005
#===============================================================================

    #% of depdendency to consider it a bus - default 10%

def cleanDSM(dsm):
    
    sX = []
    sY = []
    coords = []
    for i in range(0, len(dsm), 1):
        for j in range(0, len(dsm), 1):
            if dsm[i][j] is not "" or i==j or dsm[i][j] is not '':
                dsm[i][j]=1
                sX.append(i)
                sY.append(j)
            else:
                dsm[i][j]=0

    dsm[0][0]=1
    return sX,sY

#ANALISIS METHOD HEADER
def calculateClusteringCost(dsm):
    print "::::::::::::::::::::::::::::::"

    start_time = time.time()
    cost = 0
    alpha = 5
    lamd = 2
    
    print "size"
    print len(dsm)

    dsmo = [['','A','B','C','D','E','F'],
           ['A','','1','1','','',''],
           ['B','','','','1','',''],
           ['C','','','','','1',''],
           ['D','','','','','',''],
           ['E','','1','','','','1'],
           ['F','','1','','','','']
    ]


    #remove first row and first column
    vals = [line[1:] for line in dsm[1:]]
    #dsmFix = cleanDSM(vals)
    #plt.plot(dsmFix)

    sx,sy = cleanDSM(vals)
    clusters=[]

    #Calculate DSM Vertical Buses
    buses = []
    mayor = -1
    may = 0
    for i in range(1, len(dsm), 1):
        colCount = 0
        for j in range(1, len(dsm), 1):
            #print dsm[i][j]
            if dsm[j][i] != "":
                colCount+=1
        if colCount!=0:
            y = float(colCount)
            z= float(len(dsm)-1)
            w = float(100/z)
            x = float(w*y)
            if x > alpha:
                buses.append(dsm[0][i])
        
            if colCount > mayor:
                mayor = colCount
                may = i
                    
                    
    plt.plot(sy,sx, 'ro')
    #plt.plot([204, 204], [250, 0], 'k-', lw=2)
    for bus in buses:
        plt.plot([bus,bus],[len(dsm), 0], 'k-', lw=2)

    plt.gca().invert_yaxis()
    plt.show()


#  cluster = [[1,2],[3,4]]
#   for depp in cluster:
#       print depp[0]

           #            [[i,j],[i2,j2],...cluster N] ]

        #   for row in dsm:
        #    for j in row:
        #        print j
        #        data.append(j)

# data generation
    #data = vstack((rand(150,2) + array([.5,.5]),rand(150,2)))
#    data = vstack(data2)
    
    
# now with K = 3 (3 clusters)
#    centroids,_ = kmeans(data,3)
#    idx,_ = vq(data,centroids)

#plot(data[idx==0,0],data[idx==0,1],'ob',
#   data[idx==1,0],data[idx==1,1],'or',
#   data[idx==2,0],data[idx==2,1],'og') # third cluster points
    #plot(centroids[:,0],centroids[:,1],'sm',markersize=8)
#    plt.show()

#
#    X = rand(10,100)
#    X[0:5,:] *= 2
#    Y = pdist(X)
#    Z = linkage(Y)
#    dendrogram(Z)
#    plt.show()

#print "USED in " + str(colCount) + " from " + str(len(dsm)-1)
#print "------"


    print len(buses)
    print buses
    print mayor
    print may

    print "CLUSTERING-COST: " + str(cost)
    print "TIME NEEDED: " + str(time.time()-start_time)
    print "::::::::::::::::::::::::::::::"

    response ={}
    response["value"]= cost
    response["time"]= time.time() - start_time
    return response


def calculateDSMTotalCost():
    
    costDSM=[]
    totalCost=0
    for i in range(1, len(dsm), 1):
        for j in range(1, len(dsm), 1):

            if dsm[i][j]!="" or dsm[i][j]!=0:
                cSize = clusterShareSize(dsm[i][j])
                if cSize!=-1:
                    tCost =cSize**lamd
                    totalCost+=tCost
                    costDSM[i][j]=tCost
                else:
                    tCost =(len(dsm))**lamd
                    totalCost+=(len(dsm))**lamd
                    costDSM[i][j]=tCost

#Return -1 if they are not in te same cluster
def clusterShareSize(dependen):

    for cluster in clusters:
        for dep in cluster:
            if dep==dependen:
                return len(cluster)
    return -1

def isNotInList(input,list):

    for element in list:
        if element == input:
            return false
    return true

#def calculateClusterCost(cluster):
    # cluster = [[[i,j],[i2,j2],...cluster 1],
    #            [[i,j],[i2,j2],...cluster N] ]

    # SOLO INDICIES

#   for dep in clusters:
        #check if j or i is a vertical bus
        #       if checkIfIsInVerticalBus

# elif

#       else:



def checkIfIsInVerticalBus(dep):

    for number in buses:
        if number[1]==num:
            return True

    return False


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

    dsm = [['','A','B','C','D','E','F'],
       ['A','','1','1','','',''],
       ['B','','','','1','',''],
       ['C','','','','','1',''],
       ['D','','','','','',''],
       ['E','','1','','','','1'],
       ['F','','1','','','','']
       ]
    
    
    dsm = numpy.array(dsm)


#######################################################
if __name__ == '__main__':
    calculatePropagationCost(None)
    #run_test(None, '/Users/dasein/Documents/TESIS/scribe-java-master', None)
#run_test(None, os.path.dirname(__file__), None)
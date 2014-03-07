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
from math import sqrt
#===============================================================================
# Analyze DSMs and calculate the clustering cost. Based on MacCormack et.al 2005
#===============================================================================

    #% of depdendency to consider it a bus - default 10%
clusters=[]
buses=[]
dsm=[]
dsmSize=-1
alpha = 5.0
lamd = 2.0

def getLamda():
    global lamd
    return float(lamd)

def getAlpha():
    global alpha
    return float(alpha)

def setBuses(buss):
    global buses
    buses=buss

def getBuses():
    global buses
    return buses

def getClusters():
    global clusters
    return clusters

def setClusters(cl):
    global clusters
    clusters=cl

def getDSM():
    global dsm
    return dsm

def setDSM(ds):
    global dsm
    dsm=ds

def getDsmSize():
    global dsmSize
    return dsmSize

def setDsmSize(size):
    dsmSize =size

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
    
    print "DSM size: " + str(len(dsm))

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
    sx,sy = cleanDSM(vals)
    
    setDSM(vals)
    setDsmSize(len(vals))

    #Calculate DSM Vertical Buses
    mayor = -1
    may = 0
    buses=[]
    coords = []
    coordsProjected = []
    for i in range(1, len(dsm), 1):
        colCount = 0
        for j in range(1, len(dsm), 1):
            #print dsm[i][j]
            if dsm[j][i] != "":
                colCount+=1
                coords.append([i,j])
                coordsProjected.append([0,j])
        if colCount!=0:
            y = float(colCount)
            z= float(len(dsm)-1)
            w = float(100/z)
            x = float(w*y)
            if x > getAlpha():
                buses.append(dsm[0][i])
        
            if colCount > mayor:
                mayor = colCount
                may = i


    plt.plot(sy,sx, 'ro')
    #plt.plot([204, 204], [250, 0], 'k-', lw=2)
    for bus in buses:
        plt.plot([bus,bus],[len(dsm), 0], 'k-', lw=2)
    
    
    
    
    #plt.gca().invert_yaxis()
    #plt.show()

    deletes = 0
    for i in range(0,len(coords),1):
        print coords[i][0]
        if isInList(coords[i][0],buses):
            del coordsProjected[i-deletes]
            print "deleted" + str(i)
            deletes+=1



    ar = [1, 2, 3, 60, 70, 80, 100, 220, 230, 250]
    for cluster in parse(sy, 5):
        print(cluster)

    

# data generation
    #data = vstack((rand(150,2) + array([.5,.5]),rand(150,2)))
    data = vstack(coordsProjected)
    #data = vstack(sy)
    
# now with K = 3 (3 clusters)
    num = round(int(getDsmSize())*float(getAlpha()/100))
    centroids,_ = kmeans(data,3)
    idx,_ = vq(data,centroids)

    ''''
    plot(data[idx==0],'ob',
    data[idx==1],'or',
    data[idx==2],'og') # third cluster points
    plot(centroids[:,0],'sm',markersize=8)
    plt.gca().invert_yaxis()
    plt.show()
     '''
    for i in range(0,len(centroids),1):
        plt.plot([0,len(dsm)],[centroids[i,1],centroids[i,1]], 'k-', lw=2)

    co=0
    for i in range(0,len(centroids),1):
        print "centroid: "+str(centroids[i,1]) + "  j: "+str(centroids[i,0])

    #Sort ascending the centroids
    sortedCent = []
    ult = -1
    for i in range(0,len(centroids),1):
        if ult!=-1:
            for j in range(0,len(sortedCent),1):
                if centroids[i,1] < sortedCent[j]:
                    sortedCent.insert(j, centroids[i,1])
                    ult = i
                    break
            if ult!=i:
                sortedCent.append(centroids[i,1])
        else:
            sortedCent.append(centroids[i,1])
            ult = i

    #Add coordinates to corresponding cluster
    intersections= []
    last = 0
    for i in range(1,len(sortedCent),1):
        middle =((sortedCent[i]-sortedCent[i-1])/2.0 )+sortedCent[i-1]
        intersections.append(middle)

    print "INTEREECTIONs" + str(intersections)

    clusters = []
    for coord in coords:
        print "COORD TO INSERT: "+ str(coord)
       
        for i in range(0,len(intersections),1):
                
            if i==0:
                anterior =0
            else:
                anterior =intersections[i-1]
            
            if coord[0]<intersections[i] and coord[1]<intersections[i] and coord[0]>anterior and coord[1]>anterior:
                #clusters['i'].append(coord)
                if clusters is None or len(clusters)==0:
                    cl = {'deps':[coord]}
                    clusters.append(cl)
                else:
                    clusters[i]['deps'].append(coord)
                break
                        
            if i==len(intersections)-1 and coord[0]>intersections[i] and coord[1]>intersections[i]:
                if clusters[i+1] is None:
                    cl = {'deps':[coord]}
                    clusters.append(cl)
                else:
                    clusters[i+1]['deps'].append(coord)
                break

            #ELEMENTS OUTSIDE DIAGONAL CLUSTERS
            elif i==len(intersections)-1:
                print len(clusters)
                if len(clusters)<=i+2:
                    cl = {'deps':[coord]}
                    clusters.append(cl)
                else:
                    clusters[i+2]['deps'].append(coord)
                break
                        
                        
                    
    print "$$$$$"
    print clusters
    print "$$$$$"



    plot(data[idx==0,0],data[idx==0,1],'ob',
    data[idx==1,0],data[idx==1,1],'or',
    data[idx==2,0],data[idx==2,1],'og') # third cluster points
    plot(centroids[:,0],centroids[:,1],'sm',markersize=8)
    plt.gca().invert_yaxis()
    plt.show()
   





    '''
    for i in range(0, len(data[idx==0,0]), 1):
        #clusters += [[data[0],data[1]]]
        print data[0]
        print data[1]
        #clusters[i].append([data[i][idx==i,0],data[i][idx==i,0]])

    print clusters
    '''

    #UPDATE GLOBAL VARIABLES
    setBuses(buses)
    setClusters(clusters)

    #print clusters
    print "CLUSTER COST v1: " +str(calculateTotalClustersCost())

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

#CLUSTERS Structure

# clusters["id"] = id
# clusters["cost"] = current_cost
# clusters["new_cost"] = new_bid_cost
# clusters["deps"] = [[dep1],[dep2],...,[depN]

def getFromDict(dataDict, mapList):
    return reduce(lambda d, k: d[k], mapList, dataDict)

def setInDict(dataDict, mapList, value):
    getFromDict(dataDict, mapList[:-1])[mapList[-1]] = value

def stat(lst):
    """Calculate mean and std deviation from the input list."""
    n = float(len(lst))
    mean = sum(lst) / n
    stdev = sqrt((sum(x*x for x in lst) / n) - (mean * mean))
    return mean, stdev

def parse(lst, n):
    cluster = []
    for i in lst:
        if len(cluster) <= 1:    # the first two values are going directly in
            cluster.append(i)
            continue
        
        mean,stdev = stat(cluster)
        if abs(mean - i) > n * stdev:    # check the "distance"
            yield cluster
            cluster[:] = []    # reset cluster to the empty list
        
        cluster.append(i)
    yield cluster           # yield the last cluster




#Get total clustering cost - with clusters array
def calculateTotalClustersCost():

    dsm = getDSM()
    
    cost = 0
    for cluster in getClusters():
        clusterSize = getClusterSize(cluster)
        clusterSize = int(clusterSize)
        clusterCost = 0
        for dep in cluster['deps']:
            i = dep[0]
            j = dep[1]
            depCost = 0
            #Vertical bus, dep cost = 1
            # print "depee: "+str(j)
            if isInList(j,getBuses()):
                depCost = 1
                #print "BUS "+ str(j)+ " USED"
            elif i!=j:
                #check if they are in the same cluster
                #print "same cl: "+str(dep) + " cluster: "+str(cluster)
                if isInList(dep,cluster['deps']):
                    #print "SAME CLUSTER"
                    lamd = float(getLamda())
                    depCost =clusterSize**lamd
                else:
                    #print "ANOTHER CLUSTER"
                    depCost =(len(dsm))**float(getLamda())

            clusterCost+=depCost
            if depCost!=1 and j!=i:
                print "depCosts: " + str(depCost) + "i:" + str(i) + "j: " + str(j)
        cost+=clusterCost
    return cost


#GET size of cluster
def getClusterSize(cluster):
    
    if len(cluster['deps'])==1:
        return 1

    maxI = 0.0
    maxJ = 0.0
    for dep in cluster['deps']:
        i = dep[0]
        j = dep[1]
        if i>maxI:
            maxI=i
        elif j>maxJ:
            maxJ=j

    if maxI>maxJ:
        return maxI
    else:
        return maxJ


#Get random dependency and calculate the bid change




def calculateDSMTotalCost():
    
    costDSM=[]
    totalCost=0
    for i in range(1, len(dsm), 1):
        for j in range(1, len(dsm), 1):

            if dsm[i][j]!="" or dsm[i][j]!=0:
                cSize = clusterShareSize(dsm[i][j])
                if cSize!=-1:
                    tCost =cSize**getLamda()
                    totalCost+=tCost
                    costDSM[i][j]=tCost
                else:
                    tCost =(len(dsm))**getLamda()
                    totalCost+=(len(dsm))**getLamda()
                    costDSM[i][j]=tCost

#Return -1 if they are not in te same cluster
def clusterShareSize(dependen):

    for cluster in clusters:
        for dep in cluster:
            if dep==dependen:
                return len(cluster)
    return -1

def isInList(input,list):

    for element in list:
        if str(element) == str(input):
            return True
    return False

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
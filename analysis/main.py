import json
from pprint import pprint
#import numpy as np
#import matplotlib.pyplot as plt
from pymongo import MongoClient

DATA_PATH = '../data'

MONGO_HOST = ''
MONGO_PORT = 0
MONGO_USER = ''
MONGO_PWD = ''
MONGO_DB = ''
MONGO_COLL = ''
MONGO_COLL_VERSION = 'cversion'
MONGO_COLL_BLACKLIST = 'blacklist'
MONGO_COLL_REPO_VERSIONS = 'repoVersions'

db_repos = ''
collectionVersion =''
collectionRepoVersions =''
collectionBlacklist = ''


usesDict={}
color_dict = {0: 'red', 1: 'black', 2: 'yellow', 3: 'blue', 4: 'green',
5: 'purple', 6: '#ff69b4', 7: 'black', 8: 'cyan', 'JUAN': 'magenta', 'FLORENCE': '#faebd7',
'ANDREW': '#2e8b57', 'GEORGES': '#eeefff', 'ISIDORE': '#da70d6', 'IVAN': '#ff7f50', 'CINDY': '#cd853f',
'DENNIS': '#bc8f8f', 'RITA': '#5f9ea0', 'IDA': '#daa520'}
color = ['r','g','b','k','m','y', 'c']


def main():
    
    global db_repos
    global collectionVersion
    global collectionRepoVersions
    global collectionBlacklist
    
    
    # Load config information
    load_config()
    
    # Connect to databse
    print "Mongo host %s and port %s" % (MONGO_HOST, MONGO_PORT)
    client = MongoClient(MONGO_HOST, MONGO_PORT)
    db = client[MONGO_DB]
    db.authenticate(MONGO_USER, MONGO_PWD)
    db_repos = db[MONGO_COLL]
    collectionVersion = db[MONGO_COLL_VERSION]
    collectionRepoVersions = db[MONGO_COLL_REPO_VERSIONS]
    collectionBlacklist = db[MONGO_COLL_BLACKLIST]
  
  
    #plt.title('Title')
    
    
    numErrors=0
    total=0
    completed=0
    for ver in collectionVersion.find():
        total+=1
        if "dsm" in ver:
            if "error" in ver["dsm"]:
                print "ERROR IN DSM"
                numErrors+=1
            elif "error" in ver["dsm"]["dsm_packages"]:
                print "ERROR IN DSM PACKAGES"
                numErrors+=1
            else:
                print ver["dsm"]["dsm_packages"]
                if ver["state"]=="completedV5":
                    completed+=1
                print ver["state"]


    print "********************"
    print "Number of erros %i of total versions %i" % (numErrors,total)
    print "Completed V5 %i" % completed
    print "total - errors %i" % (total-numErrors)
    
    
    '''
    for repo in db_repos.find({"language": "Java"}):
    
        vers = getVersionsForRepo(repo["_id"])
    
        print "Versions number: %i" % len(vers)
    
        if len(vers)>1:
            
            print repo["html_url"]
    
            for vv in vers:
                if "error" not in vv["dsm"]:
                    print vv["dsm"]
                    print repo["_id"]

        
   
   
         #analyzeVersions(repo["_id"],plt, "dsm_packages_propagation_cost", True)
         #analyzeVersions(repo["_id"],plt, "dsm_packages_clustering_cost", True)
         #analyzeVersions(repo["_id"],plt, "dsm_packages_size", False)
        arr, ok = analyzeVersions(repo["_id"],plt, "dsm_classes_size", False)
        print "VERSIONS NUM: %i " % len(arr)
    
        if ok and len(arr)>1:
            
            num = []
            for i in range(0,len(arr),1):
                plt.plot(i,arr[i], '.-r')
                if len(num)==0:
                    num=[0]
                else:
                    num.append(i)
            
            plt.plot(num,arr, '-g')
            plt.show()
    '''
    
    #load_data()

    #getMetricEvolution(123, "dsm_packages_propagation_cost")


def getVersionsForRepo(repo_id):

    versions = []
    for ver in collectionVersion.find({"repo_id":repo_id}):
        if len(versions)==0:
            versions=[ver]
        else:
            versions.append(ver)
    return versions


def analyzeVersions(repo_id,plt, metric, value):

    ok=False
    arr = []
    for ver in collectionVersion.find({"repo_id":repo_id}):

        if "dsm" in ver:
            ok=True
            if "error" not in ver["dsm"] and metric in ver["dsm"] :
                
                if len(arr)==0:
                    if value:
                        arr = [ver["dsm"][metric]["value"]]
                    else:
                        arr = [ver["dsm"][metric]]
                
                else:
                    if value:
                        arr.append(ver["dsm"][metric]["value"])
                    else:
                        arr.append(ver["dsm"][metric])

    
    return arr, ok


def getMetricEvolution(repo_id, metric):

    global db_repos

    for repo in db_repos.find({"language": "Java", "state":"completedV3"}):

        if "evolution" in repo:
            if metric in repo["evolution"]:
                #print repo["evolution"][metric]
                arr = getDataInArray(repo["evolution"][metric])
            #if "dates" in repo["evolution"]:
            #    dates = repo["evolution"]["dates"]


            num = []
            for i in range(0,len(arr),1):
                plt.plot(i,arr[i], '.-r')
                if len(num)==0:
                    num=[0]
                else:
                    num.append(i)

            print num
            print arr
            plt.plot(num,arr, '-g')
    plt.show()
                


def getDataInArray(data):
    '''
        imput: repo["evolution"][metric]
        
    '''
    arr = []
    for i in range(0,len(data),1):
        
        if len(arr)==0:
            arr=[data[i]["value"]]
        else:
            arr.append(data[i]["value"])
    
    return arr


def load_data():
    count =0
    data = []
    with open('data/evolutionV3_1.json') as f:
        for line in f:
            data.append(json.loads(line))
            if count > 10000:
                break
            else:
                count+=1

    #getTotalProcessingTimeSize(data)
    
    
    
    getEvolutionStats(data)


def getEvolutionStats(data):
    
    global color_dict
    global color

    metric = "dsm_classes_clustering_cost"

    count = 0
    error = 0
    complete = 0
    postive=0
    nuetral=0
    negative=0
    for repo in data:
        
        if "error" in repo["dsm"]:
        
            print "Error in dsm analysis"
            error+=1
        else:

            print repo["_id"]
            if metric in repo["evolution"]:
                same = -1
                arr = getArrayFromData(repo["evolution"][metric])
                
                print "DESVSTAND: %i" % np.std(arr)
                print "MEAN: %i" % np.mean(arr)
                print arr
                
                for i in range(0,len(repo["evolution"]["dates"]),1):
                    plt.plot(i,repo["evolution"][metric][i]["value"], '.'+str(color[count])+'-')
        
                complete+=1
            
            else:
                error+=1
            
            
            count+=1
            if count==len(color):
                count=0
            
    plt.show()


            
    print "#####################################"
    print ""
    print "Repo with Errors: %i" % (error)
    print "Repo Completed: %i" % (complete)
    print "REpo with negative evolution: %i" % negative
    print "REpo with nuetral evolution: %i" % nuetral
    print "REpo with positive evolution: %i" % postive
    print "Total repos: %i " % len(data)
    print "#####################################"

def getArrayFromData(data):
    '''
    imput: repo["evolution"][metric]
    
    '''
    arr = []
    for i in range(0,len(data),1):

        if data[i]["value"]!=0:
            if len(arr)==0:
                arr=[data[i]["value"]]
            else:
                arr.append(data[i]["value"])

    return arr

def tendence(array):

    sum = 0
    num = len(array)
    for po in array:
        sum += poj


def getTotalProcessingTimeSize(data):

    emptyDSMClasses = 0
    timeSuccessDSMs = 0
    time=0
    size=0
    for line in data:
    
    
        try:
            num = float(line["modularity"]["dsm"]["dsm_process_time"])
            if type(num) is float:
                time+=num
        
            if line["modularity"]["dsm"]["dsm_classes"]=="|\n|":
                emptyDSMClasses+=1
                timeSuccessDSMs+=num
        
        
            num2 = int(line["modularity"]["dsm"]["project_size"])
            if type(num2) is int:
                size+=num2

            deps = line["modularity"]["external_dependencies"]
            if deps is not None:
            
                for dep in deps:
                #print dep + " - usedBy: "+str(line["name"])
                    insertDependencyUse(dep,line["name"])
                    if dep=="slf4j-api":
                        print line
            #print deps[0]
            #jdata = json.load(deps)
            #print jdata[0]
                #print line["modularity"]["external_dependencies"].values()
            
            #print "+++++++++++++++"
            #print usesDict
        except KeyError:
            print "[****ERROR****]: KeyError. "
        except NameError:
            print "[****ERROR****]: NameError. "
        except TypeError:
            print "[****ERROR****]: typeError. "
    
            
    totalSuccessDSMs = len(data)-emptyDSMClasses
    
    print "Total projects analized: " + str(len(data))
    print "Total Succes DSM_classes obtained: " + str(totalSuccessDSMs)
    print "Total Time processing DSMs: " + str(time)
    print "Average Time processing DSMs: " + str(time/len(data))
    print "Total Time processing succeed DSMs: " + str(timeSuccessDSMs)
    print "Average Time processing succeed DSMs: " + str(timeSuccessDSMs/totalSuccessDSMs)
    print "Total Size of project analyzed: " + str(size)
    print "Averga size of project's: "+ str(size/len(data))
    print "The 100 project's more reusable in order are: " + str(getNDictionayItemsByListNumber(usesDict,100))


    print ""

def insertDependencyUse(projectUsed, usedBy):

    #Search if reusable project exits in the dictionary
    
    inserted = 0
    for project in usesDict:
        #print project
        if project==projectUsed:
            #print "mmmmmm  " + usesDict[project][0]
            usesDict[project].append(usedBy)
            #usedBy["'"+project+"'"].append(usedBy)
            #print  str(usesDict) + " size: " +str(len(usesDict[project]))
            inserted=1

    if inserted==0:
        list = []
        list.append(usedBy)
        usesDict[projectUsed]=list
        #print usesDict

def getNDictionayItemsByListNumber(dictionary, nFirst):

    ItemsByNumbher = []

    for k in sorted(dictionary, key=lambda k: len(dictionary[k]), reverse=True):
        if len(ItemsByNumbher)<nFirst:
            ItemsByNumbher.append(k+":"+str(len(dictionary[k]))),

    return ItemsByNumbher


def load_config():
    global MONGO_PWD, MONGO_USER, MONGO_HOST, MONGO_PORT, MONGO_DB, MONGO_COLL, MONGO_COLL_VERSION, MONGO_COLL_REPO_VERSIONS, MONGO_COLL_BLACKLIST
    
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


if  __name__ =='__main__':
    main()
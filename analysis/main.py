import json
from pprint import pprint

usesDict={}

def load_data():
    count =0
    data = []
    with open('data/firstRoundCompleted.json') as f:
        for line in f:
            data.append(json.loads(line))
            if count > 10000:
                break
            else:
                count+=1

    getTotalProcessingTimeSize(data)


    # for line in data:
        #print line["modularity"]["external_dependencies"]

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
    print "Total Succes DSM_classes obtained" + str(totalSuccessDSMs)
    print "Total Time processing DSMs: " + str(time)
    print "Average Time processing DSMs: " + str(time/len(data))
    print "Total Time processing succeed DSMs: " + str(timeSuccessDSMs)
    print "Average Time processing succeed DSMs: " + str(timeSuccessDSMs/totalSuccessDSMs)
    print "Total Size of project analyzed: " + str(size)
    print "Averga size of project's: "+ str(size/len(data))
    print "The 100 project's more reusable in order are: " + str(getNDictionayItemsByListNumber(usesDict,100))

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




def main():
    load_data()

if  __name__ =='__main__':
    load_data()
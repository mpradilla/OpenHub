'''
Created on Feb 12, 2014
    
@author: Mauricio Pradilla, mpradilla
'''

import os
from xml.dom.minidom import parse
import xml.etree.ElementTree as xml

def run_test(id, path, repo_db):
    print "Getting project dependencies ..."
    
    try :
        for root, subFolders, files in os.walk(path):
            for f in files:
                if 'POM.XML' in f.upper():
                    pomFile = xml.parse(os.path.join(root, f))
                    root = pomFile.getroot()
                    mappings = getMappings(root)
                    print mappings
                    return mappings
    except:
        print "POM_NOT_FOUND"
        return ""

#Auxiliar method to analyze the pom.xml file
def getMappingsNode(node, nodeName):
    if node.findall('*'):
        for n in node.findall('*'):
            if nodeName in n.tag:
                return n
        else:
            return getMappingsNode(n, nodeName)
#Method to extract the dependencies and put ir all togther in a python dictionary #structure
def getMappings(rootNode):
    mappingsNode = getMappingsNode(rootNode, 'dependencies')
    mapping = {}
    
    for prop in mappingsNode.findall('*'):
        key = ''
        val = ''
        
        for child in prop.findall('*'):
            if 'artifactId' in child.tag:
                key = child.text
            
            if 'version' in child.tag:
                val = child.text
        
        if val and key:
            mapping[key] = val
    
    return mapping


if __name__ == '__main__':
    #run_test(None, '/Users/dasein/Documents/TESIS/github-readme-maven-plugin-master/', None)
    run_test(None, os.path.dirname(__file__), None)
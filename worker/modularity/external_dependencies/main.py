#AUTOR: Mauricio Pradilla

import os
from xml.dom.minidom import parse
import xml.etree.ElementTree as xml

def run_test(id, path, repo_db):
    print "Getting project dependencies ..."
    
    try :
        pomFile = xml.parse(path+'pom.xml')
        root = pomFile.getroot()
        mappings = getMappings(root)
        print mappings
        return mappings
    except:
        print "Pom.xml not found"
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
    run_test(None, '/Users/dasein/Documents/TESIS/github-readme-maven-plugin-master/', None)

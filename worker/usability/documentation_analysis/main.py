'''
Created on Aug 18, 2013

@author: Gabriel Farah
'''
from markdown import markdown
import sys
from bs4 import BeautifulSoup
from sklearn.externals import joblib
import numpy as np
import os

def features_extraction(clean_html):
    result_list = ""
    soup = BeautifulSoup(clean_html)
    #------------------------------- text = ''.join(soup.findAll(text=True))
    tags = ['b','h1','h2','h3','h4','h5','h6']
    for link in soup.find_all(tags):
        temp_text = link.text
        for ch in ['&','#','_','?','.','-','1','2','3','4','5','6','7','8','9','0']:
            if ch in temp_text:
                temp_text=temp_text.replace(ch," ")
        result_list = result_list+" "+temp_text
    return result_list

def load_and_clear_file(local_path):
    with open(local_path, 'r') as content_file:
        content = content_file.read()
    content_file.close()
    return markdown(content.decode('utf-8'))

def run_test(id, path, repo_db):
    for root, subFolders, files in os.walk(path):
        for f in files:
            if 'README' in f.upper():
                print "Analyzing README docs"
                clean_html = load_and_clear_file(os.path.join(root, f))
                result_list = features_extraction(clean_html)
                #Load the saved classifier
                classifier = joblib.load(os.path.dirname(os.path.realpath(__file__))+'/pickles/model.pkl')
                #convert our list of headers to an array
                X_test = np.array([result_list])
                # target_names = ['Bad', 'Average', 'Good']
                #predict the class
                return int(classifier.predict(X_test))

    print "No README file"
    return 0


if __name__=='__main__':
    run_test(None, os.path.dirname(__file__), None)

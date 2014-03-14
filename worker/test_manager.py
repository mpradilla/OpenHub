import manager as m
import unittest


def funcx(x):
    return x+1

def test_getCleanCommitList():
    
    #test with scribe-java
    versions = m.getCleanCommitList("889932", "https://github.com/fernandezpablo85", path, "fernandezpablo85/scribe-java")
    assert funcx(3)==4

def test_getDateAsNumber():

    assert m.getDateAsNumber("2011-07-15T10:38:04Z")=="20110715"
    assert m.getDateAsNumber("1999-12-31T10:38:04Z")=="19991231"
    assert m.getDateAsNumber("2001-01-01T20:38:04Z")=="20010101"



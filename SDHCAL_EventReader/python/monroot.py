import os
import socket
import httplib, urllib
try:
   import elementtree.ElementTree as ET
   from elementtree.ElementTree import parse,fromstring, tostring
   import sqlite as SQLITE
except:
   print "SLC6"
   import xml.etree.ElementTree as ET
   from xml.etree.ElementTree import parse,fromstring, tostring
   import sqlite3 as SQLITE
from copy import deepcopy
from operator import itemgetter
from ROOT import *
import fnmatch
import os

def listHistos(mypath,pattern='*'): 
	matches = []
	for root, dirnames, filenames in os.walk(mypath):
		for filename in fnmatch.filter(filenames,pattern):
				matches.append(os.path.join(root, filename))
	
	return matches

def getHisto(name):
	f=open(name)
	sxml=f.read()
	s=sxml.replace('\n','')
	tree =ET.XML(s)
	h=TBufferXML.ConvertFromXML(ET.tostring(tree))
	return h
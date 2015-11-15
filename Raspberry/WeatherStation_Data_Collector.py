#! /usr/bin/python
from socket import error as SocketError
import errno
import socket

import serial
import urllib2
import urllib
import time

from urllib2 import HTTPError

import logging
logging.basicConfig(filename='raspduino_meteo.log',level=logging.DEBUG,format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')

def getUri(uri) :
	try :
		ret=urllib2.urlopen(uri).read()
		return ret
	except HTTPError, e:
		if e.getcode() == 500:
			content = e.read()
			print ("erreur 500 \n")
			logging.warning("erreur 500")
		else :
			logging.error("Error with HTTP not handled !")
			logging.error(e, exc_info=True)
			raise
	except SocketError as e:
		if e.errno != errno.ECONNRESET:
			logging.error("Error with Socket not handled !")
			logging.error(e, exc_info=True)
			raise # Not error we are looking for
		pass # Handle error here.


def getDate() :
	uri='http://yourserver.com/meteo/getDate.php'
	myDate=getUri(uri)
	print(myDate)

	return myDate

def syncDate() :
	myDate = getDate()
	btSerial.write('d '+myDate)

btSerial = serial.Serial	("/dev/rfcomm1", baudrate=9600)
syncDate()
while True:
	try :
		btSerial.write('m')
		data = btSerial.readline()
	except e:
		logging.error("Error with btSerial not handled !")
		logging.error(e, exc_info=True)

		btSerial.close()
		btSerial = serial.Serial	("/dev/rfcomm1", baudrate=9600)
		btSerial.write('m')
		data = btSerial.readline()
		pass
	
	if (data != ""):
		processed_data = data.split(";")
		
		uri="http://yourserver.com/meteo/?add&"			
		args = {'date':processed_data[0],'inttempc':processed_data[1],'inthumidity':processed_data[2],'pressure':processed_data[3],'exttempc':'-1','exthumidity':'-1'}
		uri+=urllib.urlencode(args)

		getUri(uri)
		
		# Wait 2 minutes
		time.sleep(2 * 60)

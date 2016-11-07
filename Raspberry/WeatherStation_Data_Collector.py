#! /usr/bin/python

import serial
import urllib2
import urllib
import time

from urllib2 import HTTPError

import logging
from logging.handlers import RotatingFileHandler

logger = logging.getLogger("Rotating Log")
logger.setLevel(logging.DEBUG)

# add a rotating handler
handler = RotatingFileHandler('raspduino_meteo.log', maxBytes=4000000,
								backupCount=1)
# create formatter
formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
# add formatter to handler
handler.setFormatter(formatter)
logger.addHandler(handler)
logger.info('Starting...')

def getUri(uri):
	try:
		ret = urllib2.urlopen(uri).read()
	except urllib2.HTTPError as e:
		if e.code == 500:
			# content = e.read()
			print ("erreur 500 \n")
			logger.warning("erreur 500")
		else:
			logger.error("Error with HTTP not handled !")
			logger.error(e, exc_info=True)
			raise
	except urllib2.URLError as e:
		logger.error("Error with URL not handled !")
		logger.error(e, exc_info=True)
		raise # Not error we are looking for
	else:
		logger.debug("Tracing : return:%s - Address : %s"%("200",uri))
		return ret


def getDate():
	uri='http://yourserver.com/meteo/getDate.php'
	myDate = getUri(uri)
	print(myDate)
	return myDate

def syncDate():
	myDate = getDate()
	btSerial.write('d '+ myDate)

btSerial = serial.Serial("/dev/rfcomm1", baudrate=9600)
syncDate()
while True:
	try:
		btSerial.write('m')
		data = btSerial.readline()
	except serial.SerialException as e:
		logger.error("Error with btSerial not handled !")
		logger.error(e, exc_info=True)

		btSerial.close()
		btSerial = serial.Serial("/dev/rfcomm1", baudrate=9600)
		btSerial.write('m')
		data = btSerial.readline()
		pass

	if (data != ""):
		processed_data = data.split(";")

		uri = "http://yourserver.com/meteo/?add&"
		args = {'date':processed_data[0], 'inttempc':processed_data[1], \
				'inthumidity':processed_data[2], 'pressure':processed_data[3], \
				'exttempc':'-1', 'exthumidity':'-1'}
		uri += urllib.urlencode(args)

		getUri(uri)

		# Wait 2 minutes
		time.sleep(2 * 60)

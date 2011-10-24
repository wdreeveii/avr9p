#!/usr/bin/python

import serial
import time
import sys
import struct

def settime(ser):
	msgtype = '\x02'
	timestamp = int(time.time())
	tstamp = str(timestamp)
	header = chr(len(msgtype) + len(tstamp)) + chr(1)
	print "|"+header+":"+msgtype+":"+tstamp+"|"
	print ser.readline()
	print ser.readline()
	print ser.readline()
	ser.write(header + msgtype + tstamp)
	while True:
		print ser.readline(),


def newevent(ser,on_time, off_time, port):
	
	starttime = int(time.time()) + 3
	msgtype = '\x04'
	msgdata = struct.pack('=LHHB', starttime, on_time, off_time, port)
	header = chr(len(msgtype) + len(msgdata)) + chr(1)
	print "|"+msgdata+"|"
	print ser.readline()
	print ser.readline()
	print ser.readline()
	ser.write(header + msgtype + msgdata)
	while True:
		print ser.readline(),

	
def deleteevent(ser, index):
	msgtype = '\x06'
	msgdata = chr(index)
	header = chr(len(msgtype) + len(msgdata)) + chr(1)
	print "|"+msgdata+"|"
	print ser.readline()
	print ser.readline()
	print ser.readline()
	ser.write(header + msgtype + msgdata)
	while True:
		print ser.readline(),

	
def listevents(ser):
	msgtype = '\x05'
	header = chr(len(msgtype)) + chr(1)
	print ser.readline()
	print ser.readline()
	print ser.readline()
	ser.write(header + msgtype)
	while True:
		print ser.readline(),
	
def setoutput(ser, state, port):
	msgtype = '\x01'
	msgdata = struct.pack('=BB', port,state)
	header = chr(len(msgtype) + len(msgdata)) + chr(1)
	print "|"+msgdata+"|"
	print ser.readline()
	print ser.readline()
	print ser.readline()
	ser.write(header + msgtype + msgdata)
	while True:
		print ser.readline(),
		
def ping(ser):
	msgtype = '\x07'
	header = chr(len(msgtype)) + chr(1)
	print ser.readline()
	print ser.readline()
	print ser.readline()
	ser.write(header+msgtype)
	while True:
		print ser.readline(),
	

	
if (len(sys.argv) > 1):
	ser = serial.Serial('/dev/ttyS0', 57600, timeout=1)
	
	if (sys.argv[1] == 'ping'):
		ping(ser)
	elif( sys.argv[1] == 'settime' ):
		settime(ser)
	elif (sys.argv[1] == 'listevents'):
		listevents(ser)
	elif (sys.argv[1] == 'setoutput'):
		setoutput(ser,int(sys.argv[2]), int(sys.argv[3]))
	elif( sys.argv[1] == 'newevent' ):
		newevent(ser, int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4]))
	elif( sys.argv[1] == 'deleteevent' ):
		deleteevent(ser, int(sys.argv[2]))
	
	ser.close()

else:
    print "ping, settime, listevents, setoutput <state> <port>, newevent <on time> <off time> <port>, deleteevent <index>"

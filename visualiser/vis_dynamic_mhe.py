#!/usr/bin/env python

import sys, time

from PyQt4 import QtCore, QtGui

import numpy as np

import pyqtgraph as pg

from vis_helpers import *
from zmq_protobuf_helpers import *

import DynamicMheTelemetry_pb2 as mheProto

from collections import deque

app = QtGui.QApplication([])

layout = pg.GraphicsLayout(border = (100, 100, 100))
view = pg.GraphicsView()
view.setCentralItem( layout )
view.show()
view.setWindowTitle( "Telemetry for MHE" )
view.resize(1024, 768)

#
# Deserialization
# TODO in protobuf version 2.5.0 this can be achieved a bit easier
#

xNames = mheProto._DYNAMICMHEMSG_XNAMES
zNames = mheProto._DYNAMICMHEMSG_ZNAMES
uNames = mheProto._DYNAMICMHEMSG_UNAMES

class DynamicMheWorker( ZmqSubProtobufWorker ):
	def __init__(self, address, queue, bufferSize = 20, topic = ""):
		
		# Make the xNames enum dictionary, while removing "idx_" from keys 
		self._xMap = dict((v.name.split("_", 1)[ 1 ], v.number) for v in xNames.values)
		# ... and do same for zNames and uNames
		self._zMap = dict((v.name.split("_", 1)[ 1 ], v.number) for v in zNames.values)
		self._uMap = dict((v.name.split("_", 1)[ 1 ], v.number) for v in uNames.values)

		self._simpleFieldNames = ["solver_status", "kkt_value", "obj_value",
								  "exec_fdb", "exec_prep", "ts_trigger", "ts_elapsed"]

		ZmqSubProtobufWorker.__init__(self, address, mheProto.DynamicMheMsg, self._simpleFieldNames,
									  queue, bufferSize, topic)
		
		# Define new buffers for horizons of d. states + RPY, a. states, and control
		for k in self._xMap.keys() + ["roll", "pitch", "yaw"]:
			self._buffer[ k ] = list( xrange(self._msg.N + 1) )
		for k in self._zMap.keys() + self._uMap.keys():
			self._buffer[ k ] = list( xrange( self._msg.N ) )

	def deserialize( self ):
		def updateBufferSimple( name ):
			self._buffer[ name ].popleft()
			self._buffer[ name ].append( getattr(self._msg, name) )
		
		map(updateBufferSimple, self._simpleFieldNames)
		
		def updateHorizonBuffers(nMap, name):
			el = getattr(self._msg, name)
			for k, v in nMap.items():
				self._buffer[ k ] = el[ v ].h._values
		
		updateHorizonBuffers(self._xMap, "x")
		updateHorizonBuffers(self._zMap, "z")
		updateHorizonBuffers(self._uMap, "u")
		
		for n in xrange(self._msg.N + 1):
			# yaw = atan2(e12, e11)
			self._buffer["yaw"][ n ] = np.rad2deg( np.arctan2(self._buffer["e12"][ n ], self._buffer["e11"][ n ]) )
			# pitch = real(asin(-e13))
			self._buffer["pitch"][ n ] = np.rad2deg( np.arcsin( -self._buffer["e13"][ n ] ) )
			# roll = atan2(e23, e33)
			self._buffer["roll"][ n ] = np.rad2deg( np.arctan2(self._buffer["e23"][ n ], self._buffer["e33"][ n ]) )

	def getSimpleFieldNames( self ):
		return self._simpleFieldNames

#
# Generic fields, that all protobufs _must_ have
#
genNames = ["ts_trigger", "ts_elapsed"]

#
# Test fields
#

#
# Window organization
#
mhePlots = dict()

# x, y, z
posNames = ["x", "y", "z"]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), posNames, posNames) )
# RPY, or e11_...e33
rpyNames = ["roll", "pitch", "yaw"]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), rpyNames, rpyNames) )
# dx, dy, dz
velNames = ["dx", "dy", "dz"]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), velNames, velNames) )
# w_... x, y, z
gyroNames = ["w_bn_b_x", "w_bn_b_y", "w_bn_b_z"]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), gyroNames, gyroNames) )

layout.nextRow()

# aileron, elevator; daileron, delevator
ctrlNames = ["aileron", "daileron", "elevator", "delevator"]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), ctrlNames, ctrlNames) )
# ddelta, motor_torque, dmotor_torque, [cos, sin delta]
carNames = ["ddelta", "motor_torque", "dmotor_torque"]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), carNames, carNames) )
# r, dr, ddr, dddr
cableNames = ["r", "dr", "ddr", "dddr"]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), cableNames, cableNames) )
# obj_value, kkt_value, exec_prep, exec_fdb
perfNames = ["obj_value", "kkt_value", "exec_prep", "exec_fdb"]
mhePlots.update( addPlotsToLayout(layout.addLayout( ), perfNames, perfNames,
				options = {"obj_value": ["semilogy"],
						   "kkt_value": ["semilogy"]}) )

horizonNames = posNames + rpyNames + velNames + gyroNames + \
			   ctrlNames + carNames + cableNames
historyNames = perfNames

#
# Setup update of the plotter
#

# Queue for data exchange between the worker and the main thread
import Queue
q1 = Queue.Queue(maxsize = 10)

def updatePlots():
	global q1
	global mhePlots
	global horizonNames, historyNames

	def updateGroup(q, plots):
		try:
			data = q.get_nowait()
			timeStamps = data[ "ts_trigger" ]
			# Update 
			map(lambda name: plots[ name ].setData( data[ name ] ), horizonNames)
			map(lambda name: plots[ name ].setData(timeStamps, data[ name ]), historyNames)
			
		except Queue.Empty:
			pass
		
	# Update all plots
	updateGroup(q1, mhePlots)

timer = QtCore.QTimer()
timer.timeout.connect( updatePlots )
timer.start( 100 )

mheNamesExt = genNames + historyNames + horizonNames

#
# ZMQ part:
#

host = "192.168.1.110"

DynamicMhePort = "5570"

# Create workers
workers = []

# NOTE: All buffer lenghts are set to correspond to last 20 sec.
# TODO: Somehow we should automate this...

#workers.append(ZmqSubProtobufWorker(host + ":" + DynamicMhePort, mheProto.DynamicMheMsg, mheNamesExt,
#									q1, bufferSize = 20 * 25))

workers.append(DynamicMheWorker(host + ":" + DynamicMhePort, q1, bufferSize = 20 * 25))

# Start Qt event loop unless running in interactive mode.
#
if __name__ == '__main__':

	for worker in workers:
		worker.start()
	
	import sys
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		QtGui.QApplication.instance().exec_()
	
	for worker in workers:
		worker.stop()
#! python3
#==================================================================================================
# simple_acq.py - a simple Python script for ultrasound data acquisition with the USPlatform.
#--------------------------------------------------------------------------------------------------
# Copyright (c) 2019 us4us Ltd. / MIT License
#==================================================================================================
import os, time
from ctypes import *
import numpy
import matplotlib.pyplot as plt
from matplotlib.pyplot import ion
from Queue import *

## Define constants ##
NSAMPLES = 4096		# number of samples in each RF line
NCHANNELS = 192		# number of channels

# Define HAL callback & structs
MY_CALLBACK_FUNC = CFUNCTYPE(c_void_p, c_int)

class Metadata(Structure):
	_fields_ = [('totalTriggers', c_int),
				('totalChunks', c_int),
				('skippedChunks', c_int),
				('frameIdx', c_int)]

class CallbackResult:
	def __init__(self, dataArray, frameIdx):
		self.dataArray = dataArray
		self.frameIdx = frameIdx

queue = Queue()		# create a data queue

def myOutputCallback(idx):
	global halDLL, halPointer, queue, NSAMPLES, NCHANNELS
	print 'callback', idx
	outputData = halDLL.GetData(halPointer, idx)
	ap = cast(outputData, POINTER(c_int16 * NCHANNELS * NSAMPLES))
	outputNumpyArray = (numpy.frombuffer(ap.contents, numpy.int16)).reshape(NSAMPLES, NCHANNELS)
	metadata = (cast(halDLL.GetMetadata(halPointer, idx), POINTER(Metadata))).contents
	queue.put(CallbackResult(outputNumpyArray.copy(), metadata.frameIdx))

# ------------
# Script main
# ------------
currPath = os.path.dirname(__file__)
halDLL = pydll.LoadLibrary(currPath + r'\USHAL.dll')	# load HAL library
print halDLL

halDevicesNamesPointer = c_void_p()
halDevicesNumber = halDLL.GetAvailableHALDevicesNames(addressof(halDevicesNamesPointer))
longestNameLength = 20
halDevicesNames = cast(halDevicesNamesPointer, POINTER(POINTER(c_char * longestNameLength)))

print 'Detected devices:'
for i in range(halDevicesNumber):
	rawName = str(halDevicesNames[i].contents[:])
	print '---', rawName[0:rawName.find('\0')]

halPointer = halDLL.GetHALInstance('USPLATFORMHAL')	# open real USPlatform hardware

f = open('PA-BFR.json', 'r')	# open TX/RX JSON file
json = f.read()
f.close()

# Setup USPlatform HAL
halOutputCallback = MY_CALLBACK_FUNC(myOutputCallback)
halDLL.RegisterCallback(halPointer, halOutputCallback)
halDLL.Configure(halPointer, json)

halDLL.Start(halPointer)	# start TX/RX

# Perform data acquisition and display
for i in range(200):
	callbackResult = queue.get(block=True, timeout=20)
	image = callbackResult.dataArray
	if i == 0:
		imgplot = plt.imshow(image, cmap=plt.cm.gray, aspect='auto')
		ion()
		plt.show()
	else:
		imgplot.set_data(image)
		plt.draw()
		plt.pause(0.01)

	halDLL.Sync(halPointer, callbackResult.frameIdx)
	#halDLL.SoftTrigger(halPointer)

halDLL.Stop(halPointer)		# stop TX/RX
halDLL.UnregisterCallback(halPointer)
halDLL.Release(halPointer)

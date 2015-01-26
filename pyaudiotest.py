import pyaudio
import numpy
import wave
import sys
import math

CHUNK = 1024
FORMAT = pyaudio.paInt16
CHANNELS = 2
RATE = 44100

p = pyaudio.PyAudio()

stream = p.open(format=FORMAT,
	channels=CHANNELS,
	rate=RATE,
	input=True,
	output=True,
	frames_per_buffer=CHUNK,
	input_device_index=5,
	output_device_index=5)

data = stream.read(CHUNK)

S_max = 0;

hostory = 3

prevData = []

while data != '':
	dataArr = numpy.fromstring(data, dtype=numpy.int16)
	#print dataArr
	for i in range(len(dataArr)):
		#(dataArr[i] - 32767) / 32768;
		#conversion to 1 to -1
		current = dataArr[i] / 32768

		if(current > 1):
			current = 1
		if(current < -1):
			current = -1

		dataArr[i] = current;

		if S_max < math.abs(current):
			S_max = math.abs(current)
	
	gain = 0.95 / S_max
	prevData.append(gain)
	
	gain = (9.0/16.0)*gain + (1.0/16.0)*prev_gain[0] + (1.0/8.0)*prev_gain[1] + (1.0/4.0)*prev_gain[2];

	if(len(prevData) > history):
		prevData.pop(0)

	for i in range(len(dataArr)):
		dataArr[i] = dataArr[i] * gain
		if(dataArr[i] > 1):
			dataArr[i] = 1
		if(dataArr[i] < -1):
			dataArr = -1

		dataArr[i] = dataArr * 32768
		if(dataArr[i] == 32768):
			dataArr[i] = dataArr[i] - 1

	data = dataArr.tostring()



	stream.write(data)
	data = stream.read(CHUNK)

stream.stop_stream()
stream.close()

p.terminate()


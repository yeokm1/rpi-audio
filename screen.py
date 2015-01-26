import serial
import time
import os

ser = serial.Serial('/dev/ttyUSB0', 9600)  # open first serial port
print ser.name          # check which port was really used
time.sleep(2)
#wait for microview to start
percent = 80

ser.write(percent)
os.system("amixer -c 0 set PCM " + percent + "%")

while True:
        time.sleep(0.05)
        readByte = ser.read(1)
        if readByte == "u" :
                ser.write("@")
                percent += 1
                ser.write(percent)
                os.system("amixer -c 0 set PCM " + percent + "%")
        elif readByte == "d" :
                ser.write("@")
                percent -= 1
                ser.write(percent)
                os.system("amixer -c 0 set PCM " + percent + "%")

ser.close()             # close port

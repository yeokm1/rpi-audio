import serial
import time

ser = serial.Serial('/dev/ttyUSB0', 9600)  # open first serial port
print ser.name          # check which port was really used
time.sleep(2)
#wait for microview to start

while True:
        time.sleep(0.05)
        print("new")
        readByte = ser.read(1)
        if readByte == "u" :
                ser.write("@")
                ser.write("up")
        elif readByte == "d" :
                ser.write("@")
                ser.write("down")

ser.close()             # close port

# rpi-audio

#For input and output manipulation
```bash
g++ channel.cpp -o channel -lasound
./channel default plughw:0, 1
```

#Using sox library via piping
```bash
./laggynorm.sh
```

#For receiving input from microcontroller
```bash
sudo apt-get install libboost1.50-all
g++ -lboost_system -lpthread -std=c++0x threads.cpp -o threads
#Use dmesg to check serial port USB number
./threads /dev/ttyUSBn

#Python
sudo apt-get install python-pip
pip install pyserial
```

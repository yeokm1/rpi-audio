# rpi-audio

```bash
sudo apt-get install libboost1.50-all
g++ -lboost_system -lpthread -std=c++0x threads.cpp -o threads
#Use dmesg to check serial port USB number
./threads /dev/ttyUSBn
```

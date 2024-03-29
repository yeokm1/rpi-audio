#include <iostream>
#include <thread>
#include <string>
#include "SimpleSerial.h"
#include <cstdlib>

using namespace std;
using namespace boost;

int percent = 85;

void threadSerial(string serialPortAddress){
	 try {
		     SimpleSerial serial(serialPortAddress,9600);

		     sleep(2);
			
		     serial.writeString(std::to_string(percent));
		while(true){
			string input = serial.readLine();
			if(input.compare("u") == 0){
				percent++;
			} else if (input.compare("d") == 0){
			
				percent--;
			} else {
				continue;
			}

			cout << percent << endl;
			 serial.writeString("@");
			serial.writeString(std::to_string(percent));
		//	string command = "amixer -c 0 set PCM ";
		//	command  =  command + string(percent);
		//	command = command + "%";
			
			char buffer [50];
char audio2buff[100];			
sprintf(buffer, "amixer -c 0 set PCM %d%%", percent);
sprintf(audio2buff, "amixer -c 1 cset iface=MIXER,name='HPOUT2 Digital Volume' %d%%", percent);			
std::system(buffer);
std::system(audio2buff);
cout << buffer << endl;
		}

		     

	 } catch(boost::system::system_error& e)
	 {
	   cout<<"Error: "<<e.what()<<endl;
	 }


}


int main(int argc, char* argv[])
{
    thread t1(threadSerial, argv[1]);

    while(true){
        sleep(1);
    }

    return 0;
}

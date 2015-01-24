#include <iostream>
#include <thread>
#include <string>
#include "SimpleSerial.h"

using namespace std;
using namespace boost;

int percent = 50;

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

		}

		     

	 } catch(boost::system::system_error& e)
	 {
	   cout<<"Error: "<<e.what()<<endl;
	 }


}


int main(int argc, char* argv[])
{
    thread t1(threadSerial, argv[1]);

    while(true);

    return 0;
}

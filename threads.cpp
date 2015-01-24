#include <iostream>
#include <thread>
#include <string>
#include "SimpleSerial.h"

using namespace std;
using namespace boost;

void threadSerial(string serialPortAddress){
	 try {
		     SimpleSerial serial(serialPortAddress,9600);

		     sleep(2);
		     serial.writeString("Hello world\n");

		     cout<<serial.readLine()<<endl;

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

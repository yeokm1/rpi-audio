#include <stdio.h>
#include <pthread.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#define BUFFERSIZE 10 
int percent = 0;

int open_port(void)
 {
   int fd; /* File descriptor for the port */


   fd = open("/dev/ttym1", O_RDWR | O_NOCTTY | O_NDELAY);
   if (fd == -1)
   {
    /*
     * Could not open the port.
     */

     fprintf(stderr, "open_port: Unable to open /dev/ttym1 - %s\n",
             strerror(errno));
   }

   return (fd);
 }



void *thread_function( void *ptr ){
	
	char buffer;
	char writeBuffer[5];
	int result;
	while(1){
		//sleep(1);
		//fcntl(fd, F_SETFL, FNDELAY); 
		read(fd, &buffer, sizeof(buffer));
		if(buffer == 'u'){
			percent++;
		} else if(buffer == 'd'){
			percent--;
		}
		
		itoa(percent, writeBuffer, 10);
		write(fd, writeBuffer, 4);
	}
}

int main()
{
     fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
     pthread_t thread;

     pthread_create( &thread, NULL, thread_function, "thread");

     while(1){
	sleep(1);
     	printf("%d\n", percent);
     
     }
	
     
     return 0;
}

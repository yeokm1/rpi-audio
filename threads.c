#include <stdio.h>
#include <pthread.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#define BUFFERSIZE 10 
int percent = 0;
int USB;

void *thread_function( void *ptr ){
	

	
	
	char buffer;
	char writeBuffer[5];
	int result;
	while(1){
		//sleep(1);
			int n = 0, spot = 0;
			char buf;
	
			/* Whole response*/
			char response;
			memset(response, '\0', sizeof (response));
		

			n = read( USB, &buf, 1 );


			if(buf == 'u'){
				percent++;
			} else if(buffer == 'd'){
				percent--;
			}
		
		sprintf(writeBuffer, "%d", percent);
		write( USB, writeBuffer], 5 );
	}
}

int main()
{
	open( "/dev/ttyUSB0", O_RDWR| O_NOCTTY );
	struct termios tty;
	struct termios tty_old;
	memset (&tty, 0, sizeof (tty));

/* Save old tty parameters */
tty_old = tty;

/* Set Baud Rate */
cfsetospeed (&tty, (speed_t)B9600);
cfsetispeed (&tty, (speed_t)B9600);

/* Setting other Port Stuff */
tty.c_cflag     &=  ~PARENB;            // Make 8n1
tty.c_cflag     &=  ~CSTOPB;
tty.c_cflag     &=  ~CSIZE;
tty.c_cflag     |=  CS8;

tty.c_cflag     &=  ~CRTSCTS;           // no flow control
tty.c_cc[VMIN]   =  1;                  // read doesn't block
tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

/* Make raw */
cfmakeraw(&tty);

/* Flush Port, then applies attributes */
tcflush( USB, TCIFLUSH );

     
     pthread_t thread;

     pthread_create( &thread, NULL, thread_function, "thread");

     while(1){
	sleep(1);
     	printf("%d\n", percent);
     
     }
	
     
     return 0;
}

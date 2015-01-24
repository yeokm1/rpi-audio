#include <stdio.h>
#include <pthread.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#define BUFFERSIZE 10 
int percent = 0;
int fd;

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
		
		sprintf(writeBuffer, "%d", percent);
		write(fd, writeBuffer, 4);
	}
}

int main()
{
     int fd; /* port file descriptor */
     char port[20] = “/dev/ttyS0″; /* port to connect to */
     speed_t baud = B9600; /* baud rate */

     fd = open(port, O_RDWR); /* connect to port */

     /* set the other settings (in this case, 9600 8N1) */
     struct termios settings;
     tcgetattr(fd, &settings);

     cfsetospeed(&settings, baud); /* baud rate */
     settings.c_cflag &= ~PARENB; /* no parity */
     settings.c_cflag &= ~CSTOPB; /* 1 stop bit */
     settings.c_cflag &= ~CSIZE;
     settings.c_cflag |= CS8 | CLOCAL; /* 8 bits */
     settings.c_lflag = ICANON; /* canonical mode */
     settings.c_oflag &= ~OPOST; /* raw output */

     tcsetattr(fd, TCSANOW, &settings); /* apply the settings */
     tcflush(fd, TCOFLUSH);
     
     
     pthread_t thread;

     pthread_create( &thread, NULL, thread_function, "thread");

     while(1){
	sleep(1);
     	printf("%d\n", percent);
     
     }
	
     
     return 0;
}

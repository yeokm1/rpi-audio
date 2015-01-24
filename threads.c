#include <stdio.h>
#include <pthread.h>
#define BUFFERSIZE 10

int percent = 0;
void *thread_function( void *ptr ){
	
	char buffer[BUFFERSIZE];
	int result;
	while(1){
		sleep(1);
		if(fgets(buffer, BUFFERSIZE , stdin) != NULL){
			percent = strtol(buffer, (char **)NULL, 10);
		}
		
	}
}

int main()
{
     pthread_t thread;

     pthread_create( &thread, NULL, thread_function, "thread");

     while(1){
	sleep(1);
     	printf("%d\n", percent);
     
     }
	
     
     return 0;
}

// includes
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// compiler defs
#define BENCH true
#define NTHREADS 5000

void* do_nothing( void* null){
	// allocate a variable on the stack and write to it
	int i = 0;
	// exit the child thread
	pthread_exit( NULL);}

int main( int argc, char *argv[]){
	// thread return code
	int status;
	// id of created threads
	pthread_t thread_id;
	// thread initialization options
	pthread_attr_t attr;

	// initialize attr and set the 'joinable' option
	// ( the second line is actually superfluous because
	// `PTHREAD_CREATE_JOINABLE` is default )
	pthread_attr_init( &attr);
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE);

	// once for each thread
	for( int j = 0; j < NTHREADS; j++){
		// start the new thread with the options `attr`, running
		// the `do_nothing()` function on the single argument `NULL`
		status = pthread_create( &thread_id, &attr, do_nothing, NULL);
		// catch any errors
		if( status > 0){
			printf("Error: Return code from pthread_create() is %d\n", status);
			exit( -1);}

		// wait for the newly-created thread to exit
		status = pthread_join( thread_id, NULL);
		// catch any errors
		if( status > 0){
			printf( "Error: Return code from pthread_join() is %d\n", status);
			exit( -1);}}

	// deallocate attr
	pthread_attr_destroy( &attr);
	// exit the main thread
	pthread_exit( NULL);
}

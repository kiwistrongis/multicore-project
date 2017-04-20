// standard library imports
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// compiler defs
#define BENCH true

// parameters given to worker threads
struct thread_arg {
	int id;
	int seats_a;
	int* tickets;
	pthread_mutex_t* tickets_mut;
};

// function defs
void* work( void* t_arg);
int rand_20();
int rand_1_4();

// entry-point function
int main( int argc, char *argv[]){
	// error check
	if( argc != 4){
		printf( "error: not exactly 3 args\n");
		exit( -1);}

	// parse args
	size_t threads_n = atoi( argv[1]);
	int seats_n = atoi( argv[2]);
	float overselling = atof( argv[3]);
	int seats_a = seats_n + round( seats_n * overselling / 100.0f);

	// initialize tickets pointer and mutex
	int* tickets = malloc( sizeof( int));
	*tickets = 0;
	pthread_mutex_t* tickets_mut = malloc( sizeof( pthread_mutex_t));
	pthread_mutex_init( tickets_mut, NULL);

	// thread vars
	pthread_t threads[threads_n];
	struct thread_arg thread_args[threads_n];
	pthread_attr_t attr;
	pthread_attr_init( &attr);
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE);

	// other setup
	srand( time( NULL));

	int error;
	for( size_t i = 0; i < threads_n; i++){
		// initialize thread args
		thread_args[i].id = i;
		thread_args[i].seats_a = seats_a;
		thread_args[i].tickets = tickets;
		thread_args[i].tickets_mut = tickets_mut;

		// start thread
		error = pthread_create(
			&threads[i], &attr, work, (void*) &thread_args[i]);
		// error check
		if( error)
			printf( "error creating thread[%d]: %d\n", i, error);}
	// cleanup
	pthread_attr_destroy( &attr);

	// join on all threads
	for( size_t i = 0; i < threads_n; i++){
		error = pthread_join( threads[i], NULL);
		if( error)
			printf( "error joining thread[%d]: %d\n", i, error);}

	// print summary
	if( ! BENCH)
		printf(
			"Summary: %d tickets sold out of %d (%d + %.1f%%)\n",
			*tickets, seats_a, seats_n, overselling);

	// cleanup and exit
	pthread_mutex_destroy( tickets_mut);
	free( tickets_mut);
	free( tickets);

	//pthread_exit( NULL);
	exit( 0);
}

// worker thread function
void* work( void* t_arg){
	struct thread_arg* arg = (struct thread_arg*) t_arg;
	bool term = false;
	bool even_id = ( arg->id % 2) == 0;
	int threshold = even_id ? 9 : 6;

	while( true){
		pthread_mutex_lock( arg->tickets_mut);
		int seats_left = ( arg->seats_a - *arg->tickets);
		if( seats_left <= 0){
			pthread_mutex_unlock( arg->tickets_mut);
			break;}

		if( rand_20() < threshold){
			int tickets_sold = rand_1_4();
			if( tickets_sold > seats_left)
				tickets_sold = seats_left;
			*arg->tickets += tickets_sold;
			//printf( "thread[%d]: tickets: %d\n", arg->id, *arg->tickets);}
			if( ! BENCH)
				printf(
"Ticket agent %d: Successful transaction - %d tickets sold (%d total)\n",
					arg->id, tickets_sold, *arg->tickets);}
		else if( ! BENCH){
			printf( "Ticket agent %d: Unsuccessful transaction\n", arg->id);}

		pthread_mutex_unlock( arg->tickets_mut);}
	pthread_exit( NULL);}

// return a random number from [0,20)
int rand_20(){
	return rand() % 20;}

// return a random number from [1,4]
int rand_1_4(){
	return 1 + rand() % 4;}

// imports
// standard library imports
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// compiler defs
#define BENCH true
#define BROKERS_N 100
#define EXCHANGE_DELAY 0.2
#define EXCHANGE_DAYS 10000
#define STOCKS_N 5

// type defs
struct Stock { 
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	char name;
	int price;
	int amount;
};

// parameters given to worker threads
struct BrokerArg {
	int id;
	bool* term;

	struct Stock* stock;
	int buy_price;
	int buy_amount;
	int sell_price;
	int sell_amount;
};

struct ExchangeArg {
	int id;
	struct Stock* stocks;
};

// function defs
int main( int argc, char *argv[]);
void* thread_broker( void* t_arg);
void* thread_exchange( void* t_arg);
void update_stock( struct Stock* stock, bool chance, float price_var);;
bool one_in( int n);
float price_var( int a, float b, float c);

// function impls
// entry-point function
int main( int argc, char *argv[]){
	// vars
	// termination signal for broker threads
	bool term = false;
	// list of stocks currently being traded on
	struct Stock stocks[STOCKS_N];
	// broker and exchange threads
	pthread_t brokers[ BROKERS_N];
	pthread_t exchange;
	// broker and exchange thread args
	struct BrokerArg broker_args[BROKERS_N];
	struct ExchangeArg exchange_args;
	pthread_attr_t attr;
	pthread_attr_init( &attr);
	pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_JOINABLE);

	// initialize stocks
	stocks[0] = (struct Stock) { .name = 'A', .price = 100, .amount = 050};
	stocks[1] = (struct Stock) { .name = 'B', .price = 200, .amount = 150};
	stocks[2] = (struct Stock) { .name = 'C', .price = 150, .amount = 050};
	stocks[3] = (struct Stock) { .name = 'D', .price = 240, .amount = 100};
	stocks[4] = (struct Stock) { .name = 'E', .price = 300, .amount = 200};
	for( size_t i = 0; i < STOCKS_N; i++){
		pthread_mutex_init( &stocks[i].mutex, NULL);
		pthread_cond_init( &stocks[i].cond, NULL);}

	// initialize broker args
	for( size_t i = 0; i < BROKERS_N; i++){
		int j = i % STOCKS_N;
		int x = i / STOCKS_N + 1;
		broker_args[i].term = &term;
		broker_args[i].stock = &stocks[j];
		broker_args[i].buy_price = stocks[j].price - 1*( 1 + x);
		broker_args[i].buy_amount = 5;
		broker_args[i].sell_price = stocks[j].price + 1*( 1 + x);
		broker_args[i].sell_amount = 5;}

	// initialize exchange args
	exchange_args.stocks = (struct Stock*) &stocks;

	// other setup
	srand( time( NULL));

	// start brokers
	size_t i;
	int error;
	for( i = 0; i < BROKERS_N; i++){
		broker_args[i].id = i;
		error = pthread_create(
			&brokers[i], &attr, thread_broker, (void*) &broker_args[i]);
		if( error)
			printf( "error creating broker[%d]: %d\n", i, error);}
	// start exchange
	exchange_args.id = i;
	error = pthread_create(
		&exchange, &attr, thread_exchange, (void*) &exchange_args);
	if( error)
		printf( "error creating exchange: %d\n", error);
	// cleanup
	pthread_attr_destroy( &attr);

	// join on the exchange thread
	error = pthread_join( exchange, NULL);
	if( error)
		printf( "error joining exchange: %d\n", error);
	// signal the brokers to terminate
	term = true;
	for( i = 0; i < STOCKS_N; i++){
		pthread_mutex_lock( &stocks[i].mutex);
		pthread_cond_broadcast( &stocks[i].cond);
		pthread_mutex_unlock( &stocks[i].mutex);}
	// join on the broker threads
	for( size_t i = 0; i < BROKERS_N; i++){
		//printf( "joining broker[%d]\n", i);
		error = pthread_join( brokers[i], NULL);
		if( error)
			printf( "error joining broker[%d]: %d\n", i, error);}
	//printf( "all threads joined!\n", i);

	// cleanup and exit
	for( i = 0; i < STOCKS_N; i++){
		pthread_mutex_destroy( &stocks[i].mutex);
		pthread_cond_destroy( &stocks[i].cond);}
	//pthread_exit( NULL);
	return 0;}

// broker thread function
void* thread_broker( void* t_arg){
	// vars
	struct BrokerArg* arg = (struct BrokerArg*) t_arg;
	struct Stock* stock = arg->stock;
	int id = arg->id;
	int buy_price = arg->buy_price;
	int buy_amount = arg->buy_amount;
	int sell_price = arg->sell_price;
	int sell_amount = arg->sell_amount;
	// debug
	if( ! BENCH)
		printf( "Broker %d watching %c\n", id, stock->name);

	// start waiting for signals
	pthread_mutex_lock( &stock->mutex);
	pthread_cond_wait( &stock->cond, &stock->mutex);
	// while term signal has not been sent
	while( !( *arg->term)){
		// if new price is desireable, and there is some left, buy
		if( stock->price < buy_price && stock->amount > 0){
			int change = stock->amount >= buy_amount ?
				buy_amount : stock->amount;
			if( ! BENCH)
				printf( "   > Broker %d bought %02d %c\n",
					id, buy_amount, stock->name);
			stock->amount -= buy_amount;}
		// if new price is lucrative, sell
		else if( stock->price > sell_price){
			if( ! BENCH)
				printf( "   > Broker %d sold %02d %c\n",
					id, sell_amount, stock->name);
			stock->amount += sell_amount;}
		// wait for next signal
		pthread_cond_wait( &stock->cond, &stock->mutex);}
	// cleanup and exit
	pthread_mutex_unlock( &stock->mutex);
	//printf( "Broker %d terminating\n", id);
	pthread_exit( NULL);}

// exchange thread function
void* thread_exchange( void* t_arg){
	// vars
	struct ExchangeArg* arg = (struct ExchangeArg*) t_arg;
	struct Stock* stocks = arg->stocks;

	// every day
	for( int day = 0; day < EXCHANGE_DAYS; day++) {
		if( ! BENCH)
			printf("Day [%03d]:\n", day);
		// update the value of each stock and signal waiting threads
		update_stock( &stocks[0], one_in( 3), price_var( 10, 3.6, 2.3));
		update_stock( &stocks[1], one_in( 7), price_var( 12, 5.0, 2.3));
		update_stock( &stocks[2], one_in( 6), price_var(  7, 1.0, 2.1));
		update_stock( &stocks[3], one_in( 2), price_var(  8, 5.0, 1.8));
		update_stock( &stocks[4], one_in( 4), price_var(  8, 2.0, 1.4));
		if( ! BENCH)
			sleep( EXCHANGE_DELAY);}
	// exit
	//printf( "Exchange terminating\n");
	pthread_exit( NULL);}

// possibly vary stock price, signal brokers on change
void update_stock( struct Stock* stock, bool chance, float price_var){
	// if random chance check passes and price hasn't hit minumum
	if( chance && stock->price > 100){
		// vary the price randomly
		stock->price += price_var;
		if( ! BENCH)
			printf("  %c: %03d @ %05d (%+04f)\n",
				stock->name, stock->amount, stock->price, price_var);
		// and signal brokers
		pthread_mutex_lock( &stock->mutex);
		pthread_cond_broadcast( &stock->cond);
		pthread_mutex_unlock( &stock->mutex);}
	else
		if( ! BENCH)
			printf("  %c: %03d @ %05d\n",
				stock->name, stock-> amount, stock->price);}

// calculate random change 1/n
bool one_in( int n){
	return 0 == ( rand() % n);}
// calculate random price variance
float price_var( int a, float b, float c){
	return 10 * ( rand() % a - b) / c;}

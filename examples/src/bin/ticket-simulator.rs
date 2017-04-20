extern crate rand;

use std::sync::{ Arc, Mutex};
use std::thread;

// constants
const BENCH : bool = true;
const AGENT_SLEEP : u32 = 500;

fn main(){
	let threads_n : usize = 1000;
	let seats_n : u32 = 10000;
	let overselling : f32 = 0.0;
	let seats_a : u32 = seats_n + ( seats_n as f32 * overselling) as u32;
	let tickets = Arc::new( Mutex::new( 0));

	let mut handles = vec![];
	for i in 0..threads_n {
		// clone our mutex pointer!
		let tickets = tickets.clone();
		handles.push( thread::spawn(
			move ||{ work( i, seats_a, tickets);}
		));}
	for handle in handles {
		let _result = handle.join();}

	let tickets = tickets.lock().unwrap();
	if ! BENCH {
		println!(
			"Summary: {} tickets sold out of {} ({} + {:.2}%)",
			*tickets, seats_a, seats_n, overselling);}
}

// worker thread function
fn work( id: usize, seats_a: u32, tickets: Arc<Mutex<u32>>){
	let threshold = if id % 2 == 0 { 9 } else { 6 };

	loop {
		if ! BENCH {
			#[allow( deprecated)]
			thread::sleep_ms( AGENT_SLEEP);}
		// lock mutex!
		let mut tickets = tickets.lock().unwrap();
		let seats_left = seats_a - *tickets;
		if seats_left == 0 { break;}

		if rand_20() < threshold {
			let mut tickets_sold = rand_1_4();
			if tickets_sold > seats_left {
				tickets_sold = seats_left;}
			// sell us some tickets!
			*tickets += tickets_sold;
			//printf( "thread[{}]: tickets: {}", id, tickets);}
			if ! BENCH {
				println!(
					"Ticket agent {}: Successful transaction \
					- {} tickets sold ({} total)",
					id, tickets_sold, *tickets);}}
		else {
			if ! BENCH {
				println!( "Ticket agent {}: Unsuccessful transaction", id);}}}}

// return a random number from [0,20)
fn rand_20() -> u32 {
	rand::random::<u32>() % 20}

// return a random number from [1,4]
fn rand_1_4() -> u32 {
	1 + rand::random::<u32>() % 4}
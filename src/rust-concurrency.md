# \textsf{Synchronization and Concurrency in the Rust Programming Language}

Author: Kalev Andrei Kalda :: 100425828  
Date: 2017-04-19  
Text License: [Creative Commons by Attribution 4.0](
	https://github.com/kiwistrongis/multicore-project/blob/master/license.md)  
Code License: [MIT License](
	https://github.com/kiwistrongis/multicore-project/blob/master/examples/license.md)  
Sources: [`github.com/kiwistrongis/multicore-project`](
	https://github.com/kiwistrongis/multicore-project/)  

## 0.0 Abstract
Long have programmers struggled against the eternal tide of bugs. Segfaults, null pointer exceptions, use-after-frees, memory leaks, and such are just a few of the vile beasts we battle. However, a new weapon has now arrived in our arsenal that may trivialize this fight. Its name - The Rust Programming Language.

Rust is a relatively new, and incredibly innovative language aimed at systems development with a focus on multi-threading. It is fast, memory-cheap, guaranteed to be thread-safe, and completely eliminates large classes of bugs. Additionally, it fully utilizes various new ideas in programming languages, making the programmer's experience fairly pleasant.

In this paper we will introduce the basics of concurrency in Rust, make comparisons to C by analyzing equivalent code, and draw tentative conclusions regarding the advantages and disadvantages of Rust.

\setcounter{tocdepth}{5}
\tableofcontents

## 1.0 Brief Introduction to Rust
Rust has a number of useful features inspired from other recent programming languages. This list includes, but is not limited to: type inference, algebraic data types, advanced iterators, macros, pattern matching, etc. If you can name a neat feature from your favorite language, Rust most likely has it, unless it interferes with Rust's core mission.

In the following code we can see examples of the following:

 - Type inference (`x` of type `str`),
 - Iterators (`0..4`),
 - Macros (`println!`).

```rust
fn main(){
	let x = "world";
	for i in 0..4 {
		println!("Hello, {} ({:d})!", x, i);}
}
```

The `println!` macro is an interesting case. Because the standard output is not thread-safe, the standard library wraps it in a mutex. `println!` is a convenient work-around, but there's also more going on behind the scenes. The formatting part of the macro actually expands into Rust code - allowing type checks on the `x` and `i` parameters. If these two parameters are swapped, the code actually won't compile! The entire language and its standard library are built in this fashion. Whenever reasonable, errors are made into compile-time errors. While this does cause some programmer frustration at times, it is generally considered worth the effort in exchange for safer code.

***Ownership and Lifetimes***  
The one main innovation of the language is its "lifetime" system. Essentially, the compiler tracks the 'life' of every variable throughout the logic of the program. This allows the compiler to enforce some safety guarantees and work some magic. Primarily, this means that two control flows cannot 'own' the same variable. Additionally, after variables are used for the last time, they are automatically deallocated. These guarantees and restrictions are the foundations of Rust's thread-safety guarantees.

For more on this subject, see the Rust Book sections on [*ownership*][book-ownership], [*references*][book-references] and [*lifetimes*][book-lifetimes].

## 2.0 Rust Threads {#threads}
```rust
use std::thread;

let child = thread::spawn( move || {
  do_something();
});
let result = child.join();
```

Rust threads just that easy to use. Two things to note here. Firstly the `move||{}` closure takes ownership of any variables used inside it. This will be more relevent later, but be aware that any variables used inside the closure cannot be used later in the code. Secondly, as will be discussed in [section 4.1](#threads-example), Rust threads are not cheap (compared to C). Every Rust thread has its own thread-local storage and RNG in addition to its stack.

## 3.0 Rust Synchronization Primitives
**Note**: Many examples in this setion are adapted from [the `std::sync` documentaion][std-sync].

### 3.1 Arc :: Atomically Reference-Counted Pointer
```rust
use std::sync::Arc;
```

An `Arc`, or 'Atomically Reference-Counted Pointer', is necessary whenever sharing variables across threads. Normally this is impossible due to Rust's lifetime restrictions, however `Arc`s make it possible by keeping track of every reference to the contained data and deallocating it when the last reference is dropped. While useless on its own, `Arc`s are necessary for sharing all of the other synchronization primitives across threads.

### 3.2 Mutex :: Mutual Exclusion
```rust
use std::sync::Mutex;

let mutex = Mutex::new( 0);
let mut guard = mutex.lock().unwrap();
*guard += 1;
// implicit drop
drop( guard);
```

Mutexes in Rust work similarly to C, with one notable exception - they are the only way the protected data may be acessed. Access to the protected data is done through a `MutexGuard`, acquired by a blocking call to `lock()`. A non-blocking version is also available with `try_lock()`.

**Note**: As with most of these synchronization primitives, a thread panic while the mutex is locked will poison the mutex, causing the `lock()` function to possibly return errors in other threads. To work around this, it is best not to directly `unwrap()` the the return of `lock()` but to check it first. For more on safely handling `Result` types, see [the rust documentation for `Result`][std-result].

### 3.3 Condvar :: Condition Variable
```rust
use std::sync::{ Arc, Condvar, Mutex};
use std::thread;

let cond = Arc::new( Condvar::new());
let mutex = Arc::new( Mutex::new( 0));

{ // clone our Arc pointers
  let (cond, mutex) = (cond.clone(), mutex.clone());
  let child = thread::spawn( move || {
    let mut x = mutex.lock().unwrap();
    while *x == 0 {
      x = cond.wait( x);}
  });}

let mut x = mutex.lock().unwrap();
*x = 10;
cond.notify_all();
```

Rust convars work identically to C. The key functions are `wait()`, `notify_one()`, and `notify_all()`. Note the use of `Arc`s, specifically the use of `clone()` to create multiple safe references to the mutex and condvar.

### 3.4 Barrier :: Thread Synchronization
```rust
use std::sync::{ Arc, Barrier};
use std::thread;

let barrier = Arc::new( Barrier::new( 2));

{ let barrier = barrier.clone();
  let child = thread::spawn( move || {
    barrier.wait();
  });}

barrier.wait()
```

Once again, Rust barriers work identically to C. Be especially sure to set the number of threads in `Barrier::new()` correctly, as for all its bells and whistles, the compiler cannot check this for you! For a good example using a global constant, see the [stock exchange example](#stocks-example).

### 3.5 Once :: Once-Only Closure
```rust
use std::sync::{ Once, ONCE_INIT};

static START: Once = ONCE_INIT;
START.call_once(|| {
  do_something();
});
```

Many C libraries are written in such a way that they must be initialized once before used. This primitive, while may have other uses, was primarily devised a solution for such situations. C interoperability will not be covered here, but see the [Rust Book section on FFI][book-ffi] for more.

### 3.6 Atomics :: Atomically Accessed Variables
```rust
use std::sync::atomic::{ AtomicBool, Ordering};

let atomic  = AtomicBool::new( true);
atomic.store( false, Ordering::Relaxed);
let _ = atomic.load( Ordering::Relaxed);
```

The various types defined in `std::sync::atomic` are designed to provide safe, atomic access to shared variables. For one example, see the [stock exchange example](#stocks-example), or for more information see the [the `std::sync::atomic` documentaion][std-atomic].

### 3.7 RwLock :: Less Restrictive Mutex
```rust
use std::sync::RwLock;

let lock = RwLock::new( 5);

{ // many reader locks can be held at once
  let reader_1 = lock.read().unwrap();
  let reader_2 = lock.read().unwrap();
  assert_eq!( *reader_1, 5);
  assert_eq!( *reader_2, 5);
} // read locks are dropped at this point


{ // only one write lock may be held, however
  let mut writer = lock.write().unwrap();
  *writer += 1;
  assert_eq!( *writer, 6);
} // write lock is dropped here
```

Think of `RwLock`s as faster Mutexes. While Mutexes only allow one accessor to the protected data, `RwLock`s allow multiple - as long as they only read. To avoid data races, only one write lock is permitted.

**Note**: Similarly to Mutexes, `RwLock`s can be poisoned, but only during a write lock.

### 3.8 MPSC :: Multithread Queues
```rust
use std::thread;
use std::sync::mpsc::channel;

let (sender, receiver) = channel();
thread::spawn( move|| {
  sender.send(10).unwrap();
});
assert_eq!( receiver.recv().unwrap(), 10);
```

Finally, we have queues (or channels). These work similarly to channels in other recent languages. The acronym `MPSC` stands for "multiple producer single consumer queue". As should be evident from the name, the "sending" end of the queue can be shared across threads (after being wrapped in an `Arc`), but the "receiving" end cannot.

**Note**: In addition to the `channel()` in the above example, which has an unlimited size, there exists a `sync_channel()`, which constructs a queue with a fixed size. This queue will block the sender until there exists room in the queue.

## 4.0 Concurrency in C vs. Rust
So we now know the basics of concurrency in Rust, but how does it shape up? Specifically, how does it compare to the gold standard - C. We'll take a close look at both speed and memory usage. We will also examine one of the most important aspects of concurrent programming: programmer utility. This discussion of utility is extremely important, as greater safeness is the one main advantage of Rust over other programming languages.

**Note**: All comparisons were done on Archlinux with an Intel Quad-core i7-5557U CPU @ 3.10GHz, and with `gcc -O3` for C code and `cargo build --release` for Rust code.

### 4.1 Threads Example {#threads-example}
Source code: [Section 6.1.1](#thread-c) and [Section 6.1.2](#thread-rs)

```{include=examples/data/time/threads-c.txt}
```

```{include=examples/data/time/threads-rs.txt}
```

As one might expect, the Rust version of this example is significantly slower than its C counterpart. More interestingly, however, is its memory usage. According to valgrind (See [Section 6.2.1](#valgrind-c) and [Section 6.2.2](#valgrind-rs)), the Rust version allocated 1,762,032 bytes, while the the C version allocated only 34,756 bytes. This is a approximately a 50 times difference. This may, however, still be better than most languages other than C.

One other intesting item of note: code file size. The Rust source code was a measly 12 lines, while the C source code (excluding comments) clocked in at 30 lines.

### 4.2 Ticket Agent Example {#tickets-example}
Source code: [Section 6.1.3](#ticket-simulator-c) and [Section 6.1.4](#ticket-simulator-rs)

```{include=examples/data/time/tickets-c.txt}
```

```{include=examples/data/time/tickets-rs.txt}
```

Once again, Rust clearly loses in time, by around two times. However, consistent with the previous example, Rust wins in code length. The Rust code was 69 lines, while the C code was 128.

### 4.3 Stock Exchange Example {#stocks-example}
Source code: [Section 6.1.5](#stock-exchange-c) and [Section 6.1.6](#stock-exchange-rs)

```{include=examples/data/time/stocks-c.txt}
```

```{include=examples/data/time/stocks-rs.txt}
```

Here we again see results consistent with previous examples: Rust was slower by about 2 times, but shorter code size at 184 lines versus 225 for the C version.

Something to note in this example: Due to Rust's restrictions in the `update_stock()` function, the mutex had to be locked for the entire function, while in the C code, it was only locked when a `Stock` had to be changed. Additionally, all the brokers have to lock their mutex in order to check the stock price. It thus might be possible to see small performance gains by switching to a `RwLock`. This would allow the `update_stock()` to skip acquiring a write lock when unneeded, and the brokers to all `read` their stock at the same time, only acquiring a `write` lock if they needed to buy.

### 4.4 Summary
Drawing from previous results, it can be expected that most Rust concurrency work will be around two times slower, but easier to read, write, and verify. It should be noted that these examples are not ideal benchmark programs. Although a subjective matter, the author does rate the Rust implementations as much more readable.

## 5.0 Conclusion
Unsurprisingly, C consistently wins in speed, but only by two times, rather than the 10 times or more that is common in other languages. In terms of programmer ergonomics, however, C definitely shows its age. Rust has all the latest gadgets, and is awkward or annoying to write in C is often a one-liner in Rust. The additional safety guarantees that Rust provides are especially appreciated in the domain of concurrent programming. The author's recommendation?: Use Rust, but call out to C for any highly demanding processing.

### Further Reading
**Rust programs versus C gcc**  
[https://benchmarksgame.alioth.debian.org/u64q/rust.html](https://benchmarksgame.alioth.debian.org/u64q/rust.html)

**Rust programs versus C++ g++**  
[https://benchmarksgame.alioth.debian.org/u64q/compare.php?lang=rust&lang2=gpp](https://benchmarksgame.alioth.debian.org/u64q/compare.php?lang=rust&lang2=gpp)

**Rust programs versus Go**  
[https://benchmarksgame.alioth.debian.org/u64q/compare.php?lang=rust&lang2=go](https://benchmarksgame.alioth.debian.org/u64q/compare.php?lang=rust&lang2=go)

**Rust programs versus Java**  
[https://benchmarksgame.alioth.debian.org/u64q/compare.php?lang=rust&lang2=java](https://benchmarksgame.alioth.debian.org/u64q/compare.php?lang=rust&lang2=java)

## 6.0 Appendix
### 6.1 Source Code
Downloadable source code for these examples can be found here: [github.com/kiwistrongis/multicore-project](https://github.com/kiwistrongis/multicore-project/tree/master/examples).

#### 6.1.1 thread.c {#thread-c}
```{.C include=gen/thread.c}
```

#### 6.1.2 thread.rs {#thread-rs}
```{.rust include=gen/thread.rs}
```

#### 6.1.3 ticket-simulator.c {#ticket-simulator-c}
```{.C include=gen/ticket-simulator.c}
```

#### 6.1.4 ticket-simulator.rs {#ticket-simulator-rs}
```{.rust include=gen/ticket-simulator.rs}
```

#### 6.1.5 stock-exchange.c {#stock-exchange-c}
```{.C include=gen/stock-exchange.c}
```

#### 6.1.6 stock-exchange.rs {#stock-exchange-rs}
```{.rust include=gen/stock-exchange.rs}
```

### 6.2 Valgrind Results
#### 6.2.1 thread.c {#valgrind-c}
```{.C include=examples/data/mem/threads-c.txt}
```

#### 6.2.2 thread.rs {#valgrind-rs}
```{.C include=examples/data/mem/threads-rs.txt}
```

## 7.0 References
\[1\] Rust Programming Language Site  
[`rust-lang.org`](https://www.rust-lang.org/)  

\[2\] Rust Programming Language Book  
[`doc.rust-lang.org/book/`](https://doc.rust-lang.org/book/)  

\[3\] Rust Standard Library Reference  
[`doc.rust-lang.org/std/`](https://https://doc.rust-lang.org/std/)  

[book-ownership]: https://doc.rust-lang.org/book/ownership.html
[book-references]: https://doc.rust-lang.org/book/references-and-borrowing.html
[book-lifetimes]: https://doc.rust-lang.org/book/lifetimes.html
[book-ffi]: https://doc.rust-lang.org/book/ffi.html
[std-atomic]: https://doc.rust-lang.org/std/sync/atomic/index.html
[std-sync]: https://doc.rust-lang.org/std/sync/
[std-result]: https://doc.rust-lang.org/std/result/enum.Result.html

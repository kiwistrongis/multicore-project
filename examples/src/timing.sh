#!/bin/bash

echo "Timing Threads (C)"
echo "Timing Threads (C)" > data/time/threads-c.txt
echo "$ time bin/thread-opt" >> data/time/threads-c.txt
{ time bin/thread-opt;} &>> data/time/threads-c.txt
echo

echo "Timing Threads (Rust)"
echo "Timing Threads (Rust)" > data/time/threads-rs.txt
echo "$ time target/release/thread" >> data/time/threads-rs.txt
{ time target/release/thread;} &>> data/time/threads-rs.txt
echo

echo "Timing Ticket Simulator (C)"
echo "Timing Ticket Simulator (C)" > data/time/tickets-c.txt
echo "$ time bin/ticket-simulator-opt 1000 10000 0" >> data/time/tickets-c.txt
{ time bin/ticket-simulator-opt 1000 10000 0;} &>> data/time/tickets-c.txt
echo

echo "Timing Ticket Simulator (Rust)"
echo "Timing Ticket Simulator (Rust)" > data/time/tickets-rs.txt
echo "$ time target/release/ticket-simulator" >> data/time/tickets-rs.txt
{ time target/release/ticket-simulator;} &>> data/time/tickets-rs.txt
echo

echo "Timing Stock Exchange (C)"
echo "Timing Stock Exchange (C)" > data/time/stocks-c.txt
echo "$ time bin/stock-exchange-opt" >> data/time/stocks-c.txt
{ time bin/stock-exchange-opt;} &>> data/time/stocks-c.txt
echo

echo "Timing Stock Exchange (Rust)"
echo "Timing Stock Exchange (Rust)" > data/time/stocks-rs.txt
echo "$ time target/release/stock-exchange" >> data/time/stocks-rs.txt
{ time target/release/stock-exchange;} &>> data/time/stocks-rs.txt
echo

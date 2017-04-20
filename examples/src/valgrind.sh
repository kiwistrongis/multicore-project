#!/bin/bash

echo "Valgrinding Threads (C)"
echo "Valgrinding Threads (C)" > data/mem/threads-c.txt
echo "$ valgrind bin/thread" >> data/mem/threads-c.txt
valgrind bin/thread &>> data/mem/threads-c.txt
echo

echo "Valgrinding Threads (Rust)"
echo "Valgrinding Threads (Rust)" > data/mem/threads-rs.txt
echo "$ valgrind target/release/thread" >> data/mem/threads-rs.txt
valgrind target/release/thread &>> data/mem/threads-rs.txt
echo

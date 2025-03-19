This program creates a parent process that generates two files (File1 and File2) with N random numbers. Two child processes count the prime numbers in each file, and the results are compared to determine the winner.

Process

The parent generates two files with N random numbers (range: 0 to XXX+100).

Two child processes count primes in each file.

The parent compares results and prints the winner.

Requirements

C (fork, pipe, file handling)

Random number generation

Prime number checking


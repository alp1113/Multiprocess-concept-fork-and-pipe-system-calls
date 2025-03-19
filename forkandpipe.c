#include <stdio.h>     // standard input output
#include <stdlib.h>    // to generate random numbers
#include <unistd.h>    // to make system calls like fork(), pipe()...
#include <fcntl.h>     // to generate file1 and file2
#include <sys/types.h> // for defining pid_t
#include <sys/wait.h>  // to be able to use wait() for sync and prevent zombie or orphan processes
#include <time.h>      // to seed random generator

#define FILE1 "File1.txt" // file names to store the random numbers
#define FILE2 "File2.txt" // file names to store the random numbers

// This function checks if the number is prime
int is_prime(int num)
{
    if (num < 2)
        return 0;
    for (int i = 2; i * i <= num; i++)
    {
        if (num % i == 0)
            return 0;
    }
    return 1;
}

// Function to generate a file with N random numbers
void generate_file(const char *filename, int N, int range)
{
    FILE *file = fopen(filename, "w"); // this opens a file in write mode
    if (!file)
    { // to chech if it failed
        perror("Error opening file");
        exit(1);
    }
    for (int i = 0; i < N; i++)
    {                                          // this generate N many numbers to the file
        fprintf(file, "%d\n", rand() % range); // writes them
    }
    fclose(file); // closes after writing
}

// This function counts prime numbers in the files
int count_primes(const char *filename)
{
    FILE *file = fopen(filename, "r"); // opens file in read mode not write
    if (!file)
    { // checks if there is any error
        perror("Error opening file");
        exit(1);
    }
    int num, count = 0;
    while (fscanf(file, "%d", &num) != EOF)
    { // scans the numbers in file
        if (is_prime(num))
            count++; // calls the is_prime and increases the count if there are any primes
    }
    fclose(file); // close file
    return count; // return the count of the primes
}

int main()
{
    srand(time(NULL)); // seed current time for randomize

    int N, range;
    printf("Enter the value of N: ");
    scanf("%d", &N);                                // asks for the number of randoms to be generated for each file
    printf("Enter the last 3 digits of your ID: "); // user id
    scanf("%d", &range);
    range += 100; // XXX = last 3 id digit + 100 for upper range

    // calls generate_file to generate two files
    generate_file(FILE1, N, range);
    generate_file(FILE2, N, range);

    // Pipes for inter process communication: parent to child pipe1 and pipe2 to send N to children
    // child to parent pipes parent_pipe1 and paret_pipe2 to return the prime counts
    int pipe1[2];
    int pipe2[2];
    int parent_pipe1[2];
    int parent_pipe2[2];
    // 4 pipes with only read or write as it should be, array of size 2

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1 || pipe(parent_pipe1) == -1 || pipe(parent_pipe2) == -1)
    { // this checks if pipes are created correctly or not, if not displays message
        perror("Creation of the pipe failed");
        exit(1);
    }

    pid_t child1 = fork(); // fork for the first chid1
    if (child1 == 0)
    {                           // first child Process
        close(pipe1[1]);        // close the write end of parent to child pipe pipe1
        close(parent_pipe1[0]); // closes the read end of the child to parent pipe parent_pipe1

        int N_received; // reads from the parent the integer N
        read(pipe1[0], &N_received, sizeof(N_received));

        int prime_count = count_primes(FILE1);                     // calls prime count
        write(parent_pipe1[1], &prime_count, sizeof(prime_count)); // sends prime count to the parent with parent_pipe1

        close(pipe1[0]);
        close(parent_pipe1[1]);
        exit(0); // closings and exit
    }

    pid_t child2 = fork(); // fork for the child 2
    if (child2 == 0)
    {                           // second child Process 2
        close(pipe2[1]);        // close the write of pipe2 parent to child
        close(parent_pipe2[0]); // close the read end of the parent_pipe2 child to parent

        int N_received; // reads n from parent via pipe2
        read(pipe2[0], &N_received, sizeof(N_received));

        int prime_count = count_primes(FILE2);                     // calls count_primes to count the priems
        write(parent_pipe2[1], &prime_count, sizeof(prime_count)); // writes the number of the primes and sends it to the parent via parent_pipe2

        close(pipe2[0]);
        close(parent_pipe2[1]);
        exit(0); // closings
    }

    // Parent process
    close(pipe1[0]);        // close read end to write for child 1
    close(pipe2[0]);        // close read end to write for child 2
    close(parent_pipe1[1]); // close the write end to read from child1 the count of primes
    close(parent_pipe2[1]); // close the write end to read from child2 the count of primes

    // Send N to child processes
    write(pipe1[1], &N, sizeof(N)); // writes the N to the pipe
    write(pipe2[1], &N, sizeof(N)); // writes the N to the pipe

    close(pipe1[1]);
    close(pipe2[1]); // closes

    int prime_count1, prime_count2; // reads from the children the prime counts
    read(parent_pipe1[0], &prime_count1, sizeof(prime_count1));
    read(parent_pipe2[0], &prime_count2, sizeof(prime_count2));

    close(parent_pipe1[0]);
    close(parent_pipe2[0]); // closings

    int winner = (prime_count1 > prime_count2) ? 1 : 2; // determine which child has most primes

    printf("The number of positive integers in each file: %d\n", N);

    printf("The number of prime numbers in File1: %d\n", prime_count1);

    printf("The number of prime numbers in File2: %d\n", prime_count2);

    // prints n and the prime count of each file

    // Wait for child processes to finish to prevent problems like zombies
    wait(NULL);
    wait(NULL);

    printf("I am Child process P1: The winner is child process P%d\n", winner);
    printf("I am Child process P2: The winner is child process P%d\n", winner);
    // print the winner
    return 0;
}

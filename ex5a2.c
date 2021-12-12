/* Number manufacturer
==============================================================================
Written by : Meytal Abrahamian  login : meytalben  id : 211369939
			 Tomer Akrish               tomerak         315224295
==============================================================================
This program connects to the shared memory that Program A has created. 
The program is run in a loop.
Before the program enters the loop, it checks if all the manufacturers are 
connected.
In each loop loop the program produces a number. If it is primary, it adds it 
to the array in the shared memory.
She counts how many new values she added and how many times a value she added 
appeared whose frequency is in the highest array.
If the program sees that the array is full, it prints the data:
How many entries did he add first, 
and how many times did the entry he added appear, and appear the most times
Then the program sends a signal to program A and ends.
*/

// ----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <sys/types.h>
#include <unistd.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

// ----------------------------------------------------------------------------
#define ARR_SIZE 1000			// size of main program array 
#define LOCK_CELL 4				// the cell designed to lock the shared memory
#define ZERO 48					// asci value for 0
#define START_VALUE_CELL 5		// the cell that from it we start primes array
int shm_id;						// shared memory id
int* shm_ptr;					// pointer to shared memory

// ----------------------------------------------------------------------------
void validat_arg(int argc, char* argv_0);
void start(int manu_num);
void connect_to_shm();
void create_primes(int manu_num);
int next_cell_index();
int count_prime_appear(int prime);
void end_manufacturer(int new_primes, int max_appear);
int get_prime();
int is_prime(int number);
// ----------------------------------------------------------------------------

//--------------------------------main-----------------------------------------
int main(int argc, char* argv[])
{
	validat_arg(argc, argv[0]);
	srand((unsigned int)(*argv[1]) - ZERO);
	connect_to_shm();
	start((int)(*argv[1]) - ZERO);
	create_primes((int)(*argv[1]) - ZERO);

	return (EXIT_SUCCESS);
}
// ----------------------------------------------------------------------------

// this function is before start create prime numbers it check if all 
// manufacturer connected to shared memory and end only when they are connected
void start(int manu_num)
{
	int can_start = 0;			// say if can start create prime or not

	shm_ptr[manu_num] = 1;

	while (!can_start)
		if (shm_ptr[1] == 1 && shm_ptr[2] == 1 && shm_ptr[3] == 1)
			can_start = 1;
}
// ----------------------------------------------------------------------------

// this function connect this prosses to a shared memory exists. it get its key 
// and id and then connect
void connect_to_shm()
{
	key_t key;		// key for shm

	// create THE SAME key for the shm as the producer	
	if ((key = ftok(".", '5')) == -1)
	{
		perror("ftok failed: ");
		exit(EXIT_FAILURE);
	}

	// get the id of the block of memory 
	// that was, hopefully, already created by the producer
	if ((shm_id = shmget(key, ARR_SIZE, /* OR: 0*/ 0600)) == -1)
	{
		perror("shmget failed: ");
		exit(EXIT_FAILURE);
	}

	// inserting the shm in the address space of the process
	if ((shm_ptr = (int*)shmat(shm_id, NULL, 0)) == (int*)-1)
	{
		perror("shmat failed: ");
		exit(EXIT_FAILURE);
	}
}
// ----------------------------------------------------------------------------

// this function create primes and insert them into shared memory array
// when array is full it send signal to array owner
void create_primes(int manu_num)
{
	int prime,						// for prime number
		appear = 0,					// to count appearence of some prime
		count_new_primes = 0,		// to count new prime in array
		max_appear = 0;				// to keep max appearence in array

	while (1)
	{
		prime = get_prime();

		while (shm_ptr[LOCK_CELL] == 0)
			sleep(1);

		// when LOCK_CELL open
		shm_ptr[LOCK_CELL] = 0;
		if ((shm_ptr[1] + shm_ptr[2] + shm_ptr[3]) == ARR_SIZE - START_VALUE_CELL)
			end_manufacturer(count_new_primes, max_appear);

		shm_ptr[next_cell_index()] = prime;
		shm_ptr[LOCK_CELL] = 1;

		appear = count_prime_appear(prime);

		if (appear == 0)
			++count_new_primes;

		if (appear > max_appear)
			max_appear = appear;

		++shm_ptr[manu_num];
	}
}
// ----------------------------------------------------------------------------

// this function find the next free cell in array in the shared memory 
// and returns it
int next_cell_index()
{
	int index = 2;		// because 0 is for array owner and 4 is for lock cell

	return (index + shm_ptr[1] + shm_ptr[2] + shm_ptr[3]);
}
// ----------------------------------------------------------------------------

// this function count appear on some prime in shared memory array
int count_prime_appear(int prime)
{
	int i,					// for loop
		counter = 0;		// for counter

	for (i = START_VALUE_CELL; i < next_cell_index(); ++i)
		if (shm_ptr[i] == prime)
			++counter;

	return counter;
}
// ----------------------------------------------------------------------------

// this function is for end manufacturer prosses. it prints informations 
// send SIGTERM gisnal to array owner, update LOCK_CELL to 1 to let all 
// manufacturer end and end
void end_manufacturer(int new_primes, int max_appear)
{
	printf("The amount of new primes I added to the array: %d\n", new_primes);
	printf("The number with the highest frequency is: %d\n", max_appear);

	kill(shm_ptr[0], SIGTERM);
	shm_ptr[LOCK_CELL] = 1;

	exit(EXIT_SUCCESS);
}
// ----------------------------------------------------------------------------

// this function get prime num
int get_prime()
{
	int num;		// for prime num

	while (!is_prime(num = rand() % 999 + 2)) {}

	return num;
}
// ----------------------------------------------------------------------------

// this function gets number and return 1 if its a prime number and 0 if not
int is_prime(int number)
{
	int i;		    // for loop

	if (number < 2)
		return 0;

	for (i = 2; i * i < number; ++i)
	{
		if (number % i == 0)
			return 0;
	}
	return 1;
}
// ----------------------------------------------------------------------------

// this function validat that we get all parameter we exepted to get in argv
// argoment
void validat_arg(int argc, char* argv_0)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <host name>\n", argv_0);
		exit(EXIT_FAILURE);
	}
}
// ----------------------------------------------------------------------------

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
#define ARR_SIZE 1000
#define LOCK_CELL 4
#define ZERO 48
int shm_id;
int* shm_ptr;

// ----------------------------------------------------------------------------
void validat_arg(int argc, char* argv_0);
void start(int manu_num);
void open_shm(key_t* key);
void create_primes(int manu_num);
int next_cell_index();
int count_prime_appear(int prime);
void end_manufacturer(int new_primes, int max_appear);
int get_prime();
int is_prime(int number);
// ----------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	key_t key;


	validat_arg(argc, argv[0]);
	printf("past valid..\n");
	srand((unsigned int)(*argv[1]) - ZERO);
	open_shm(&key);
	printf("shm is open\n");
	start((int)(*argv[1]) - ZERO);
	printf("can start\n");
	create_primes((int)(*argv[1]) - ZERO);
	
	return (EXIT_SUCCESS);
}
// ----------------------------------------------------------------------------

void start(int manu_num)
{
	int can_start = 0;

	shm_ptr[manu_num] = 1;

	while (!can_start)
		if (shm_ptr[1] == 1 && shm_ptr[2] == 1 && shm_ptr[3] == 1)
			can_start = 1;

	shm_ptr[4] = 0;
}
// ----------------------------------------------------------------------------

void open_shm(key_t* key)
{
	// create THE SAME key for the shm as the producer	
	if (((*key) = ftok(".", '3')) == -1)
	{
		perror("ftok failed: ");
		exit(EXIT_FAILURE);
	}

	// get the id of the block of memory 
	// that was, hopefully, already created by the producer
	if ((shm_id = shmget((*key), ARR_SIZE, /* OR: 0*/ 0600)) == -1)
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

void create_primes(int manu_num)
{
	while (1)
	{
		int prime = get_prime(),
			appear = 0,
			count_new_primes = 0,
			max_appear = 0;

		while (shm_ptr[LOCK_CELL] == 0)
			sleep(1);

		shm_ptr[LOCK_CELL] = 1;
		if (shm_ptr[1] + shm_ptr[2] + shm_ptr[3] == ARR_SIZE - 5)
			end_manufacturer(count_new_primes, max_appear);

		shm_ptr[next_cell_index()] = prime;
		shm_ptr[LOCK_CELL] = 0;

		appear = count_prime_appear(prime);

		if (appear == 0)
			++count_new_primes;

		if (appear > max_appear)
			max_appear = appear;

		++shm_ptr[manu_num];
	}
}
// ----------------------------------------------------------------------------

int next_cell_index()
{
	int index = 2;		// because 0 is for array owner and 4 is for lock cell

	return index + shm_ptr[1] + shm_ptr[2] + shm_ptr[3];
}
// ----------------------------------------------------------------------------

int count_prime_appear(int prime)
{
	int i,
		counter = 0;

	for (i = 5; i < next_cell_index(); ++i)
		if (shm_ptr[i] == prime)
			++counter;

	return counter;
}
// ----------------------------------------------------------------------------

void end_manufacturer(int new_primes, int max_appear)
{
	printf("The amount of new primes I added to the array: %d\n", new_primes);
	printf("The number with the highest frequency is: %d\n", max_appear);

	kill(shm_ptr[0], SIGINT);

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

void validat_arg(int argc, char* argv_0)
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: %s <host name>\n", argv_0);
		exit(EXIT_FAILURE);
	}
}
// ----------------------------------------------------------------------------

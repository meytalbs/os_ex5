/* Array owner
==============================================================================
Written by : Meytal Abrahamian  login : meytalben  id : 211369939
			 Tomer Akrish               tomerak         315224295
==============================================================================
This program defines in a shared memory an array of ARR-SIZE integers
In the first cell the program enters its process number
For the next three cells the program inserts the value zero
And into the fifth cell used as a 'lock' the program inserts the value 1
The rest of the cells the program resets.
After setting up the array the program goes to sleep until you get a signal
When the program receives a signal it wakes up, shows how many different 
values there are in the array the minimum value and the maximum value.
Finally the program free the shared memory and finishes
*/

//-----------------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

//-----------------------------------------------------------------------------
#define ARR_SIZE 1000		// size of main program array 
#define NUM_OF_MANU 3		// num of manufacturers
#define LOCK_CELL 4			// the cell designed to lock the shared memory
#define START_VALUE_CELL 5	// the cell that from it we start primes array
int shm_id;					// shared memory id
int new_primes = 0;			// for new primes in array
int min_prime = 0;			// for minimum prime in array
int max_prime = 0;			// for  maximum prime in array 
int* shm_ptr;				// for pointer to shared memory  

//-----------------------------------------------------------------------------
void signal_handler(int sig_num);
void open_shm();
void initializ_shm();
void map_array();
int is_in_arr(int arr[], int num);
//-----------------------------------------------------------------------------

//--------------------------------main-----------------------------------------
int main() 
{
	signal(SIGTERM, signal_handler);
	signal(SIGINT, signal_handler);			// to make sure we free the memory

	open_shm();	
	initializ_shm();

	pause();								// pause program until get signal

	return (EXIT_SUCCESS);
}
//-----------------------------------------------------------------------------

// this function 'open' a shared memory. it gets key, id and connect this 
// prosses to shared memory
void open_shm()
{
	key_t key;    // key for shm

	// create a key for the shm	
	if ((key = ftok(".", '5')) == -1)
	{
		perror("ftok failed: ");
		exit(EXIT_FAILURE);
	}

	// create a id for the shm
	if ((shm_id = shmget(key, ARR_SIZE,
		IPC_CREAT | IPC_EXCL | 0600)) == -1)
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
//-----------------------------------------------------------------------------

// this function initializ the array in shared memory
void initializ_shm()
{
	int i;			// for loops

	shm_ptr[0] = getpid();				// insert to first cell prosses id
	
	for (i = 1; i < NUM_OF_MANU; ++i)
		shm_ptr[i] = 0;					// insert to each manu cell - 0

	shm_ptr[LOCK_CELL] = 1;						// to lock the shm

	// all the rest - to 0
	for (i = START_VALUE_CELL; i < ARR_SIZE - START_VALUE_CELL; i++)
		shm_ptr[i] = 0;
}
//-----------------------------------------------------------------------------

// this function is the function that handle with signals. It prints all the 
// required information, free the memory and exit
void signal_handler(int sig_num)
{
	map_array();
	printf("There is %d new primes in array\n", new_primes); 
	printf("Min prime in array: %d\n", min_prime);
	printf("Max prime in array: %d\n", max_prime);
	
	// free memory
	if (shmctl(shm_id, IPC_RMID, NULL) == -1)
	{
		perror("shmctl IPC_RMID failed: ");
		exit(EXIT_FAILURE);
	}
	
	exit(EXIT_SUCCESS);
}
//-----------------------------------------------------------------------------

// this function map the array in shared memory and find the max primem, min 
// prime and new prime in array
void map_array()
{
	int i,						// for loops	
		arr[ARR_SIZE - START_VALUE_CELL];		// array that hold all new prime in main array
	
	for (i = 0; i < ARR_SIZE - START_VALUE_CELL; ++i)
		arr[i] = 0;

	for (i = START_VALUE_CELL; i < ARR_SIZE - START_VALUE_CELL; i++)
	{
		if (shm_ptr[i] > max_prime || max_prime == 0)
			max_prime = shm_ptr[i];
		if (shm_ptr[i] < min_prime || min_prime == 0)
			min_prime = shm_ptr[i];
		if (!is_in_arr(arr, shm_ptr[i]))
			++new_primes;
	}	
}
//-----------------------------------------------------------------------------

// This function checks if the number passed to it is within the array passed 
// to it. If so, the function returns 1 otherwise it returns 0 and add the 
// number to the array
int is_in_arr(int arr[], int num)
{
	int i;			// for loop

	for (i = 0; i < ARR_SIZE - START_VALUE_CELL && arr[i] != 0; ++i)
		if (arr[i] == num)
			return 1;

	arr[i] = num;
	return 0;
}
//-----------------------------------------------------------------------------

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
#include <stdlib.h> // for exit()
#include <string.h> // for strcpy(), strcmp()
#include <unistd.h> // for sleep()
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

//-----------------------------------------------------------------------------
#define ARR_SIZE 1000
#define SUM_OF_MANU 3
int shm_id;
int new_primes = 0;
int min_prime = 0;
int max_prime = 0;
int* shm_ptr;

//-----------------------------------------------------------------------------
void signal_handler(int sig_num);
void open_shm();
void initializ_shm();
void open_lock_to_manu();
void map_array();
//-----------------------------------------------------------------------------

int main() 
{
	signal(SIGINT, signal_handler);
	open_shm();	
	printf("shm is open\n");
	initializ_shm();
	printf("initializ shm\n");
	open_lock_to_manu();

	while (1) {} // todo - it will be better with sleep()	

	return (EXIT_SUCCESS);
}
//-----------------------------------------------------------------------------

void open_lock_to_manu()
{
	int all_manu_connected = 0;

	while (!all_manu_connected)
	{
		if (shm_ptr[1] == 1 && shm_ptr[2] == 1 && shm_ptr[3] == 1)
		{
			shm_ptr[4] = 1;
			all_manu_connected = 1;
		}
	}

	printf("opend");
}
//-----------------------------------------------------------------------------

void open_shm()
{
	key_t key;    // key for shm

	// create a key for the shm	
	if ((key = ftok(".", '3')) == -1)
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

void initializ_shm()
{
	int i;

	shm_ptr[0] = getpid();				// insert to first cell prosses id
	for (i = 1; i < SUM_OF_MANU; ++i)
		shm_ptr[i] = 0;					// insert to each manu cell - 0

	shm_ptr[4] = 1;						// to lock the shm

	// all the rest - to 0
	for (i = 5; i < ARR_SIZE - 5; i++)
		shm_ptr[i] = 0;
}
//-----------------------------------------------------------------------------

void signal_handler(int sig_num)
{
	map_array();
	printf("There is %d new primes in array\n", new_primes); // todo - to think how todo it
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

void map_array()
{
	int i;

	for (i = 5; i < ARR_SIZE - 5; i++)
	{
		if (shm_ptr[i] > max_prime || max_prime == 0)
			max_prime = shm_ptr[i];
		if (shm_ptr[i] < min_prime || min_prime == 0)
			min_prime = shm_ptr[i];
	}	
}
//-----------------------------------------------------------------------------

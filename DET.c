/* Include statements for shared memory and gettimeofday timer */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/time.h>

/* Include statement for header file struct */
#include "DET.h"

/* For elapsed time calculation */
#define MICRO_SEC_IN_SEC 1000000

/* Delegation of child process work; childNum represents the child process calling the function; input pointer points to the input matrix array */
void calcDet(int childNum, int *input) {
	/* Initializing shared memory */	
	struct shm_arrays *shared_stuff;
	void *shared_memory = (void *)0;	
	int shmid;	
	
	/* Creating shared memory ID and attaching shared memory segment to child process */
	shmid = shmget((key_t)1234, sizeof(struct shm_arrays), 0666 | IPC_CREAT);
	if (shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}
	shared_memory = shmat(shmid, (void *)0, 0);
	if (shared_memory == (void *)-1) {
		fprintf(stderr, "shmat failed\n");
		exit(EXIT_FAILURE);
	}	
	shared_stuff = (struct shm_arrays *)shared_memory;

	/* Printing which element the child process is working with */
	printf("Child Process: working with element %d of D\n", (childNum + 1));
	
	/* Filtering calculation method by child process that called calcDev */
	int i, max;
	if(childNum == 0) {
		/* Calculate the element stored in column J of the matrix and finding the maximum 			value for the column; increasing the process counter */ 		
		shared_stuff->det += input[0] * ((input[4] * input[8]) - (input[5] * input[7]));
		max = input[0];		
		for(i = 0; i < 3; i++) {
			if(input[i] > max) {
				max = input[i];
			}
		}
		shared_stuff->largest[0] = max;	
		shared_stuff->counter++;
	} else if(childNum == 1) {
		/* Calculate the element stored in column J of the matrix and finding the maximum 			value for the column; increasing the process counter */ 		
		shared_stuff->det += input[1] * ((input[5] * input[6]) - (input[3] * input[8]));		
		max = input[3];
		for(i = 3; i < 6; i++) {
			if(input[i] > max) {
				max = input[i];
			}
		}
		shared_stuff->largest[1] = max;
		shared_stuff->counter++;
	} else {
		/* Calculate the element stored in column J of the matrix and finding the maximum 			value for the column; increasing the process counter */ 		
		shared_stuff->det += input[2] * ((input[3] * input[7]) - (input[4] * input[6]));
		max = input[6];		
		for(i = 6; i < 9; i++) {
			if(input[i] > max) {
				max = input[i];
			}
		}
		shared_stuff->largest[0] = max;
		shared_stuff->counter++;
	}
		
	/* Using process counter to get the last child process be inside the calcDet 
	function to print the calculated determinant of the matrix and print the largest 
	integer in the matrix */
	if(shared_stuff->counter == 3) {	
		printf("\nDeterminant (R) is: %d\n", shared_stuff->det);
		int i, max;
		max = shared_stuff->largest[0];		
		for(i = 0; i < 3; i++) {
			if(shared_stuff->largest[i] > max) {
				max = shared_stuff->largest[i];
			}
		}
		printf("Largest Integer in M: %d\n", max);
	}

	/* Detaching shared memory from current child process */
	if (shmdt(shared_memory) == -1) {
		fprintf(stderr, "shmdt failed\n");
		exit(EXIT_FAILURE);
	}
}

int main() {
	/* Mandatory Test Data Input Matrix */
	int input[9] = {20, 20, 50, 10, 6, 70, 40, 3, 2};

	/* PID variable creation */
	pid_t pid1, pid2, pid3;
	
	/* Initializing shared memory */
	struct shm_arrays *shared_stuff;
	void *shared_memory = (void *)0;
	int shmid;
	
	/* Initializing gettimeofday() calculator and starting the counter */
	struct timeval start, end;
	gettimeofday(&start, NULL);	

	/* Child Processes 1, 2, and 3 calling the calcDet function given their child number and 		input matrix */
	pid1 = fork();
	switch(pid1) {
		case -1:
			perror("fork #1 failed");
			exit(1);
		case 0:
			calcDet(0, input);
			exit(0);
	}
	pid2 = fork();
	switch(pid2) {
		case -1:
			perror("fork #2 failed");
			exit(1);
		case 0:
			calcDet(1, input);
			exit(0);
	}
	pid3 = fork();
	switch(pid3) {
		case -1:
			perror("fork #3 failed");
			exit(1);
		case 0:
			calcDet(2, input);
			exit(0);
	}
	
	/* Creating shared memory ID and attaching shared memory segment to child process */
	shmid = shmget((key_t)1234, sizeof(struct shm_arrays), 0666 | IPC_CREAT);
	if (shmid == -1) {
		fprintf(stderr, "shmget failed\n");
		exit(EXIT_FAILURE);
	}

	shared_memory = shmat(shmid, (void *)0, 0);
	if(shared_memory == (void *)-1) {
		fprintf(stderr, "parent shmat failed\n");
		exit(EXIT_FAILURE);
	}
	
	/* Waiting for child process 1, 2, and 3 to complete */
	waitpid(pid1);
	waitpid(pid2);
	waitpid(pid3);

	/* Ending the elapsed time counter */
	gettimeofday(&end, NULL);

	/* Detaching shared memory from parent process and destroy shared memory segment */
	shared_stuff = (struct shm_arrays *)shared_memory;
	if (shmdt(shared_memory) == -1) {
		fprintf(stderr, "shmdt failed\n");		
		exit(EXIT_FAILURE);
	}

	if (shmctl(shmid, IPC_RMID, 0) == -1) {
		fprintf(stderr, "shmctl(IPC_RMID) failed\n");
		exit(EXIT_FAILURE);
	}

	/* Print elapsed time of performing all the operations */
	printf("Elapsed Time: %ld micro sec\n", ((end.tv_sec * MICRO_SEC_IN_SEC + end.tv_usec) - (start.tv_sec * MICRO_SEC_IN_SEC + start.tv_usec)));

	exit(0);
}

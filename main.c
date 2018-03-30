/* Best fit algorithm simulation
	Ashish Adhikari
	run as : ./sim 9   //for 9 processes

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#define MEMORY_SIZE (int)pow(2,22)	//4MB
#define BLOCK_SIZE (int)pow(2,10)	//1 KB
//stop simulation these many allocations ??
#define REQUIRED_ALLOCATIONS (int)pow(2,5)
//for easy use of max index of memory
#define MAX_INDEX MEMORY_SIZE/BLOCK_SIZE
//generate N processes of rand()%91+10 KB 
void generate_procs(int N);
//allocate memory to each process free_memory() 
void allocate_proc(int batch, int size);
//start index of suitable hole
int find_best_index(int size);
//deallocate N oldest processes
void free_memory(void);
//return size of available hole
void update_hole(void);
int total_available_hole(void);

/*memory arranged as [1,-1,4,2,-1]
	edge cases:
		- when process batch is 4 and memory size is 2,
			we cannot allocate the last index as -1
		- when batch is 0, we must check that value
			on next index is greater than 9 to make sure 
			we don't skip it
		- when batch size is 0, similar problem arises 
			so check for next index here as well.
*/
int *MEMORY;	
int num_allocated=0;
int batch = 0;  //process batch number
int min_batch = 0; //oldest process batch number 
int *hole;			//indexed [size1,size2,....] of holes

int main(int argc, char *argv[]){
	//invalid input handling
	if (argc==1 || argc > 2){
		printf("Invalid number of parameters!!\n");
		exit(-1);
	}
	//not handled when string is provided ??
	int N = atoi(argv[1]);
	int i = 0, j = 0;	
	//initialise memory 
	MEMORY = (int*)malloc((sizeof(int)*MAX_INDEX));
	for (i = 0; i < MAX_INDEX; i++)
		MEMORY[i]=0;
	//random number seed
	struct timeval seed;
	gettimeofday(&seed,NULL);
	srand(seed.tv_usec);
	//simulate until crtiteria met 
	while(num_allocated<REQUIRED_ALLOCATIONS)
		generate_procs(N);
	return 0;
} 
/*generates N processes and and free_memory() 
	until enough memory is available*/ 
void generate_procs(int N){
	int total_size = 0;
	//batch is global batch
	int batch_num = batch;
	//[pid][col]
	int process[N][1];
	int i = 0;
	for (i = 0; i < N; i++){
		int size = rand()%91+10;
		total_size+=size;
		process[i][0] = size;
	}
	while (total_available_hole()<total_size)
			free_memory();

	//allocate memory for each process 
	for (i = 0; i < N; i++)
		allocate_proc(batch_num,process[i][0]);
	//make sure batch is consistent with code
	batch++;

}

void allocate_proc(int batch_num, int size){
	int start = find_best_index(size);
	//refer to comment at top to understand memory layout
	int i = 0;
	for (i = start+2; i <start+size-2; i++)
		MEMORY[i] = 1; //fill used up memory blocks with 1
	MEMORY[start] = batch_num;
	MEMORY[start+1]= size;
	MEMORY[start+size-1] = -1;
	num_allocated++; 
}
//free the oldest batch processes
void free_memory(){
	int i = 0, j = 0;
	for (i = 0; i <MAX_INDEX; i++){
		/*
			Memory looks something like:
		[...,1,-1,4,12,1,1,1,1,1,1,1,1,1,-1,0,0,0,3,61,1,...]
		
		*/
		if(MEMORY[i]==min_batch && MEMORY[i+1]>9){
			for (j = i; j < i+MEMORY[i+1]-1; j++)
				MEMORY[j] = 0;
		}
	}
	min_batch++;
}

//return start index acc to best fit algorithm 
int find_best_index(int size){
	int allocated = 0; 
	int cur_index = 0;
	int cur_min = pow(2,30);
	int i = 0;
	while (!allocated){
		for (i = 0; i <MAX_INDEX; i++){
			if(hole[i]>size && hole[i]<cur_min){
				cur_min = hole[i];
				cur_index = i;	
				allocated = 1;
			}


		}
		//process doesn't fit any available hole
		if(!allocated)
			free_memory();	
	}
	return cur_index;
}
/*
void update_hole(void){
	int i = 0, j = 0;
	int size = 0;
	for (int i = 0; i < MAX_INDEX; i++){
		if(MEMORY[i]==0 && MEMORY[i+1]==0){
			j = i+1;
			while(MEMORY[j]==0 && j<MAX_INDEX){
				size++;
				j++;

			}
			hole[i] = size;
		}	
	}
}
*/
int total_available_hole(void){
	int hole_size = 0;
	for (int i = 0; i < MAX_INDEX-1;i++){
		if (MEMORY[i]==0 && MEMORY[i+1]==0){
			int j = i+1;
			while(MEMORY[j]==0){
				hole_size+=1;
				j++;
			}
			i = j;
		}
	}
	return hole_size;
}



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
#define BLOCK_SIZE (int)pow(2,22)	//1 KB
//stop simulation these many allocations ??
#define REQUIRED_ALLOCATIONS (int)pow(2,10)
//for easy use of max index of memory
#define NUM_MEM_BLOCKS MEMORY_SIZE/BLOCK_SIZE

typedef struct {
	int start;
	int size;
}hole_desc;

//generate N processes of rand()%91+10 KB 
void generate_procs(int N);
//allocate memory to each process free_memory() 
void allocate_proc(int batch, int size);
//start index of suitable hole
int *find_best_index(int size);
//deallocate N oldest processes
void free_memory(void);
//return size of available hole
int total_available_hole(void);
hole_desc* get_cur_holes(void);

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
int num_holes = 0;

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
	MEMORY = (int*)malloc((sizeof(int)*NUM_MEM_BLOCKS));
	for (i = 0; i < NUM_MEM_BLOCKS; i++)
		MEMORY[i]=0;
	//random number seed
	struct timeval seed;
	gettimeofday(&seed,NULL);
	srand(seed.tv_usec);
	//simulate until crtiteria met 
	while(num_allocated<REQUIRED_ALLOCATIONS){
		generate_procs(N);
	}
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
	int size;
	for (i = 0; i < N; i++){
		size = rand()%91+10;
		printf("%d Size Generated\n",size);
		total_size+=size;
		process[i][0] = size;
	}
	int total_hole = (int)total_available_hole();

	printf("total available hole %d , total_size  %d\n",total_hole,total_size );
	while (total_hole<total_size){
			printf("%d before..\n",total_hole);
			free_memory();
			total_hole = total_available_hole();
			printf("%d after.....\n\n",total_hole);
	}
	//allocate memory for each process 
	for (i = 0; i < N; i++){
		printf("requested_size: %d\n", process[i][0]);
		allocate_proc(batch_num,process[i][0]);
		
	}
	//make sure batch is consistent with code
	batch++;

}

void allocate_proc(int batch_num, int size){
	printf("Received size: %d\n",size);
	int *index_Size = (int*)(find_best_index(size));
	int start = index_Size[0];
	int hole_size = index_Size[1];
	//free(index_Size);
	printf("best Start index: %d\n",start);
	printf("space in hole: %d\n",hole_size);
	printf("total_available_hole: %d\n",total_available_hole());
	
	if(hole_size<size){
		printf("Hole not big enough\n");
		//print the Memory
		for (int i = 0; i < NUM_MEM_BLOCKS; i++)
		{
			printf("%d, ",MEMORY[i]);
			if(i==100)
				printf("\n");
		}
		exit(-1);
	}
	//refer to comment at top to understand memory layout
	int i = 0;
	for (i = start; i < start+size-1; i++)
		MEMORY[i] = 1; //set used space to 1
	//last index has -1 for last memory index
		//[batch_num,n,[start+2,start+size]]
	MEMORY[start] = batch_num;
	MEMORY[start+1]= size;
	num_allocated++; 
	free(index_Size);
}
//free the oldest batch processes
void free_memory(void){
	int i = 0, j = 0;
	int found = 0;
	for (i = 0; i < NUM_MEM_BLOCKS-1; i++){
		/*
			MEMORY looks something like:
		[...,1,1,4,12,1,1,1,1,1,1,1,1,1,1,0,0,0,3,61,1,...]
		
	
		*/
		if(MEMORY[i]==min_batch && MEMORY[i+1]>9){
			printf("Not dead yet!\n");
			for (j = i; j < i+MEMORY[i+1]; j++)
				MEMORY[j] = 0;
			i = j;
			found = 1;
			
		}
	}
	if(found)
		min_batch++;
	if(!found){
		printf("Something is pretty fucked up!\n");
		exit(-1);
	}
}

//return start index acc to best fit algorithm 
int *find_best_index(int size){
	int min_hole_size= 1<<20;
	int cur_hole_size;
	int cur_index = 0;
	int l=0,r;
	int *answer = (int*)malloc(sizeof(int));  //[index,size]
	for (l = 0; l < NUM_MEM_BLOCKS-1 && r< NUM_MEM_BLOCKS; l++){
		//[**,.,.,.,]
		r = l+1;
		//reset cur_hole
		cur_hole_size = 0;
		//move right index and get best suited hole size in MEMORY
		while(MEMORY[l]==0 && MEMORY[r]==0 && r<NUM_MEM_BLOCKS){
			r++;
			cur_hole_size++;
		}
		if (cur_hole_size<min_hole_size && cur_hole_size>size){
			min_hole_size = cur_hole_size;
			cur_index = l;	
		}
		l = r;
	}
	answer[0] = cur_index;
	answer[1] = min_hole_size;
	printf("min_hole_size: %d\n",answer[1]);
	return answer;
}
	/*
	int allocated = 0; 
	int cur_index = 0;
	//init max mem to high
	int cur_min = pow(2,20);
	int i = 0;
	int cur_hole;
	hole_desc *holes;
	while (!allocated){
		//struct of holes  holes.size, holes.start
		holes = get_cur_holes();
		//search holes for appropriate best fit
		for (i = 0; i<num_holes;i++){
			cur_hole = holes[i].size;
			if(cur_hole<cur_min && cur_hole >size){
				allocated = 1;
				cur_min = cur_hole;
				cur_index = holes[i].start;;
			}
		}
		//if no best index found:
		if(!allocated)
			free_memory();			
		//free holes
		for (i = 0; i < num_holes; i++)
				free(&holes[i]);			
		num_holes = 0;
	
	}		
	
	//after allocated
	return cur_index;
	}
	*/



/*
void update_hole(void){
	int i = 0, j = 0;
	int size = 0;
	for (int i = 0; i < NUM_MEM_BLOCKS; i++){
		if(MEMORY[i]==0 && MEMORY[i+1]==0){
			j = i+1;
			while(MEMORY[j]==0 && j<NUM_MEM_BLOCKS){
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
	int r = 0;
	printf("At least got here\n");
	int found = 0;
	for (int l = 0; l < NUM_MEM_BLOCKS-1;l++){
		r = l+1;
		printf("Eh y u no come here\n");	
		//starting of a hole
		if (MEMORY[l]==0 && MEMORY[r]==0){
			while(MEMORY[r]==0 && r<NUM_MEM_BLOCKS){
				hole_size++;
				r++;
			}
			l = r; //for loop adds one every time
			found=1;
		}
	}
	printf("Seriously C???\n");
	if (found)
		return hole_size;
}
/*
hole_desc* get_cur_holes(void){
	hole_desc *holes = (hole_desc*)malloc(sizeof(hole_desc)*NUM_MEM_BLOCKS);
	int start;
	int hole_ix = 0;
	int j;
	for (int i = 0; i < NUM_MEM_BLOCKS-1; i++){
		if(MEMORY[i]==0&&MEMORY[i+1]==0){
			num_holes++;
			start = i;
			j = i+1;
			while(MEMORY[j]==0 && j<NUM_MEM_BLOCKS)
				j++;
			i = j;
			holes[hole_ix] = *((hole_desc*)malloc(sizeof(hole_desc)));
			holes[hole_ix].start = start;
			holes[hole_ix].size = i - start +1;
			hole_ix++;
		}
	}
	return holes;
}

*/

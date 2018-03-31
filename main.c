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
#define REQUIRED_ALLOCATIONS (int)pow(2,15)
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
//hole_desc* get_cur_holes(void);

int *MEMORY;	
int num_allocated=0;
int batch = 0;  //process batch number
int min_batch = 0; //oldest process batch number 
int num_holes = 0;
int *holes; //for recording hole size for each batch

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
	//array of total hole for each batch
	//batch will always be less than REQUIRED_ALLOCATIONS/N+N
	holes = (int*)malloc(sizeof(int)*(REQUIRED_ALLOCATIONS/N+N));
	for(i = 0; i<REQUIRED_ALLOCATIONS/N+N;i++)
		holes[i]=0;

	//simulate until crtiteria met 
	while(num_allocated<REQUIRED_ALLOCATIONS){
		generate_procs(N);
		//record hole size
		//batch is updated by generate_process()
		holes[batch-1] = total_available_hole();
		
	}

	//analysis 
	int sum = 0;

	int mean, sd;
	//calculate mean
	for (i = 0; i <=batch; i++){
		sum+=holes[i];
	}
	mean = sum/(batch+1);

	long int cur_dif=0;
	//calculate standard deviation
	for (i = 0; i <= batch; i++)
		cur_dif += (long int)pow((holes[i]-mean),2);

	sd = (int)pow((double)sum/N,0.5);
	printf("Average holes size of %d batch size is %d\n",N,mean);
	printf("standard deviation is %d \n",sd);
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
		total_size+=size;
		process[i][0] = size;
	}
	int total_hole = (int)total_available_hole();
	while (total_hole<total_size){
			free_memory();
			total_hole = total_available_hole();
	}
	//allocate memory for each process 
	for (i = 0; i < N; i++){
		allocate_proc(batch_num,process[i][0]);
		
	}
	//make sure batch is consistent with code
	batch++;

}

void allocate_proc(int batch_num, int size){
	int *index_Size = (int*)(find_best_index(size));
	int start = index_Size[0];
	int hole_size = index_Size[1];
	//free(index_Size);
	if(hole_size<size){
		printf("Hole not big enough\n");
		exit(-1);
	}
	//refer to comment at top to understand memory layout
	int i = 0;
	for (i = start+2; i < start+size-1; i++)
		MEMORY[i] = 1; //set used space to 1
	MEMORY[start] = batch_num;
	MEMORY[start+1]= size;
	MEMORY[start+size-1] = -1;	
	num_allocated++; 
}
//free the oldest batch processes
void free_memory(void){
	int l = 0, r = 1;
	int found = 0;
	int i;
	for (l = 0; l < NUM_MEM_BLOCKS-1; l++){
		/*
			MEMORY looks something like:
		[...,1,1,4,12,1,1,1,1,1,1,1,1,1,1,0,0,0,3,61,1,...]
		
	
		*/
		r = l+1;
		//assuming all process sizes are greater than 9
		if(MEMORY[l]==min_batch && MEMORY[r]>9){
			i = l;
			while(MEMORY[i]!=-1){
				MEMORY[i] = 0;
				i++;
			}
			MEMORY[i] = 0;
			found = 1;
		}
	}
	if(found)
		min_batch++;
	else{
		printf("Sorry, not sufficient memory to fit this batch's requirement allocation failed! Current min batch is%d cur batch is %d\n",min_batch,batch);
		printf("Look at the beautiful MEMORY for now ;)\n");
		for (int i = 0; i < NUM_MEM_BLOCKS; i++)
		{
			printf("%d  ",MEMORY[i]);
		}
		exit(-1);
	}
}

//return start index acc to best fit algorithm 
int *find_best_index(int size){
	int min_hole_size= 1<<20;
	int cur_hole_size = 0;
	int best_index = 0;
	int l=0,r=1;
	int *answer = (int*)malloc(sizeof(int)*2);  //[index,size]
	int allocated = 0;

	while(!allocated && min_batch< batch){
		for (l = 0; l < NUM_MEM_BLOCKS-1 && r< NUM_MEM_BLOCKS; l++){
			//[**,.,.,.,]
			r = l+1;
			//reset cur_holes
			cur_hole_size = 0;
			//move right index and get best suited hole size in MEMORY
			if(MEMORY[l]==0 && MEMORY[r]==0){
				r++;
				while(MEMORY[r]==0 && r<NUM_MEM_BLOCKS)
					r++;
				cur_hole_size = r-l+1;
				if (cur_hole_size < min_hole_size && cur_hole_size > size){
					min_hole_size = cur_hole_size;
					best_index = l;	
					if (total_available_hole()>size)
						allocated=1;
				}
				l = r-1;
			}
		}
		//if no best hole found,
		if(!allocated){
			free_memory();
		}
	}
	answer[0] = best_index;
	answer[1] = min_hole_size;
	return answer;
}

int total_available_hole(void){
	int hole_size = 0;
	int l, r = 0;
	for (l = 0; l < NUM_MEM_BLOCKS-1;l++){
		r = l+1;
		//starting of a hole
		if (MEMORY[l]==0 && MEMORY[r]==0){
			while(MEMORY[r]==0 && r<NUM_MEM_BLOCKS){
				hole_size++;
				r++;
			} 
			l = r; //for loop adds one every time
		}
	}
	return hole_size;
}

/* Best fit algorithm simulation
	Ashish Adhikari
	run as : ./sim 9   //for 9 processes

*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
int main(){
	struct timeval seed;
	gettimeofday(&seed,NULL);
	srand(seed.tv_usec);
	for (int i = 0; i < 20; ++i)
	{
		printf("%d\n",rand());
	}
	return 0;
} 
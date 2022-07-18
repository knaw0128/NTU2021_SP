#include "threadtools.h"

// Please complete this three functions. You may refer to the macro function defined in "threadtools.h"

// Mountain Climbing
// You are required to solve this function via iterative method, instead of recursion.
void MountainClimbing(int thread_id, int number){
	/* Please fill this code section. */
	ThreadInit(thread_id,number);
	Current->N=number;
	Current->i=1;
	Current->x=1;
	Current->y=1;
	if(setjmp(Current->Environment)==0)	
		longjmp(MAIN,1);

	if(Current->N==0||Current->N==1){
		sleep(1);
		printf("Mountain Climbing: 1\n");
		ThreadYield();
	}

	for(;Current->i<Current->N;Current->i++){
		sleep(1);
		Current->z=Current->x+Current->y;
		Current->x=Current->y;
		Current->y=Current->z;
		printf("Mountain Climbing: %d\n", Current->z);
		ThreadYield();
	}
	ThreadExit();
}

// Reduce Integer
// You are required to solve this function via iterative method, instead of recursion.
void ReduceInteger(int thread_id, int number){
	/* Please fill this code section. */
	ThreadInit(thread_id,number);
	Current->N=number;
	Current->i=0;

	if(setjmp(Current->Environment)==0)	
		longjmp(MAIN,1);

	if(Current->N==1){
		sleep(1);
		printf("Reduce Integer: 0\n");
		ThreadYield();
	}
		
	while (Current->N>1){
		sleep(1);
		if(Current->N%2==0){
			Current->N/=2;
			Current->i++;
		}
		else{
			Current->x=Current->N+1;
			Current->y=Current->N-1;
			Current->w=0;
			Current->z=0;
			while(Current->x%2==0){
				Current->x/=2;
				Current->w++;
			}
			while(Current->y%2==0){
				Current->y/=2;
				Current->z++;
			}
			if(Current->x<Current->y){
				Current->i+=Current->w;
				Current->N=Current->x;
			}
			else{
				Current->i+=Current->z;
				Current->N=Current->y;
			}
		}
		printf("Reduce Integer: %d\n", Current->i);
		ThreadYield();
	}

	ThreadExit();
}

// Operation Count
// You are required to solve this function via iterative method, instead of recursion.
void OperationCount(int thread_id, int number){
	/* Please fill this code section. */
	ThreadInit(thread_id,number);
	if(setjmp(Current->Environment)==0)	
		longjmp(MAIN,1);
	sleep(1);
	printf("Operation Count: %d\n",number*number/4);
	ThreadYield();
	ThreadExit();
}

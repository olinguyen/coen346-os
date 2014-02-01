// This program recursively make N threads based on the dice number rolled
// The lowest and highest number out of all the dice rolls are kept
// The depth of recursion is not more than 3
// Originally written for Windows by 346 TA Parsa P.

#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#define max(a,b) ((a) > (b) ? a : b)
#define min(a,b) ((a) < (b) ? a : b)

// A structure to keep the needed information of each thread
typedef struct
{
	long randomN;
	int level;
	bool run;
	int maxOfVals;
	int minOfVals;
} ThreadInfo;


// The start address (function) of the threads
void* ChildWork(void* a) {

	ThreadInfo* info = (ThreadInfo*)a;
	pthread_t threadid;
  int i;

	// Generate random value n
	srand(time(NULL));
	int n=rand()%6+1;


	// initialize the thread info with n value
	info->randomN=n;
	info->maxOfVals=n;
	info->minOfVals=n;


	// the depth of recursion should not be more than 3
	if(info->level > 3)
	{
		info->run = false;
    	pthread_exit(0); /* exit */
	}

	// Create n threads and run them
	ThreadInfo* childInfo = malloc(n*sizeof(ThreadInfo));
	for(i= 0; i < n; i++)
	{
		childInfo[i].level = info->level + 1;
		childInfo[i].run = true;
		pthread_create(&threadid, NULL,  ChildWork,  (void *)&childInfo[i]);
	}


	// checks if any child threads are working
	bool anyRun = true;
	while(anyRun)
	{
		anyRun = false;
		for(i= 0; i < n; i++)
		{
			anyRun = anyRun || childInfo[i].run;
		}
	}

	// once all child threads are done, we find their max and min value
	int maximum=1, minimum=6;
	for( i=0;i<n;i++)
	{
		printf("%d\n",childInfo[i].maxOfVals);


		if(childInfo[i].maxOfVals>=maximum)
			maximum=childInfo[i].maxOfVals;

		if(childInfo[i].minOfVals< minimum)
			minimum=childInfo[i].minOfVals;

	}

	info->maxOfVals=maximum;
	info->minOfVals=minimum;


	// we set the info->run value to false, so that the parrent thread of this thread will know that it is done
	info->run = false;
    pthread_exit(0); /* exit */
}

int main()
{
	ThreadInfo info;
	pthread_t threadid;

	srand(time(NULL));
	int n=rand()%6+1;

	printf("%d\n",n);

	// initializing thread info
	info.randomN=n;
	info.maxOfVals=n;
	info.minOfVals=n;
	info.level = 1;
	info.run = true;

	pthread_create(&threadid, NULL,  ChildWork,  (void *)&info);

	while(info.run);

	info.maxOfVals= max(info.randomN,info.maxOfVals);
	info.minOfVals= min(info.randomN,info.minOfVals);

	printf("Max is: %d and Min is: %d\n", info.maxOfVals, info.minOfVals);

	return 0;
}

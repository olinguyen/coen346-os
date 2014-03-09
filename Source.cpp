#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <time.h>
#include <queue>
#include <fstream>

using namespace std;

ofstream out;
const int TIME_SLICE = 1000;
bool schedulerIsRunning = true;

struct ProcessInfo
{
	ProcessInfo()
	{
		readyTime=0;
		isStarted = false;
		hasCPU = false;
		isFinished = false;
	}
	int id;
	//when thread should be started
	int readyTime;
	//How long thread should work
	int duration;
	//Scheduler set this boolean to simulate giving time shared to each thread
	bool hasCPU;
	//Indicate if the thread is started
	bool isStarted;
	//Indicate if the thread is finished
	bool isFinished;
};

void log(int processId, string state)
{
	stringstream ss(stringstream::in | stringstream::out);
	int time = clock() / 1000;
	ss << "Time: " << time << ", Process " << processId << ": " << state << endl;
	out << ss.str();
	cout << ss.str();
}

void* Process(void* a)
{
	ProcessInfo* info = (ProcessInfo*)a;

	long executionDuration = 0;
	//Continue until total execution reaches the duration of the thread
	while(executionDuration < info->duration)
	{
		//Wait until get time slice from the scheduler
		while(!info->hasCPU){}

		//Calculate current cycle execution time
		long executionStart = clock();
		long cycleDuration = 0;
		//Continue the calculation until the time shared expires or thread finishes
		while(info->hasCPU && executionDuration + cycleDuration < info->duration){
			cycleDuration = clock() - executionStart;
		}
		executionDuration += cycleDuration;
	}

	//Set the thread as finished
	info->isFinished = true;
	return 0;
}

//Round-robin Scheduler
void* scheduler_RR(void* a)
{
	//Input thread infos
	queue<ProcessInfo*>* waitingThreads = (queue<ProcessInfo*>*)a;
  pthread_t threads[waitingThreads->size()];
  int i = 0;

	while(!waitingThreads->empty())
	{
		//Get first element of the queue
		ProcessInfo *curProcessInfo = waitingThreads->front();
		waitingThreads->pop();

		//Check if ready time has been reached
		if(curProcessInfo->readyTime > clock())
		{
			//push back this thread and go for next thread
			waitingThreads->push(curProcessInfo);
			continue;
		}

		//Check if the thread has already been started.
		if(!curProcessInfo->isStarted)
		{
			curProcessInfo->isStarted = true;

			//start thread
			log(curProcessInfo->id, "started");
       pthread_create(&threads[i], NULL, Process, (void*)&curProcessInfo);
       i++;
		}

		//Give time share to the current thread
		curProcessInfo->hasCPU = true;

		log(curProcessInfo->id, "resumed");

		long startTimeSlice = clock();
		while (!curProcessInfo->isFinished && clock() - startTimeSlice < TIME_SLICE){}

		//Take the time share back from the current thread
		curProcessInfo->hasCPU = false;

		log(curProcessInfo->id, "paused");

		//If the thread is no finished push it to the waiting queue
		if(!curProcessInfo->isFinished)
			waitingThreads->push(curProcessInfo);
		else
			log(curProcessInfo->id, "finished");

	}
	schedulerIsRunning = false;
	return 0;
}


//Read input thread infos and return them as a queue of pointers
queue<ProcessInfo*>* readProcessInfos(string fileName)
{
	ifstream in(fileName);

	queue<ProcessInfo*>* processInfos = new queue<ProcessInfo*>();
	int idTemp = 0;
	while(in.good())
	{
		//Create each processInfo instance
		ProcessInfo* info = new ProcessInfo();
		info->id = idTemp;
		//in >> info->readyTime;
		in >> info->duration;
		//info->readyTime *= 1000;
		info->duration *= 1000;

		//push new process in right place (make the queue ordered based on processes' ready time)
		bool isInserted = false;
		int size = processInfos->size();
		for(int i = 0; i < size; i++)
		{
			ProcessInfo* front = processInfos->front();
			if(front->duration < info->duration)
			{
				processInfos->pop();
				processInfos->push(front);
			}
			else
			{
				if(!isInserted)
				{
					processInfos->push(info);
					isInserted = true;
				}
				processInfos->pop();
				processInfos->push(front);
			}
		}
		if(!isInserted)
		{
			processInfos->push(info);
			isInserted = true;
		}

		idTemp++;
	}

	return processInfos;
}

void main()
{
	//output file
	out = ofstream("output.txt");

	//reading thread infos (start time and duration of each thread)
	queue<ProcessInfo*>* processInfos = readProcessInfos("input2.txt");

	//Start scheduler thread
  pthread_t threadid;
  pthread_create(threadid, NULL, scheduler_RR, processInfos);

	while(schedulerIsRunning){}

	getchar();
  return 0;
}





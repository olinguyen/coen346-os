#ifndef __VMM_H__
#define __VMM_H__
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <string>
#include <queue>
#include <deque>
#include <algorithm>
#include <fstream>

// Holds information of a thread for the scheduler
typedef struct {
  double quantum;        // quantum for a given run
  double remaining_time; // Execution time left
  int arrival_time;      // Time process is ready
  double waiting_time;   // Time spent in waiting queue
  double duration;       // Total time spent running so far
  int id;
  bool isReady;
  bool isFinished;
  double burst_time;     // total of time execution needed
} process_t;

void* run_process(void* a); // Thread function that will simulate a process
void init_flag(); // Initializes  mutex and condition variables
void set_thread_flag(int flag_value); // Gives CPU tp a specific thread
void start_rr(); // scheduling used by main(scheduler thread)
void print_queue(std::deque<process_t>);
void log(int processId, char* state);
void checkArrivalTime();

#endif

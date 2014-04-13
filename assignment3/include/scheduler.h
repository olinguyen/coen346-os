#ifndef __SCHEDULER_H__
#define __SCHEDULER_H__

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <queue>
#include <deque>
#include <algorithm>
#include <fstream>


// Holds information of a thread for the scheduler
typedef struct {
  double quantum;        // quantum for a given run
  double remaining_time; // Execution time left
  int arrival_time;      // Time process is ready
  double duration;       // Total time spent running so far
  int id;
  bool isReady;
  bool isFinished;
  double burst_time;     // total of time execution needed
} process_t;

typedef struct {
  int value;
  std::string variableId;
  std::string command;
} command_t;

void* run_process(void* a); // Thread function that will simulate a process
void init_flag(); // Initializes  mutex and condition variables
void set_thread_flag(int flag_value); // Gives CPU tp a specific thread
void start_rr(); // scheduling used by main(scheduler thread)
void print_queue(std::deque<process_t>);
void log(int processId, char* state);
void checkArrivalTime();
int read_processes(const char *filename);
std::vector<command_t> read_commands(const char* filename);

#endif

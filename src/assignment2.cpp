// TODO:
// Create thread on arrival time?
// What queue is used for processes that have not arrived yet?
// Oldest process first if same execution time
// Specify time in waiting queue

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

using namespace std;

FILE* output;

#define BILLION  1000000000L;

#define DEBUG 1

// Used to keep track of older threads and their count
int thread_count = 0;

// Used to indicate which PID should be running
int process_to_run;

int g_time = 1;

// Holds information of a thread for the scheduler
typedef struct {
  double quantum;
  // Execution time left
  double remaining_time;
  // Time that the process begins
  int arrival_time;
  // Time the process spent in waiting queue
  double waiting_time;
  // Total time the process has run so far
  double duration;
  int id;
  bool isReady;
  bool isFinished;
} process_t;

struct shortest_arrival_time
{
  inline bool operator() (const process_t& struct1, const process_t& struct2)
  {
    return (struct1.remaining_time < struct2.remaining_time);
  }
};

struct longest_arrival_time
{
  inline bool operator() (const process_t& struct1, const process_t& struct2)
  {
    return (struct1.arrival_time < struct2.arrival_time);
  }
};

std::deque<process_t> running_queue;
std::deque<process_t> waiting_queue;
pthread_mutex_t thread_flag_mutex;
pthread_cond_t thread_flag_cv;

// Thread function that will simulate a process
void* run_process(void* a);
// Initialize  mutex and condition variables
void init_flag();
// Function to give CPU for a specific thread
void set_thread_flag(int flag_value);
void start_rr();
void print_queue(std::deque<process_t>);
int read_input(const char* filename);
void log(int processId, char* state);

int main(int argc, const char *argv[])
{
  const char* filepath = "input2.txt";
  read_input(filepath);

  init_flag();

  pthread_t threads[thread_count];

  for (int i = 0; i < thread_count; i++) {
    pthread_create(&threads[i], NULL, run_process, (void*)&waiting_queue[i]);
  }
  start_rr();

  return 0;
}

void* run_process(void* a)
{
  process_t* info = (process_t*)a;
  while(info->remaining_time > 0.0) {
    // Lock mutex before accessing flag value
    pthread_mutex_lock(&thread_flag_mutex);
    // Check if the current thread is given CPU
    while (process_to_run != info->id) {
      pthread_cond_wait(&thread_flag_cv, &thread_flag_mutex);
    }

    pthread_mutex_unlock(&thread_flag_mutex);

    log(info->id, "resumed");
    printf("Time %d, Process %d resumed with remaining time %f\n", g_time, info->id, info->remaining_time);
    struct timespec start, stop;
    double runTime = 0.0;

    if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
      perror( "clock gettime" );
      return NULL;
    }

    double quantum = 0.1 * info->remaining_time;
    while(runTime < quantum) {

      if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
        perror( "clock gettime" );
        return NULL;
      }
      runTime = ( stop.tv_sec - start.tv_sec )
             + (double)( stop.tv_nsec - start.tv_nsec )
               / (double)BILLION;
    }

    info->duration += quantum;
    info->remaining_time -= quantum;

    int id;
    for (int i = 0 ; i < running_queue.size() ; ++i) {
      if (running_queue[i].id == info->id) {
       id = i;
      }
    }

    running_queue[id].duration = info->duration;
    running_queue[id].remaining_time = info->remaining_time;

    for (int i = 0 ; i < running_queue.size() ; ++i) {
      if(i != id) {
        running_queue[i].waiting_time += quantum;
      }
    }
    for (int i = 0; i < waiting_queue.size() ; i++) {
        waiting_queue[i].waiting_time += quantum;
    }

    printf( "Run time: %lf\n", quantum);
    log(info->id, "paused");
    printf("Time %d, Process %d paused with remaining time: %f\n", g_time, info->id, info->remaining_time);
    // Return CPU to scheduler
    set_thread_flag(0);
  }

  info->isFinished = true;
  printf("Time %d, Process %d finished\n", g_time, info->id);
  log(info->id, "finished");
  return NULL;
}

void set_thread_flag(int flag_value)
{
  // Lock mutex before accessing flag value
  pthread_mutex_lock(&thread_flag_mutex);
  // Set thread value, then signal in case run_process is blocked waiting for flag to be set
  process_to_run = flag_value;
  pthread_cond_broadcast(&thread_flag_cv);
  // Unlock mutex
  pthread_mutex_unlock(&thread_flag_mutex);
}

void start_rr()
{
  bool areProcessFinished;
  printf("Starting round-robin scheduler\n");
  std::sort(waiting_queue.begin(), waiting_queue.end(), longest_arrival_time());
  while(1) {
    areProcessFinished = true;
    // Add to run queue
    for (int i = 0; i < waiting_queue.size(); i++) {
      // check for arrival time
      if(waiting_queue.front().arrival_time >= g_time) {
        running_queue.push_back(waiting_queue.front());
        waiting_queue.pop_front();
      } else {
        break;
      }
    }

    std::sort(running_queue.begin(), running_queue.end(), shortest_arrival_time());

    // Run all processes in run queue
    for (int i = 0; i < running_queue.size(); i++) {
      // Wake first pid in queue and give it the CPU
      if(DEBUG) {
        printf("Scheduler resuming pid %d\n", running_queue[i].id);
      }
      set_thread_flag(running_queue[i].id);
      // Sleep until cpu is returned to scheduler
      pthread_mutex_lock(&thread_flag_mutex);
      while (process_to_run != 0) {
        pthread_cond_wait(&thread_flag_cv, &thread_flag_mutex);
      }
      pthread_mutex_unlock(&thread_flag_mutex);
      printf("Scheduler resumed\n");
      g_time++;
    }
    print_queue(running_queue);
    if (waiting_queue.size() == 0) {
      for (int i = 0 ; i < running_queue.size() ; ++i) {
          if (running_queue[i].remaining_time > 0.0000000000001) {
            areProcessFinished = false;
//            printf("pid %d has remaining time %f\n", running_queue[i].id, running_queue[i].remaining_time);
          }
      }
    } else {
      areProcessFinished = false;
    }
    if (areProcessFinished)
      break;
  }
}


void init_flag()
{
  pthread_mutex_init(&thread_flag_mutex, NULL);
  pthread_cond_init(&thread_flag_cv, NULL);
  process_to_run = 0;
}

void print_queue(std::deque<process_t> queue)
{
  for (unsigned i = 0; i < queue.size(); i++) {
    printf("id = %d, arrival = %d, remaining time = %f, execution time = %f, waiting time = %f\n"
        , queue.at(i).id, queue[i].arrival_time, queue[i].remaining_time
            , queue[i].duration, queue[i].waiting_time);
  }
}

int read_input(const char* filename)
{
  FILE* fp;
  int y;
  float i;

  fp = fopen(filename, "r");
  if (!fp) {
    printf("Could not open file\n");
    return -1;
  }

  // Assign input to array
  while ( fscanf (fp, "%d %f", &y, &i ) != EOF )
  {
    process_t* temp = new process_t;
    temp->waiting_time = temp->duration =  0.0;
    temp->isFinished = temp->isReady = false;
    temp->arrival_time = y;
    temp->remaining_time = i;
    temp->id = ++thread_count;
    waiting_queue.push_back(*temp);
    if(DEBUG){
      printf("id = %d, arrival time = %d, remaining time = %.2f\n",
        temp->id, temp->arrival_time, temp->remaining_time);
    }
  }

  fclose(fp);
  return 1;

}

void log(int processId, char* state)
{
  output = fopen("output.txt", "a");
  fprintf(output, "Time: %d, Process %d: %s\n", g_time, processId, state);
//  ss << "Time: " << time << ", Process " << processId << ": " << state << endl;
  fclose(output);
}

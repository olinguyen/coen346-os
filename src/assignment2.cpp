// Olivier Nguyen, ID: 6605583
// Amine Mhedhbi, ID: 6245137

// To compile (must be on Linux)
// g++ assignment2.cpp -lpthread -o assignment

/* To run
In terminal:
  ./assignment2 input.txt
*/

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

#define BILLION  1000000000L
#define EPSILON  0.00000001
#define DEBUG 1

// Used to keep track of older threads and their count
int thread_count = 0;

// Used to indicate which PID should be running
int process_to_run;

int g_time = 1;

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

void* run_process(void* a); // Thread function that will simulate a process
void init_flag(); // Initializes  mutex and condition variables
void set_thread_flag(int flag_value); // Gives CPU tp a specific thread
void start_rr(); // scheduling used by main(scheduler thread)
void print_queue(std::deque<process_t>);
int read_input(const char* filename);
void log(int processId, char* state);
void checkArrivalTime();

int main(int argc, const char *argv[])
{
  /*
  if (argc == 2) {
    const char* filepath = argv[1];
    read_input(filepath);
    init_flag();
    pthread_t threads[thread_count];

    for (int i = 0; i < thread_count; i++) {
      pthread_create(&threads[i], NULL, run_process, (void*)&waiting_queue[i]);
    }
    start_rr();
    output = fopen("output.txt", "a");
    for (int i = 0; i < running_queue.size(); i++) {
        fprintf(output, "Process %d has initial burst time %f\n"
            , running_queue[i].id, running_queue[i].duration);
        printf("Process %d has initial burst time %f\n"
            , running_queue[i].id, running_queue[i].duration);
    }

    fclose(output);

  } else {
    printf ("wrong number of arguments passed, program should be used as following:\n");
    printf ("./assignment2 <input_filename>\n");
  }
  */
  read_input("input.txt");
  init_flag();
  pthread_t threads[thread_count];

  for (int i = 0; i < thread_count; i++) {
    pthread_create(&threads[i], NULL, run_process, (void*)&waiting_queue[i]);
  }
  set_thread_flag(2);
  // Sleep until cpu is returned to scheduler
  pthread_mutex_lock(&thread_flag_mutex);
  while (process_to_run != 0) {
    pthread_cond_wait(&thread_flag_cv, &thread_flag_mutex);
  }
  pthread_mutex_unlock(&thread_flag_mutex);

  return 0;
}

void* run_process(void* a)
{
  process_t* info = (process_t*)a;
  int id;
  // Run thread until reaches total burst time
  printf("process started with burst time %f\n", info->burst_time);
  while(info->burst_time - info->duration > EPSILON) {
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
    // Run for 10% of remaining time
    double quantum = 3.0;
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

    for (int i = 0 ; i < running_queue.size() ; ++i) {
      if (running_queue[i].id == info->id) {
       id = i;
      }
    }

    running_queue[id].duration = info->duration;
    running_queue[id].remaining_time = info->remaining_time;


    if(DEBUG)
      printf( "Run time: %lf\n", quantum);
    log(info->id, "paused");
    printf("Time %d, Process %d paused with remaining time: %f\n", g_time, info->id, info->remaining_time);
    // Return CPU to scheduler
    set_thread_flag(0);
  }
  info->isFinished = running_queue[id].isFinished = true;
  printf("Time %d, Process %d finished with run time %f\n", g_time, info->id, info->duration);
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
    checkArrivalTime();
    std::sort(running_queue.begin(), running_queue.end(), shortest_arrival_time());

    // Run all processes in run queue
    for (int i = 0; i < running_queue.size(); i++) {
      if (!running_queue[i].isFinished) {// Wake first pid in queue and give it the CPU
        if(DEBUG)
          print_queue(running_queue);
        set_thread_flag(running_queue[i].id);
        // Sleep until cpu is returned to scheduler
        pthread_mutex_lock(&thread_flag_mutex);
        while (process_to_run != 0) {
          pthread_cond_wait(&thread_flag_cv, &thread_flag_mutex);
        }
        pthread_mutex_unlock(&thread_flag_mutex);
        printf("Scheduler resumed\n");
        g_time++;
        checkArrivalTime();
      }
    }
    std::sort(running_queue.begin(), running_queue.end(), shortest_arrival_time());
    if (waiting_queue.size() == 0) {
      for (int i = 0 ; i < running_queue.size() ; ++i) {
          if (!running_queue[i].isFinished) {
            areProcessFinished = false;
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
    printf("id = %d, arrival = %d, remaining time = %f, execution time = %f\n"
        , queue.at(i).id, queue[i].arrival_time, queue[i].remaining_time
            , queue[i].duration);
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
    temp->isFinished = temp->isReady = false;
    temp->arrival_time = y;
    temp->remaining_time = temp->burst_time = i;
    temp->id = ++thread_count;
    waiting_queue.push_back(*temp);
  }

  fclose(fp);
  return 1;
}

void log(int processId, char* state)
{
  output = fopen("output.txt", "a");
  fprintf(output, "Time: %d, Process %d: %s\n", g_time, processId, state);
  fclose(output);
}

void checkArrivalTime()
{
  for (int i = 0; i < waiting_queue.size(); i++) {
    // check for arrival time
    if(waiting_queue.front().arrival_time == g_time) {
      running_queue.push_back(waiting_queue.front());
      log(waiting_queue.front().id, "starting");
      waiting_queue.pop_front();
    } else {
      break;
    }
  }
}

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
#define BILLION  1000000000L;

#define DEBUG 1

// Used to keep track of older threads and their count
int thread_count = 0;

// Used to indicate which PID should be running
int process_to_run;

int g_time = 1;

// Holds information of a thread for the scheduler
typedef struct {

  // Execution time left
  double remaining_time;
  // Time that the process begins
  int arrival_time;
  // Time the process spent in waiting queue
  double waiting_time;
  // Total time the process has run so far
  double duration;
  int id;
  double quantum;
  bool isReady;

} process_t;

struct shortest_arrival_time
{
  inline bool operator() (const process_t& struct1, const process_t& struct2)
  {
    return (struct1.remaining_time < struct2.remaining_time);
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
int schedule();
void start_rr();
void print_queue(std::deque<process_t>);
int read_input(const char* filename);

int main(int argc, const char *argv[])
{
  const char* filepath = "input2.txt";
  read_input(filepath);

  init_flag();

  pthread_t threads[thread_count];

  // Check for arrival time and start process
  /*
    for(int i = 0; i < waiting_queue.size(); ++i)
    {
      if(g_time >= waiting_queue[i].arrival_time)
        pthread_create(&threads[i], NULL, run_process, (void*)&waiting_queue[i]);
    }
  */


  for (int i = 0; i < thread_count; i++) {
    pthread_create(&threads[i], NULL, run_process, (void*)&waiting_queue[i]);
  }

  // Sort waiting queue
  std::sort(waiting_queue.begin(), waiting_queue.end(), shortest_arrival_time());

  // if duplicates, sort by oldest process

  // Add to run queue
  for (int i = 1; i <= thread_count; i++) {
    running_queue.push_back(waiting_queue.front());
    waiting_queue.pop_front();
  }

  printf("Starting round-robin scheduler\n");
  // Run all processes in run queue
  for (int i = 1; i <= thread_count; i++) {
    // Wake first pid in queue and give it the CPU
    process_t current = running_queue.front();
    set_thread_flag(current.id);
    // Sleep until cpu is returned to scheduler
    pthread_mutex_lock(&thread_flag_mutex);
    while (process_to_run != 0) {
      pthread_cond_wait(&thread_flag_cv, &thread_flag_mutex);
    }
    pthread_mutex_unlock(&thread_flag_mutex);
    printf("Scheduler resumed\n");
    // Add to waitign queue, remove from running queue
    waiting_queue.push_back(running_queue.front());
    running_queue.pop_front();
  }

  return 0;
}

void* run_process(void* a)
{
  process_t* info = (process_t*)a;
  while(info->remaining_time > 0) {
    // Lock mutex before accessing flag value
    pthread_mutex_lock(&thread_flag_mutex);
    // Check if the current thread is given CPU
    while (process_to_run != info->id) {
      pthread_cond_wait(&thread_flag_cv, &thread_flag_mutex);
    }

    pthread_mutex_unlock(&thread_flag_mutex);

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


    info->remaining_time -= info->duration;
    printf( "Run time: %lf\n", runTime);

    printf("Time %d, Process %d resumed\n", g_time, info->id);
    /*
    // Check if current thread is finished
    if(info->remaining_time == 0) {
      printf("Time %d, Process %d finished\n", g_time, info->id);

      info->quantum = -1;

      // Return to scheduler
      // TODO: Does the thread end when its remaining time is over?
      set_thread_flag(0);
      pthread_exit(0);
    }
    */

    // Decrement remaining time
    g_time++;

    printf("Time %d, Process %d paused with remaining time: %f\n", g_time, info->id, info->remaining_time);
    // Return CPU to scheduler
    set_thread_flag(0);
  }

  printf("Time %d, Process %d finished\n", g_time, info->id);
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

int schedule()
{
  // Check arrival time
  for (int i = 0; i < running_queue.size(); i++) {
    if(g_time == running_queue[i].arrival_time)
      return running_queue[i].id;
  }

  // Move waiting queue to running queue

  // Compute quantum for each process, 10% of of remaining execution time

  // Sort for shortest remaining time
  std::sort(running_queue.begin(), running_queue.end(), shortest_arrival_time());

  // if duplicates, sort by oldest process

  return 0;
}

void init_flag()
{
  pthread_mutex_init(&thread_flag_mutex, NULL);
  pthread_cond_init(&thread_flag_cv, NULL);
  process_to_run = 0;
}

void print_queue(std::deque<process_t> queue)
{
  for (int i = 0; i < (signed)queue.size(); i++) {
    printf("id = %d, arrival = %d, remaining time = %f, execution time = %f\n", queue.at(i).id, queue[i].arrival_time, queue[i].remaining_time, queue[i].duration);
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

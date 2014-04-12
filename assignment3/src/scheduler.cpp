#include "../include/scheduler.h"

#define BILLION  1000000000L
#define EPSILON  0.00000001
#define DEBUG 0

const char* cmd_path = "commands.txt";

FILE* output;
int process_to_run;
int thread_count = 0;
int g_time = 1;
std::deque<process_t> running_queue;
std::deque<process_t> waiting_queue;
pthread_mutex_t thread_flag_mutex;
pthread_cond_t thread_flag_cv;

void print_queue(std::deque<process_t> queue)
{
  for (unsigned i = 0; i < queue.size(); i++) {
    printf("id = %d, arrival = %d, remaining time = %f, execution time = %f, waiting time = %f\n"
        , queue.at(i).id, queue[i].arrival_time, queue[i].remaining_time
            , queue[i].duration, queue[i].waiting_time);
  }
}

void* run_process(void* a)
{
  process_t* info = (process_t*)a;
  int id;
  // Run thread until reaches total burst time
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

    if(DEBUG)
      printf( "Run time: %lf\n", quantum);
    log(info->id, "paused");
    printf("Time %d, Process %d paused with remaining time: %f\n", g_time, info->id, info->remaining_time);
    // Return CPU to scheduler
    set_thread_flag(0);
  }
  info->isFinished = running_queue[id].isFinished = true;
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
  double wait_time [thread_count];
  bool areProcessFinished;
  printf("Starting round-robin scheduler\n");
  while(1) {
    areProcessFinished = true;
    // Add to run queue
    checkArrivalTime();

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

std::vector<command_t> read_commands(const char* filename)
{
  std::string store = "Store";

  std::vector<command_t> commands;

  char curr_cmd[10];
  char curr_id[10];
  FILE* fp;
  command_t cmd;
  fp = fopen(filename, "r");
  while (fscanf (fp, "%s %s", curr_cmd, curr_id) != EOF)
  {
    cmd.command.assign(curr_cmd);
    cmd.variableId.assign(curr_id);
    if(DEBUG)
      printf("%s %s", cmd.command.c_str(), cmd.variableId.c_str());
    if(strcmp(cmd.command.c_str(), store.c_str()) == 0)
    {
      fscanf (fp, "%d\n", &cmd.value);
      if(DEBUG)
        printf(" %d\n", cmd.value);
    }
    else {
      if(DEBUG)
        printf("\n");
    }
    commands.push_back(cmd);
  }

  return commands;
}

#include <vmm.h>
#include <scheduler.h>

using namespace std;

extern int process_to_run;
extern int thread_count;
extern int g_time;
extern double g_clock;
extern std::deque<process_t> running_queue;
extern std::deque<process_t> waiting_queue;
extern pthread_mutex_t thread_flag_mutex;
extern pthread_cond_t thread_flag_cv;

const char* processes = "processes.txt";
const char* vm = "vm.txt";
const char* commands = "commands.txt";
const char* memconfig = "memconfig.txt";
const char* output_file = "output.txt";

vmm* get_main_memory(const char* filename);
vmm* vm_manager;
FILE* out_fp;
vector<command_t> cmds;

int main(int argc, const char *argv[])
{
  // Read all inputs
  vm_manager = get_main_memory(memconfig);
  cmds = read_commands(commands);
  read_processes(processes);
  init_flag();
  pthread_t threads[thread_count];

  for (int i = 0; i < thread_count; i++) {
    pthread_create(&threads[i], NULL, run_process, (void*)&waiting_queue[i]);
  }
  start_rr();
  out_fp = fopen(output_file, "a");

  fclose(out_fp);

  return 0;
}

vmm* get_main_memory(const char* filename)
{
  FILE* fp;
  int size;

  fp = fopen(filename, "r");
  if (!fp) {
    printf("Could not open file\n");
    return NULL;
  }
  fscanf(fp, "%d", &size);
  return new vmm(size);
}

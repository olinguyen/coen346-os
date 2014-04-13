#include <vmm.h>
#include <scheduler.h>

using namespace std;

extern double g_clock;

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

#include <vmm.h>

#define LOGICAL_ADDRESS_SIZE 32
#define PAGE_TABLE_ENTRIES 256
#define TLB_ENTRIES 16
#define FRAME_SIZE 256
#define PAGE_SIZE FRAME_SIZE
#define NUMBER_OF_FRAMES 256
#define NUMBER_OF_PAGES NUMBER_OF_FRAMES

using namespace std;

int read_input(const char* filename);
int read_commands(const char* filename);

const char* processes = "processes.txt";
const char* vm = "vm.txt";
const char* commands = "commands.txt";
const char* memconfig = "memconfig.txt";

int main(int argc, const char *argv[])
{
  vmm test(0);
  test.memStore("1", 2);
  read_commands(commands);
  return 0;
}

int read_commands(const char* filename)
{
  string store = "Store";
  string free  = "Release";
  string lookup = "Lookup";

  char curr_cmd[80];

  FILE* fp;
  fp = fopen(filename, "r");
  fscanf(fp, "%s", curr_cmd);
  printf("%s\n", curr_cmd);

}
int read_input(const char* filename)
{
  FILE* fp;
  int y, memory_size, starting_time, burst_time;
  float i;

  fp = fopen(filename, "r");
  if (!fp) {
    printf("Could not open file\n");
    return -1;
  }

  // Assign input to array
  while ( fscanf (fp, "%d %f", &y, &i ) != EOF )
  {
  }

  fclose(fp);
  return 1;
}

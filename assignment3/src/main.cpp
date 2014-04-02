#include <stdio.h>
#include <vmm.h>
#include <time.h>

#define LOGICAL_ADDRESS_SIZE 32
#define PAGE_TABLE_ENTRIES 256
#define TLB_ENTRIES 16
#define FRAME_SIZE 256
#define PAGE_SIZE FRAME_SIZE
#define NUMBER_OF_FRAMES 256
#define NUMBER_OF_PAGES NUMBER_OF_FRAMES

using namespace std;

int read_input(const char* filename);
void log(int processId, char* state);

const char* processes = "processes.txt";
const char* vm = "vm.txt";
const char* commands = "commands.txt";
const char* memconfig = "memconfig.txt";

int main(int argc, const char *argv[])
{
  /*
  printf("Hello World!\n");
  time_t rawtime;
  struct tm * timeinfo;

  time (&rawtime);
  timeinfo = localtime (&rawtime);
  printf ("Current local time and date: %s", asctime(timeinfo));

  std::map<string,int> mymap;
  std::map<string,int>::iterator it;
  mymap["gros"] = 50;
  mymap.insert( make_pair( "dawg", 1337  )  );
  it=mymap.find("dawg");
  printf("%s %d\n", it->first.c_str(), it->second);
  */

  vmm test(5);
  test.memStore("1", 50);
  int ret = test.memLookup("1");
  printf("%d\n", ret);
  return 0;
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

void log(int processId, char* state)
{
  FILE* output = fopen("output.txt", "a");
  fprintf(output, "\n");
  fclose(output);
}

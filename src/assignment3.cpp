// Olivier Nguyen, ID: 6605583
// Amine Mhedhbi, ID: 6245137

// To compile (must be on Linux)
// g++ assignment3.cpp -lpthread -o assignment

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <fstream>

#define DEBUG 1
#define TIME_SLICE 3000 // miliseconds
#define CPU_CORES 2

const char* process_file = "processses.txt";
const char* memconfig = "memconfig.txt";
const char* vm = "vm.txt";
const char* cmd = "commands.txt";

using namespace std;

class vmm
{
public:
  vmm(int size);
  void put(string id, int value);
  int get(string id);
  void delete_id(string id);
};

typedef struct {
  string variableId;
  int value;
  int lastAccessTime;
} variable_t;

FILE* output;

int read_input(const char* filename);
// This instruction stores the given variableId and its value in the first unassigned spot in the memory
void memStore(string variableId, unsigned int value);
// This instruction removes the variable Id and its value from the memory , so the page which was holding this variable becomes available for storage.
void memFree(string variableId);
// If the variableId exists in the main memory it returns its value. If the variableId is not in the main memory but exists in disk space (i.e. page fault occurs), then it should move this variable in to the memory.
// Notice that if no spot is available in the memory, program needs to swap this variable with the least recently accessed variable , i.e. the variable with smallest Last Access time, in the main memory.
// Finally, if the variableId does not exist in the main memory and disk space, the API should return - 1.
int memLookup(string variableId);

int main(int argc, const char *argv[])
{
  if (argc == 2) {
    const char* filepath = argv[1];
    read_input(filepath);

    output = fopen("output.txt", "a");

    fclose(output);

  } else {
    printf ("wrong number of arguments passed, program should be used as following:\n");
    printf ("./assignment3 <input_filename>\n");
  }

  return 0;
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
  }

  fclose(fp);
  return 1;
}

void log(int processId, char* state)
{
  output = fopen("output.txt", "a");
  //fprintf(output, "Time: %d, Process %d: %s\n", g_time, processId, state);
  fclose(output);
}

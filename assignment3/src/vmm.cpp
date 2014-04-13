#include "../include/vmm.h"

#define DEBUG 1

vmm::vmm(int size)
{
  max_size = size;
}

/* Subtract the `struct timeval' values X and Y,
    storing the result in RESULT.
    Return 1 if the difference is negative, otherwise 0.
*/
int
timespec_subtract (struct timespec* result, struct timespec* x, struct timespec* y)
 {
   /* Perform the carry for the later subtraction by updating y. */
   if (x->tv_nsec < y->tv_nsec) {
     int nsec = (y->tv_nsec - x->tv_nsec) / 1000000 + 1;
     y->tv_nsec -= 1000000 * nsec;
     y->tv_sec += nsec;
   }
   if (x->tv_nsec - y->tv_nsec > 1000000) {
     int nsec = (x->tv_nsec - y->tv_nsec) / 1000000;
     y->tv_nsec += 1000000 * nsec;
     y->tv_sec -= nsec;
   }

   /* Compute the time remaining to wait.
      tv_nsec is certainly positive. */
   result->tv_sec = x->tv_sec - y->tv_sec;
   result->tv_nsec = x->tv_nsec - y->tv_nsec;

   /* Return 1 if result is negative. */
   return x->tv_sec < y->tv_sec;
 }

// This instruction stores the given variableId and its value in the first unassigned spot in the memory
int vmm::memStore(std::string variableId, unsigned int value)
{
  FILE* fp;
  // Update timestamp for last access
  variable_t tmp;
  clock_gettime(CLOCK_REALTIME, &tmp.access_time);
  tmp.variableId = variableId;
  tmp.value = value;

  // Verify we have enough space in main memory
  if(page_table.size() >= max_size)
  {
    // No more space, append to disk space
    if(DEBUG) {
//      printf("Writing to disk %s %d %s", variableId.c_str(), value, asctime(timeinfo));
    }
    virtual_memory.push_back(tmp);
    fp = fopen("vm.txt", "a");
    fprintf(fp, "%s %d\n", variableId.c_str(), value);
  }
  else {
    // We have enough space!
    page_table.push_back(tmp);
  }
  return 1;
}

// This instruction removes the variable Id and its value from the memory , so the page which was holding this variable becomes available for storage.
void vmm::memFree(std::string variableId)
{
  //TODO: Should this be removed from disk space as well?

  // Search in memory for variableId
  for (int i = 0; i < page_table.size(); i++) {
    if(page_table[i].variableId == variableId) {
      page_table.erase(page_table.begin() + i);
    }
  }

  // Search in disk for variableId
  for (int i = 0; i < virtual_memory.size(); i++) {
    if(virtual_memory[i].variableId == variableId) {
      virtual_memory.erase(virtual_memory.begin() + i);
    }
  }
}

// If the variableId exists in the main memory it returns its value. If the variableId is not in the main memory but exists in disk space (i.e. page fault occurs), then it should move this variable in to the memory.
// Notice that if no spot is available in the memory, program needs to swap this variable with the least recently accessed variable , i.e. the variable with smallest Last Access time, in the main memory.
// Finally, if the variableId does not exist in the main memory and disk space, the API should return - 1.
int vmm::memLookup(std::string variableId)
{
  // Search main memory for variableId
  for (int i = 0; i < page_table.size(); i++) {
    if(page_table[i].variableId == variableId) {
      clock_gettime(CLOCK_REALTIME, &page_table[i].access_time);
      return page_table[i].value;
    }
  }

  // Look up in disk space, handle page fault if it occurs
  for (int i = 0; i < virtual_memory.size(); i++) {
    if(virtual_memory[i].variableId == variableId) {
      swap_memory(variableId);
      return page_table.back().value;  
    }
  } 

  // variable does not exist in main memory
  return -1;
}

// This function will find the least recently accessed variable in main memory and swap it with the element with variableId in disk
void vmm::swap_memory(std::string variableId)
{
  // Least recently accessed variable, or smallest last access time should be swapped
  int index = 0; // index for least recently accessed variable
  struct timespec smallest_time = page_table[0].access_time;

  // Search for element with smallest last access time and keep the index
  for(int i = 1; i < page_table.size(); ++i)
  {
    struct timespec result;
    int ret = timespec_subtract(&result, &smallest_time, &page_table[i].access_time);

    if(ret == 1) {
      index = i - 1;
    }
  }
  if(DEBUG) {
    printf("Memory Manager SWAP: Variable %s from disk with %s from main\n", variableId.c_str(), page_table[index].variableId.c_str());
  }

  // Append variable from main memory to vm.txt
  virtual_memory.push_back(page_table[index]);

  // Erase element at index
  page_table.erase(page_table.begin() + index);

  // Insert variableid from disk to main memory
  for (int i = 0; i < virtual_memory.size(); i++) {
    if(virtual_memory[i].variableId == variableId) {
      clock_gettime(CLOCK_REALTIME, &virtual_memory[i].access_time);
      page_table.push_back(virtual_memory[i]);
      // Remove variableId from disk
      virtual_memory.erase(virtual_memory.begin() + i);
      break;
    }
  }
}

// Threads will call upon this command to run the next instruction in the queue
bool vmm::execute_next_command(std::vector<command_t> cmd_list)
{
  if(cmd_list.size() != 0 )
  {
    command_t curr_cmd = cmd_list.front();
    cmd_list.erase(cmd_list.begin());
    if(strcmp(curr_cmd.command.c_str(), "Store") == 0)
    {
      if(DEBUG) {
        printf("Store %s %d\n", curr_cmd.variableId.c_str(), curr_cmd.value);
    }
      memStore(curr_cmd.variableId, curr_cmd.value);
      return 1;
    }  
    else if (strcmp(curr_cmd.command.c_str(), "Release") == 0)
    {
      if(DEBUG) {
        printf("Release %s\n", curr_cmd.variableId.c_str());
      }
      memFree(curr_cmd.variableId.c_str());
      return 1;
    }
    else if (strcmp(curr_cmd.command.c_str(), "Lookup") == 0)
    {
      if(DEBUG) {
        printf("Lookup %s\n", curr_cmd.variableId.c_str());
      }
      memLookup(curr_cmd.variableId);
      return 1;
    }
    else {
      printf("Unknown command\n");
      return 0;
    }
  }
  return 0;
}

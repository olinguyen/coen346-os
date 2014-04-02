#include "../include/vmm.h"

vmm::vmm(int size)
{
  max_size = size;
}

// This instruction stores the given variableId and its value in the first unassigned spot in the memory
int vmm::memStore(std::string variableId, unsigned int value)
{
  // Verify we have enough space in main memory
  if(page_table.size() < max_size)
    page_table.insert( make_pair(variableId, value) );
  else {
    // append to disk space
  }
  // Update timestamp for last access
  return 1;
}

// This instruction removes the variable Id and its value from the memory , so the page which was holding this variable becomes available for storage.
void vmm::memFree(std::string variableId)
{
  page_table.erase(variableId);
  // Should this be removed from disk space as well?
}

// If the variableId exists in the main memory it returns its value. If the variableId is not in the main memory but exists in disk space (i.e. page fault occurs), then it should move this variable in to the memory.
// Notice that if no spot is available in the memory, program needs to swap this variable with the least recently accessed variable , i.e. the variable with smallest Last Access time, in the main memory.
// Finally, if the variableId does not exist in the main memory and disk space, the API should return - 1.
int vmm::memLookup(std::string variableId)
{
  std::map<std::string,int>::iterator iter = page_table.find(variableId);
  if(iter != page_table.end())
    return iter->second;
  else{
    // Look up in disk space, handle page fault
  }
  return 1;
}

void vmm::swap_memory(std::string variableId)
{
  // Least recently accessed variable, or smallest last access time should be swapped
}

void vmm::handle_page_fault(void)
{
  // Look up in vm.txt
}

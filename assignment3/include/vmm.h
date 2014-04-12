#ifndef __VMM_H__
#define __VMM_H__
#include <string>
#include <map>
#include <vector>
#include <stdio.h>
#include <time.h>

typedef struct {
  int value;
  std::string variableId;
  time_t lastAccessTime;
} variable_t;

class vmm
{
public:
  vmm(int size);
  int memStore(std::string variableId, unsigned int value);
  void memFree(std::string variableId);
  int memLookup(std::string variableId);
  /* data */
  int max_size;
//  std::map<std::string,int> page_table;
  std::vector<variable_t> page_table;
  std::vector<variable_t> virtual_memory;
  void swap_memory(std::string variableId);
  void handle_page_fault();
  void vmm_log(int pid, std::string event, std::string variableId, int value);

};

#endif

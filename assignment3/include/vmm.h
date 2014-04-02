#ifndef __VMM_H__
#define __VMM_H__
#include <string>
#include <map>

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
private:
  /* data */
  int max_size;
  std::map<std::string,int> page_table;
  void swap_memory(std::string variableId);
  void handle_page_fault();

};

#endif

// using GoogleTests
#include "gtest/gtest.h"
#include <vmm.h>
#include <scheduler.h>
/*
Store 1 5
Store 2 3
Store 3 7
Lookup 3
Lookup 2
Release 1
Store 1 8
Lookup 1
Lookup 2
*/
class vm_test : public ::testing::Test {
	protected:
	virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

//
TEST_F(vm_test, memStoreTest) {
  vmm test(2);
  test.memStore("1", 2);
  test.memStore("2", 3);
  test.memStore("3", 4);

  // Test size
  ASSERT_EQ(test.page_table.size(), 2);

  // Test value
  ASSERT_EQ(test.page_table[0].value, 2);
  ASSERT_EQ(test.page_table[1].value, 3);

  // Test variableId
  ASSERT_STREQ("1", test.page_table[0].variableId.c_str());
  ASSERT_STREQ("2", test.page_table[1].variableId.c_str());

  ASSERT_EQ(test.virtual_memory.size(), 1);
  ASSERT_EQ(test.virtual_memory[0].value, 4);
  ASSERT_STREQ("3", test.virtual_memory[0].variableId.c_str());
}

TEST_F(vm_test, readCommands) {
  std::vector<command_t> test_vector = read_commands("commands.txt");
  char* all_commands[] = {"Store", "Store", "Store", "Lookup", "Lookup", "Release", "Store", "Lookup", "Lookup"};
  char* all_id[] = {"1", "2", "3", "3", "2", "1", "1", "1", "2"};
  ASSERT_EQ(9, test_vector.size());
  for (int i = 0; i < test_vector.size(); i++) {
    ASSERT_STREQ(all_commands[i], test_vector[i].command.c_str());
    ASSERT_STREQ(all_id[i], test_vector[i].variableId.c_str());
  }

  ASSERT_EQ(5, test_vector[0].value);
}

TEST_F(vm_test, memFreeTest) {
  vmm test(1);
  test.memStore("1", 2);
  test.memStore("2", 3);
  ASSERT_STREQ("1", test.page_table[0].variableId.c_str());
  test.memFree("1");
  ASSERT_EQ(0, test.page_table.size());
  ASSERT_EQ(1, test.virtual_memory.size());
  test.memFree("2");
  ASSERT_EQ(0, test.virtual_memory.size());
}

TEST_F(vm_test, memSwapTest) {
  vmm test(1);
  test.memStore("1", 2);
  test.memStore("2", 3);

  ASSERT_STREQ("1", test.page_table[0].variableId.c_str());
  ASSERT_STREQ("2", test.virtual_memory[0].variableId.c_str());
  test.swap_memory("2");

  ASSERT_STREQ("2", test.page_table[0].variableId.c_str());
  ASSERT_STREQ("1", test.virtual_memory[0].variableId.c_str());
}

TEST_F(vm_test, memAccessTimeTest) {
  vmm test(2);
  test.memStore("1", 2);
  sleep(1);
  test.memStore("2", 3);
  sleep(1);
  ASSERT_LT(test.page_table[0].lastAccessTime, test.page_table[1].lastAccessTime);
  test.memLookup("1");

  ASSERT_GT(test.page_table[0].lastAccessTime, test.page_table[1].lastAccessTime);

}

TEST_F(vm_test, memLookupTest) {
  vmm test(1);
  test.memStore("1", 2);
  variable_t tmp;
  tmp.value = 5;
  tmp.variableId = "9";
  test.virtual_memory.push_back(tmp);

  ASSERT_EQ(1, test.page_table.size());
  ASSERT_STREQ("1", test.page_table[0].variableId.c_str());
  test.memLookup("9");
  ASSERT_STREQ("9", test.page_table[0].variableId.c_str());
}

TEST_F(vm_test, executeCommands) {
  vmm test(2);
  std::vector<command_t> cmds = read_commands("commands.txt");
  while(test.execute_next_command(cmds))
  {
    cmds.erase(cmds.begin());
  }
}

TEST_F(vm_test, LookupSwap) {
  vmm test(2);
  test.memStore("1", 2);
  test.memStore("2", 3);
  test.memStore("3", 4);
  test.memLookup("3");

}

TEST_F(vm_test, timespecTest) {
  vmm test(2);
  test.memStore("1", 2);
  test.memStore("2", 3);
  
  struct timespec result;
  int ret_value = timespec_subtract(&result, &test.page_table[0].access_time, &test.page_table[1].access_time);
  ASSERT_EQ(1, ret_value);


}

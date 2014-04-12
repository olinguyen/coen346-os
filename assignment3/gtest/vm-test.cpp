// using GoogleTests
#include "gtest/gtest.h"
#include <vmm.h>
#include <scheduler.h>

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

#include "gtest/gtest.h"
#include "stub.h"
#ifdef FUZZ
#include "secodefuzz/Secodefuzz/Cmd.h"
#endif
#include <cstdio>
#include <cstdlib>
#include <iostream>
using namespace std;

GTEST_API_ int main(int argc, char *argv[]) {
  cout << "Running main() from agent_unit_test.cpp\n";

  // 生成LLT报告
  testing::GTEST_FLAG(output) = "xml:./report/";
  testing::InitGoogleTest(&argc, argv);
#ifdef FUZZ
  cmd(argc, argv);
#endif
  int ret = RUN_ALL_TESTS();

  return ret;
}

/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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

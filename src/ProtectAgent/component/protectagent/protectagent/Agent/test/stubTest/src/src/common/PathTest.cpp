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
#include "common/PathTest.h"
#include "stub.h"
#include "common/ErrorCode.h"
#include "securec.h"

#include <libgen.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace std;

static char *StubTestDirname(char * path){
  return (char*)"/home/test/Agent/bin";
}

/*------------------------------------------------------------
Function Name: 
Description  : 这个测试用例只是一个stub的测试示例
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
TEST_F(PathTest, JustAnExampleForStub){
  try{
    stub.set(dirname, StubTestDirname);
    CPath &path = CPath::GetInstance();
    mp_char str[] = "/home/test/Agent/bin/rdagent";
    path.Init(str);
    mp_string expect = "/home/test/Agent";
    EXPECT_EQ(expect, path.GetRootPath());
  }catch(...){
    printf("Error on %s file %d line.\n", __FILE__, __LINE__);
    exit(0);
  }
}

/*------------------------------------------------------------
Function Name: 
Description  : 对CPath::Init(mp_char *)函数进行测试
Return       :
Call         :
Called by    :
Modification :
Others       :--------------------------------------------------------*/
TEST_F(PathTest, InitHomeRdadminAgent){
  CPath &path = CPath::GetInstance();

  mp_char str[] = "/home/rdadmin/Agent/bin/rdagent";
  int ret = 0;
  ret = path.Init(str);
  mp_string expect = "/home/rdadmin/Agent";
  EXPECT_EQ(ret, MP_SUCCESS);
  EXPECT_EQ(expect, path.GetRootPath());
  expect = "/home/rdadmin/Agent/bin";
  EXPECT_EQ(expect, path.GetBinPath());
}

TEST_F(PathTest, InitException){
  CPath &path = CPath::GetInstance(); 
  int ret = 0;

  ret = path.Init("");
  EXPECT_EQ(ret, ERROR_COMMON_OPER_FAILED);

  ret = path.Init("/home/");
  EXPECT_EQ(ret, MP_SUCCESS);
}


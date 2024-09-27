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
#include "common/FileSearcherTest.h"
#include "common/Log.h"
#include "stub.h"

#include <algorithm>
using namespace std;

TEST_F(CFileSearcherTest, SetPathTest){
  CFileSearcher fSearch;
  
  mp_string in = "/home/ls";
  fSearch.SetPath(in.c_str());
  mp_string path = fSearch.GetPath();
  EXPECT_EQ(path, in);
 
  in = "   /home/ls:/Agent/test ";
  fSearch.SetPath(in.c_str());
  path = fSearch.GetPath();
  in = "/home/ls:/Agent/test";
  EXPECT_EQ(path, in);
  
  in = " /home/l s:/Agent/test";
  fSearch.SetPath(in.c_str());
  path = fSearch.GetPath();
  in = "/home/l s:/Agent/test";
  EXPECT_EQ(path, in);
  
  in = "";
  fSearch.SetPath(in.c_str());
  path = fSearch.GetPath();
  EXPECT_EQ(path, in);
}

TEST_F(CFileSearcherTest, AddPathTest){
  CFileSearcher fSearch;
  
  fSearch.AddPath("");
  fSearch.AddPath("/zwg/mount");
  fSearch.AddPath("/home/rdadmin/Agent");
  mp_string sRet = fSearch.GetPath();
  mp_string expectStr = "/zwg/mount:/home/rdadmin/Agent";
  EXPECT_EQ(sRet, expectStr);
  
  fSearch.Clear();
  sRet = fSearch.GetPath();
  expectStr = "";
  EXPECT_EQ(sRet, expectStr);
  
  fSearch.Clear();
  fSearch.AddPath("/");
  sRet = fSearch.GetPath();
  expectStr = "/";
  EXPECT_EQ(sRet, expectStr);
}

typedef mp_void (*StubLogType)(void);
typedef mp_void (CLogger::*LogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);
    
static mp_void StubCLoggerLog(void){
    return;
}

TEST_F(CFileSearcherTest, SearchTest){
  stub.set(&CLogger::Log, StubCLoggerLog);
  CFileSearcher fSearch;
  
  fSearch.SetPath("/bin/");
  fSearch.AddPath("/sbin/");
  
  mp_string path;
  fSearch.Search("ls", path);
  EXPECT_EQ("/bin/ls", path);
  
  fSearch.Search("init", path);
  EXPECT_EQ("/sbin/init", path);
  
  fSearch.Clear();
  
  fSearch.AddPath("/usr/share/screen/utf8encodings/");
  fSearch.AddPath("/usr/bin/");
  vector<mp_string> fVect;
  fSearch.Search("cc", fVect);
  sort(fVect.begin(), fVect.end());
  
  EXPECT_EQ("/usr/bin/cc", fVect[0]);
  // EXPECT_EQ("/usr/share/screen/utf8encodings/cc", fVect[1]); //TODO
}

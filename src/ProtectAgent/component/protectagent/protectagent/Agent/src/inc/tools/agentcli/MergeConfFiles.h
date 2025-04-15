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
#ifndef AGENTCLI_MERGE_CONF_FILES_H_
#define AGENTCLI_MERGE_CONF_FILES_H_
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>

#include "common/Log.h"
#include "common/Defines.h"
#include "common/Path.h"

using namespace std;

struct FileIni {
    mp_string fileName;
    vector<mp_string> confSection;
    map<mp_string, vector<pair<mp_string, mp_string>>> fileIniContent;
};

class MergeConfFiles {
public:
    mp_int32 MergeFileHandle(const mp_string oldFilePath, const mp_string newFilePath);
    mp_int32 ReadFileIni(FileIni &fileIni);
    mp_void MergeIniFiles(FileIni &fileIniOld, FileIni &fileIniNew);
    mp_void SetFilePath(mp_string filePathNew, mp_string filePathOld);
    mp_void SaveInNewFile(const mp_string &filePath);

private:
    mp_void ParseFileIni(FileIni &fileInitent, const vector<mp_string> &content);
    pair<mp_string, mp_string> SetPairs(mp_string &fileLine);
    mp_void CheckSection(mp_string section);
    bool CheckKeyValuePair(pair<mp_string, mp_string> &pair1, vector<pair<mp_string, mp_string>> vec1);

private:
    FileIni mOldFileIni;
    FileIni mNewFileIni;
};

#endif  // AGENTCLI_MERGE_CONF_FILES_H_
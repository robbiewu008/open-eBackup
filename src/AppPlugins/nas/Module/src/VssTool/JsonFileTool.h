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
#ifndef VSS_TOOL_JSON_FILE_TOOL_H
#define VSS_TOOL_JOSN_FILE_TOOL_H
#include <fstream>
#include "common/JsonHelper.h"

namespace JsonFileTool {
    constexpr auto MODULE = "JsonFileTool";
    template <class T>
    bool ReadFromFile(const std::string& path, T& t)
    {
        std::ifstream readFd {};
        std::stringstream fileBuffer {};
        readFd.open(path.c_str(), std::ios::in);
        if (!readFd.is_open()) {
            HCP_Log(ERR, MODULE)<<"ReadFromFile open file failed,path:" << path <<HCPENDLOG;
            return false;
        }
        fileBuffer << readFd.rdbuf();
        readFd.close();
        std::string contents(fileBuffer.str());
        if (contents.empty()) {
            HCP_Log(ERR, MODULE)<<"the file content is empty,path:" << path <<HCPENDLOG;
            return false;
        }
        bool result = Module::JsonHelper::JsonStringToStruct(contents, t);
        if (!result) {
            HCP_Log(ERR, MODULE)<<"json string to struct failed"<<HCPENDLOG;
            return false;
        }
        HCP_Log(DEBUG, MODULE) << "read from file success,path:"<< path <<HCPENDLOG;
        return true;
    }
    template <class T>
    bool WriteToFile(T& t, const std::string& path)
    {
        std::string contents;
        bool bRet = Module::JsonHelper::StructToJsonString(t, contents);
        if (!bRet) {
            HCP_Log(ERR, MODULE)<<"struct to json string failed"<<HCPENDLOG;
            return false;
        }
        std::ofstream file(path, std::ios::out | std::ios::trunc);
        if (!file.is_open()) {
            HCP_Log(ERR, MODULE) << "WriteToFile open file failed,path:"<< path << " errno"
                << strerror(errno) << HCPENDLOG;
            return false;
        }
        file << contents;
        if (file.fail()) {
            HCP_Log(ERR, MODULE) << "Failed to write path:"<< path << HCPENDLOG;
            file.close();
            return false;
        }
        file.close();
        HCP_Log(DEBUG, MODULE) << "The struct transform to file success,path:"<< path <<HCPENDLOG;
        return true;
    }
}

#endif
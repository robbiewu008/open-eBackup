/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * @file JsonFileTool.h
 * @version 1.6.0
 * @date 2023-12-15
 * @author b00608411
*/

#ifndef SCANNER_JSON_FILE_TOOL_H
#define SCANNER_JSON_FILE_TOOL_H

#include <fstream>
#include "JsonHelper.h"

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
        HCP_Log(ERR, MODULE) << "WriteToFile open file failed,path:"<< path << " errno" << strerror(errno) << HCPENDLOG;
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
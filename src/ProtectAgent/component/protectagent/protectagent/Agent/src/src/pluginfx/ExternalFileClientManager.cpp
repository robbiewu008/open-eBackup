/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file AppProtectJobHandler.cpp
 * @brief Implement for excuting external job
 * @version 1.1.0
 * @date 2024-07-16
 * @author dengbowen d30043983
 */

#include <fstream>
#include <sstream>
#include <algorithm>
#include "common/Path.h"
#include "common/Log.h"
#include "common/JsonUtils.h"
#include "common/ErrorCode.h"
#include "common/JsonHelper.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "securecom/RootCaller.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "common/File.h"
#include "pluginfx/ExternalFileClientManager.h"
namespace {
const mp_string FILECLIENT_CONFIG_NAME = "fileclient_attribute_";
const mp_string FILECLIENT_CPU_LIMIT = "cpu_limit";
const mp_string FILECLIENT_MEMORY_LIMIT = "memory_limit";
const mp_string FILECLIENT_DIR = "FileClient";
const mp_string FILECLIENT_CONFIG_DIR = "conf";
const mp_string JSON_FORMAT_NAME = ".json";
}  // namespace

mp_int32 ExternalFileClientManager::Init()
{
    mp_int32 fileClientPid = AcquirePidByProcess();
    if (fileClientPid != 0) {
        WARNLOG("The system not install FileClient or the service is not running.");
        return MP_FAILED;
    }
    m_FileClientPid = fileClientPid;
    mp_int32 iRet = AcquireFileClientConfig();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Acquire fileClient config failed.");
        return iRet;
    }
    SetCGroupByConfig();
    return MP_SUCCESS;
}

mp_int32 ExternalFileClientManager::AcquirePidByProcess()
{
#ifdef WIN32
    return 0;
#endif
    mp_string strCmd = "ps -ef | grep /DataBackup/FileClient/FileClient | grep -v grep | awk '{print $2}'";
    std::vector<mp_string> vecRes;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRes);
    if (iRet != MP_SUCCESS || vecRes.size() == 0) {
        WARNLOG("The system not install FileClient or the service is not running.");
        return 0;
    }
    mp_int32 pid = atoi(vecRes[0].c_str());
    INFOLOG("the current FileClient pid is:%s.", vecRes[0].c_str());
    return pid;
}

mp_int32 ExternalFileClientManager::AcquireFileClientConfig()
{
    mp_string FileClientConfDir = CPath::GetInstance().GetRootPath() + PATH_SEPARATOR + ".." + PATH_SEPARATOR + ".." +
                                  PATH_SEPARATOR + FILECLIENT_DIR + PATH_SEPARATOR + FILECLIENT_CONFIG_DIR;
    std::vector<mp_string> vecName;
    CMpFile::GetFolderFile(FileClientConfDir, vecName);
    mp_string fileClientConfigName;
    for (mp_string strName : vecName) {
        if ((strName.find(JSON_FORMAT_NAME.c_str()) != std::string::npos) &&
            ((strName.find(FILECLIENT_CONFIG_NAME.c_str()) != std::string::npos))) {
            fileClientConfigName = strName;
            break;
        }
    }
    if (fileClientConfigName.empty()) {
        ERRLOG("The FileClient config file is not exists");
        return MP_FAILED;
    }

    mp_string strFileName = FileClientConfDir + PATH_SEPARATOR + fileClientConfigName;

    std::ifstream infile;
    infile.open(strFileName.c_str(), std::ifstream::in);
    if ((infile.fail() && infile.bad()) || ((infile.rdstate() & std::ifstream::failbit) != 0)) {
        ERRLOG("Open file %s failed, failed[%d], bad[%d]. errno[%d]:%s.",
            strFileName.c_str(),
            infile.fail(),
            infile.bad(),
            errno,
            strerror(errno));
        infile.close();
        return MP_FAILED;
    }

    Json::Reader jsonReader;
    Json::Value jsonValue;
    if (!jsonReader.parse(infile, jsonValue)) {
        ERRLOG("strFileName[%s] JsonData is invalid.", strFileName.c_str());
        infile.close();
        return MP_FAILED;
    }

    if (jsonValue.isObject() && jsonValue.isMember(FILECLIENT_CPU_LIMIT) && jsonValue[FILECLIENT_CPU_LIMIT].isInt()) {
        m_cpuLimit = jsonValue[FILECLIENT_CPU_LIMIT].asInt();
        INFOLOG("Set FileClient cpuLimit %d.", m_cpuLimit);
    }
    if (jsonValue.isObject() && jsonValue.isMember(FILECLIENT_MEMORY_LIMIT) &&
        jsonValue[FILECLIENT_MEMORY_LIMIT].isInt()) {
        m_memoryLimit = jsonValue[FILECLIENT_MEMORY_LIMIT].asInt();
        INFOLOG("Set FileClient memoryLimit %d.", m_memoryLimit);
    }
    return MP_SUCCESS;
}

mp_void ExternalFileClientManager::SetCGroupByConfig()
{
#ifdef LINUX
    if (m_FileClientPid == 0) {
        WARNLOG("FileClient process is not exists.");
        return;
    }
    std::ostringstream scriptParam;
    scriptParam << "PluginName=FileClient" << NODE_COLON << "PluginPID=" << m_FileClientPid << NODE_COLON
                << "CpuLimit=" << m_cpuLimit << NODE_COLON << "MemoryLimit=" << m_memoryLimit << NODE_COLON
                << "BlkioWeight=-1";
    CRootCaller rootCaller;
    if (rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_SET_CGROUP, scriptParam.str(), NULL) != MP_SUCCESS) {
        WARNLOG("Limit FileClient cpu or memory Failed.");
    }
#endif
    return;
}

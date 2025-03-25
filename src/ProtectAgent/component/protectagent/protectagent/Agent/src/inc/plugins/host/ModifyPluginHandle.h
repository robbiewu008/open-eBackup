/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file ModifyPluginHandle.cpp
 * @brief Contains functions that handle the modify of agent
 */
#ifndef MODIFY_HANDLE_H_
#define MODIFY_HANDLE_H_

#include <chrono>
#include <map>
#include <vector>
#include "common/Types.h"
#include "common/Defines.h"
#include "message/curlclient/CurlHttpClient.h"
#include "common/JsonHelper.h"

class ModifyJobLog {
public:
    uint64_t startTime = 0;
    mp_string jobId;
    mp_string logDetail;
    mp_string logInfo;
    mp_string level;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(startTime, startTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(jobId, jobId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(logDetail, logDetail)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(logInfo, logInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(level, level)
    END_SERIAL_MEMEBER
};

class ModifyLable {
public:
    mp_uint32 status = 0;
    mp_int32 progress = 0;
    std::vector<ModifyJobLog> jobLogs;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(progress, progress)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(jobLogs, jobLogs)
    END_SERIAL_MEMEBER
};

class ModifyPluginHandle {
public:
    ModifyPluginHandle() {};
    ~ModifyPluginHandle() {};

    // 升级线程启动函数
#ifdef WIN32
    static DWORD WINAPI ModifyPluginAgentHandle(mp_void* param);
#else
    static mp_void* ModifyPluginAgentHandle(mp_void* param);
#endif
    // 升级流程功能函数
    static mp_int32 CheckBeforeModify();
    static mp_int32 ObtainModifyPluginPac();
    static mp_int32 PrepareForModifyPlugin();
    static mp_int32 CallModifyPluginScript();
    static mp_int32 UpdateModifyPluginStatus(const mp_string& strModifyStatus);
    static mp_int32 LabelRepoter(
        const mp_string &jobId, mp_int32 process, const mp_string &lableName, mp_int32 status);

    static mp_int32 GetDownloadInfo();
    static mp_int32 SecurityConfiguration(HttpRequest& req);
    static mp_int32 InitRequest(HttpRequest& req);
    static void SetDomainResolve(HttpRequest& req, const mp_string& pmIp);
    static mp_int32 SendRequest(IHttpClient* httpClient, const HttpRequest& req, mp_string& disposition);
    static mp_int32 InitRequestCommon();

private:
    static std::vector<mp_string> m_vecPMIp;
    static mp_string m_pmIp;
    static mp_string m_pmPort;
    static mp_string m_domainName;
    static mp_string m_modifyUrl;
    static mp_string m_sha256;
    static mp_string m_signature;
    static mp_int32 m_modifyPackageSize;
    static mp_string m_packageType;
};

#endif    // MODIFY_HANDLE_H_
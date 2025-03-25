/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file UpgradeHandle.cpp
 * @brief Contains functions that handle the upgration of agent
 * @version 1.0.0.0
 * @date 2021-08-13
 * @author sheny sWX1041731
 */
#ifndef _UPGRADE_HANDLE_H_
#define _UPGRADE_HANDLE_H_

#include <chrono>
#include <map>
#include <vector>
#include "common/Types.h"
#include "common/Defines.h"
#include "message/curlclient/CurlHttpClient.h"
#include "common/JsonHelper.h"

class JobLog {
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

class Lable {
public:
    mp_uint32 status = 0;
    mp_int32 progress = 0;
    std::vector<JobLog> jobLogs;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(progress, progress)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(jobLogs, jobLogs)
    END_SERIAL_MEMEBER
};

class UpgradeHandle {
public:
    UpgradeHandle() {};
    ~UpgradeHandle() {};

    // 升级线程启动函数
#ifdef WIN32
    static DWORD WINAPI UpgradeAgentHandle(mp_void* param);
#else
    static mp_void* UpgradeAgentHandle(mp_void* param);
#endif
    // 升级流程功能函数
    static mp_int32 CheckBeforeUpgrade(const mp_string &jobId);
    static mp_int32 ObtainUpgradePac();
    static mp_int32 PrepareForUpgrade();
    static mp_int32 CallUpgradeScript();
    static mp_int32 UpdateUpgradeStatus(const mp_string& strUpgradeStatus);
    static mp_int32 LabelRepoter(const mp_string &jobId, mp_int32 process, const mp_string &lableName,
                                 mp_int32 status, const mp_string &labelLevel);

    static mp_int32 GetDownloadInfo();
    static mp_int32 SecurityConfiguration(HttpRequest& req);
    static mp_int32 InitRequest(HttpRequest& req);
    static void SetDomainResolve(HttpRequest& req, const mp_string& pmIp);
    static mp_int32 SendRequest(IHttpClient* httpClient, const HttpRequest& req, mp_string& disposition);
    static mp_int32 InitRequestCommon();
    static mp_void UpgradeAgentHandleInner(const mp_string &jobId);

private:
    static std::vector<mp_string> m_vecPMIp;
    static mp_string m_pmIp;
    static mp_string m_pmPort;
    static mp_string m_domainName;
    static mp_string m_upgradeUrl;
    static mp_string m_sha256;
    static mp_string m_signature;
    static mp_int32 m_upgradePackageSize;
    static mp_string m_packageType;
};

#endif    // _UPGRADE_HANDLE_H_
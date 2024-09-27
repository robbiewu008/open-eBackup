/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file OSAClient.h
 * @author h00606494
 * @brief OpenStorage API Client
 * @version 0.1
 * @date 2024-04-05
 */
#ifndef __OSA_CLIENT_H__
#define __OSA_CLIENT_H__

#ifdef LINUX
#include <string>
#include "common/Types.h"
#include "message/curlclient/CurlHttpClient.h"

/*
 * Request body: {"task_type":"backup","destination_ip":"192.168.115.120"}
 */
struct IpPolicyParams {
    std::string oper = "add"; // "add"/"delete"
    std::string taskType = "backup";
    std::string destinationIp;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(taskType, task_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(destinationIp, destination_ip)
    END_SERIAL_MEMEBER
};

/*
 * Response body: {"error":{"code":0}}
 */
class OSAClient {
public:
    explicit OSAClient() {}
    explicit OSAClient(const std::string &osaIp) : m_osaIp(osaIp) {}
    virtual ~OSAClient() {}
    virtual int32_t ModifyIpPolicy(const IpPolicyParams &reqParams);

private:
    bool CheckIpPolicyParams(const IpPolicyParams &reqParams);
    template<typename T> bool BuildRequestBody(const T &reqParams, std::string &body);

private:
    std::string m_osaIp = "protectengine.dpa.svc.cluster.local";
};
#endif

#endif // __OSA_CLIENT_H__
/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 * @file PmRestClient.h
 * @brief common function for access pm
 * @version 1.1.0
 * @date 2023-11-21
 * @author h00668904
 */
#ifndef PM_REST_CLIENT_H
#define PM_REST_CLIENT_H
#include <vector>
#include "common/Defines.h"
#include "message/curlclient/HttpClientInterface.h"
#include "message/rest/http_status.h"
#include "message/curlclient/RestClientCommon.h"
#include "message/curlclient/DmeRestClient.h"

class BaseRestClient {
public:
    mp_int32 SendReq(const HttpReqCommonParam& httpParam, HttpResponse& httpResponse);
private:
    mp_int32 UpdateUrlIp(HttpRequest& req, const mp_string& ip, const HttpReqCommonParam &httpParam);
    mp_int32 ReceiveCheckStatus(HttpResponse& httpResponse, HttpRequest req);
};

class PmRestClient : public BaseRestClient {
public:
    static PmRestClient& GetInstance()
    {
        static PmRestClient instance;
        return instance;
    }
    ~PmRestClient() {};

    mp_int32 SendRequest(HttpReqCommonParam& httpParam, HttpResponse& httpResponse);
    void SetTimeOut(int timeout);
    
private:
    PmRestClient() {};
    mp_int32 GetPMIPandPort(std::vector<std::string>& pmIps, std::string& pmPort);
    std::vector<std::string> GetConnectIps(const std::vector<std::string>& ips, const std::string& port);

private:
    int m_timeout = 0;
};
#endif
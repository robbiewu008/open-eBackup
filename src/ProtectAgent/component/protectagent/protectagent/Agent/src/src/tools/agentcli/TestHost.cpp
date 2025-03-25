#include <sstream>
#include "common/Log.h"
#include "common/StaticConfig.h"
#include "message/curlclient/HttpClientInterface.h"
#include "tools/agentcli/TestHost.h"

namespace {
    mp_int32 ONE_K = 1000;
}

mp_int32 TestHost::CheckConnect(const mp_string& strHostIp, const mp_string& strHostPort,
    uint32_t timeout, const mp_string& strSrcHost)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to connect ip(%s), port(%s).", strHostIp.c_str(), strHostPort.c_str());
    std::shared_ptr<IHttpClient> httpClient = IHttpClient::CreateNewClient();
    if (httpClient == nullptr) {
        COMMLOG(OS_LOG_ERROR, "HttpClient create failed.");
        return MP_FAILED;
    }

    if (!httpClient->TestConnectivity(strHostIp, strHostPort, timeout, strSrcHost)) {
        COMMLOG(OS_LOG_WARN, "Can not connect ip(%s), port(%s) src host(%s).",
            strHostIp.c_str(), strHostPort.c_str(), strSrcHost.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Can connect ip(%s), port(%s), src host(%s).",
        strHostIp.c_str(), strHostPort.c_str(), strSrcHost.c_str());
    return MP_SUCCESS;
}

mp_int32 TestHost::Handle(const mp_string& strHostIp, const mp_string& strHostPort,
    const mp_string& strTimeout, const mp_string& strSrcHost)
{
    mp_int32 miSecTimeOut = 0;

    std::stringstream ssTimeout;
    ssTimeout << strTimeout;
    ssTimeout >> miSecTimeOut;
    miSecTimeOut /= ONE_K;

    COMMLOG(OS_LOG_INFO, "strHostIp: %s, strHostPort:%s, strTimeout:%d, strSrcHost:%s.",
        strHostIp.c_str(), strHostPort.c_str(), miSecTimeOut, strSrcHost.c_str());
    if (StaticConfig::IsInnerAgent() && strSrcHost.empty()) {
        COMMLOG(OS_LOG_INFO, "Inner agent without source host test connect.");
        return InnerAgentHandle(strHostIp, strHostPort, miSecTimeOut);
    }

    return CheckConnect(strHostIp, strHostPort, miSecTimeOut, strSrcHost);
}

mp_int32 TestHost::InnerAgentHandle(const mp_string& strHostIp, const mp_string& strHostPort,
    uint32_t timeout)
{
    std::vector<std::string> ips;
    if (!StaticConfig::GetInnerAgentNodeIps(ips)) {
        COMMLOG(OS_LOG_ERROR, "Inner agent get backup netPlane ips failed.");
        return MP_FAILED;
    }
    for (auto ip : ips) {
        if (CheckConnect(strHostIp, strHostPort, timeout, ip) == MP_SUCCESS) {
            COMMLOG(OS_LOG_INFO, "Inner agent access success with ip(%s).", ip.c_str());
            return MP_SUCCESS;
        }
        COMMLOG(OS_LOG_WARN, "Inner agent can not access with ip(%s).", ip.c_str());
    }
    return MP_FAILED;
}
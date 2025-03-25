#ifndef _DME_REST_CLIENT_H
#define _DME_REST_CLIENT_H
#include <vector>
#include <mutex>
#include <thread>
#include "common/Defines.h"
#include "apps/appprotect/plugininterface/ShareResource.h"
#include "message/curlclient/HttpClientInterface.h"
#include "message/rest/http_status.h"
#include "common/CMpThread.h"

namespace {
constexpr int EXTERNAL_DME_PORT = 30065;
constexpr int INTERNAL_DME_PORT = 18089;
}
class DmeRestClient {
public:
    struct HttpReqParam {
        HttpReqParam() : method(""), url(""), body("")
        {}
        HttpReqParam(const mp_string& action, const mp_string& endpoint,
            const mp_string& content, mp_uint32 timeout = HTTP_TIME_OUT)
            : method(action), url(endpoint), body(content), nTimeOut(timeout)
        {}
        mp_string method;  // POST/GET/PUT/DELETE
        mp_string url;     // 如/v1/dme-unified/shared-resources
        mp_string body;
        mp_uint32 nTimeOut = HTTP_TIME_OUT;
        mp_uint32 port = 0;
        std::vector<mp_string> vecIpList;
        mp_string mainJobId;
    };

public:
    static DmeRestClient* GetInstance();
    ~DmeRestClient();

    mp_void UpdateDmeAddr(
        const mp_string &mainJobId, const std::vector<mp_string> &dmeIpList, mp_uint32 dmePort = EXTERNAL_DME_PORT);
    std::vector<mp_string> GetDmeAddr();
    mp_int32 GetDmeAddrByMainId(const mp_string &mainJobId, std::vector<mp_string> &ubcIps);
    mp_void DeleteDmeAddrByMainId(const mp_string &mainJobId);
    void RefreshDmeAddrOrder(const std::vector<mp_string>& srcDmeIpList, std::vector<mp_string>& dstDmeIpList);
    void AdjustMoreProperDmeIp(int index, const std::string& ip);
    void AdjustMoreProperDmeIpByMainId(const mp_string &mainJobId, const mp_string &ip);

    mp_int32 SendRequest(const HttpReqParam& httpParam, HttpResponse& httpResponse);
    mp_int32 SendKeyRequest(const HttpReqParam &httpParam, HttpResponse& httpResponse, mp_int32 expectCode = SC_OK);
    mp_int32 CheckEcsMetaInfo();
    
private:
    DmeRestClient();
    mp_uint32 InitUrl(std::string& url);
    EXTER_ATTACK mp_int32 UpdateUrlIp(HttpRequest& req, const mp_string& ip, const HttpReqParam &httpParam);
    bool IsNetworkError(mp_int32 statusCode);
    mp_int32 UpdateSecureInfo();
    mp_bool IsDmeIpValid(const mp_string& dmeIp);
    mp_int32 CheckEcsIpConnect(mp_string& metadataServerIp, mp_string& metadataServerPort);
    mp_void ChangeCertForInternalTask(const HttpReqParam &httpParam, HttpRequest &req);
private:
    std::vector<mp_string> m_dmeIpList;
    bool m_dmeIpListUpdated = false;

    thread_lock_t m_dmeInfoMutex;
    mp_uint32 m_dmePort {EXTERNAL_DME_PORT};
    mp_string m_domainName;
    mp_int32 m_secureChannel {0};
    mp_string m_strDeployType;
    std::vector<mp_string> m_localIpv4List;
    std::vector<mp_string> m_localIpv6List;
    static thread_lock_t m_dmeInstanceMutex;
    static DmeRestClient* m_intance;
    static bool m_init;

    std::mutex m_dmeIpMapMutex;
    std::map<mp_string, std::vector<mp_string>> m_mainJobDmeIpList;
};
#endif

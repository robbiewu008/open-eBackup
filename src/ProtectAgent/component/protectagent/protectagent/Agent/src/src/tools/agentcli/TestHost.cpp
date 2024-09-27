#include <sstream>
#include "common/Log.h"
#include "message/curlclient/HttpClientInterface.h"
#include "tools/agentcli/TestHost.h"

mp_int32 TestHost::Handle(const mp_string& strHostIp, const mp_string& strHostPort, const mp_string& strTimeout)
{
    mp_int32 miSecTimeOut = 0;

    std::stringstream ssTimeout;
    ssTimeout << strTimeout;
    ssTimeout >> miSecTimeOut;

    COMMLOG(OS_LOG_INFO, "Begin to cennect ip(%s), port(%s).", strHostIp.c_str(), strHostPort.c_str());
    std::shared_ptr<IHttpClient> httpClient = IHttpClient::CreateNewClient();
    if (httpClient == nullptr) {
        COMMLOG(OS_LOG_ERROR, "HttpClient create failed.");
        return MP_FAILED;
    }

    if (!httpClient->TestConnectivity(strHostIp, strHostPort)) {
        COMMLOG(OS_LOG_WARN, "Can not cennect ip(%s), port(%s).", strHostIp.c_str(), strHostPort.c_str());
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Can cennect ip(%s), port(%s).", strHostIp.c_str(), strHostPort.c_str());
    return MP_SUCCESS;
}

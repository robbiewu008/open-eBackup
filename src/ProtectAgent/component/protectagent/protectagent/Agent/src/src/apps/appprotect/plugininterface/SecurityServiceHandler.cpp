#include "apps/appprotect/plugininterface/SecurityServiceHandler.h"
#include "common/Log.h"
#include "common/MpString.h"
#include "common/Ip.h"
#include "message/curlclient/CurlHttpClient.h"
#include "pluginfx/ExternalPluginManager.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"

namespace AppProtect {
SecurityServiceHandler::SecurityServiceHandler()
{}

SecurityServiceHandler::~SecurityServiceHandler()
{}

EXTER_ATTACK void SecurityServiceHandler::CheckCertThumbPrint(ActionResult& _return,
    const std::string& ip, const int32_t port, const std::string& thumbPrint)
{
    LOGGUARD("");
    std::string tempIp = CIP::FormatFullUrl(ip);
    std::string url = "https://" + tempIp + ":" +  CMpString::to_string(port);
    DBGLOG("Check cert thumbprint url is %s", url.c_str());
    CurlHttpClient curlClient;
    mp_string strThumbPrint;
    mp_int32 iRet = curlClient.GetGeneralThumbprint(url, strThumbPrint, "SHA-256");
    if (iRet != MP_SUCCESS || strThumbPrint.empty()) {
        ERRLOG("Get cert thumbprint failed.");
        _return.code = iRet;
        _return.message = mp_string("Get cert thumbprint failed.");
        return;
    }
    strThumbPrint.erase(std::remove(strThumbPrint.begin(), strThumbPrint.end(), ':'), strThumbPrint.end());
    std::transform(strThumbPrint.begin(), strThumbPrint.end(), strThumbPrint.begin(), ::toupper);
    if (strThumbPrint != thumbPrint) {
        ERRLOG("Service %s cert thumbprint is invalidity", ip.c_str());
        _return.message = mp_string("cert thumbprint is invalidity.");
        _return.code = MP_FAILED;
        return;
    }
    _return.code = MP_SUCCESS;
}

void SecurityServiceHandler::Update(std::shared_ptr<messageservice::RpcPublishEvent> event)
{
    LOGGUARD("");
    m_processorName = "SecurityService";
    std::shared_ptr<servicecenter::IService> handler = shared_from_this();
    std::shared_ptr<SecurityServiceHandler> srHandler = std::dynamic_pointer_cast<SecurityServiceHandler>(handler);
    m_processor = std::make_shared<SecurityServiceProcessor>(srHandler);

    if (event->GetThriftServer().get() == nullptr) {
        COMMLOG(OS_LOG_ERROR, "ShareResourceHandler receives a null event");
        return;
    }
    if (!event->GetThriftServer()->RegisterProcessor(m_processorName, m_processor)) {
        COMMLOG(OS_LOG_ERROR, "ShareResourceHandler register processor failed.");
    }
}

bool SecurityServiceHandler::Initailize()
{
    LOGGUARD("");
    std::shared_ptr<servicecenter::IService> handler = shared_from_this();
    std::shared_ptr<SecurityServiceHandler> prHandler = std::dynamic_pointer_cast<SecurityServiceHandler>(handler);
    ExternalPluginManager::GetInstance().RegisterObserver(messageservice::EVENT_TYPE::RPC_PUBLISH_TYPE, prHandler);
    return true;
}

bool SecurityServiceHandler::Uninitailize()
{
    return true;
}
}

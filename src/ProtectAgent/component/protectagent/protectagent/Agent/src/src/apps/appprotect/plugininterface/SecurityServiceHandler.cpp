/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "apps/appprotect/plugininterface/SecurityServiceHandler.h"
#include "common/Log.h"
#include "common/MpString.h"
#include "common/Ip.h"
#include "common/CSystemExec.h"
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

EXTER_ATTACK void SecurityServiceHandler::RunCommand(CmdResult &_return, const std::string& cmdParaStr)
{
    LOGGUARD("");
    _return.result = CSystemExec::ExecSystemWithEcho(cmdParaStr, _return.output, false);
    if (_return.result != MP_SUCCESS) {
        ERRLOG("ExecSystemWithEcho failed.");
    }
    return;
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
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
#include "PluginThriftServer.h"
#include "log/Log.h"
#include "servicefactory/ServiceFactory.h"
#include "ApplicationServiceImp.h"
#include "PluginServiceImp.h"
#include "ProtectServiceImp.h"
#include "CertificateHandler.h"
#include "PluginCertificatePathProxy.h"
#include "ICertificateService.h"
#ifdef WIN32
#include "ThriftService.h"
#include "CertificateService.h"
#endif

using namespace AppProtect;
using namespace apache::thrift;
using namespace thriftservice;
using namespace certificateservice::detail;
using namespace servicecenter;

PluginThriftServer::PluginThriftServer()
{}

PluginThriftServer::~PluginThriftServer()
{}

PluginThriftServer& PluginThriftServer::GetInstance()
{
    static PluginThriftServer inst;
    return inst;
}

void PluginThriftServer::Configure(std::string ip, uint32_t port)
{
    INFOLOG("PluginThriftServer ip: %s, port: %u", ip.c_str(), port);
    m_ip = ip;
    m_port = port;
}

bool PluginThriftServer::Init()
{
    m_thriftService = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    if (!m_thriftService) {
        return false;
    }
    auto certifiService =
        ServiceFactory::GetInstance()->GetService<certificateservice::ICertificateService>("ICertificateService");
    auto certHandler = certifiService->GetCertificateHandler();
    if (!certHandler) {
        return false;
    }

    m_thriftServer = m_thriftService->RegisterSslServer(m_ip, m_port, certHandler);
    if (m_thriftServer == nullptr) {
        ERRLOG("m_thriftServer is nullptr");
        return false;
    }
    return true;
}

bool PluginThriftServer::Start()
{
    if (!Init()) {
        ERRLOG("Failed to register thrift server.");
        return false;
    }

    std::shared_ptr<ApplicationServiceImp> applicationServiceHandler = std::make_shared<ApplicationServiceImp>();
    std::shared_ptr<TProcessor> applicationServiceProcessor =
        std::make_shared<ApplicationServiceProcessor>(applicationServiceHandler);
    bool ret = m_thriftServer->RegisterProcessor("ApplicationService", applicationServiceProcessor);
    if (!ret) {
        ERRLOG("Register thrift service[ApplicationService] failed.");
        return false;
    }

    std::shared_ptr<PluginServiceImp> pluginServiceHandler = std::make_shared<PluginServiceImp>();
    std::shared_ptr<TProcessor> pluginServiceProcessor =
        std::make_shared<PluginServiceProcessor>(pluginServiceHandler);
    ret = m_thriftServer->RegisterProcessor("PluginService", pluginServiceProcessor);
    if (!ret) {
        ERRLOG("Register service[PluginService] failed.");
        return false;
    }

    std::shared_ptr<ProtectServiceImp> protectServiceHandler = std::make_shared<ProtectServiceImp>();
    std::shared_ptr<TProcessor> protectServiceProcessor =
        std::make_shared<ProtectServiceProcessor>(protectServiceHandler);
    ret = m_thriftServer->RegisterProcessor("ProtectService", protectServiceProcessor);
    if (!ret) {
        ERRLOG("Register plugin failed.");
        return false;
    }
    INFOLOG("Plugin RegisterProcessor success");
    ret = m_thriftServer->Start();
    if (!ret) {
        ERRLOG("Plugin thrift server start failed");
        return false;
    }
    return true;
}

bool PluginThriftServer::Stop()
{
    INFOLOG("Thrift Service Stop!");
    bool ret = m_thriftServer->Stop();
    if (!ret) {
        ERRLOG("Stop the plugin registration server failed.");
    }
    return ret;
}

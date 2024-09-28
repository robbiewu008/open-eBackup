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
#include "common/EnvVarManager.h"
#include "servicefactory/ServiceFactory.h"
#include "ShareResource.h"
#include "JobService.h"
#include "CertCN.h"
#include "RegisterPluginService.h"
#include "SecurityService.h"
#include "FrameworkService.h"
#include "CertificateHandler.h"
#include "PluginCertificatePathProxy.h"
#include "ICertificateService.h"
#include "PluginThriftClient.h"


using namespace startup;
using namespace thriftservice;
using namespace AppProtect;
using namespace certificateservice::detail;
using namespace servicecenter;

namespace {
    constexpr auto MODULE = "PluginThriftClient";
#ifdef WIN32
    const std::string THRIFT_CERT_PATH = R"(\DataBackup\ProtectClient\ProtectClient-E\nginx\conf\server.pem)";
#else
    const std::string THRIFT_CERT_PATH = "/DataBackup/ProtectClient/ProtectClient-E/nginx/conf/server.pem";
#endif
}

PluginThriftClient::PluginThriftClient()
{}

PluginThriftClient::~PluginThriftClient()
{
    Stop();
}

bool PluginThriftClient::Stop()
{
    bool ret = false;
    if (m_thriftClient != nullptr) {
        ret = m_thriftClient->Stop();
        if (ret != true) {
            NAS_PLUGIN_LOG(ERROR, "Stop the plugin registration client failed.");
        }
        m_thriftClient = nullptr;
    }
    return ret;
}

std::shared_ptr<thriftservice::IThriftClient> PluginThriftClient::GetAgentClientFromService()
{
    std::string ip = PluginThriftClientConfig::GetInstance().GetServerIP();
    uint32_t port = PluginThriftClientConfig::GetInstance().GetServerPort();
    auto thriftService = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    if (thriftService == nullptr) {
        HCP_Log(ERR, MODULE) << "init thrift service failed!" << HCPENDLOG;
        return nullptr;
    }
    std::string thriftCertFile = Module::EnvVarManager::GetInstance()->GetAgentHomePath() + THRIFT_CERT_PATH;
    std::string certCN;
    GetHostFromCert(thriftCertFile, certCN);
    auto certifiService =
        ServiceFactory::GetInstance()->GetService<certificateservice::ICertificateService>("ICertificateService");
    auto certHandler = certifiService->GetCertificateHandler();
    auto thriftClient = thriftService->RegisterSslClient(certCN, port, certHandler);
    return thriftClient;
}

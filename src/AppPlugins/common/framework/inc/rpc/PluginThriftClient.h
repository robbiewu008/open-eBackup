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
#ifndef PLUGIN_THRIFT_CLIENT_H
#define PLUGIN_THRIFT_CLIENT_H

#include <string>
#include <vector>
#include <thread>
#include "PluginTypes.h"
#include "thriftservice/IThriftService.h"
#include "ShareResource.h"
#include "JobService.h"
#include "RegisterPluginService.h"
#include "SecurityService.h"
#include "PluginThriftClientConfig.h"
#include "log/Log.h"
#include "common/Thread.h"

namespace startup {

#define RETRYTIMES 3
#ifdef WIN32
class AGENT_API PluginThriftClient {
#else
class PluginThriftClient {
#endif
public:
    PluginThriftClient();
    ~PluginThriftClient();
    void Configure(std::string ip, uint32_t port);
    bool Stop();
    template<typename T>
    std::shared_ptr<T> GetAgentClient(const std::string& serviceName)
    {
        int retryTimes = RETRYTIMES;
        while (retryTimes > 0) {
            try {
                m_thriftClient = GetAgentClientFromService();
                if (m_thriftClient == nullptr) {
                    HCP_Log(ERR, "PluginThriftClient") << "m_thriftClient is nullptr" << HCPENDLOG;
                    return nullptr;
                }
                std::shared_ptr<T> serviceClient = m_thriftClient->GetConcurrentClientIf<T>(serviceName);
                if (serviceClient != nullptr && m_thriftClient->Start()) {
                    return serviceClient;
                }
            } catch (apache::thrift::transport::TTransportException& ex) {
                HCP_Log(ERR, "PluginThriftClient") << "TTransportException: " << ex.what() << HCPENDLOG;
            } catch (const std::exception &ex) {
                HCP_Log(ERR, "PluginThriftClient") << "Standard C++ Exception: " << ex.what() << HCPENDLOG;
            } catch (...) {
                HCP_Log(ERR, "PluginThriftClient") << "Unknown exception" << HCPENDLOG;
            }
            HCP_Log(ERR, "PluginThriftClient") << "start thrift client failed! retry : " << retryTimes << HCPENDLOG;
            retryTimes--;
            Module::SleepFor(std::chrono::seconds(1));
        }
        HCP_Log(ERR, "PluginThriftClient") << "Start the thrift client failed!" << HCPENDLOG;
        return nullptr;
    }

private:
    std::shared_ptr<thriftservice::IThriftClient> m_thriftClient { nullptr };
    std::shared_ptr<thriftservice::IThriftClient> GetAgentClientFromService();
};
}

#endif // _PLUGIN_THRIFT_CLIENT_H_
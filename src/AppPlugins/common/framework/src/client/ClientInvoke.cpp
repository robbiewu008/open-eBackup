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
#include "ClientInvoke.h"
#include "FrameworkService.h"

using namespace startup;
using namespace AppProtect;
namespace {
    constexpr int INVOKE_AGENT_INTERFACE_RETRY_TIMES = 3;
    constexpr int INVOKE_AGENT_INTERFACE_RETRY_INTERVAL = 20;
    constexpr auto MODULE = "ClientInvoke";
    constexpr int INVOKE_PLUGIN_SUCCESS = 0;
    constexpr int INVOKE_PLUGIN_FAILED = -1;
}

void ShareResource::CreateResource(ActionResult& returnValue, const Resource& resource, const std::string& mainJobId)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<ShareResourceConcurrentClient>("ShareResource");
            if (agentClient != nullptr) {
                agentClient->CreateResource(returnValue, resource, mainJobId);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void ShareResource::QueryResource(ResourceStatus& returnValue, const Resource& resource, const std::string& mainJobId)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<ShareResourceConcurrentClient>("ShareResource");
            if (agentClient != nullptr) {
                agentClient->QueryResource(returnValue, resource, mainJobId);
            }
            return;
        } catch (AppProtectFrameworkException &e) {
            throw e;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    return;
}

void ShareResource::UpdateResource(ActionResult& returnValue, const Resource& resource, const std::string& mainJobId)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<ShareResourceConcurrentClient>("ShareResource");
            if (agentClient != nullptr) {
                agentClient->UpdateResource(returnValue, resource, mainJobId);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void ShareResource::DeleteResource(ActionResult& returnValue, const Resource& resource, const std::string& mainJobId)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<ShareResourceConcurrentClient>("ShareResource");
            if (agentClient != nullptr) {
                agentClient->DeleteResource(returnValue, resource, mainJobId);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void ShareResource::LockResource(ActionResult& returnValue, const Resource& resource, const std::string& mainJobId)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<ShareResourceConcurrentClient>("ShareResource");
            if (agentClient != nullptr) {
                agentClient->LockResource(returnValue, resource, mainJobId);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void ShareResource::UnLockResource(ActionResult& returnValue, const Resource& resource, const std::string& mainJobId)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<ShareResourceConcurrentClient>("ShareResource");
            if (agentClient != nullptr) {
                agentClient->UnLockResource(returnValue, resource, mainJobId);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void ShareResource::LockJobResource(ActionResult& returnValue, const Resource& resource, const std::string& mainJobId)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<ShareResourceConcurrentClient>("ShareResource");
            if (agentClient != nullptr) {
                agentClient->LockJobResource(returnValue, resource, mainJobId);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void JobService::AddNewJob(ActionResult& returnValue, const std::vector<SubJob>& job)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->AddNewJob(returnValue, job);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void JobService::ReportJobDetails(ActionResult& returnValue, const SubJobDetails& jobInfo)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->ReportJobDetails(returnValue, jobInfo);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void JobService::ReportCopyAdditionalInfo(ActionResult& returnValue, const std::string& jobId, const Copy& copy)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->ReportCopyAdditionalInfo(returnValue, jobId, copy);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void JobService::ComputerFileLocationInMultiFileSystem(std::map<std::string, std::string>& returnValue,
    const std::vector<std::string>& files, const std::vector<std::string>& fileSystems)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->ComputerFileLocationInMultiFileSystem(returnValue, files, fileSystems);
            }
            return;
        } catch (AppProtectFrameworkException &e) {
            throw e;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    return;
}

void JobService::QueryPreviousCopy(Copy& returnValue, const Application& application,
    const std::set<AppProtect::CopyDataType>& _types, const std::string& copyId, const std::string& mainJobId)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->QueryPreviousCopy(returnValue, application, _types, copyId, mainJobId);
            }
            return;
        } catch (AppProtectFrameworkException &e) {
            throw e;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    return;
}

void JobService::MountRepositoryByPlugin(ActionResult& returnValue, const PrepareRepositoryByPlugin& mountinfo)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->MountRepositoryByPlugin(returnValue, mountinfo);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (AppProtectFrameworkException &e) {
            throw e;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}
 
void JobService::UnMountRepositoryByPlugin(ActionResult& returnValue, const PrepareRepositoryByPlugin& mountinfo)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->UnMountRepositoryByPlugin(returnValue, mountinfo);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (AppProtectFrameworkException &e) {
            throw e;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void JobService::SendAlarm(ActionResult& returnValue, const AlarmDetails& alarm)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->SendAlarm(returnValue, alarm);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (AppProtectFrameworkException &e) {
            throw e;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void JobService::ClearAlarm(ActionResult& returnValue, const AlarmDetails& alarm)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->ClearAlarm(returnValue, alarm);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (AppProtectFrameworkException &e) {
            throw e;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void JobService::AddIpWhiteList(ActionResult& returnValue, const std::string &jobId, const std::string &ipListStr)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->AddIpWhiteList(returnValue, jobId, ipListStr);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (AppProtectFrameworkException &e) {
            throw e;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void JobService::ReportAsyncJobDetails(ActionResult& returnValue, const std::string &jobId,
    const int &code, const ResourceResultByPage &results)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                HCP_Log(INFO, MODULE) << "ReportAsyncJobDetails. Code: " << code << HCPENDLOG;
                agentClient->ReportAsyncJobDetails(returnValue, jobId, code, results);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (AppProtectFrameworkException &e) {
            throw e;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void JobService::GetHcsToken(ApplicationEnvironment& env, const std::string &projectId, const std::string &isWorkSpace)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<JobServiceConcurrentClient>("JobService");
            if (agentClient != nullptr) {
                agentClient->GetHcsToken(env, projectId, isWorkSpace);
            } else {
                env.__set_extendInfo("");
            }
            return;
        } catch (AppProtectFrameworkException &e) {
            throw e;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    env.__set_extendInfo("");
    return;
}

void RegisterPluginService::RegisterPlugin(ActionResult& returnValue, const ApplicationPlugin& plugin)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<RegisterPluginServiceConcurrentClient>("RegisterPluginService");
            if (agentClient != nullptr) {
                agentClient->RegisterPlugin(returnValue, plugin);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void RegisterPluginService::UnRegisterPlugin(ActionResult& returnValue, const ApplicationPlugin& plugin)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<RegisterPluginServiceConcurrentClient>("RegisterPluginService");
            if (agentClient != nullptr) {
                agentClient->UnRegisterPlugin(returnValue, plugin);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
}

void SecurityService::CheckCertThumbPrint(ActionResult& returnValue, const std::string& ip,
    const int32_t port, const std::string& thumbPrint)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<SecurityServiceConcurrentClient>("SecurityService");
            if (agentClient != nullptr) {
                agentClient->CheckCertThumbPrint(returnValue, ip, port, thumbPrint);
            } else {
                returnValue.code = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.code = Module::FAILED;
    return;
};

void SecurityService::RunCommand(CmdResult& returnValue, const std::string& cmdParaStr)
{
    int retryTimes = INVOKE_AGENT_INTERFACE_RETRY_TIMES;
    while (retryTimes > 0) {
        try {
            PluginThriftClient client;
            auto agentClient = client.GetAgentClient<SecurityServiceConcurrentClient>("SecurityService");
            if (agentClient != nullptr) {
                agentClient->RunCommand(returnValue, cmdParaStr);
            } else {
                returnValue.result = Module::FAILED;
            }
            return;
        } catch (apache::thrift::transport::TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (apache::thrift::TApplicationException& ex) {
            HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (const std::exception& ex) {
            HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
        }
        Module::SleepFor(std::chrono::seconds(INVOKE_AGENT_INTERFACE_RETRY_INTERVAL));
        retryTimes--;
    }
    returnValue.result = Module::FAILED;
    return;
}

void FrameworkService::HeartBeat(ActionResult& returnValue)
{
    try {
        PluginThriftClient client;
        auto agentClient = client.GetAgentClient<AppProtect::FrameworkServiceConcurrentClient>("FrameworkService");
        if (agentClient != nullptr) {
            agentClient->HeartBeat(returnValue);
        } else {
            returnValue.code = Module::FAILED;
        }
        return;
    } catch (apache::thrift::transport::TTransportException& ex) {
        HCP_Log(ERR, MODULE) << "TTransportException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
    } catch (apache::thrift::protocol::TProtocolException& ex) {
        HCP_Log(ERR, MODULE) << "TProtocolException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
    } catch (apache::thrift::TApplicationException& ex) {
        HCP_Log(ERR, MODULE) << "TApplicationException. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
    } catch (const std::exception& ex) {
        HCP_Log(ERR, MODULE) << "Standard C++ Exception. " << WIPE_SENSITIVE(ex.what()) << HCPENDLOG;
    } catch (...) {
        HCP_Log(ERR, MODULE) << "Unknown exception." << HCPENDLOG;
    }

    returnValue.code = Module::FAILED;
    return;
};

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
#include "EngineFactory.h"
#include "log/Log.h"
#ifndef WIN32
#include "protect_engines/kubernetes/KubernetesProtectEngine.h"
#include "protect_engines/hcs/HCSProtectEngine.h"
#include "protect_engines/openstack/OpenStackProtectEngine.h"
#include "protect_engines/cnware/CNwareProtectEngine.h"
#include "protect_engines/apsara_stack/ApsaraStackProtectEngine.h"
#else
#include "protect_engines/hyperv/HyperVProtectEngine.h"
#endif

namespace {
const std::string MODULE_NAME = "EngineFactory";
const std::string ENV_APPTYPE_KUBERNETES = "Kubernetes";
const std::string ENV_APPTYPE_HCS = "HCSContainer";
const std::string ENV_APPTYPE_HCSENVOP = "HcsEnvOp";
const std::string ENV_APPTYPE_OPENSTACK = "OpenStackContainer";
const std::string ENV_APPTYPE_CNWARE = "CNware";
const std::string ENV_APPTYPE_APSARASTACK = "ApsaraStack";
const std::string ENV_APPTYPE_HYPERV_SCVMM = "HyperV.SCVMM";
const std::string ENV_APPTYPE_HYPERV_CLUSTER = "HyperV.Cluster";
const std::string ENV_APPTYPE_HYPERV_HOST = "HyperV.Host";
const std::string ENV_APPTYPE_HYPERV_VM = "HyperV.VM";
}

namespace VirtPlugin {
EngineFactoryPipeline EngineFactory::m_enginePipeline = {
#ifndef WIN32
    {ENV_APPTYPE_KUBERNETES, CreateK8SEngine},
    {ENV_APPTYPE_HCS, CreateHCSEngine},
    {ENV_APPTYPE_HCSENVOP, CreateHCSEngine},
    {ENV_APPTYPE_OPENSTACK, CreateOpenStackEngine},
    {ENV_APPTYPE_APSARASTACK, CreateApsaraStackEngine},
    {ENV_APPTYPE_CNWARE, CreateCNwareEngine}
#else
    {ENV_APPTYPE_HYPERV_SCVMM, CreateHyperVEngine},
    {ENV_APPTYPE_HYPERV_CLUSTER, CreateHyperVEngine},
    {ENV_APPTYPE_HYPERV_HOST, CreateHyperVEngine},
    {ENV_APPTYPE_HYPERV_VM, CreateHyperVEngine}
#endif
};

std::shared_ptr<ProtectEngine> EngineFactory::CreateProtectEngine(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo, std::string jobId, std::string subJobId)
{
    if (jobInfo == nullptr) {
        ERRLOG("No jobInfo provided, null pointer.");
        return nullptr;
    }
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    auto func = m_enginePipeline.find(jobHandle->GetAppEnv().subType);
    if (func == m_enginePipeline.end()) {
        ERRLOG("Application(subType:%s)not support yet, create protect engine failed.",
            jobHandle->GetAppEnv().subType.c_str());
        return nullptr;
    }
    return func->second(jobHandle, jobId, subJobId);
}

#ifndef WIN32
std::shared_ptr<ProtectEngine> EngineFactory::CreateK8SEngine(std::shared_ptr<JobHandle> jobHandle,
    std::string jobId, std::string subJobId)
{
    INFOLOG("Creating kubernetes ProtectEngine");
    return std::make_shared<KubernetesPlugin::KubernetesProtectEngine>(jobHandle, jobId, subJobId);
}

std::shared_ptr<ProtectEngine> EngineFactory::CreateHCSEngine(std::shared_ptr<JobHandle> jobHandle,
    std::string jobId, std::string subJobId)
{
    INFOLOG("Creating HCS ProtectEngine");
    return std::make_shared<HcsPlugin::HCSProtectEngine>(jobHandle, jobId, subJobId);
}

std::shared_ptr<ProtectEngine> EngineFactory::CreateOpenStackEngine(std::shared_ptr<JobHandle> jobHandle,
    std::string jobId, std::string subJobId)
{
    INFOLOG("Creating OpenStack ProtectEngine");
    return std::make_shared<OpenStackPlugin::OpenStackProtectEngine>(jobHandle, jobId, subJobId);
}

std::shared_ptr<ProtectEngine> EngineFactory::CreateApsaraStackEngine(std::shared_ptr<JobHandle> jobHandle,
    std::string jobId, std::string subJobId)
{
    INFOLOG("Creating ApsaraStack ProtectEngine");
    return std::make_shared<ApsaraStackPlugin::ApsaraStackProtectEngine>(jobHandle, jobId, subJobId);
}

std::shared_ptr<ProtectEngine> EngineFactory::CreateCNwareEngine(std::shared_ptr<JobHandle> jobHandle,
    std::string jobId, std::string subJobId)
{
    INFOLOG("Creating CNware ProtectEngine");
    return std::make_shared<CNwarePlugin::CNwareProtectEngine>(jobHandle, jobId, subJobId);
}

#else

std::shared_ptr<ProtectEngine> EngineFactory::CreateHyperVEngine(std::shared_ptr<JobHandle> jobHandle,
    std::string jobId, std::string subJobId)
{
    INFOLOG("Creating Hyper-V ProtectEngine");
    return std::make_shared<HyperVPlugin::HyperVProtectEngine>(jobHandle, jobId, subJobId);
}

#endif

std::shared_ptr<ProtectEngine> EngineFactory::CreateProtectEngineWithoutTask(const std::string appType)
{
    std::shared_ptr<ProtectEngine> retValuePtr = nullptr;
#ifndef WIN32 // 目前的HCS等插件不支持windows,先屏蔽
    if (appType == ENV_APPTYPE_KUBERNETES) {
        retValuePtr = std::make_shared<KubernetesPlugin::KubernetesProtectEngine>();
    } else if (appType == ENV_APPTYPE_HCS || appType == ENV_APPTYPE_HCSENVOP) {
        retValuePtr = std::make_shared<HcsPlugin::HCSProtectEngine>();
    } else if (appType == ENV_APPTYPE_OPENSTACK) {
        retValuePtr = std::make_shared<OpenStackPlugin::OpenStackProtectEngine>();
    } else if (appType == ENV_APPTYPE_CNWARE) {
        retValuePtr = std::make_shared<CNwarePlugin::CNwareProtectEngine>();
    } else if (appType == ENV_APPTYPE_APSARASTACK) {
        retValuePtr = std::make_shared<ApsaraStackPlugin::ApsaraStackProtectEngine>();
    } else {
        WARNLOG("Can't create projectEngine object. appType: %s", appType.c_str());
        retValuePtr = nullptr;
    }
#else
    // HyperV等windows类型插件
    if (appType == ENV_APPTYPE_HYPERV_SCVMM ||
        appType == ENV_APPTYPE_HYPERV_CLUSTER ||
        appType == ENV_APPTYPE_HYPERV_HOST ||
        appType == ENV_APPTYPE_HYPERV_VM) {
        retValuePtr = std::make_shared<HyperVPlugin::HyperVProtectEngine>();
    } else {
        WARNLOG("Can't create projectEngine object. appType: %s", appType.c_str());
        retValuePtr = nullptr;
    }
#endif
    return retValuePtr;
}
}

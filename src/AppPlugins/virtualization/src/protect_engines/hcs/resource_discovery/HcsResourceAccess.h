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
#ifndef HCS_RESOURCE_ACCESS_H
#define HCS_RESOURCE_ACCESS_H
#include <vector>
#include <map>
#include "common/cert_mgr/CertMgr.h"
#include "common/Structs.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "protect_engines/hcs/api/sc/model/VdcListDetail.h"
#include "HcsMessageInfo.h"
#include "protect_engines/hcs/api/iam/IamClient.h"
#include "protect_engines/hcs/api/sc/ScClient.h"
#include "protect_engines/hcs/api/ecs/EcsClient.h"
#include "protect_engines/hcs/api/evs/EvsClient.h"
#include "protect_engines/hcs/api/nova/HcsNovaClient.h"
#include "protect_engines/hcs/utils/DistributedStorageMgr.h"
#include "protect_engines/openstack/api/nova/model/ServerDetail.h"
#include "common/login_host/LoginHost.h"

using namespace AppProtect;
using OpenStackPlugin::OpenStackServerInfo;
using OpenStackPlugin::GetProjectServersRequest;

namespace HcsPlugin {
class HcsResourceAccess {
public:
    explicit HcsResourceAccess(ApplicationEnvironment appEnv);
    HcsResourceAccess(ApplicationEnvironment appEnv, Application application);
    HcsResourceAccess(ApplicationEnvironment appEnv, QueryByPage pageInfo);
    HcsResourceAccess(ApplicationEnvironment appEnv, Application application, QueryByPage pageInfo);
    ~HcsResourceAccess();
    void SetApplication(Application application);
    // 检查连通性
    int32_t CheckAppConnect(ActionResult &returnValue);
    int32_t GetTenantInfo(ResourceResultByPage &page);
    int32_t GetProjectLists(ResourceResultByPage &page, std::string &parentId);
    int32_t GetVirtualMachineDetailInfo(ResourceResultByPage &returnValue, const std::vector<std::string> &vmLists);
    int32_t GetVirtualMachineList(ResourceResultByPage& page, std::string &parentId);
    int32_t GetVolumeResourceList(ResourceResultByPage& page, std::string &parentId);
    int32_t CheckStorageConnect(ApplicationEnvironment &returnEnv);
    bool CheckStorageInfo(std::vector<StorageInfo> &storageVector, Json::Value &body, bool &result);
    int32_t GetVDCResourceList(ResourceResultByPage& page, std::vector<std::string>& errorCodeAndName,
        std::string &parentId);

protected:
    int32_t CheckConnectionParams(GetTokenRequest &request, ActionResult &returnValue);
    int32_t CheckIfVdcSrvMgr(const std::string &domainName);
    bool AssemblyTenantInfo(std::vector<ApplicationResource> &applicationResources,
        const std::shared_ptr<QueryVdcListResponse> &response);
    int32_t AssemblyGetServerListRequest(GetProjectServersRequest &request, const AuthObj &auth,
                                      const ProjectResource &projectResource);
    int32_t GetDisksDetailInfo(std::string &hostExtendInfoStr, HostResource &hostResource,
        const std::vector<std::string> &diskIds, const ApplicationResource &vmDetailInfo);
    // 获取指定工程的详细信息
    int32_t GetSpcifiedProject(ApplicationResource &returnValue,
        const std::string &projectId, const ResourceExtendInfoVdcs &vdcInfo);
    bool AssemblyProjectInfo(std::string &projectInfo,
        const ProjectDetail &projectDetail, const ResourceExtendInfoVdcs &vdcInfo);
    int32_t GetTopLevelVdcs(std::vector<ResourceExtendInfoVdcs>& topVdc, std::vector<ResourceExtendInfoVdcs>& missVdc,
        std::string &parentId);
    bool GetVdcsInfoUnderTenantByScApi(VdcListDetail &vdcList, std::string &parentId, bool needTry = true);
    bool GetVdcsExtendInfoParams(AuthObj &auth, QueryVdcListRequest &vdcListRequest, Json::Value &extendInfoJson);
    bool GetVdcsInfoFromTaskParams(TaskVdcList &taskVdcList);
    int FilterTopLevelVdc(std::vector<ResourceExtendInfoVdcs>& missVdc,
        std::vector<ResourceExtendInfoVdcs> &topVdc, VdcListDetail &vdcList, TaskVdcList &taskVdcList);
    void FilterAvaibleVdc(const std::vector <ResourceExtendInfoVdcs> &topLevelVdcsInfo,
                          std::vector <ResourceExtendInfoVdcs> &topLevelVdcs,
                          std::vector <ResourceExtendInfoVdcs> &missVdc);
    int32_t GetSpcifiedVmInfo(ApplicationResource &returnValue, HostResource &hostResource,
        std::vector<std::string> &vmDiskId);
    int32_t GetSpcifiedEVSDisk(DiskInfo &diskInfo, const std::string &diskId);
    int32_t HostExtendInfoParse(ProjectResource &projectResource) const;
    int32_t ComposeVMDetail(ApplicationResource& vmDetailInfo, const OpenStackServerInfo& hostInfo,
        const std::string &userName);
    int32_t ComposeVolumeDetail(ApplicationResource& volumeDetail, const HostVolume& volumeInfo,
        const std::string &userName);
    void GetResourceListRequest(QueryResourceListRequest& request, const ResourceExtendInfoVdcs& vdcinfo);
    bool GetvdcUserDetail(const ResourceExtendInfoVdcs &vdcUser, UserDetail &userDetail, bool retry);
    bool InitControlDeviceInfo(ControlDeviceInfo &controlDeviceInfo, StorageInfo &storageInfo);
    int32_t CheckNodeConnect(const StorageInfo& info);

    Application m_application;
    ApplicationEnvironment m_appEnv;
    QueryByPage m_condition;
};
}
#endif
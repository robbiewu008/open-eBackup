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
#ifndef __HYPERV_PROTECT_ENGINE_H__
#define __HYPERV_PROTECT_ENGINE_H__

#ifdef HYPERV_DLL_EXPORTS  // specified in vs
#define HYPERV_API __declspec(dllexport)
#else
#define HYPERV_API __declspec(dllimport)
#endif

#include <map>
#include <vector>
#include "cstdint"
#include "protect_engines/ProtectEngine.h"
#include "protect_engines/hyperv/resource_discovery/HyperVResourceAccess.h"
#include "protect_engines/hyperv/api/wmi_client/WMIClient.h"
#include "protect_engines/hyperv/api/powershell/PSClient.h"
#include "log/Log.h"
#include "common/Structs.h"

using namespace VirtPlugin;


namespace HyperVPlugin {

class HYPERV_API HyperVProtectEngine : public ProtectEngine {
public:
    HyperVProtectEngine() {}
    explicit HyperVProtectEngine(std::shared_ptr<VirtPlugin::JobHandle> &m_jobHandle, std::string jobId,
        std::string subJobId) : ProtectEngine(m_jobHandle, jobId, subJobId)
    {
    }

    /**
     *  @brief 任务的前置钩子，用于不同引擎的差异化处理
     *
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t PreHook(const ExecHookParam &para);

    /**
     *  @brief 任务的后置钩子，用于不同引擎的差异化处理
     *
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t PostHook(const ExecHookParam &para);

    /**
     * @brief 恢复前检查
     *
     * @param vmObj 附本虚拟机信息
     * @return int32_t
     */
    virtual int32_t CheckBeforeRecover(const VMInfo &vmObj);

    /**
     * @brief 备份前检查
     *
     * @return int32_t
     */
    virtual int32_t CheckBeforeBackup();

    /**
     *  @brief 创建生产端快照
     *  Notice!
     *  X-series all-in-one DataBackup Box ESN needs to be set to snapshot description when creating snapshot,
     *  so that identifying specific snapshots when deleting.
     *  @param snapshot     [OUT]产生的快照信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t CreateSnapshot(SnapshotInfo &snapshot, std::string &errCode);

    /**
     *  @brief 刪除生产端快照
     *
     *  @param snapshot [IN]要删除的快照信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t DeleteSnapshot(const SnapshotInfo &snapshot);

    /**
     *  @brief 查询虚拟机快照
     *
     *  @param snapshot [IN]要查询的快照
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t QuerySnapshotExists(SnapshotInfo &snapshot);

    /**
     *  @brief 获取虚拟机的元数据
     *
     *  @param vmMetadata [OUT]虚拟机元数据信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t GetMachineMetadata(VMInfo &vmInfo);

    /**
     *  @brief 获取卷的元数据
     *
     *  @param vmMetadata   [IN]所属虚拟机的信息
     *  @param volsMetadata [OUT]卷的元数据信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t GetVolumesMetadata(const VMInfo &vmInfo,
        std::unordered_map<std::string, std::string> &volsMetadata);

    /**
     *  @brief 获取卷的handler
     *
     *  @param volInfo [IN]卷信息
     *  @param volHandler [OUT]卷处理器
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t GetVolumeHandler(const VolInfo &volInfo, std::shared_ptr<VolumeHandler> &volHandler);

    /* * 创建卷，用于恢复时创建新卷
     * @brief Create a Volume object
     *
     * @param volObj 待创卷的基本对象信息
     * @param volMetaData 待创卷的元数据信息
     * @param vmMoRef 卷所在的虚拟机moref
     * @param dsInfo 数据存储信息
     * @param newVol 新创卷信息
     * @return int32_t
     */
    virtual int32_t CreateVolume(const VolInfo &volObj, const std::string &volMetaData, const std::string &vmMoRef,
                                 const DatastoreInfo &dsInfo, VolInfo &newVol);

    /**
     * @brief 从生产存储卸载卷，用于卷挂载到备份客户端
     *
     * @param volHandler
     * @return int32_t
     */
    virtual int32_t DetachVolume(const VolInfo &volObj);

    /**
     *  @brief 挂载卷，将卷挂载到虚拟机（虚拟机信息从JobHandle中获取）
     *
     *  @param volHandler [IN]卷对应的Handler
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t AttachVolume(const VolInfo &volObj);

    /**
     *  @brief 删除卷，将卷从虚拟机上删除掉（虚拟机信息从JobHandle中获取）
     *
     *  @param volHandler [IN]卷对应的Handler
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t DeleteVolume(const VolInfo &volObj);

    /**
     *  @brief 替换卷，将虚拟机上的卷替换成目标卷(虚拟机信息从JobHandle中获取)
     *
     *  @param tgtVolHandler [IN]目标卷对应的Handler
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t ReplaceVolume(const VolInfo &volObj);

    /**
     *  @brief 创建虚拟机
     *
     *  @param vmInfo [IN,OUT]入参包含要创建虚拟机的基本信息和卷元数据信息；出参包含创建的虚拟机的标识moref或uri
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t CreateMachine(VMInfo &vmInfo);

    /**
     *  @brief 删除虚拟机
     *
     *  @param vmInfo [IN]要删除的虚拟机的信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t DeleteMachine(const VMInfo &vmInfo);

    /**
     *  @brief 重命名虚拟机
     *
     *  @param vmInfo  [IN]虚拟机的信息
     *  @param newName [IN]新名称
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t RenameMachine(const VMInfo &vmInfo, const std::string &newName);

    /**
     *  @brief 上电虚拟机
     *
     *  @param vmInfo [IN]虚拟机的信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t PowerOnMachine(const VMInfo &vmInfo);

    /**
     *  @brief 下电虚拟机
     *
     *  @param vmInfo [IN]虚拟机的信息
     *  @return 错误码：0 成功，非0 失败
     */

    virtual int32_t PowerOffMachine(const VMInfo &vmInfo);

    /**
     *  @brief 节点是否可执行当前任务
     *
     *  @param job [IN]任务参数
     *  @return 错误码：0 可执行，非0 不可执行
     */
    virtual int32_t AllowBackupInLocalNode(const AppProtect::BackupJob &job, int32_t &errorCode);

    /**
     *  @brief 节点是否可执行当前备份子任务
     *
     *  @param job [IN]主任务参数
     *  @param subJob [IN]子任务参数
     *  @return 错误码：0 可执行，非0 不可执行
     */
    virtual int32_t AllowBackupSubJobInLocalNode(const AppProtect::BackupJob& job,
        const AppProtect::SubJob& subJob, int32_t &errorCode);

    /**
     *  @brief 节点是否可执行当前恢复任务
     *
     *  @param job [IN]任务参数
     *  @return 错误码：0 可执行，非0 不可执行
     */
    virtual int32_t AllowRestoreInLocalNode(const AppProtect::RestoreJob &job, int32_t &errorCode);

    /**
     *  @brief 节点是否可执行当前恢复子任务
     *
     *  @param job [IN]主任务参数
     *  @param subJob [IN]子任务参数
     *  @return 错误码：0 可执行，非0 不可执行
     */
    virtual int32_t AllowRestoreSubJobInLocalNode(const AppProtect::RestoreJob& job,
        const AppProtect::SubJob& subJob, int32_t &errorCode);

    /**
     *  @brief 检查备份类型
     *
     *  @param jobTypeParam [IN]任务参数，快照信息
     *  @param checkRet [OUT]检查结果，true-本节点可执行，false-本节点不可执行
     *  @return 错误码：0 全量，非0 失败（特定错误码才执行增转全）
     */
    virtual int32_t CheckBackupJobType(const JobTypeParam &jobTypeParam, bool &checkRet);

    /**
     * @brief 在主机中按一个应用类型列出应用程序,无身份验证,同步功能
     *
     * @param returnValue [out]
     * @param appType [IN] 应用的类型
     */
    virtual void DiscoverApplications(std::vector<Application> &returnValue, const std::string &appType);

    /**
     *  @brief 检查应用是否存在
     *
     *  @param returnValue : [out] returnValue.code is 0, 失败：otherwise
     *  @param appEnv : [in] 受保护对象所在环境
     *  @param application : [in] 受保护对象信息
     */
    virtual void CheckApplication(ActionResult &returnValue, const ApplicationEnvironment &appEnv,
                                  const AppProtect::Application &application);

    /**
     * @brief 根据条件查询资源列表
     *
     * @param returnValue ： [out] 资源列表
     * @param appEnv ： [in] 受保护对象所在环境
     * @param application ：[in] 受保护对象信息
     * @param parentResource： [in] 父资源信息
     */
    virtual void ListApplicationResource(std::vector<ApplicationResource> &returnValue,
                                         const ApplicationEnvironment &appEnv, const Application &application,
                                         const ApplicationResource &parentResource);

    virtual void ListApplicationResourceV2(ResourceResultByPage &page, const ListResourceRequest &request);

    /**
     * @brief 查询主机集群信息
     *
     * @param returnEnv ： [out] 远程主机列表,不存在时应用程序列表为空
     * @param appEnv ： [in] 作为群集成员的应用程序环境信息
     */
    virtual void DiscoverHostCluster(ApplicationEnvironment &returnEnv, const ApplicationEnvironment &appEnv);

    /**
     * @brief 查询应用集群信息
     *
     * @param returnEnv ： [out] 远程主机列表,不存在时应用程序列表为空
     * @param appEnv ：[in] 作为群集成员的应用程序环境信息
     * @param application ：[in] 应用信息
     */
    virtual void DiscoverAppCluster(ApplicationEnvironment &returnEnv, const ApplicationEnvironment &appEnv,
                                    const Application &application);

    /**
     * @brief 生成卷映射匹配对信息
     *
     * @param vmObj 目标虚拟机对象
     * @param volumePair 卷匹配对，也做出参填充恢复目标卷信息
     * @param volinfo 源卷信息
     * @return int
     */
    virtual int GenVolPair(VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol,
        VolMatchPairInfo &volPairs);

    /**
     * @brief 获取指定卷之前创建的所有快照
     *
     * @param volInfo 卷信息
     * @param snapList 返回匹配的快照列表
     * @return 错误码，0执行成功，非0执行失败
     */
    virtual int32_t GetSnapshotsOfVolume(const VolInfo &volInfo, std::vector<VolSnapInfo> &snapList);

    int32_t PrepareFile(std::shared_ptr<RepositoryHandler> &repoHandler,
        const std::string &repoPath, const VolInfo &volInfo);

    int32_t OpenCopyFile(std::shared_ptr<RepositoryHandler> &repoHandler,
        const std::string &repoPath, const VolInfo &volObj);
    
    virtual int32_t CheckBeforeMount()
    {
        return SUCCESS;
    }

    virtual int32_t CancelLiveMount(const VMInfo &liveVm)
    {
        return SUCCESS;
    }

    virtual int32_t CreateLiveMount(const VMInfo &copyVm, VMInfo &newVm)
    {
        return SUCCESS;
    }

protected:
    template<typename T>
    int32_t Executor(const std::string &command, T &res, const Json::Value &param);
    int32_t GetTargetType(const ApplicationEnvironment &appEnv, std::string &targetType);
    void AddScanHostErrorInfo(ResourceResultByPage &page, const std::string &errorCode);
    void AddInvalidItemInfo(ResourceResultByPage &page, const std::string& extendInfo);
    void InitRepoHandler();
    bool UpdateVolSnapInfo(const VolumeInfo  &curVol, SnapshotInfo &preSnapshotInfo);
    bool CheckAndUpdatePreSnapShotInfo();
    bool UpdatePreSnapShotInfo(const std::list<VolumeInfo> &diskList, const std::string &preSnapFile);
    bool InitJobPara();
    bool GenerateAndSaveReferPointInfo(void);
    bool SaveReferPointInfo(std::shared_ptr<ReferrencePointResponse> &response,
        const std::string &curSnapFile, SnapshotInfo &curSnap);
    std::shared_ptr<ReferrencePointResponse> GenerateReferPoint(SnapshotInfo &curSnapshotInfo);
    bool ExecuteAccessMethod(HyperVResourceAccess &hyperVResourceAccess, ResourceResultByPage &page,
                             const std::string &targetType);
    int32_t GetRepoPathByType(RepositoryDataType::type type, std::string& repoPath);
    int32_t PrepareVmModifyParam(const VMInfo& vmInfo, VmModifyParam& param);
    int CreateDiskFileWithNameAndPath(const VolInfo &copyVol, const std::string& absPath, std::string& realPath);
    int32_t InitWmiClient();
    int32_t BackupVmFiles();
    int32_t ExecPowerOnMachine(const VMInfo &vmInfo);
    int32_t ExecAttachVolume(const VolInfo &volObj);
    void ThrowPluginException(const int32_t &code, const std::string &message);
    int32_t CheckProtectEnvConn(const AppProtect::ApplicationEnvironment &env,
        const AppProtect::Application& app, const std::string &vmId);
    int32_t GetVMMetaData(VMInfo &vmInfo);
    int32_t SaveVMInfoToCache(VMInfo &vmInfo, const std::string &vmFileInCache);
    bool UpdateCurSnapShotInfoWhenConvertRefFail();
    std::shared_ptr<GetVMHardDiskDriveResponse> GetVMDiskDriver();
    bool SplitIdAndGetResult(const std::string &splitStr, std::string &result);
    bool AddRctToVolSnapShotExtendInfo(VolSnapInfo &volSnap,
        std::shared_ptr<ReferrencePointResponse> &response, const std::string &diskId);
    bool DeletePreReferencePoint();
    int32_t GetVolOfVm(VMInfo &vmInfo, VolInfo &volInfo);
    int32_t InitParaAndGetTargetVolume(const ApplicationResource &targetVol,
        Json::Value &targetVolume);
    int32_t CreateNewDisk(const VolInfo &copyVol, const ApplicationResource &targetVol,
        Json::Value &targetDiskJson, VolInfo &targetVolInPair);
    int32_t TransVolumeInfoToVolInfo(const VolumeInfo &volObj, VolInfo &vmVol);
    bool ClearRemainCheckPointOfVM();
    int32_t CreateVmCheckPoint(SnapshotInfo &snapshot);
    void PreHookPostJob();
    int32_t DeleteOriginVm();
    std::string GetHardDiskName(const std::string &originDiskPath);
    std::string GenerateDiskNameForNewVm(const std::string &targetPath,
        const std::string &originDiskName, const std::string &uuid,
        const std::string &targetDiskFileFormat);
    int32_t CheckRestoreParams();

private:
    std::string m_taskId;
    AppProtect::ApplicationEnvironment m_appEnv;
    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_cacheRepoHandler = nullptr;
    std::string m_metaRepoPath;
    std::string m_cacheRepoPath;
    HyperVPlugin::PSClient m_psClient;
    std::shared_ptr<AppProtect::BackupJob> m_backupPara = nullptr;
    std::shared_ptr<AppProtect::RestoreJob> m_restorePara = nullptr;
    std::shared_ptr<RepositoryHandler> m_repoHandler = nullptr;
    bool CreateNewFile(const VolInfo &copyVol, const std::string& targetFileName);
    std::shared_ptr<WMIClient> wmiClient = nullptr;
    std::string m_snapShotId;
    RestoreLevel m_restoreLevel;
    VMInfo m_vmInfo;
    bool m_machineMetaCached { false };
    bool m_ifDeleteOriginVM = false;
};
}

#endif // __HYPERV_PROTECT_ENGINE_H__
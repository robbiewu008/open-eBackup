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
#pragma once

#include <cstdint>
#include <string>
#include "protect_engines/ProtectEngine.h"
#include "protect_engines/kubernetes/common/KubeCommonInfo.h"
#include "protect_engines/kubernetes/common/KubeMacros.h"
#include "protect_engines/kubernetes/rest/client/StorageClient.h"
#include "common/Structs.h"

using VirtPlugin::ProtectEngine;
using VirtPlugin::SnapshotInfo;
using VirtPlugin::VolumeHandler;
using VirtPlugin::VolInfo;
using VirtPlugin::DatastoreInfo;
using VirtPlugin::VMInfo;
using VirtPlugin::VolPair;
using VirtPlugin::VolMatchPairInfo;

namespace KubernetesPlugin {

class KubernetesProtectEngine : public ProtectEngine {
public:
    KubernetesProtectEngine() {}

    explicit KubernetesProtectEngine(std::shared_ptr<VirtPlugin::JobHandle> &m_jobHandle, std::string jobId,
        std::string subJobId)
        :ProtectEngine(m_jobHandle, jobId, subJobId)
    {
    }

    virtual ~KubernetesProtectEngine() {}

    /**
    *  @brief 任务的后置钩子，用于不同引擎的差异化处理
    *
    *  @return 错误码：0 成功，非0 失败
    */
    virtual int32_t PostHook(const VirtPlugin::ExecHookParam &para);

    /**
    *  @brief 任务的前置钩子，用于不同引擎的差异化处理
    *
    *  @return 错误码：0 成功，非0 失败
    */
    virtual int32_t PreHook(const VirtPlugin::ExecHookParam &para);

    /**
     *  @brief 创建生产端快照
     *  @param snapshotName [IN]快照名
     *  @param snapshot     [OUT]产生的快照信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t CreateSnapshot(SnapshotInfo &snapshot, std::string &errCode);

    /**
     *  @brief 刪除生产端快照
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
     *  @param vmMetadata [OUT]虚拟机元数据信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t GetMachineMetadata(VMInfo &vmInfo);

    /**
     *  @brief 获取卷的元数据
     *  @param vmMetadata   [IN]所属虚拟机的元数据
     *  @param volsMetadata [OUT]卷的元数据信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t GetVolumesMetadata(const VMInfo &vmInfo,
                                       std::unordered_map<std::string, std::string> &volsMetadata);

    /**
     *  @brief 获取卷的handler
     *  @param uuid [IN]卷的uuid
     *  @param volHandler [OUT]卷处理器
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t GetVolumeHandler(const VolInfo &volInfo, std::shared_ptr<VolumeHandler> &volHandler);

    /**
     *  @brief 创建卷，用于恢复时创建新卷
     *  @param volInfo    [IN]要创建的卷的元数据信息
     *  @param volInfo    [IN]要创建的卷的所在的Datastore信息
     *  @param volHandler [OUT]新创卷对应的Handler
     *  @return 错误码：0 成功，非0 失败
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
     *  @param volHandler [IN]卷对应的Handler
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t AttachVolume(const VolInfo &volObj);

    /**
     *  @brief 删除卷，将卷从虚拟机上删除掉（虚拟机信息从JobHandle中获取）
     *  @param volHandler [IN]卷对应的Handler
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t DeleteVolume(const VolInfo &volObj);

    /**
     *  @brief 替换卷，将虚拟机上的卷替换成目标卷（虚拟机信息从JobHandle中获取）
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
     *  @param vmInfo  [IN]虚拟机的信息
     *  @param newName [IN]新名称
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t RenameMachine(const VMInfo &vmInfo, const std::string &newName);

    /**
     *  @brief 上电虚拟机
     *  @param vmInfo [IN]虚拟机的信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t PowerOnMachine(const VMInfo &vmInfo);

    /**
     *  @brief 下电虚拟机
     *  @param vmInfo [IN]虚拟机的信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t PowerOffMachine(const VMInfo &vmInfo);

    /**
     *  @brief 节点是否可执行当前任务
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
    virtual int32_t AllowBackupSubJobInLocalNode(const AppProtect::BackupJob &job,
                                                 const AppProtect::SubJob &subJob, int32_t &errorCode);

    /**
     *  @brief 节点是否可执行当前任务
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
    virtual int32_t AllowRestoreSubJobInLocalNode(const AppProtect::RestoreJob &job,
                                                  const AppProtect::SubJob &subJob, int32_t &errorCode);

    /**
     *  @brief 检查备份类型
     *  @return 错误码：0 全量，非0 失败（特定错误码才执行增转全）
     */
    virtual int32_t CheckBackupJobType(const VirtPlugin::JobTypeParam &jobTypeParam, bool &checkRet);

    /**
     * @brief 在主机中按一个应用类型列出应用程序,无身份验证,同步功能
     * @param returnValue [out]
     * @param appType [IN] 应用的类型
    */
    virtual void DiscoverApplications(std::vector<Application> &returnValue, const std::string &appType);

    /**
     *  @brief 检查应用是否存在
     *  @param returnValue : [out] returnValue.code is 0, 失败：otherwise
     *  @param appEnv : [in] 受保护对象所在环境
     *  @param application : [in] 受保护对象信息
    */
    virtual void CheckApplication(ActionResult &_return, const ApplicationEnvironment &appEnv,
                                  const Application &application);

    /**
     * @brief 根据条件查询资源列表
     * @param returnValue ： [out] 资源列表
     * @param appEnv ： [in] 受保护对象所在环境
     * @param application ：[in] 受保护对象信息
     * @param parentResource： [in] 父资源信息
    */
    virtual void ListApplicationResource(
            std::vector<ApplicationResource> &returnValue, const ApplicationEnvironment &appEnv,
            const Application &application, const ApplicationResource &parentResource);

    /**
     * @brief list appliation resource by page, synchronization function, .eg query data file of database
     * @param request list resource request
     * @return application resource list in the host with one page, list is empty when no application resource exists
     */
    void ListApplicationResourceV2(ResourceResultByPage &_return, const ListResourceRequest &request);

    /**
    * @brief 查询主机集群信息
    * @param returnEnv ： [out] 远程主机列表,不存在时应用程序列表为空
    * @param appEnv ： [in] 作为群集成员的应用程序环境信息
    */
    virtual void DiscoverHostCluster(ApplicationEnvironment &returnEnv, const ApplicationEnvironment &appEnv);

    /**
    * @brief 查询应用集群信息
    * @param returnEnv ： [out] 远程主机列表,不存在时应用程序列表为空
    * @param appEnv ：[in] 作为群集成员的应用程序环境信息
    * @param application ：[in] 应用信息
    */
    virtual void DiscoverAppCluster(ApplicationEnvironment &returnEnv, const ApplicationEnvironment &appEnv,
                                    const Application &application);

    virtual int GenVolPair(VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol,
        VolMatchPairInfo &volPairs);

    int32_t CheckBeforeBackup() override;

    int32_t CheckBeforeRecover(const VMInfo &vmObj);

    /**
     * @brief 获取指定卷的之前创建的所有快照
     *
     * @param volInfo 卷信息
     * @param snapList 返回匹配的快照列表
     * @return 错误码，0执行成功，非0执行失败
     */
    virtual int32_t GetSnapshotsOfVolume(const VolInfo &volInfo, std::vector<VirtPlugin::VolSnapInfo> &snapList);

    /**
    * @brief 检查代理机与K8S生产环境的连通性
    *
    * @param env 生产环境信息
    * @return 错误码，0执行成功，非0执行失败
    */
    int32_t CheckProtectEnvConn(const AppProtect::ApplicationEnvironment &env);

    /**
    * @brief 检查代理机与卷指定生产存储的连通性
    *
    * @param volInfo 卷信息
    * @param authExtendInfo 认证信息
    * @return 错误码，0执行成功，非0执行失败
    */
    int32_t CheckStorageConnection(const VolInfo &volInfo, const std::string &authExtendInfo);

    /**
    * @brief 检查代理机的环境信息
    *
    * @return 0执行成功，非0执行失败
    */
    int32_t CheckAgentEnv();

    /**
   * @brief 检查代理机与所有的生产存储的连通性
   *
   * @param authExtendInfo 认证信息
   * @return 错误码，0执行成功，非0执行失败
   */
    int32_t CheckAllStorageConnection(const std::string &authExtendInfo);

    int32_t CheckBeforeMount() override;

    int32_t CancelLiveMount(const VMInfo &liveVm) override;

    int32_t CreateLiveMount(const VMInfo &copyVm, VMInfo &newVm) override;

    bool IfDeleteAllSnapshotWhenFailed() override
    {
        return true;
    }

private:
    void FillActionResult(ActionResult &_return, int code, int errorCode, const std::string &message,
        const std::vector<std::string> &bodyErrParams = {});

    void ThrowListFailException(const std::string &message);

    std::shared_ptr<AppProtect::BackupJob> GetBackupParam();

    std::shared_ptr<AppProtect::RestoreJob> GetRestoreParam();

    std::pair<int32_t, std::vector<std::string>> ParseVolumeNamesFromBackupParam(
            std::shared_ptr<AppProtect::BackupJob> &backupJob);

    int32_t FilterPodsByPreScript(const std::vector<ApplicationResource> &pods,
                                  const std::shared_ptr<AppProtect::BackupJob> &backupParam,
                                  std::vector<ApplicationResource> &backupPods);

    /**
     *  给定k8s环境信息+stateFulset信息，进入stateFulSet中所有Pod，执行指定脚本
     *
     * @param scriptName 脚本名称
     * @param restoreJobPtr 恢复job指针
     * @return  pair<int32_t, map<string, string>> 错误码 + 各个pod中命令执行结果(map), key: pod名 value: 命令执行结果
     */
    std::pair<int32_t, std::map<std::string, std::string> > RunScriptOnAllPodsInTheStateFulSet(
            const std::string &scriptName, const std::shared_ptr<AppProtect::RestoreJob> &restoreJobPtr,
            TaskStage stage);

    int32_t GenerateStorageParams(const std::string &appEnvAuthExtendInfo, std::vector<StorageParam> &storages);

    int32_t FindOutTheBackupStorage(const std::vector<StorageParam> &storages,
                                    std::vector<BatchSnapshotParam> &createParams);

    int32_t TakeConsistentSnapshots(BatchSnapshotParam &batchSnapshotInfo,
                                    std::map <std::string, ConsistentActivationInfo> &esnMap);

    int32_t ActiveConsistentSnapshots(std::map <std::string, ConsistentActivationInfo> &esnMap);

    void GenerateVolSnapInfo(const BatchSnapshotParam &batchSnapshotInfo, SnapshotInfo &snapshot);

    int32_t FingerOutBackupPvs(const std::shared_ptr<AppProtect::BackupJob> &backupJob,
                               const std::vector<std::string> &volumeNames,
                               std::vector<BatchSnapshotParam> &createParams);

    int32_t CreateSnapshot(const Pv &pv, std::shared_ptr<StorageClient> storageClient, unsigned long long timestamp,
                           std::vector<std::string> &snapshotIds, std::vector<PerSnapshotParam> &snapshotParams);

    int32_t ParseBackupPvsAndStorage(std::vector<BatchSnapshotParam> &createParams, const bool &labelFlag = false);

    int32_t BackupPostHook(const std::string &scriptPath, TaskStage stage);

    void GenerateLabel(JobLogLevel::type level, const std::string &label, const std::vector<std::string> &params);

    bool GenerateScriptLabel(const std::string &excResult, const std::string &podName, const std::string &containerName,
                             const std::string &script, TaskStage stage);

    int32_t PostScriptTaskForRestore(const int32_t jobExecResult);

    int32_t PostScriptTaskForBackup(const int32_t jobExecResult);

    int32_t AddStsNameInApplicationExtendInfo(const ListResourceRequest &request, Application &application);

    int32_t CheckEveryStorage(ActionResult &_return, const std::vector<StorageParam> &storageParamList,
        const std::set<std::string> &storageUrlSet);

    int32_t PreDeleteSnapshot(const VirtPlugin::VolSnapInfo &item, std::shared_ptr<StorageClient> storageClient);

    int32_t CheckAndReportStorage(const std::set<std::string> &overthresholdPoolNameSet, const int32_t &storageLimit,
        const BatchSnapshotParam &batchSnapshotInfo);

    int32_t GetStorageLimit();

    int32_t GetPoolIdByLunName(std::shared_ptr<StorageClient> storageClient, const std::string &lunName,
        StoragePoolData &poolData, std::map <std::string, std::set<std::string>> &poolMap);

    int32_t CheckStorageThreshold(const BatchSnapshotParam &batchSnapshotInfo,
        std::map <std::string, std::set<std::string>> &poolMap, const int32_t &storageLimit);

    int32_t GetPvcLunInfo(BatchSnapshotParam &createParam);

    std::vector<BatchSnapshotParam> m_snapshotCache;
    std::unordered_map<std::string, VolInfo> m_restoreVolMap;  // 待恢复卷Map key-源卷的uuid, value-目标卷的VolInfo

    bool m_noNeedInitOceanVolHandler { false };
    bool m_metaDataCached { false };
};
}

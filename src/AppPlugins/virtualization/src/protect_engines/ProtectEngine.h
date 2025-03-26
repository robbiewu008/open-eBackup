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
#ifndef _PROTECT_ENGINE_H_
#define _PROTECT_ENGINE_H_

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "common/Structs.h"
#include "common/Macros.h"
#include "common/Constants.h"
#include "common/utils/Utils.h"
#include "job/JobCommonInfo.h"
#include "volume_handlers/VolumeHandler.h"

using AppProtect::ApplicationEnvironment;
using AppProtect::ResourceResultByPage;
using AppProtect::Application;
using AppProtect::ApplicationResource;
using AppProtect::ActionResult;
using AppProtect::ListResourceRequest;
using AppProtect::JobLogLevel;

namespace VirtPlugin {

class ProtectEngine {
public:
    ProtectEngine()
    {
    }
    ProtectEngine(std::shared_ptr<JobHandle> jobHandle, std::string jobId, std::string subJobId)
        : m_jobHandle(jobHandle), m_jobId(jobId), m_subJobId(subJobId)
    {
    }

    virtual ~ProtectEngine()
    {
    }

    /**
     *  @brief 获取任务句柄，从任务句柄中可以获取所需的信息
     *
     *  @return JobHandle 任务句柄对象
     */
    std::shared_ptr<JobHandle> GetJobHandle() const
    {
        return this->m_jobHandle;
    };

    void SetJobHandle(std::shared_ptr<JobHandle> jobHandle)
    {
        if (jobHandle == nullptr) {
            ERRLOG("JobHandle is null.");
            return;
        }
        m_jobHandle = jobHandle;
    };

    /**
     *  @brief 任务的前置钩子，用于不同引擎的差异化处理
     *
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t PreHook(const ExecHookParam &para)
    {
        return SUCCESS;
    }

    /**
     *  @brief 任务的后置钩子，用于不同引擎的差异化处理
     *
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t PostHook(const ExecHookParam &para)
    {
        return SUCCESS;
    }

    /**
     * @brief 恢复前检查
     *
     * @param vmObj 附本虚拟机信息
     * @return int32_t
     */
    virtual int32_t CheckBeforeRecover(const VMInfo &vmObj) = 0;

    /**
     * @brief 备份前检查
     *
     * @return int32_t
     */
    virtual int32_t CheckBeforeBackup()
    {
        return SUCCESS;
    }

    /**
     *  @brief 创建生产端快照
     *  Notice!
     *  X-series all-in-one DataBackup Box ESN needs to be set to snapshot description when creating snapshot,
     *  so that identifying specific snapshots when deleting.
     *  @param snapshot     [OUT]产生的快照信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t CreateSnapshot(SnapshotInfo &snapshot, std::string &errCode) = 0;

    /**
     *  @brief 刪除生产端快照
     *
     *  @param snapshot [IN]要删除的快照信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t DeleteSnapshot(const SnapshotInfo &snapshot) = 0;

    /**
     *  @brief 查询虚拟机快照
     *
     *  @param snapshot [IN]要查询的快照
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t QuerySnapshotExists(SnapshotInfo &snapshot) = 0;

    /**
     *  @brief 获取虚拟机的元数据
     *
     *  @param vmMetadata [OUT]虚拟机元数据信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t GetMachineMetadata(VMInfo &vmInfo) = 0;

    /**
     *  @brief 获取卷的元数据
     *
     *  @param vmMetadata   [IN]所属虚拟机的信息
     *  @param volsMetadata [OUT]卷的元数据信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t GetVolumesMetadata(const VMInfo &vmInfo,
                                       std::unordered_map<std::string, std::string> &volsMetadata) = 0;

    /**
     *  @brief 获取卷的handler
     *
     *  @param volInfo [IN]卷信息
     *  @param volHandler [OUT]卷处理器
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t GetVolumeHandler(const VolInfo &volInfo, std::shared_ptr<VolumeHandler> &volHandler) = 0;

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
                                 const DatastoreInfo &dsInfo, VolInfo &newVol) = 0;

    /**
     * @brief 从生产存储卸载卷，用于卷挂载到备份客户端
     *
     * @param volHandler
     * @return int32_t
     */
    virtual int32_t DetachVolume(const VolInfo &volObj) = 0;

    /**
     *  @brief 挂载卷，将卷挂载到虚拟机（虚拟机信息从JobHandle中获取）
     *
     *  @param volHandler [IN]卷对应的Handler
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t AttachVolume(const VolInfo &volObj) = 0;

    /**
     *  @brief 删除卷，将卷从虚拟机上删除掉（虚拟机信息从JobHandle中获取）
     *
     *  @param volHandler [IN]卷对应的Handler
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t DeleteVolume(const VolInfo &volObj) = 0;

    /**
     *  @brief 替换卷，将虚拟机上的卷替换成目标卷(虚拟机信息从JobHandle中获取)
     *
     *  @param tgtVolHandler [IN]目标卷对应的Handler
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t ReplaceVolume(const VolInfo &volObj) = 0;

    /**
     *  @brief 创建虚拟机
     *
     *  @param vmInfo [IN,OUT]入参包含要创建虚拟机的基本信息和卷元数据信息；出参包含创建的虚拟机的标识moref或uri
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t CreateMachine(VMInfo &vmInfo) = 0;

    /**
     *  @brief 删除虚拟机
     *
     *  @param vmInfo [IN]要删除的虚拟机的信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t DeleteMachine(const VMInfo &vmInfo) = 0;

    /**
     *  @brief 重命名虚拟机
     *
     *  @param vmInfo  [IN]虚拟机的信息
     *  @param newName [IN]新名称
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t RenameMachine(const VMInfo &vmInfo, const std::string &newName) = 0;

    /**
     *  @brief 上电虚拟机
     *
     *  @param vmInfo [IN]虚拟机的信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t PowerOnMachine(const VMInfo &vmInfo) = 0;

    /**
     *  @brief 下电虚拟机
     *
     *  @param vmInfo [IN]虚拟机的信息
     *  @return 错误码：0 成功，非0 失败
     */

    virtual int32_t PowerOffMachine(const VMInfo &vmInfo) = 0;

    /**
     *  @brief 节点是否可执行当前任务
     *
     *  @param job [IN]任务参数
     *  @return 错误码：0 可执行，非0 不可执行
     */
    virtual int32_t AllowBackupInLocalNode(const AppProtect::BackupJob &job, int32_t &errorCode) = 0;

    /**
     *  @brief 节点是否可执行当前备份子任务
     *
     *  @param job [IN]主任务参数
     *  @param subJob [IN]子任务参数
     *  @return 错误码：0 可执行，非0 不可执行
     */
    virtual int32_t AllowBackupSubJobInLocalNode(const AppProtect::BackupJob& job,
        const AppProtect::SubJob& subJob, int32_t &errorCode) = 0;

    /**
     *  @brief 节点是否可执行当前恢复任务
     *
     *  @param job [IN]任务参数
     *  @return 错误码：0 可执行，非0 不可执行
     */
    virtual int32_t AllowRestoreInLocalNode(const AppProtect::RestoreJob &job, int32_t &errorCode) = 0;

    /**
     *  @brief 节点是否可执行当前恢复子任务
     *
     *  @param job [IN]主任务参数
     *  @param subJob [IN]子任务参数
     *  @return 错误码：0 可执行，非0 不可执行
     */
    virtual int32_t AllowRestoreSubJobInLocalNode(const AppProtect::RestoreJob& job,
        const AppProtect::SubJob& subJob, int32_t &errorCode) = 0;

    /**
     *  @brief 检查备份类型
     *
     *  @param jobTypeParam [IN]任务参数，快照信息
     *  @param checkRet [OUT]检查结果，true-本节点可执行，false-本节点不可执行
     *  @return 错误码：0 全量，非0 失败（特定错误码才执行增转全）
     */
    virtual int32_t CheckBackupJobType(const JobTypeParam &jobTypeParam, bool &checkRet) = 0;

    /**
     * @brief 在主机中按一个应用类型列出应用程序,无身份验证,同步功能
     *
     * @param returnValue [out]
     * @param appType [IN] 应用的类型
     */
    virtual void DiscoverApplications(std::vector<Application> &returnValue, const std::string &appType) = 0;

    /**
     *  @brief 检查应用是否存在
     *
     *  @param returnValue : [out] returnValue.code is 0, 失败：otherwise
     *  @param appEnv : [in] 受保护对象所在环境
     *  @param application : [in] 受保护对象信息
     */
    virtual void CheckApplication(ActionResult &returnValue, const ApplicationEnvironment &appEnv,
                                  const AppProtect::Application &application) = 0;

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
                                         const ApplicationResource &parentResource) = 0;

    virtual void ListApplicationResourceV2(ResourceResultByPage &page, const ListResourceRequest &request) = 0;

    /**
     * @brief 查询主机集群信息
     *
     * @param returnEnv ： [out] 远程主机列表,不存在时应用程序列表为空
     * @param appEnv ： [in] 作为群集成员的应用程序环境信息
     */
    virtual void DiscoverHostCluster(ApplicationEnvironment &returnEnv, const ApplicationEnvironment &appEnv) = 0;

    /**
     * @brief 查询应用集群信息
     *
     * @param returnEnv ： [out] 远程主机列表,不存在时应用程序列表为空
     * @param appEnv ：[in] 作为群集成员的应用程序环境信息
     * @param application ：[in] 应用信息
     */
    virtual void DiscoverAppCluster(ApplicationEnvironment &returnEnv, const ApplicationEnvironment &appEnv,
                                    const Application &application) = 0;

    /**
     * @brief 生成卷映射匹配对信息
     *
     * @param vmObj 目标虚拟机对象
     * @param volumePair 卷匹配对，也做出参填充恢复目标卷信息
     * @param volinfo 源卷信息
     * @return int
     */
    virtual int GenVolPair(VMInfo &vmObj, const VolInfo &copyVol, const ApplicationResource &targetVol,
        VolMatchPairInfo &volPairs) = 0;

    virtual void SetReportJobDetailHandler(const std::function<void(const ApplicationLabelType &)> &handler) final
    {
        m_reportJobDetailHandler = handler;
    }

    /**
     *  @brief 节点是否可执行当前任务
     *
     *  @param job [IN]任务参数
     *  @return 错误码：0 可执行，非0 不可执行
     */
    virtual int32_t AllowDelCopyInLocalNode(const AppProtect::DelCopyJob &job, ActionResult& returnValue)
    {
        return SUCCESS;
    }

    /**
     *  @brief 节点是否可执行当前备份子任务
     *
     *  @param job [IN]主任务参数
     *  @param subJob [IN]子任务参数
     *  @return 错误码：0 可执行，非0 不可执行
     */
    virtual int32_t AllowDelCopySubJobInLocalNode(const AppProtect::DelCopyJob& job,
        const AppProtect::SubJob& subJob, ActionResult& returnValue)
    {
        return SUCCESS;
    }

    /**
     * @brief 获取指定卷之前创建的所有快照
     *
     * @param volInfo 卷信息
     * @param snapList 返回匹配的快照列表
     * @return 错误码，0执行成功，非0执行失败
     */
    virtual int32_t GetSnapshotsOfVolume(const VolInfo &volInfo, std::vector<VolSnapInfo> &snapList) = 0;

    /**
     * @brief 挂载前检查 检查虚拟机名称是否重复
     *
     * @param
     * @return 错误码，0执行成功，非0执行失败
     */
    virtual int32_t CheckBeforeMount() = 0;

    /**
     * @brief 取消即时挂载
     *
     * @param liveVm 即时挂载元数据
     * @return 错误码，0执行成功，非0执行失败
     */
    virtual int32_t CancelLiveMount(const VMInfo &liveVm) = 0;

     /**
     * @brief 创建即时挂载
     *
     * @param copyVm 副本元数据
     * @param newVm 新创建即时挂载虚拟机元数据
     * @return 错误码，0执行成功，非0执行失败
     */
    virtual int32_t CreateLiveMount(const VMInfo &copyVm, VMInfo &newVm) = 0;

     /**
     * @brief 挂载前检查 检查虚拟机名称是否重复
     *
     * @param liveVm 即时挂载元数据
     * @return 错误码，0执行成功，非0执行失败
     */
    virtual int32_t MigrateLiveVolume(const VMInfo &liveVm)
    {
        return SUCCESS;
    }

    virtual int32_t GetWhiteListForLivemount(std::string &ipStr)
    {
        return SUCCESS;
    }

    /**
     * @brief 卸载前检查 检查虚拟机状态是否支持卸载
     *
     * @param
     * @return 错误码，0执行成功，非0执行失败
     */
    virtual int32_t CheckBeforeUnmount()
    {
        return FAILED;
    }

    /**
     * @brief 任务前检查 检查资源状态是否支持任务执行
     *
     * @param
     * @return 错误码，0执行成功，非0执行失败
     */
    virtual int32_t CheckResourceUsage(const std::string &resourceId)
    {
        return SUCCESS;
    }

    /**
     * @brief 获取上报信息
     */
    virtual void GetReportParam(ReportJobDetailsParam &reportParam)
    {
        reportParam = m_reportParam;
        m_reportParam.label = "";
    }

    /**
     * @brief 获取上报信息参数
     */
    virtual void GetReportArgs(std::vector<std::string> &reportArgs)
    {
        reportArgs = m_reportArgs;
        m_reportArgs.clear();
    }

    virtual int32_t ActiveSnapConsistency(const SnapshotInfo &m_snapshotInfo, int32_t &erroCode)
    {
        return SUCCESS;
    }

    virtual void SetLiveMountType(const LivemountType &livemountType)
    {
        m_livemountType = livemountType;
    }

    virtual void SetXNNEsn(const std::string &esn, const std::string &taskId) final
    {
        m_xNNEsn = esn;
        /* Update snap description */
        m_snapDescription = m_xNNEsn + SNAP_DESCRIPTION_REMINDER + "_" + taskId;
    }
 
    virtual bool GetifDeleteSnapshot(AppProtect::BackupJobType backuptype,  AppProtect::JobResult::type res)
    {
        if (backuptype == AppProtect::BackupJobType::FULL_BACKUP && res == AppProtect::JobResult::SUCCESS) {
            return true;
        }
        return false;
    }

    virtual bool IfDeleteLatestSnapShot()
    {
        return false;
    }

    virtual bool DeleteSnapshotCreateVolumes(const VolSnapInfo &snapInfo)
    {
        return true;
    }

    virtual void SetErrorCodeParam(const int32_t errorCode, std::vector<std::string> &certParams)
    {
        return;
    }

    virtual void ClearLabel()
    {
        m_reportParam.label = "";
        m_reportArgs = {};
        return;
    }

    virtual void SetJobResult(const AppProtect::JobResult::type &result) final
    {
        m_jobResult = result;
    }

    virtual int32_t RestoreVolMetadata(VolMatchPairInfo &volPairs, const VMInfo &vmInfo)
    {
        return SUCCESS;
    }

    virtual std::vector<std::string> GetNoTasksArgs()
    {
        return m_noTasksArgs;
    }

    virtual void SetNoTasksArgs(const std::vector<std::string> &argsList)
    {
        m_noTasksArgs = argsList;
        m_isSetArgs = true;
    }

    virtual void ThrowPluginException(
        const int32_t &code = FAILED,
        const std::string &message = "",
        const std::vector<std::string> &codeParam = {})
    {
        AppProtect::AppProtectPluginException exception;
        exception.__set_code(code);
        exception.__set_message(message);
        exception.__set_codeParams(codeParam);
        throw exception;
    }

    virtual bool IfDeleteAllSnapshotWhenFailed()
    {
        return false;
    }

protected:
    virtual void ReportJobDetail(const ApplicationLabelType &appLable) final
    {
        if (!m_reportJobDetailHandler) {
            WARNLOG("Report job detail handler not provided.");
        } else {
            m_reportJobDetailHandler(appLable);
        }
    }

    virtual std::string GenerateSnapshotName(const std::string &lunId) final
    {
        return "Protect_" + lunId + "_SNAP_" + std::to_string(time(0));
    }

    virtual bool MatchSnapshotName(const std::string &snapshotName) final
    {
        std::regex reg("^Protect_.*_SNAP_[0-9]{10,}$");
        bool ret = std::regex_match(snapshotName, reg);
        if (ret) {
            DBGLOG("The snapshot name matched, snapshotName: %s", snapshotName.c_str());
            return true;
        }
        return false;
    }

    virtual bool MatchSnapshotDescription(const std::string &description) final
    {
        std::string::size_type pos = description.find(m_xNNEsn);
        if (pos != std::string::npos) {
            DBGLOG("The snapshot description matched, description: %s", description.c_str());
            return true;
        }
        return false;
    }

protected:
    std::shared_ptr<JobHandle> m_jobHandle;
    std::vector<ApplicationLabelType> m_appLabelsToReport;
    std::function<void(const ApplicationLabelType &)> m_reportJobDetailHandler;
    std::vector<std::string> m_reportArgs;
    ReportJobDetailsParam m_reportParam;
    std::vector<std::string> m_noTasksArgs;
    bool m_isSetArgs { false };
    /* X Series all-in-one Box ESN */
    std::string m_xNNEsn;
    std::string m_snapDescription;
    LivemountType m_livemountType = LivemountType::UNKNOWN;

    std::string m_jobId;
    std::string m_subJobId;
    AppProtect::JobResult::type m_jobResult {AppProtect::JobResult::type::SUCCESS};
};
}

#endif  // _PROTECT_ENGINE_H_
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

#include <ctime>
#include <protect_engines/ProtectEngine.h>
#include <protect_engines/kubernetes/common/KubeCommonInfo.h>
#include <protect_engines/kubernetes/common/KubeErrorCodes.h>
#include <protect_engines/kubernetes/common/KubeMacros.h>
#include <protect_engines/kubernetes/rest/KubernetesApi.h>
#include "protect_engines/kubernetes/rest/client/StorageClient.h"
#include "common/Structs.h"

namespace KubernetesPlugin {

using VirtPlugin::VolInfo;
using VirtPlugin::DatastoreInfo;

namespace KubeHelper {
const std::string MODULE_NAME = "KubeHelper";
const int LEN_POD_INFO = 8;

/**
 * 将ApplicationEnvironment.auth.extendInfo生成kubernentesApi 对应API
 *
 * @param appEnvAuthExtend ApplicationEnvironment.auth.extendInfo
 * @return KubernetesApi 或 std::nullopt
 */
std::optional<KubernetesApi> GetKubernetesApiFromAppEnv(const std::string &appEnvAuthExtend);

/**
 * 将ApplicationEnvironment生成存储Vector
 *
 * @param appEnvAuthExtend ApplicationEnvironment.auth.extendInfo
 * @return std::vector<StorageParam> 存储信息 或 std::nullopt
 */
std::optional<std::vector<StorageParam>> GetStorageParamVecFromAppEnv(const std::string &appEnvAuthExtend);

/**
 * 将StorageParam转化为StorageClient
 *
 * @param storageParam
 * @return StorageClient 存储信息 或 相关错误码
 */
std::pair<int32_t, std::shared_ptr<StorageClient>> GetStorageClientFromStorageParam(const StorageParam &storageParam);

/**
 * 将StorageParam转化为StorageClient，登录用，没有失败重试，返回错误ip
 *
 * @param storageParam
 * @param accessAuthParam
 * @return StorageClient 存储信息 或 相关错误码
 */
std::pair<int32_t, std::shared_ptr<StorageClient>> CheckStorageClientFromStorageParam(
    KubernetesPlugin::AccessAuthParam accessAuthParam, const StorageParam &storageParam);

/**
 * 从StorageParamList列表中选择符合条件的存储信息
 *
 * @param StorageParamList， 存储参数列表
 * @param volExtendInfo, 恢复卷的扩展信息
 * @return 某个符合条件的存储参数
 */
std::optional<StorageParam> GetStorageParamFromVol(std::vector<StorageParam> StorageParamList,
                                                   std::string volExtendInfo);

/**
 * 根据 storageDeviceInfo、lunInfoData、storageParam填充volInfo
 *
 * @param volInfo 出参 VolInfo
 * @param storageDeviceInfo 入参 存储设备信息
 * @param lunInfoData 入参 lun信息
 * @param storageParam 入参 存储ip信息
 */
void FulfillVolInfo(VolInfo &volInfo, const StorageDeviceInfo &storageDeviceInfo,
                    const LunInfoData &lunInfoData, const StorageParam &storageParam);

/**
 * 根据 lunInfoData信息填充部分volInfo
 *
 * @param volInfo
 * @param lunInfoData
 */
void FulfillVolInfo(VolInfo &volInfo, const LunInfoData &lunInfoData);

/**
 * 根据保护对象extendInfo转换为StateFulSet内部结构
 *
 * @param ApplicationResource.extendInfo 转换为StateFulSet
 * @return StateFulSet信息 或 std::nullopt
 */
std::optional<StateFulSet> GetStateFulSetFromExtendInfo(const std::string &appExtend);

/**
 *  恢复时使用，根据指定的volResource，生成目标VolInfo。
 *
 * @param volResource 源卷信息
 * @param storageParam 目的环境的存储参数
 * @param storageClient 目的环境的存储Client
 * @return
 */
std::optional<VolInfo> GenTargetVolFromInfos(const ApplicationResource &volResource,
                                             const StorageParam &storageParam,
                                             std::shared_ptr<StorageClient> storageClient);

/**
 * 将pod信息从pvc名称中删除
 * 为CBS应用定制函数。将尾部8个字节去除， -1-1-m-0。
 *  例：
 *  输入 gmdbredo-adaptermdb-1-1-m-0
 *  输出 gmdbredo-adaptermdb
 *
 *  输入 gmdbredo-adaptermdb-1-2-s-1
 *  输出 gmdbredo-adaptermdb
 *
 * @param pvcName
 * @return 不带pod信息的pvc名称。
 */
std::string GetPvcNameWithOutPodInfo(const std::string pvcName);

/**
 *  从extendInfo中取出脚本
 *
 * @param appExtend 保护环境中的string
 * @param scriptType 脚本类型
 * @return 脚本名称
 */
std::optional<std::string> GetScriptFromExtendInfo(const std::string &appExtend, ScriptType scriptType);


/**
 * 检查kube命令执行结果是否包含关键词
 *
 * @param kubeExecRet kube命令执行结果
 * @return 是否
 */
bool CheckKubeExecRetContainKey(const std::string &kubeExecRet);

/**
 * 时间戳格式化
 *
 * @param timeStamp 时间戳
 * @return 格式化后字符串
 */
std::string ConvertTimeStamp2TimeStr(time_t timeStamp);

/**
 * 在存储集合中，找到sn匹配的存储
 *
 * @param storages 存储信息
 * @param sn 存储SN
 * @return 匹配的存储
 */
std::optional<StorageParam> FindMatchSnStorage(const std::vector<StorageParam> &storages, const std::string &sn);

/**
 * 在存储池map中，找到sn匹配的存储池id
 *
 * @param poolMap 存储池map
 * @param poolId 存储池id
 * @param sn 存储SN
 * @return 是否找到id
 */
bool FindStorageIdInMap(std::map <std::string, std::set<std::string>> &poolMap, const std::string &poolId,
    const std::string &sn);

/**
 * 从备份任务参数中找出匹配
 * @param snapshotInfo 快照信息（创建快照后才有）
 * @param backupParam 备份任务参数
 * @return 存储客户端
 */
std::unordered_map<std::string, std::shared_ptr<StorageClient>> FindMatchSnapInfoStorageClients(
    const VirtPlugin::SnapshotInfo &snapshotInfo,
    const std::shared_ptr<AppProtect::BackupJob> &backupParam);

/**
 * 检查POD中的PVs是都在同一个存储上
 *
 * @param createParam 创建参数
 * @return 是否满足要求
 */
std::pair<bool, std::string> CheckPodPvsInSameStorage(BatchSnapshotParam &createParam);

/**
 * 快照信息生成VolInfo
 *
 * @param snapshotInfo 单个快照信息
 * @param backupParam 备份参数
 * @return 是否成功，VolInfo
 */
std::pair<int32_t, VolInfo> GenerateVolInfo(PerSnapshotParam snapshotInfo,
                                            std::shared_ptr<AppProtect::BackupJob> &backupParam,
                                            BatchSnapshotParam &createParam);

/**
 * 用“, ”串联卷名称
 *
 * @param createParam 创建快照参数
 * @return 卷名称
 */
std::string JoinLunNames(const BatchSnapshotParam &createParam);

/**
 * 获取没有找到Pv的卷名称
 *
 * @param volumeNames 卷名称数组
 * @param pvs pv信息
 * @return 没有找到Pv的卷名称（多个卷之间用“，”分隔）
 */
std::string FindLostVolumeNamesInPvs(const std::vector<std::string> &volumeNames, const std::vector<Pv> &pvs);

/**
 * 根据执行的脚本结果获取标签信息
 *
 * @param excResult 执行脚本的结果信息
 * @param stage 任务阶段
 * @return first：脚本是否执行成功；second：标签信息
 */
std::pair<bool, std::string> GetLabelByExecKubeResult(const std::string &excResult, const TaskStage &stage);

/**
 * 获取前置脚本执行的结果
 *
 * @param preScriptExecRet 前置脚本的结果信息
 * @return 前置脚本结果
 */
std::string GetKubePreScriptExecResult(const std::string &preScriptExecRet);
}
}
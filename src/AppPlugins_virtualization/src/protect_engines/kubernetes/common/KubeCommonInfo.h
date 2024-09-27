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
#ifndef KUBE_COMMON_INFO_H
#define KUBE_COMMON_INFO_H

#include <ApplicationProtectBaseDataType_types.h>
#include <string>
#include <set>
#include <vector>
#include "common/JsonHelper.h"
#include "common/CleanMemPwd.h"
#include "common/utils/Utils.h"
#include "KubeMacros.h"
namespace KubernetesPlugin {

/**
 *  http应答信息
 */
struct HttpResponseInfo {
    HttpResponseInfo() : m_version(""), m_statusCode(0), m_statusString(""), m_body("")
    {
        m_heads.clear();
    }

    std::string m_version;
    uint32_t m_statusCode;
    std::string m_statusString;
    std::string m_body;
    std::set<std::pair<std::string, std::string> > m_heads;
    std::set<std::string> cookies;
};

/**
 *  接入认证参数
 */
struct AccessAuthParam {
    AccessAuthParam() {}

    AccessAuthParam(const std::string &user, const std::string &pwd, const std::string &scope)
        : m_userName(user), m_userkey(pwd), m_scope(scope) {}

    std::string m_userName;
    std::string m_userkey;
    std::string m_scope;
};

/**
 * 存储设备参数
 */
struct StorageParam {
    std::string username;
    std::string password;
    std::string ip;
    int32_t port;
    std::string sn;
    std::string ipList;

    ~StorageParam()
    {
        Module::CleanMemoryPwd(password);
        Module::CleanMemoryPwd(password);
        Module::CleanMemoryPwd(password);
    }

    bool operator==(const StorageParam &that) const
    {
        return (that.sn == sn);
    }

    void GetStorageUrl(std::vector<std::string> &storageUrlList) const
    {
        std::vector<std::string> ipVector;
        VirtPlugin::Utils::GetIpVectorFromString(ipVector, ip, ipList);
        for (const auto &singleIp : ipVector) {
            storageUrlList.push_back("https://" + singleIp + ":" +
                std::to_string(port) + "/deviceManager/rest");
        }
        return;
    }

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(username, username)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(password, password)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(ip, ip)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(port, port)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(sn, sn)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(ipList, ipList)
    END_SERIAL_MEMEBER
};

/**
 * k8s集群扩展参数
 */
struct AppEnvExtendInfo {
    std::string config;
    std::string storages;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(config, config)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(storages, storages)
    END_SERIAL_MEMEBER
};

/**
 * flexvolume PV 结构，用于statefulSet资源扩展参数 -> Pod -> PV
 */
struct Pv {
    std::string name;
    std::string volumeName;
    std::string pvcName;
    std::string lunName;
    std::string size;
    std::string storageUrl;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeName, volumeName)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(pvcName, pvcName)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(lunName, lunName)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(size, size)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(storageUrl, storageUrl)
    END_SERIAL_MEMEBER
};

/**
 * flexvolume Pod 结构，用于statefulSet资源扩展参数 -> Pod
 */
struct Pod {
    std::string name;
    std::vector<std::string> containerNames;
    std::vector<Pv> pvs; // vector<Pv> -> JsonString

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(containerNames, containerNames)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(pvs, pvs)
    END_SERIAL_MEMEBER
};

/**
 * k8s StateFulSet API结构
 */
struct StateFulSet {
    std::string id;
    std::string name;
    std::string nameSpace;
    int replicasNum;
    std::vector<std::string> volumeNames;
    std::vector<Pod> pods;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(id, id)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(nameSpace, nameSpace)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(replicasNum, replicasNum)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeNames, volumeNames)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(pods, pods)
    END_SERIAL_MEMEBER
};

/**
 * k8s StateFulSet Resource扩展参数
 */
struct StateFulSetExtend {
    std::string sts;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(sts, sts)
    END_SERIAL_MEMEBER
};

/**
 * k8s 恢复时 subobject 的 Pv 扩展参数
 */
struct PvExtend {
    std::string pv;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(pv, pv)
    END_SERIAL_MEMEBER
};

/**
 * storage device auth data
 */
struct StorageDeviceAuthData {
    std::string iBaseToken;
    std::string deviceId;
    std::set<std::string> cookies;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(iBaseToken, iBaseToken)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceId, deviceid)
    END_SERIAL_MEMEBER
};

/**
 * storage response error
 */
struct StorageResponseError {
    int code = -1;
    std::string description;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(code, code)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(description, description)
    END_SERIAL_MEMEBER
};

/**
 * storage device auth response
 */
struct StorageDeviceAuthResponse {
    StorageDeviceAuthData storageDeviceAuthData;
    StorageResponseError error;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(storageDeviceAuthData, data)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(error, error)
    END_SERIAL_MEMEBER
};

struct LunGroupInfo {
    std::string id;
    std::string description;
    std::string isAdd2MappingView;
    std::string name;
    std::string smartQosPolicyId;
    std::string type;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(id, ID)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(description, DESCRIPTION)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(isAdd2MappingView, ISADD2MAPPINGVIEW)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(name, NAME)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(smartQosPolicyId, SMARTQOSPOLICYID)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(type, TYPE)
    END_SERIAL_MEMEBER
};

struct LunGroupInfoResponse {
    std::vector<LunGroupInfo> data;
    StorageResponseError error;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(data, data)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(error, error)
    END_SERIAL_MEMEBER
};

/**
 * storage device base info data
 */
struct StorageDeviceBaseInfoData {
    std::string name;
    std::string wwn;
    std::string productMode;
    std::string productVersion;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(name, NAME)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(wwn, wwn)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(productMode, PRODUCTMODE)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(productVersion, PRODUCTVERSION)
    END_SERIAL_MEMEBER
};

/**
 * storage device base info
 */
struct StorageDeviceBaseInfoResponse {
    StorageResponseError error;
    StorageDeviceBaseInfoData baseInfoData;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(baseInfoData, data)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(error, error)
    END_SERIAL_MEMEBER
};

struct StorageDeviceInfo {
    std::string name;
    std::string sn;
    std::string version;
    std::string productMode;
    std::string productVersion;
};

struct StorageCommonResponse {
    StorageResponseError m_error;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_error, error)
    END_SERIAL_MEMEBER
};

/**
 * k8s备份任务扩展参数
 */
struct BackupJobParamExtendInfo {
    std::string m_moRef;                       // statefulset ID
    std::string m_volumeNames;                 // 用户选择的卷名称
    std::string m_preScript;
    std::string m_postScript;
    std::string m_failedScript;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeNames, volumeNames)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_moRef, vmRef)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_preScript, pre_script)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_postScript, post_script)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_failedScript, failed_script)
    END_SERIAL_MEMEBER
};

/**
 * k8s任务，目标stateFulSet的扩展参数(含前置、后置、失败脚本）
 */
struct ScriptExtendInfo {
    std::string m_preScript;
    std::string m_postScript;
    std::string m_failedScript;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_preScript, pre_script)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_postScript, post_script)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_failedScript, failed_script)
    END_SERIAL_MEMEBER
};

enum class ScriptType {
    PRE_SCRIPT, POST_SCRIPT, FAILED_SCRIPT
};

/**
 * LUN info data
 */
struct LunInfoData {
    std::string m_id;
    std::string m_name;
    std::string m_wwn;
    std::string m_parentId;
    std::string m_sectorSize;
    std::string m_capacity;
    std::string m_subtype;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, ID)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, NAME)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_wwn, WWN)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_parentId, PARENTID)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sectorSize, SECTORSIZE)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_capacity, CAPACITY)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_subtype, SUBTYPE)
    END_SERIAL_MEMEBER
};

/**
 * LUN base info
 */
struct LunInfoResponse {
    StorageResponseError m_error;
    std::vector<LunInfoData> m_lunInfoDatas;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lunInfoDatas, data)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_error, error)
    END_SERIAL_MEMEBER
};

struct StoragePoolData {
    std::string m_id;
    std::string m_name;
    std::string m_parentId;
    int32_t m_parentType;
    std::string m_healthStatus;
    std::string m_userTotalCapaity;
    std::string m_userFreeCapacity;
    std::string m_usageType;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, ID)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, NAME)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_parentId, PARENTID)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_parentType, PARENTTYPE)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_healthStatus, HEALTHSTATUS)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userTotalCapaity, USERTOTALCAPACITY)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userFreeCapacity, USERFREECAPACITY)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_usageType, USAGETYPE)
    END_SERIAL_MEMEBER
};

/**
 * Storage pool info
 */
struct StoragePoolDResponse {
    StorageResponseError m_error;
    StoragePoolData m_storagePoolDatas;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storagePoolDatas, data)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_error, error)
    END_SERIAL_MEMEBER
};

/**
 * 创建快照参数
 */
struct SnapshotCreateParam {
    std::string m_lunId;
    std::string m_name;
    std::string m_description;
    int32_t m_type = 11;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lunId, PARENTID)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_type, PARENTTYPE)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, NAME)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, DESCRIPTION)
    END_SERIAL_MEMEBER
};

/**
 * snapshot info
 */
struct SnapshotInfoData {
    std::string m_id;
    std::string m_name;
    std::string m_runningStatus;
    std::string m_wwn;
    std::string m_description;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, ID)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, NAME)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_runningStatus, RUNNINGSTATUS)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_wwn, WWN)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, DESCRIPTION)
    END_SERIAL_MEMEBER
};

/**
 * 批量查询快照参数
 */
struct SnapshotInfoResponse {
    StorageResponseError m_error;
    std::vector<SnapshotInfoData> m_SnapshotInfoDatas;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_SnapshotInfoDatas, data)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_error, error)
    END_SERIAL_MEMEBER
};

/**
 * LUN base info
 */
struct SnapshotCreateResponse {
    StorageResponseError m_error;
    SnapshotInfoData m_snapshotInfoData;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotInfoData, data)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_error, error)
    END_SERIAL_MEMEBER
};

/**
 * 快照批量激活参数
 */
struct SnapshotBatchActParam {
    std::vector<std::string> m_ids;

    SnapshotBatchActParam(const std::vector<std::string> &ids) : m_ids(ids)
    {
    }

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ids, SNAPSHOTLIST)
    END_SERIAL_MEMEBER
};

/**
 * 单个快照相关数据
 */
struct PerSnapshotParam {
    Pv m_pv;
    LunInfoData m_lunInfoData;
    SnapshotInfoData m_snapshotInfoData;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_pv, pv)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lunInfoData, lunInfo)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotInfoData, snapshotInfo)
    END_SERIAL_MEMEBER
};

/**
 * 批量打快照相关数据
 */
struct BatchSnapshotParam {
    ApplicationResource m_pod;
    std::vector<Pv> m_pvs;
    std::vector<PerSnapshotParam> m_snapshotInfos;
    StorageDeviceInfo m_storageDeviceInfo;
    StorageParam m_storage;
    unsigned long long m_activeTime;
};

/**
 * pod资源扩展信息
 */
struct PodExtendInfo {
    std::vector<std::string> m_containerNames;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_containerNames, containerNames)
    END_SERIAL_MEMEBER
};

/**
 * 一致性激活相关数据
 */
struct ConsistentActivationInfo {
    // 快照所在存储的设备信息
    StorageParam m_storage;
    // 同一设备下未激活的快照Id信息
    std::vector <std::string> m_snapshotIds;
    // 同一设备下未激活的快照名称信息（上报日志信息使用）
    std::vector <std::string> m_snapshotNames;
};

/**
 * 任务阶段
 */
enum TaskStage {
    Pre_Task = 0,
    Post_Task = 1,
    Fail_Task = 2
};

/**
 * 脚本执行结果类型
 */
enum ScriptRetType {
    SCRIPT_SUCCESS = 0,
    SCRIPT_FAILED = 1,
    SCRIPT_NO_PERMIT = 2,
    SCRIPT_FORMAT_ERR = 3,
    SCRIPT_NOT_FOUND = 4
};
}

#endif
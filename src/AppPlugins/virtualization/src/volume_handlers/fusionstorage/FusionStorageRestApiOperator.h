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
#ifndef FUSION_STORAGE_REST_API_OPERATOR_H
#define FUSION_STORAGE_REST_API_OPERATOR_H

#include "json/json.h"
#include "volume_handlers/common/DiskCommDef.h"
#include "FusionStorageApi.h"
#include "FusionStorageRestApiErrorCode.h"
#include "client/FusionStorageRestClient.h"
#include "client/FusionStorageRestApiRequest.h"
#include "protect_engines/hcs/resource_discovery/HcsMessageInfo.h"
#include "common/JsonUtils.h"
#include "common/Constants.h"
#include "volume_handlers/common/ControlDevice.h"
#include "common/cert_mgr/CertMgr.h"
#include "FusionStorageStructs.h"
#include "common/login_host/LoginHost.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

const int32_t RESTAPISUCCESS = 0;
const int32_t RESTAPIWRONGACION = 1;
const int32_t RESTAPIFAILED = 2;

using ErrorCode = FusionStorageRestApiErrorCode;

class FusionStorageRestApiOperator : public FusionStorageApi {
public:
    FusionStorageRestApiOperator(const std::string &fusionstorMgrIp, const std::string &poolID,
        const std::string &jobId, const std::string &subJobId);

    FusionStorageRestApiOperator();
    ~FusionStorageRestApiOperator();

    int32_t CreateBitmapVolume(const std::shared_ptr<RepositoryHandler> cacheRepoHandler,
        const std::string &filePath, BitmapVolumeInfo &info, std::string &errDes);

    int32_t DeleteBitmapVolume(const std::string &volumeName, std::string &errDes);

    int32_t AttachVolume(const std::string &volumeName, std::string &diskDevicePath, std::string &errorDes);

    int32_t DetachVolume(const std::string &volumeName, std::string &errorDes);

    int32_t DeleteVolumeAllMappingCaller(const std::string &volume, std::string &errorDes);

    int32_t QueryBitmapVol(BitmapVolumeInfo &info, std::string &errDes);

    int32_t GetStorageInfo(const std::string &authExtendInfo, const VolInfo &volInfo, bool &isBackup);

    int32_t AttachBitmapVolume(const std::string &bitmapVolumeName, std::string &diskDevicePath, std::string &errorDes);

    int32_t QuerySnapshotInfoByName(const std::string &snapshotName, std::string &responseBody, std::string &errorDes);

    int32_t QueryStoragePoolUsedRate(double &usedCapacityRate);

    int32_t TryGetToken();
    /**
     * @brief 初始化apiOperator相关指针和对象
     * @param
     * @return 返回初始化结果
     */
    int32_t InitRestApiOperator();
    void SetOpService(bool isOpService)
    {
        m_isOpService = isOpService;
    }

    /**
     * @brief 查询查询当前session，避免多次获取token导致问题
     * @param
     * @return
     */
    int32_t GetCurrentSession();
    int32_t DeleteCurrentSession();
    int32_t GetVersion(int32_t &version, std::string &errDes);
    int32_t CheckVbsNodeConnect();
    int32_t RetryToQueryBitmap(BitmapVolumeInfo &info);
    VBSNodeInfo m_vbsNodeInfo;

protected:
    int32_t GetClientIp(const std::string &responseBody, std::string &responseResult);

    /**
     * @brief 执行扫盘操作
     * @param 传入卷名仅仅是用于显示，WWN用于获得路径，
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t DoScanDisk(const std::string &volumeName, const std::string &volumeWwn, std::string &diskDevicePath);
    /**
     * @brief 获取主机
     * @param 主机名
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t GetHostByName(const std::string &hostName, std::string &errorDes);

    /**
     * @brief 获得主机列表
     * @param 无需任何参数
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t GetHostList(std::string &hostList, std::string &errorDes);

    /**
     * @brief 创建主机
     * @param 主机名、ip地址就是查询出的nodemgrip
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t CreateHost(const std::string &hostName, std::string &errorDes);

    /**
     * @brief 删除主机
     * @param 主机名
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t DeleteHost(const std::string &hostName, std::string &errorDes);

    /**
     * @brief 查询启动器，用以判断启动器是否存在
     * @param 仅传入启动器iqn即可
     * @inreturn 主机列表、CHAP认证状态、创建时间、启动器名称、启动器ID、回报
     * @return
     */
    int32_t GetIscsiInitiatorByName(const std::string &portName, std::string &errorDes);

    /**
     * @brief 创建启动器
     * @param 仅传入启动器名称iqn即可
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t CreateIscsiInitiator(const std::string &portName, std::string &errorDes);

    /**
     * @brief 删除启动器
     * @param 仅传入启动器名称iqn即可
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t DeleteIscsiInitiator(const std::string &portName, std::string &errorDes);

    /**
     * @brief 添加启动器到主机中
     * @param 主机名、启动器iqn
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t AddIscsiInitiatorToHost(const std::string &hostName, const std::string &portName, std::string &errorDes);

    /**
     * @brief 删除启动器，从主机中删除
     * @param 传入主机名、ISCSI启动器名列表
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t DeleteIscsiInitiatorFromHost(
        const std::string &hostName, const std::string &portName, std::string &errorDes);

    /**
     * @brief 查询卷映射主机，用于检查挂载
     * @param LUN名称
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t QueryHostFromVolume(const std::string &volumeName, std::string &responseBody, std::string &errorDes);

    /**
     * @brief 解除当前卷的所有映射
     * @param LUN名称
     * @return 返回result：0成功，-1失败
     */
    int32_t DeleteVolumeAllMapping(const std::string &volume, std::string &errorDes);
    
    /**
     * @brief 获得当前卷上的所有映射的主机，获得主机列表
     * @param LUN名称
     * @return 返回result：0成功，-1失败
     */
    int32_t GetVolumeMappedHost(
        const std::string &volume, std::vector<std::string> &hostNameList, std::string &errorDes);

    /**
     * @brief 从查询卷信息的响应中解析主机列表的响应体
     * @param LUN名称
     * @return 返回result：0成功，-1失败
     */
    int32_t ParseMappedHostList(const std::string &responseBody, std::vector<std::string> &hostNameList);

    /**
     * @brief 增加lun和主机的映射
     * @param 传入主机名、Lun名/快照名列表
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t AddLunToHost(const std::string &hostName, const std::string &volumeName, std::string &errorDes);

    /**
     * @brief 删除lun和主机的映射
     * @param 传入主机名、Lun名/快照名列表
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t DeleteLunFromHost(const std::string &hostName, const std::string &volumeName, std::string &errorDes);

    /**
     * @brief 检查主机是否在查到的主机列表中
     * @param 传入包含主机列表的返回体、查询的主机名
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t CheckHostInList(const std::string &hostQueryResponseBody, const std::string &hostName);

    /**
     * @brief 建立ISCSI映射视图
     * @param 传入主机名、卷名，名称来源于管理IP和扩展信息
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t CreateHostLunMapping(const std::string &hostName, const std::string &volumeName, std::string &errorDes);

    /**
     * @brief 创建并添加启动器和主机
     * @param 主机名
     * @return
     */
    int32_t CreateIscsiInitiatorAndHost(const std::string &hostName, std::string &errorDes);

    /**
     * @brief 检查并登录目标器
     * @param  TargetIp来源于任务信息
     * @return 返回result：0成功，1多错误码description，2单错误码errCode
     */
    int32_t CheckIscsiTarget(const std::string &hostName, std::string &errorDes, bool &needLogin);

    /**
     * @brief 执行登录目标器
     * @param 目标逻辑端口
     * @return 返回登录结果
     */
    int32_t LoginTarget();
    int32_t DoLoginTarget(const std::string &targetPortalIp);
    int32_t CheckExistLoginedTarget(std::vector<Json::Value> &targetIscsiPortalList);
    /**
     * @brief 检查并添加ISCSI启动器
     * @param 主机名、启动器iqn
     * @return
     */
    int32_t CheckAndCreateIscsiInitiator(const std::string &hostName, std::string &portName, std::string &errorDes);

    /**
     * @brief 检查主机卷映射
     * @param 主机名、卷名
     * @return
     */
    int32_t QueryHostLunMapping(const std::string &hostName, const std::string &volumeName, std::string &errorDes);

    /**
     * @brief 删除映射视图
     * @param 主机名、卷名
     * @return
     */
    int32_t DeleteHostLunMapping(const std::string &hostName, const std::string &volumeName, std::string &errorDes);

    /**
     * @brief 查询ISCSI端口相关信息
     * @param
     * @return 将信息写入响应体
     */
    int32_t QueryIscsiPortal(std::string &responseBody, std::string &errorDes);

    /**
     * @brief 从响应体中获得ISCSI端口信息，选取到第一个active的portal就行。
     * @param
     * @return
     */
    int32_t GetTargetIscsiPortal(
        const std::string &responseBody, std::vector<Json::Value> &targetIscsiPortal, std::string &errorDes);
    int32_t GetTargetIscsiPortalInner(Json::Value &iscsiPortalList);

    /**
     * @brief 从句柄获得扩展信息，将扩展信息解析成结构体并发送请求获得token
     * @param
     * @return
     */
    int32_t GetToken();

    /**
     * @brief 获得token返回体
     * @param
     * @return
     */
    int32_t GetTokenByAppEnv(std::string &responseBody, std::string &errorDes);
    // token请求体
    RequestInfo GetTokenByAppEnvInner();

    /**
     * @brief 获得token的最后一步，从gettokenbyappenv中获得的响应体解析出来
     * @param
     * @return
     */
    int32_t DoGetToken(const std::string &responseBody, std::string &token);

    /**
     * @brief 通过卷名查询信息
     * @param
     * @return
     */
    int32_t QueryVolumeInfoByName(const std::string &volumeName, std::string &responseBody, std::string &errorDes);

    /**
     * @brief 从返回体中获得快照、卷的WWN
     * @param
     * @return
     */
    int32_t GetSnapshotOrVolumeInfo(const std::string &responseBody, std::string &errorDes);

    /**
     * @brief 通过查询获得卷wwn，用于确认挂载正确（暂不使用）
     * @param
     * @return
     */
    int32_t GetVolumeWwnByName(const std::string &volumeName, std::string &errorDes);

    /**
     * @brief 查询ISCSI连接，充当查询querybitmap的作用，用于健康检查，实际备份不会进行相关Bitmap操作
     * @param
     * @return
     */
    int32_t QueryIscsiSession(const std::string &hostName, std::string &errorDes);

    /**
     * @brief 格式化请求体
     * @param
     * @return requestinfo
     */
    RequestInfo FormatRequestInfo(const std::string requestMethod, const std::string resourcePath);

    /**
     * @brief 检查响应消息
     * @param
     * @return result
     */
    int32_t checkResponseResult(RequestInfo &requestInfo, std::shared_ptr<ResponseModel> response, bool errorType);

    /**
     * @brief 刪除bitmap卷
     * @param 卷名
     * @return result
     */
    int32_t DoDeleteBitmapVolume(const std::string &volumeName, std::string &errDes);
 
    /**
     * @brief 创建bitmap卷
     * @param bitmap卷信息
     * @return result
     */
    int32_t DoCreateBitmapVol(BitmapVolumeInfo &info, std::string &errDes);

    // 读取json字符串中的result值
    int32_t GetResponseResult(const std::string &responseBody, bool codeFlag);
    std::string GetErrString(const int32_t &errorCode);
    int32_t GetStringField(Json::Value &json, const string &field, std::string &result);
    int32_t GetIntField(Json::Value &jsonBody, const string &field, int32_t &result);

    // 检查target是否已经在已登录的IP中
    int32_t CheckLoginedTarget(const std::string &aimTarget);
    int32_t CheckContain(const std::string &strTarget, std::vector<IscsiSessionStatus> &loginedSessions);
    int32_t MapIscsiToHost(const std::string &portName, const std::string &hostName,
        std::string &errorDes);
    int32_t QueryHostIscsiMapped(const std::string &portName, std::vector<std::string> &mappedHosts,
        std::string &errorDes);
    int32_t ParseMapResult(const std::string &responseBody, const std::string &iscsiIqn,
        std::vector<std::string> &mappedHosts);

    std::string GetFusionStorageMgrIp();
    std::string GetFusionStorageMgrPort();
    std::string GetFusionStorageToken();
    std::string GetHostName();
    std::string GetVolumeWwn();
    std::string GetCurDiskDevicePath();
    bool GetIsInternalScence();
    void SetFusionStorageMgrIp(const std::string &mgrIp);
    void SetFusionStorageMgrPort(const std::string &mgrPort);
    void SetFusionStorageToken(const std::string &token);
    void SetHostName(const std::string &hostName);
    void SetCurrentVolWWn(const std::string &volWwn);
    void SetCurDiskDevicePath(const std::string &disDevicePath);
    int32_t GetControlDeviceInfo(
        std::vector<StorageInfo> &storageVector, ControlDeviceInfo &controlDeviceInfo, const VolInfo &volInfo);
    void InitRequestInfo(RequestInfo &requestInfo,
        const std::string requestMethod, const std::string resourcePath);
    void SetVbsNodeInfo(const StorageInfo& info);

private:
    std::string m_hostName;
    std::string m_fusionStorageMgrIp;             // 存储环境IP
    std::string m_fusionStorageMgrPort = "8088";  // 端口，默认8088
    std::string m_token = "";
    std::string m_volumeWwn;
    std::string m_curDiskDevicePath;
    std::string m_currentSessionInfo;
    bool m_isBackupJob;
    bool m_isInternalScence = false;
    std::string m_jobId;
    std::string m_subJobId;
    bool m_hasReportVbsConnectFailed = false;

    std::string m_bitmapWwn;
    std::unordered_map<std::string, std::string> m_diskPathSet;
    std::vector<Json::Value> m_targetIscsiPortalList;
    std::map<int32_t, std::string> m_errorCodeToErrorDes;
    AppProtect::ApplicationEnvironment m_appEnv;
    std::shared_ptr<FusionStorageRestClient> m_restClient;
    ControlDeviceInfo m_controlDeviceInfo;
    bool m_isOpService {false};
    std::string m_iscsiIqnName = "";
};

VIRT_PLUGIN_NAMESPACE_END

#endif
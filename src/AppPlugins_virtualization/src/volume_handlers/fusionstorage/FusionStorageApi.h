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
#ifndef FUSIONSTORAGE_API_H
#define FUSIONSTORAGE_API_H
#include <string>
#include <map>
#include <vector>
#include "common/Macros.h"
#include "common/Constants.h"
#include "security/cmd/CmdParam.h"
#include "common/JsonHelper.h"
#include "repository_handlers/RepositoryHandler.h"

namespace VirtPlugin {
enum DSWareApiErrorCode {
    DSWARE_API_OK = 0,
    DSWARE_API_ERR_DSWARE_AGENT_NOT_INSTALL = 1000,
    DSWARE_API_ERR_JAVA_FILE_IS_NOT_EXIST = 1001,
    DSWARE_API_ERR_JAVA_FILE_HAVE_NO_PERMISSION = 1002,
    DSWARE_API_ERR_VBSTOOL_CONF_FAULT = 1003,
    DSWARE_API_ERR_SNAP_IS_NOT_EXIST = 50150006,
    DSWARE_API_ERR_VOLUME_IS_NOT_EXIST = 50150005,
    DSWARE_API_ERR_VOLUME_HAS_BEEN_ATTACH = 50151401,
    DSWARE_API_ERR_VOLUME_HAS_BEEN_DETACH = 50151601,
    DSWARE_API_ERR_VOLUME_OR_SNAPSHOT_IS_NOT_EXIST = 50151404,
    DSWARE_API_ERR_VOLUME_IS_CREATING_BITMAP_VOLUME = 50151006,
    DSWARE_API_ERR_CREATE_BITMAP_VOL_SNAP_BRANCH_ERR = 50153007,
    DSWARE_API_ERR_CREATE_BITMAP_SNAPSHOT_REPEATED_ERR = 50560010,
    DSWARE_API_ERR_UNKNOWN = 0xffffffff
};

struct BitmapVolumeInfo {
    std::string volName;
    int32_t status;
    int32_t volSize;
    int32_t snapSize;
    std::string snapNameFrom;
    std::string snapNameTo;
    int32_t blockSize;
    int64_t createTime;
    bool usedInRestore;
    int32_t createTimeOut;    // 单位s
    bool isLastBitmap;
    bool isFirstBitmap;

    BitmapVolumeInfo() : status(1), volSize(0), snapSize(0), blockSize(0), createTime(0), usedInRestore(false),
        createTimeOut(0), isLastBitmap(false), isFirstBitmap(false)
    {}

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volName, volName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volSize, volSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(snapSize, snapSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(snapNameFrom, snapNameFrom)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(snapNameTo, snapNameTo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(blockSize, blockSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(createTime, createTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(usedInRestore, usedInRestore)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(createTimeOut, createTimeOut)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isLastBitmap, isLastBitmap)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isFirstBitmap, isFirstBitmap)
    END_SERIAL_MEMEBER
};

class FusionStorageApi {
public:
    FusionStorageApi(const std::string &fusionStorMgrIp, const std::string &poolID);

    FusionStorageApi()
    {}

    virtual ~FusionStorageApi();

    /**
     *  @brief 创建差量位图卷
     *
     *  @param info     [IN]创建差量位图卷所需的参数
     *  @param errDes   [OUT]错误描述
     *  @return 错误码：0 成功，非0 失败
     */
    int32_t CreateBitmapVolume(const std::shared_ptr<RepositoryHandler> cacheRepoHandler,
        const std::string &filePath, BitmapVolumeInfo &info, std::string &errDes);

    /**
     *  @brief 删除差量位图卷
     *
     *  @param volumeName     [IN]差量位图卷名称
     *  @param errDes         [OUT]错误描述
     *  @return 错误码：0 成功，非0 失败
     */
    int32_t DeleteBitmapVolume(const std::string &volumeName, std::string &errDes);

    /**
     *  @brief 挂载卷
     *
     *  @param volumeName     [IN]卷名称
     *  @param diskDevicePath [IN]卷映射到主机上的路径:/dev/xxxx
     *  @param errDes         [OUT]错误描述
     *  @return 错误码：0 成功，非0 失败
     */
    int32_t AttachVolume(const std::string &volumeName, std::string &diskDevicePath, std::string &errDes);

    /**
     *  @brief 卸载卷
     *
     *  @param volumeName     [IN]卷名称
     *  @param errDes         [OUT]错误描述
     *  @return 错误码：0 成功，非0 失败
     */
    int32_t DetachVolume(const std::string &volumeName, std::string &errDes);

    /**
     *  @brief 更新FusionStorage 管理IP和存储池ID
     *
     *  @param mgrIP     [IN]FusionStorage 管理IP
     *  @param poolID    [IN]存储池ID
     */
    void UpdateMgrIPAndPoolID(const std::string &mgrIP, const std::string &poolID);

    /**
     *  @brief 获取存储池ID
     *
     *  @return 存储池ID
     */
    std::string GetPoolID() const;

    /**
     *  @brief 删除差量位图卷, 不重试
     *
     *  @param volumeName     [IN]差量位图卷的名称
     *  @param errDes         [OUT]错误描述
     *  @return 错误码：0 成功，非0 失败
     */
    int32_t DeleteBitmapVolumeNoRetry(const std::string &volumeName, std::string &errDes);

    /**
     *  @brief 查询位图卷信息
     *
     *  @param info     [IN] 卷描述
     *  @param errDes   [OUT]错误描述
     *  @return 错误码：0 成功，非0 失败
     */
    int32_t QueryBitmapVol(BitmapVolumeInfo &info, std::string &errDes);

    /**
     *  @brief 查询快照是否存在
     *
     *  @param snapshotName     [IN] 快照名称
     *  @return 错误码：0 成功，非0 失败
     */
    int32_t QuerySnapshot(const std::string &snapshotName, std::string &errDes);
    int32_t RetryToQueryBitmap(BitmapVolumeInfo &info);

private:
    /**
     *  @brief 执行创建位图卷操作
     *
     *  @param info     [IN] 卷描述
     *  @param errDes   [OUT]错误描述
     *  @return 错误码：0 成功，非0 失败
     */
    int32_t DoCreateBitmapVol(BitmapVolumeInfo &info, std::string &errDes);

    std::map<std::string, std::string> ToKevValue(const std::vector<std::string> &strings);
    std::string GetErrString(const int32_t code);
    std::string VectorToStr(std::vector<std::string> &vecStr);
    void GetDswareAgentIp(const std::string &dswareMgrIp);
    void SetAttachVolumeCmdParam(std::vector<Module::CmdParam> &cmdParam, const std::string &volumeName);
    void SetQueryBitmapCmdParam(std::vector<Module::CmdParam> &cmdParam, const BitmapVolumeInfo &info);
    void SetCreateBitmapCmdParam(std::vector<Module::CmdParam> &cmdParam, const BitmapVolumeInfo &info);
    void SetDetachVolumeCmdParam(std::vector<Module::CmdParam> &cmdParam, const std::string &volumeName);

private:
    std::string m_fusionStorMgrIp;
    std::string m_poolID;
    std::map<int32_t, std::string> m_codeToErrorDes;
};
}

#endif  // FUSIONSTORAGE_API_H
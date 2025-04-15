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
#ifndef _VOLUME_HANDLER_H_
#define _VOLUME_HANDLER_H_

#include "common/Macros.h"
#include "common/DirtyRanges.h"
#include "common/Structs.h"
#include "common/sha256/Sha256.h"
#include "common/cert_mgr/CertMgr.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "common/DiskDeviceFile.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

enum class VolOpenMode {
    READ_ONLY = 0,
    READ_WRITE
};

struct BlockShaData {
    off_t offset{0};
    std::shared_ptr<uint8_t[]> sha256Value{nullptr};
};

class VolumeHandler {
public:
    VolumeHandler(std::shared_ptr<JobHandle> jobHandle, const VolInfo &volInfo, std::string jobId,
        std::string subJobId)
        : m_jobHandle(jobHandle), m_volInfo(volInfo), m_jobId(jobId), m_subJobId(subJobId)
    {
    }
    virtual ~VolumeHandler()
    {
    }

    /**
     *  @brief 获取卷特定块的增量位置信息
     *
     *  @param curSnapshot [IN]当前快照信息
     *  @param preChangeID [IN]上一次CBT或快照ID
     *  @param dirtyRanges [IN,OUT]该卷的增量位置信息
     *  @param startOffset [IN]块的起始位置
     *  @param endOffset   [IN]块的结束位置
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t GetDirtyRanges(const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot,
                                   DirtyRanges &dirtyRanges, const uint64_t startOffset, uint64_t &endOffset) = 0;

    /**
     *  @brief 打开卷，用于卷备份前的操作，如挂卷等
     *
     *  @param mode     [IN]打开模式，只读或读写
     *  @param jobInfo  [IN]子任务对象信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t Open(const VolOpenMode &mode, const BackupSubJobInfo &jobInfo) = 0;

    /**
     *  @brief 打开卷，用于卷恢复操作
     *
     *  @param mode     [IN]打开模式，只读或读写
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t Open(const VolOpenMode &mode) = 0;

    /**
     *  @brief 关闭卷，用于卷备份后的操作，如卸卷等
     *
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t Close() = 0;

    /**
     *  @brief 按位置读卷中的块
     *
     *  @param offsetInBytes  [IN]起始偏移
     *  @param bufSizeInBytes [IN]要读的内容长度
     *  @param buf            [IN]读缓冲区
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t ReadBlocks(const uint64_t &offsetInBytes, uint64_t &bufSizeInBytes,
                               std::shared_ptr<uint8_t[]> &buf, std::shared_ptr<uint8_t[]> &calBuffer,
                                std::shared_ptr<uint8_t[]> &readBuffer) = 0;

    /**
     *  @brief 按位置写卷中的块
     *
     *  @param offsetInBytes  [IN]起始偏移
     *  @param bufSizeInBytes [IN]要写的内容长度
     *  @param buf            [IN]写缓冲区
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t WriteBlocks(const uint64_t &offsetInBytes, uint64_t &bufSizeInBytes,
                                std::shared_ptr<uint8_t[]> &buf) = 0;

    /**
     *  @brief 获取卷大小
     *
     *  @param  volumeSizeInBytes  [OUT]卷大小
     *  @return 错误码：0 成功，非0 失败
     */
    virtual uint64_t GetVolumeSize() = 0;

    /**
     *  @brief 检查卷对应生产存储连通性
     *
     *  @param  authExtendInfo  [IN]存储认证信息
     *  @return 错误码：0 成功，非0 失败
     */
    virtual int32_t TestDeviceConnection(const std::string &authExtendInfo, int32_t &erroCode) = 0;

    /**
     * @brief 获取卷UUID
     *
     * @return std::string
     */
    virtual std::string GetVolumeUUID()
    {
        return m_volInfo.m_uuid;
    }

    virtual bool GetIfMarkBlockValidData()
    {
        return ifmarkBlockValidData;
    }

    /**
     * @brief 清理资源
     */
    virtual int32_t CleanLeftovers() = 0;

    virtual int32_t Flush() = 0;

    virtual int32_t CalculateHashValue(const std::shared_ptr<unsigned char[]> &buffer,
        const std::shared_ptr<uint8_t[]> &calcHashBuffer, std::shared_ptr<uint8_t[]> &shaBuffer,
        uint64_t bufferSize)
    {
        return CalculateSha256::CalculateSha256Value(buffer, bufferSize, shaBuffer);
    }
    /**
     * @brief 获取上报agent的信息
     */

    virtual void GetReportPara(ReportJobDetailsParam &reportParam)
    {
        reportParam = m_reportPara;
        m_reportPara.label = "";
    }

    /**
     * @brief 获取上报agent的参数
     */
    virtual void GetReportArgs(std::vector<std::string> &reportArgs)
    {
        reportArgs = m_reportArgs;
        m_reportArgs.clear();
    }

    virtual int32_t DoWaitVolumeStatus(const std::string &volId, const std::string &status,
        std::vector<std::string> intermediateState = {}, uint32_t interval = 30, uint32_t retryCount = 5)
    {
        return SUCCESS;
    }

    virtual void InitRepoHandler()
    {
        if (m_metaRepoHandler != nullptr && m_cacheRepoHandler != nullptr && m_volumeInfoRepoHandler != nullptr) {
            return;
        }
        if (m_jobHandle == nullptr) {
            ERRLOG("Job handle is null.");
            return;
        }
        std::vector<AppProtect::StorageRepository> repos = m_jobHandle->GetStorageRepos();
        for (const auto &repo : repos) {
            if (repo.path.size() == 0) {
                WARNLOG("Repo path size is 0.");
                return;
            }
            if (repo.repositoryType == RepositoryDataType::META_REPOSITORY) {
                m_metaRepoHandler = RepositoryFactory::CreateRepositoryHandler(repo);
                m_volumeInfoRepoHandler = RepositoryFactory::CreateRepositoryHandler(repo);  // 创建用于记录卷信息的仓库句柄
                m_metaRepoPath = repo.path[0];
            } else if (repo.repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
                m_cacheRepoHandler = RepositoryFactory::CreateRepositoryHandler(repo);
                m_cacheRepoPath = repo.path[0];
            }
        }
        return;
    }

    virtual void SetReportJobDetailHandler(const std::function<void(const ApplicationLabelType &)> &handler) final
    {
        m_reportJobDetailHandler = handler;
    }

    virtual std::shared_ptr<DiskDeviceFile> GetDiskDeviceFile() = 0;

    virtual int32_t NeedToBackupBlock(std::shared_ptr<uint8_t[]> &buffer, struct io_event* event)
    {
        return CALCULATE_INITIAL_STATE;
    }

    virtual int32_t QueryStoragePoolUsedRate(double &usedCapacityRate)
    {
        usedCapacityRate = VirtPlugin::DEFAULT_STORAGE_THRESHOLD_LIMIT;
        return SUCCESS;
    }

    static std::shared_ptr<uint8_t[]> GetAllZeroDirtyRangeDataPtr()
    {
        if (m_allZeroDirtyRangeDataPtr != nullptr) {
            return m_allZeroDirtyRangeDataPtr;
        }
        static std::mutex mtx;
        std::lock_guard<std::mutex> lock(mtx);
        if (m_allZeroDirtyRangeDataPtr == nullptr) {
            m_allZeroDirtyRangeDataPtr = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
            memset_s(m_allZeroDirtyRangeDataPtr.get(), DIRTY_RANGE_BLOCK_SIZE, 0, DIRTY_RANGE_BLOCK_SIZE);
        }
        return m_allZeroDirtyRangeDataPtr;
    }

public:
    std::mutex m_volMutex;

protected:
    virtual void ReportJobDetail(const ApplicationLabelType &appLable) final
    {
        if (!m_reportJobDetailHandler) {
            WARNLOG("Report job detail handler not provided.");
        } else {
            m_reportJobDetailHandler(appLable);
        }
    }

protected:
    std::shared_ptr<JobHandle> m_jobHandle;
    std::vector<std::string> m_reportArgs;
    ReportJobDetailsParam m_reportPara;
    VolInfo m_volInfo;
    bool ifmarkBlockValidData = true; // 标记是否需要记录块有效数据（融合V5 Thick lun无法区分有效数据位图）
    std::string m_jobId;
    std::string m_subJobId;

    std::shared_ptr<RepositoryHandler> m_metaRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_cacheRepoHandler = nullptr;
    std::shared_ptr<RepositoryHandler> m_volumeInfoRepoHandler = nullptr;
    std::string m_metaRepoPath;
    std::string m_cacheRepoPath;
    std::function<void(const ApplicationLabelType &)> m_reportJobDetailHandler;

    static std::shared_ptr<uint8_t[]> m_allZeroDirtyRangeDataPtr;
};

VIRT_PLUGIN_NAMESPACE_END

#endif  // _VOLUME_HANDLER_H_

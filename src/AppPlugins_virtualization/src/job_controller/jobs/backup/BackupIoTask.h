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
#ifndef BACKUP_IO_TASK_H
#define BACKUP_IO_TASK_H

#include <cstdint>
#include <memory>
#include "job_controller/io_scheduler/BlockTask.h"
#include "volume_handlers/VolumeHandler.h"
#include "repository_handlers/RepositoryHandler.h"
#include "common/Structs.h"
#include "define/Defines.h"

namespace VirtPlugin {
class BackupIoTask : public BlockTask {
public:
    BackupIoTask(uint64_t startAddr, uint64_t bufSize, bool genShaFile)
        : m_startAddr(startAddr), m_bufferSize(bufSize), m_isCopyVerify(genShaFile)
    {}

    ~BackupIoTask(){};

    int32_t Exec() override;

    /**
     *  @brief  设置IO任务的读端
     *  @param  reader 读端
     */
    void SetVolumeHandler(std::shared_ptr<VolumeHandler> vh)
    {
        m_volumeHandler = vh;
    }

    /**
     *  @brief  设置IO任务的写端
     *  @param  writer 写端
     */
    void SetRepositoryHandler(std::shared_ptr<RepositoryHandler> rh)
    {
        m_repositoryHandler = rh;
    }

    /**
     *  @brief  GetSha256Data 获取Sha256数据
     *  @param  pBuffer Sha256数据
     *  @param  startAddr 卷数据偏移量
     *  @return 0 成功; 非0 失败
     */
    int32_t GetSha256Data(std::shared_ptr<unsigned char[]>& pBuffer, uint64_t& startAddr, bool& calcRet) const;

    /**
     * @brief 生成副本校验HASH值
     *
     */
    void GenCopyVerifySha256();

private:
    uint64_t m_startAddr;  // 块起始地址
    uint64_t m_bufferSize; // 块大小
    std::shared_ptr<VolumeHandler> m_volumeHandler;
    std::shared_ptr<RepositoryHandler> m_repositoryHandler;
    std::shared_ptr<uint8_t[]> m_shaDataBuff;
    bool m_isCopyVerify;
    bool m_calcSha256Success = true;
};
}
#endif
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
#ifndef RESTORE_IO_TASK_H
#define RESTORE_IO_TASK_H

#include <cstdint>
#include <memory>
#include "define/Defines.h"
#include "common/Structs.h"
#include "job_controller/io_scheduler/BlockTask.h"
#include "volume_handlers/VolumeHandler.h"
#include "repository_handlers/RepositoryHandler.h"
#include "ArchiveStreamService.h"

namespace VirtPlugin {
class RestoreIOTask : public BlockTask {
public:
    RestoreIOTask(uint64_t startAddr, uint64_t bufSize, std::shared_ptr<VolumeHandler> volHandler,
        std::shared_ptr<RepositoryHandler> repoHandler)
        : m_startAddr(startAddr),
          m_bufferSize(bufSize),
          m_volumeHander(volHandler),
          m_repoHandler(repoHandler)
    {}
    RestoreIOTask(uint64_t startAddr, uint64_t bufSize, std::shared_ptr<VolumeHandler> volHandler,
        std::shared_ptr<ArchiveStreamService>& clientHandler, const ArchiveStreamGetFileReq& req)
        : m_startAddr(startAddr),
          m_bufferSize(bufSize),
          m_volumeHander(volHandler),
          m_clientHandler(clientHandler),
          m_req(req)
    {
        m_isArchiveRestore = true;
    }
    ~RestoreIOTask(){};
    int32_t Exec() override;
    void SetIgnoreBadBlockFlag(bool flag);

private:
    int32_t RestoreRead();
    int32_t ArchiveRestoreRead();
    uint64_t m_startAddr;  // 块起始地址
    uint64_t m_bufferSize; // 块大小
    std::shared_ptr<VolumeHandler> m_volumeHander;
    std::shared_ptr<RepositoryHandler> m_repoHandler;
    std::shared_ptr<ArchiveStreamService> m_clientHandler;
    bool m_isArchiveRestore {false};
    ArchiveStreamGetFileReq m_req;
    bool m_ignoreBadBlock {false};
};
}
#endif
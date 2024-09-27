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
#ifndef BLOCK_CHECK_TASK_H
#define BLOCK_CHECK_TASK_H

#include <cstdint>
#include <memory>
#include "job_controller/io_scheduler/BlockTask.h"
#include "repository_handlers/RepositoryHandler.h"
#include "common/Structs.h"
#include "define/Defines.h"

namespace VirtPlugin {
class BlockCheckTask : public BlockTask {
public:
    BlockCheckTask(uint64_t blockID, uint64_t imgSize, std::shared_ptr<uint8_t[]> &blockCheckSum,
        std::shared_ptr<RepositoryHandler> repoHandler)
        : m_blockID(blockID), m_imgSize(imgSize), m_blockCheckSum(blockCheckSum), m_repoHandler(repoHandler)
    {}

    ~BlockCheckTask(){};

    int32_t Exec() override;

private:
    uint64_t m_blockID = 0;                                // 块编号
    uint64_t m_imgSize = 0;                                // 数据镜像文件大小
    std::shared_ptr<uint8_t[]> m_blockCheckSum = nullptr;  // 块SHA256值
    std::shared_ptr<RepositoryHandler> m_repoHandler = nullptr;
};
}
#endif
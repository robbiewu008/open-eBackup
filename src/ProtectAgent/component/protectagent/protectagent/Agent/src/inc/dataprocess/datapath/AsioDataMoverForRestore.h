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
#ifndef DATA_MOVER_FOR_RESTORE_H
#define DATA_MOVER_FOR_RESTORE_H

#include "dataprocess/datapath/AsioDataMover.h"

class AsioDataMoverForRestore : public AsioDataMover {
public:
    AsioDataMoverForRestore(int fileHandler, std::shared_ptr<IOEngine> vddkHandler, uint64_t startDirtyRangePos,
        std::vector<tag_dirty_range_info> dirtyRanges, bool skipWriteZeros);
protected:
    void HandleEvent(struct io_event* event, io_context_t ioContext, uint64_t& dirtyRangePos) override;
    void Init(int dmInFd, std::shared_ptr<IOEngine> dmOutFd, uint64_t offsetDirtyRangePos,
              const struct DataMoverConfig& dmConfig) override;
    int SendAsyncRead(
            io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, int fd, off_t offset, size_t ioSize) override;
    uint64_t SendAsyncWrite(
            io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, int fd, off_t offset, size_t ioSize) override;
    std::shared_ptr<IOEngine> outFd;
};
#endif  // DATA_MOVER_H

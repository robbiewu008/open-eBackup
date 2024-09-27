// Copyright (c) Huawei Technologies Co., Ltd. 2012-2023. All rights reserved.
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

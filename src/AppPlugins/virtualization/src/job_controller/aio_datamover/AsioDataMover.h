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
#ifndef DATA_MOVER_H
#define DATA_MOVER_H

#include <chrono>
#include <map>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include "libaio.h"
#include "volume_handlers/VolumeHandler.h"

struct DataMoverConfig {
    size_t blockSize;
    size_t maxMemory;
    uint32_t maxNumThreads;
};

constexpr int DATA_MOVER_DEFAULT_BLOCK_SIZE = 4 * 1024 * 1024;
constexpr uint64_t DATA_MOVER_DEFAULT_MAX_MEMORY = 1024 * 1024 * 1024;
constexpr int DATA_MOVER_MAX_NUM_THREADS = 4;

constexpr DataMoverConfig DEFAULT_DATA_MOVER_CONFIG = { .blockSize = DATA_MOVER_DEFAULT_BLOCK_SIZE,
    .maxMemory = DATA_MOVER_DEFAULT_MAX_MEMORY, .maxNumThreads = DATA_MOVER_MAX_NUM_THREADS };

struct DataMoverAsyncReq {
    std::shared_ptr<uint8_t[]> data;
    struct iocb iocb;
    struct iocb* iocbPtr;
    int threadId = 0;
    uint64_t dirtyRangePos = 0;
};

class DataMoverLog {
public:
    virtual void Info(const std::string&) = 0;
    virtual void Debug(const std::string&) = 0;
    virtual void Warn(const std::string&) = 0;
    virtual void Error(const std::string&) = 0;
};

class AsioDataMover  {
public:
    AsioDataMover():skipWriteZeros(false) {};
    AsioDataMover(std::vector<VirtPlugin::DirtyRange> dirtyRanges, std::shared_ptr<DataMoverLog>,
                  std::shared_ptr<VirtPlugin::VolumeHandler>, std::vector<VirtPlugin::BlockShaData>*,
                  bool skipWriteZeros = false);
    ~AsioDataMover();

    AsioDataMover(const AsioDataMover &) = delete;
    AsioDataMover(AsioDataMover &&) = delete;
    AsioDataMover& operator=(const AsioDataMover &) = delete;
    AsioDataMover& operator=(AsioDataMover &&) = delete;

    void WaitForMoveThreads();

    bool IsAborted() const
    {
        return isAbortRequested && numThreads == 0;
    }

    void SetIfBackup(const bool& backup)
    {
        m_isBackup = backup;
    }

    int GetNumThreads() const
    {
        return numThreads;
    }

    int GetSha256Success() const
    {
        return m_sha256Success;
    }

    void SetNumThreads(int numOfThreads)
    {
        {
            std::lock_guard<std::mutex> lock(numThreadsMtx);
            this->numThreads = numOfThreads;
        }
    }

    /* Thread safe progress implementation.
     *
     * Each thread updates a single entry in the vector, aggregation can be
     * done during thread update without locking.
     */
    size_t GetBytesTransfered() const
    {
        if (numDeployedThreads == 0) {
            return 0;
        }

        size_t bytes{ 0 };
        for (auto& o : transferedBytes) {
            bytes += o;
        }
        return bytes;
    }

    std::uint64_t GetCompleteBlocksCount() const
    {
        std::uint64_t completedBlocksCount = m_completedBlocksCount.load();
        return completedBlocksCount;
    }

    /* Returns the minimum of all offsets among all threads */
    off_t GetMinThreadOffset() const
    {
        return *std::min_element(threadOffset.begin(), threadOffset.end());
    }
    std::atomic<bool> isAbortRequested {false};
    std::shared_ptr<DataMoverLog> m_logger;
    bool Init(int dmInFd, int dmOutFd, uint64_t offsetDirtyRangePos,
              const struct DataMoverConfig& dmConfig);
protected:
    void CheckIgnoreBadBlock(struct io_event* event);
    void StartThreads(uint64_t startDirtyRangePos);
    std::unique_ptr<std::thread> StartMoveThread(const uint64_t&  startDirtyRangePos);
    void HandleEvent(struct io_event* event, io_context_t ioContext, uint64_t& dirtyRangePos);

    void AsyncIOCompletionHandler(io_context_t ioContext);

    uint64_t GetNextDirtyRangePos(const uint64_t& currentPos);

    void SendAsyncRead(
        io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, int fd, off_t offset, size_t ioSize);
    uint64_t SendAsyncWrite(
        io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, off_t offset, size_t ioSize);
    void HandleForCopyVerify(struct io_event* event, std::shared_ptr<uint8_t[]>& calBuffer);

    struct DataMoverConfig config;
    uint64_t numAsyncCalls = 0;
    uint64_t numIOsPerThread = 0;
    uint64_t firstBatchDirtyRangePos = 0;
    std::vector<std::unique_ptr<std::thread>> moveThreads;
    int numThreads;
    std::mutex numThreadsMtx;
    size_t batchSize;
    std::map<void*, std::unique_ptr<DataMoverAsyncReq>> asyncReqMap;
    uint64_t maxDirtyRangePos = 0;
    int inFd;
    int outFd;
    int numDeployedThreads;
    std::vector<uint64_t> threadOffset;
    bool skipWriteZeros = false;
    std::vector<size_t> transferedBytes;
    std::vector<VirtPlugin::DirtyRange> m_dirtyRanges;
    std::atomic<std::uint64_t> m_dirtyRangesPos;
    std::atomic<std::uint64_t> m_completedBlocksCount;
    bool m_isBackup = false;
    std::shared_ptr<VirtPlugin::VolumeHandler> m_volumeHandler;
    std::vector<VirtPlugin::BlockShaData>* m_shaPtr;
    std::mutex m_shaPtrMtx;
    bool m_sha256Success = true;
    bool m_ifCalculateIncrement = false;
};
#endif  // DATA_MOVER_H

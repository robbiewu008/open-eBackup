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
#include <iostream>
#include <unistd.h>
#include <string>
#include "common/ConfigXmlParse.h"
#include "dataprocess/datapath/AsioDataMover.h"

namespace {
    constexpr int AIO_NUM_HANDLE_EVENTS = 200;

    constexpr int DATA_MOVER_DEFAULT_BLOCK_SIZE = 4 * 1024 * 1024;
    constexpr uint64_t DATA_MOVER_DEFAULT_MAX_MEMORY = 1024 * 1024 * 1024;
    constexpr int DATA_MOVER_MAX_NUM_THREADS = 1;
    constexpr bool NON_BLOCKING_DATA_MOVE = false;
    constexpr DataMoverConfig DEFAULT_DATA_MOVER_CONFIG = { .blockSize = DATA_MOVER_DEFAULT_BLOCK_SIZE,
        .maxMemory = DATA_MOVER_DEFAULT_MAX_MEMORY, .maxNumThreads = DATA_MOVER_MAX_NUM_THREADS };
    const uint64_t ALL_BLOCKS_ZERO = 99;
}


AsioDataMover::AsioDataMover(bool skipWriteZeros)
    : skipWriteZeros(skipWriteZeros)
{}

void AsioDataMover::StartThreads(uint64_t startDirtyRangePos)
{
    while (startDirtyRangePos < firstBatchDirtyRangePos) {
        ERRLOG("startDirtyRangePos: %llu", startDirtyRangePos);
        threadOffset.push_back(0);
        transferedBytes.push_back(0);
        moveThreads.push_back(StartMoveThread(startDirtyRangePos));
        numDeployedThreads = GetThreadNum();
        startDirtyRangePos += numIOsPerThread;
    }
}

AsioDataMover::~AsioDataMover()
{
    WaitForMoveThreads();
    std::map<void*, std::unique_ptr<DataMoverAsyncReq>> emptyMap;
    asyncReqMap.clear();
}

void AsioDataMover::WaitForMoveThreads()
{
    ERRLOG("begin");
    for (auto& t : moveThreads) {
        if (t && t->joinable()) {
            t->join();
        }
    }
    ERRLOG("end");
}

uint64_t AsioDataMover::GetThreadNum()
{
    uint64_t num = 0;
    for (auto& t : moveThreads) {
        if (t != nullptr) {
            num++;
        }
    }
    return num;
}

uint64_t AsioDataMover::GetNextDirtyRangePos(const uint64_t& currentPos)
{
    uint64_t nextPos = currentPos + numAsyncCalls;
    return nextPos;
}

/* async WRITE completion needs to trigger the next read.
 * Since READs are done in batches of maxMemory (aligned to blockSize), the
 * next offset is currentOffset + aligned(maxMemory).
 */
void AsioDataMover::AsyncIOCompletionHandler(io_context_t ioContext, const int& threadId)
{
    struct timespec timeout = { .tv_sec = 0, .tv_nsec = 10000000 };
    struct io_event events[AIO_NUM_HANDLE_EVENTS];
    handledEvents[threadId] = 0;
    submitedEvents[threadId] = 0;
    uint64_t tempPos = 0;
    off_t offset = static_cast<off_t>(0);
    COMMLOG(OS_LOG_ERROR, "Starting completion handler, maxDirtyRangePos: %llu", maxDirtyRangePos);
    bool allEventFinished = false;
    while (!allEventFinished) {
        auto numEvents = io_getevents(ioContext, AIO_NUM_HANDLE_EVENTS, AIO_NUM_HANDLE_EVENTS, events, &timeout);
        allEventFinished = true;
        /* Check if the lowest minDirtyRangePos is still in range to finish iteration */
        for (int i = 0; i < numEvents; i++) {
            try {
                HandleEvent(&events[i], ioContext, tempPos);
            } catch (const std::exception& e) {
                COMMLOG(OS_LOG_ERROR, "Caught exception: %s, during data move, aborting.", e.what());
                isAbortRequested = true;
            }
            if (tempPos <= maxDirtyRangePos) {
                allEventFinished = false;
            }
        }
        if (numEvents == 0) {
            COMMLOG(OS_LOG_DEBUG, "waiting for more events to be ready");
            allEventFinished = false;
        }
        if (allEventFinished && (handledEvents[threadId] < submitedEvents[threadId])) {
            COMMLOG(OS_LOG_DEBUG, "thread %llu,  reaches the end position,"
                                  "but some events're still need to be handled", threadId);
            allEventFinished = false;
        }
        if (isAbortRequested) {
            COMMLOG(OS_LOG_INFO, "AsioDataMover  completion stopped on abort request");
            break;
        }
    }
    COMMLOG(OS_LOG_INFO, "AsioDataMover  completion handler completed successfully");
    io_destroy(ioContext);
    {
        std::lock_guard<std::mutex> lock(numThreadsMtx);
        if (numThreads > 0) {
            numThreads--;
        }
    }
}

std::unique_ptr<std::thread> AsioDataMover::StartMoveThread(const uint64_t&  startDirtyRangePos)
{
    int threadId = numDeployedThreads;
    io_context_t ioContext;
    memset_s(&ioContext, sizeof(ioContext), 0, sizeof(ioContext));
    auto res = io_setup(numAsyncCalls, &ioContext);  // destroyed in completion handler
    if (res != 0) {
        throw std::runtime_error("Io setup return" + std::to_string(res));
    }
    uint64_t nextDirtyRangePos = startDirtyRangePos;
    uint64_t firstBatchMaxDirtyRangePos = std::min<uint64_t>(maxDirtyRangePos,
        startDirtyRangePos + numIOsPerThread - 1);
    COMMLOG(OS_LOG_INFO, "firstBatchMaxDirtyRangePos: %llu", firstBatchMaxDirtyRangePos);
    off_t offset = static_cast<off_t>(0);
    bool allBlocksZero = nextDirtyRangePos <= firstBatchMaxDirtyRangePos ? true : false;
    while (nextDirtyRangePos <= firstBatchMaxDirtyRangePos) {
        size_t ioSize = config.blockSize;
        auto asyncReq = std::make_unique<struct DataMoverAsyncReq>();
        asyncReq->data = std::make_unique<uint8_t[]>(ioSize);
        asyncReq->iocbPtr = &asyncReq->iocb;
        asyncReq->threadId = threadId;
        asyncReq->dirtyRangePos = nextDirtyRangePos;
        off_t offset = static_cast<off_t>(m_dirtyRanges[nextDirtyRangePos].start);
        if (SendAsyncRead(ioContext, asyncReq.get(), inFd, offset, ioSize) != ALL_BLOCKS_ZERO) {
            allBlocksZero = false;
        };
        asyncReqMap[asyncReq->data.get()] = std::move(asyncReq);
        nextDirtyRangePos++;
    }
    if (allBlocksZero) {
        COMMLOG(OS_LOG_INFO, "No need to start thread since it is all zero");
        return nullptr;
    }
    /* First send the first batch so that IO on device is sequential */
    auto completionHandler = std::make_unique<std::thread>(
        [this, ioContext, threadId]() { AsioDataMover::AsyncIOCompletionHandler(ioContext, threadId); });
    if (completionHandler != nullptr) {
        {
            std::lock_guard<std::mutex> lock(numThreadsMtx);
            numThreads++;  // decreased in thread function
        }
        COMMLOG(OS_LOG_INFO, "Current thread num: %llu ", numThreads);
    }
    return completionHandler;
}
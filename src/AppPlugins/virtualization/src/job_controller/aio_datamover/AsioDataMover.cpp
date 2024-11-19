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
#include "AsioDataMover.h"
#include <iostream>
#include <unistd.h>
#include <string>
#include "Constants.h"

namespace {
    constexpr int AIO_NUM_HANDLE_EVENTS = 200;
    const std::string IO_FAILED = "18446744073709551611";
    constexpr bool NON_BLOCKING_DATA_MOVE = false;

    static const mp_string CFG_DATAPROCESS_DATAMOVER_SECTION = "DataProcessDataMover";
    static const mp_string CFG_DATAMOVER_MEMORY = "memory";
    static const mp_string CFG_DATAMOVER_THREAD_NUM = "threadNum";
}

using namespace VirtPlugin;


AsioDataMover::AsioDataMover(std::vector<VirtPlugin::DirtyRange> dirtyRanges, std::shared_ptr<DataMoverLog> logger,
                             std::shared_ptr<VirtPlugin::VolumeHandler> vh, std::vector<BlockShaData> *shaPtr,
                             bool skipWriteZeros)
    : skipWriteZeros(skipWriteZeros), m_shaPtr(shaPtr)
{
    m_logger = logger;
    m_volumeHandler = vh;
    if (m_shaPtr == nullptr) {
        m_logger->Info("will not calculate sha256 in this datamover");
    }
    m_dirtyRanges = dirtyRanges;
    m_completedBlocksCount.store(0);
}


bool AsioDataMover::Init(int dmInFd, int dmOutFd, uint64_t startDirtyRangePos,
    const struct DataMoverConfig& dmConfig)
{
    if (startDirtyRangePos >= m_dirtyRanges.size()) {
        m_logger->Error("Invalid data move, " +  std::to_string(startDirtyRangePos) + " > " +
            std::to_string(m_dirtyRanges.size()));
        return false;
    }
    if (m_dirtyRanges[0].size != DATA_MOVER_DEFAULT_BLOCK_SIZE) {
        m_logger->Error("Invalid dirty range block size, " + std::to_string(m_dirtyRanges[0].size) + " != " +
            std::to_string(DATA_MOVER_DEFAULT_BLOCK_SIZE));
        return false;
    }
    this->config = dmConfig;
    uint64_t bytesInMB = 1024 * 1024;
    // parse task pool worker thread count
    mp_int32 workerThreadCount = Module::ConfigReader::getUint("VOLUME_DATA_PROCESS", "MAX_TASK_AIO_THREADS");
    m_logger->Info("workerThreadCount" + std::to_string(workerThreadCount));
    this->config.maxNumThreads =  workerThreadCount;

    mp_int32 memoryInMB = Module::ConfigReader::getUint("VOLUME_DATA_PROCESS", "MAX_TASK_AIO_MEM_IN_MB");
    m_logger->Info("memoryInMB:" + std::to_string(memoryInMB));
    this->config.maxMemory =  static_cast<uint64_t>(memoryInMB) * bytesInMB;

    this->inFd = dmInFd;
    this->outFd = dmOutFd;
    maxDirtyRangePos = m_dirtyRanges.size();
    // change size to pos by 'maxDirtyRangePos--'
    maxDirtyRangePos--;
    numThreads = 0;
    numDeployedThreads = 0;
    threadOffset.reserve(DATA_MOVER_MAX_NUM_THREADS);
    transferedBytes.reserve(DATA_MOVER_MAX_NUM_THREADS);
    numAsyncCalls = config.maxMemory / config.blockSize;

    numIOsPerThread = numAsyncCalls / config.maxNumThreads;
    firstBatchDirtyRangePos = startDirtyRangePos + std::min<uint64_t>(maxDirtyRangePos, numAsyncCalls);
    m_logger->Info("firstBatchDirtyRangePos:" + std::to_string(firstBatchDirtyRangePos) + "numIOsPerThread:" +
        std::to_string(numIOsPerThread));
    StartThreads(startDirtyRangePos);
    return true;
}

void AsioDataMover::StartThreads(uint64_t startDirtyRangePos)
{
    while (startDirtyRangePos < firstBatchDirtyRangePos) {
        m_logger->Info("startDirtyRangePos: " + std::to_string(startDirtyRangePos));
        threadOffset.push_back(0);
        transferedBytes.push_back(0);
        moveThreads.push_back(StartMoveThread(startDirtyRangePos));
        numDeployedThreads++;
        startDirtyRangePos += numIOsPerThread;
    }
}

AsioDataMover::~AsioDataMover()
{
    WaitForMoveThreads();
    for (auto& req : asyncReqMap) {
        req.second->data.reset();
        req.second.reset();
    }
    asyncReqMap.clear();
}

void AsioDataMover ::WaitForMoveThreads()
{
    for (auto& t : moveThreads) {
        if (t && t->joinable()) {
            t->join();
        }
    }
}

uint64_t AsioDataMover::GetNextDirtyRangePos(const uint64_t& currentPos)
{
    uint64_t nextPos = currentPos + numAsyncCalls;
    return nextPos;
}


void AsioDataMover::SendAsyncRead(
    io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, int fd, off_t offset, size_t ioSize)
{
    io_prep_pread(&asyncReq->iocb, fd, asyncReq->data.get(), ioSize, offset);
    m_logger->Debug("reading pos: " + std::to_string(static_cast<uint64_t>(offset)));
    auto res = io_submit(ioContext, 1, &asyncReq->iocbPtr);
    if (res != 1) {
        std::string errDesc = "Error submitting read IO at offset " + std::to_string(offset) + ",io_submit returned " +
                              std::to_string(res);
        m_logger->Error("SendAsyncRead err:" + errDesc);
    }
    // for reading next offset
    m_dirtyRangesPos++;
}

uint64_t AsioDataMover::SendAsyncWrite(
    io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, off_t offset, size_t ioSize)
{
    m_logger->Debug("writing pos:" + std::to_string(static_cast<uint64_t>(offset)) + "outFd:" + std::to_string(outFd));
    io_prep_pwrite(&asyncReq->iocb, outFd, asyncReq->data.get(), ioSize, offset);
    auto res = io_submit(ioContext, 1, &asyncReq->iocbPtr);
    if (res != 1) {
        std::string errDesc = "Error submitting write IO at offset " + std::to_string(offset) + ",io_submit returned " +
                              std::to_string(res);
        throw std::runtime_error(errDesc);
    }
    return 0;
}

void AsioDataMover::CheckIgnoreBadBlock(struct io_event* event)
{
    auto obj = event->obj;
    if (obj->u.c.nbytes != event->res) {
        std::string isSkipBadBlock = Module::ConfigReader::getString("General", "RecoverIgnoreBadBlock");
        // restore job:read disk block and skip the bad block
        if (!m_isBackup && obj->aio_lio_opcode == IO_CMD_PREAD && isSkipBadBlock == "yes") {
            obj->aio_lio_opcode = IO_CMD_PWRITE;
            m_logger->Warn("Skipping bad blocks when reading backup data");
        } else {
            throw std::runtime_error("Error on READ completion" + std::to_string(event->res));
        }
    }
}

void AsioDataMover::HandleForCopyVerify(struct io_event* event, std::shared_ptr<uint8_t[]>& calBuffer)
{
    auto obj = event->obj;
    auto req = asyncReqMap.at(obj->u.c.buf).get();
    uint64_t tempOffset = static_cast<uint64_t>(event->obj->u.c.offset);
    uint64_t tempBlockSize = static_cast<uint64_t>(obj->u.c.nbytes);
    if (m_shaPtr == nullptr || !m_sha256Success) {
        m_logger->Debug("No need to handle");
        return;
    }
    std::shared_ptr<uint8_t[]> shaDataBuff = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    if (shaDataBuff == nullptr) {
        std::string errDesc = "malloc for shaDataBuff failed";
        throw std::runtime_error(errDesc);
    }
    memset_s(shaDataBuff.get(), SHA256_DIGEST_LENGTH, 0, SHA256_DIGEST_LENGTH);
    if (!m_ifCalculateIncrement) {
        m_logger->Debug("calculate sha256 for copy verification");
        if (CalculateSha256::CalculateSha256Value(req->data, tempBlockSize, shaDataBuff) != SUCCESS) {
            m_sha256Success = false;
            m_logger->Error("Failed to calculate the SHA256 value of the current block data,block offset:"
                            + std::to_string(tempOffset));
        }
    } else {
        memcpy_s(shaDataBuff.get(), SHA256_DIGEST_LENGTH, calBuffer.get(), SHA256_DIGEST_LENGTH);
    }
    uint64_t shaOffset = 0;
    if (CalculateSha256::CalculateSha256Deviation(tempOffset, shaOffset) != SUCCESS) {
        m_sha256Success = false;
        m_logger->Error("Failed to calculate the SHA256 offset of the current block data.");
    }
    BlockShaData blockShaData;
    blockShaData.offset = shaOffset;
    m_logger->Debug("Current block offsetNo=" + std::to_string((tempOffset / tempBlockSize)) + ",block offset=" +
                    std::to_string(tempOffset) + ",sha offset=" + std::to_string(shaOffset));
    blockShaData.sha256Value = shaDataBuff;
    {
        std::lock_guard<std::mutex> lock(m_shaPtrMtx);
        m_shaPtr->push_back(blockShaData);
    }
}

void AsioDataMover::HandleEvent(struct io_event* event, io_context_t ioContext, uint64_t& dirtyRangePos)
{
    auto obj = event->obj;
    auto req = asyncReqMap.at(obj->u.c.buf).get();
    off_t offset = static_cast<uint64_t>(0);
    CheckIgnoreBadBlock(event);
    // We skip write if skipWriteZeros == 1  and the read data is all zeros

    uint64_t readSize = obj->u.c.nbytes;
    bool skipCurrentOffset = false;
    if (obj->aio_lio_opcode == IO_CMD_PREAD) {
        if (m_volumeHandler != nullptr) {
            uint64_t ret = CALCULATE_INITIAL_STATE;
            if (m_shaPtr != nullptr || skipWriteZeros) {
                ret = m_volumeHandler->NeedToBackupBlock(req->data, event);
            }
            if (ret == DATA_SAME_IGNORE_WRITE || (skipWriteZeros && ret == DATA_ALL_ZERO_IGNORE_WRITE)) {
                m_logger->Debug("skip for reason:" + std::to_string(ret));
                skipCurrentOffset = true;
            }
            m_ifCalculateIncrement = ret == CALCULATE_INITIAL_STATE ? false : true;
            m_sha256Success = (ret != CALCULATE_SHA_FAILED && m_ifCalculateIncrement) ? true : false;
        }
    }
    if (obj->aio_lio_opcode == IO_CMD_PWRITE || skipCurrentOffset) {
        dirtyRangePos = GetNextDirtyRangePos(req->dirtyRangePos);
        size_t ioSize = obj->u.c.nbytes;
        if (!skipCurrentOffset) {
            transferedBytes[req->threadId] += ioSize;
        }
        if (dirtyRangePos <= maxDirtyRangePos) {
            offset = static_cast<off_t>(m_dirtyRanges[dirtyRangePos].start);
            req->dirtyRangePos = dirtyRangePos;
            SendAsyncRead(ioContext, req, inFd, offset, ioSize);
        }
    } else if (obj->aio_lio_opcode == IO_CMD_PREAD) {
        offset = event->obj->u.c.offset;
        size_t ioSize = event->obj->u.c.nbytes;
        SendAsyncWrite(ioContext, req, offset, ioSize);
    }
    threadOffset[req->threadId] = dirtyRangePos > maxDirtyRangePos ? maxDirtyRangePos : dirtyRangePos;
}

/* async WRITE completion needs to trigger the next read.
 * Since READs are done in batches of maxMemory (aligned to blockSize), the
 * next offset is currentOffset + aligned(maxMemory).
 */
void AsioDataMover::AsyncIOCompletionHandler(io_context_t ioContext)
{
    struct timespec timeout = { .tv_sec = 0, .tv_nsec = 10000000 };
    struct io_event events[AIO_NUM_HANDLE_EVENTS];
    uint64_t tempPos = 0;
    off_t offset = static_cast<off_t>(0);
    m_logger->Info("Starting completion handler, maxDirtyRangePos" + std::to_string(maxDirtyRangePos));
    bool allEventFinished = false;
    while (!allEventFinished) {
        auto numEvents = io_getevents(ioContext, AIO_NUM_HANDLE_EVENTS, AIO_NUM_HANDLE_EVENTS, events, &timeout);
        allEventFinished = true;
        /* Check if the lowest minDirtyRangePos is still in range to finish iteration */
        for (int i = 0; i < numEvents; i++) {
            try {
                HandleEvent(&events[i], ioContext, tempPos);
            } catch (const std::exception& e) {
                m_logger->Error("Caught exception:" + std::string(e.what()));
                isAbortRequested = true;
            }
            if (tempPos <= maxDirtyRangePos) {
                allEventFinished = false;
            }
        }
        if (isAbortRequested) {
            m_logger->Info("AsioDataMover completion stopped on abort request");
            break;
        }
    }
    m_logger->Info("AsioDataMover completion handler completed successfully");
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
    m_logger->Info("firstBatchMaxDirtyRangePos:" + std::to_string(firstBatchMaxDirtyRangePos));
    off_t offset = static_cast<off_t>(0);
    while (nextDirtyRangePos <= firstBatchMaxDirtyRangePos) {
        size_t ioSize = config.blockSize;
        auto asyncReq = std::make_unique<struct DataMoverAsyncReq>();
        asyncReq->data = std::make_unique<uint8_t[]>(ioSize);
        asyncReq->iocbPtr = &asyncReq->iocb;
        asyncReq->threadId = threadId;
        asyncReq->dirtyRangePos = nextDirtyRangePos;
        offset = static_cast<off_t>(m_dirtyRanges[nextDirtyRangePos].start);
        SendAsyncRead(ioContext, asyncReq.get(), inFd, offset, ioSize);
        asyncReqMap[asyncReq->data.get()] = std::move(asyncReq);
        nextDirtyRangePos++;
    }
    /* First send the first batch so that IO on device is sequential */
    auto completionHandler = std::make_unique<std::thread>(
        [this, ioContext]() { AsioDataMover::AsyncIOCompletionHandler(ioContext); });
    if (completionHandler != nullptr) {
        {
            std::lock_guard<std::mutex> lock(numThreadsMtx);
            numThreads++;  // decreased in thread function
        }
        m_logger->Info("Current thread num:" + std::to_string(numThreads));
    }
    return completionHandler;
}
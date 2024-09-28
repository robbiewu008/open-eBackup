#include "common/ConfigXmlParse.h"
#include "dataprocess/datapath/AsioDataMoverForRestore.h"

namespace {
    static const mp_string CFG_DATAPROCESS_DATAMOVER_SECTION = "DataProcessDataMover";
    static const mp_string CFG_DATAMOVER_MEMORY = "memory";
    static const mp_string CFG_DATAMOVER_THREAD_NUM = "threadNum";
    constexpr int DATA_MOVER_MAX_NUM_THREADS = 1;
    constexpr int DATA_MOVER_DEFAULT_BLOCK_SIZE = 4 * 1024 * 1024;
    constexpr uint64_t DATA_MOVER_DEFAULT_MAX_MEMORY = 1024 * 1024 * 1024;
    constexpr DataMoverConfig DEFAULT_DATA_MOVER_CONFIG = { .blockSize = DATA_MOVER_DEFAULT_BLOCK_SIZE,
        .maxMemory = DATA_MOVER_DEFAULT_MAX_MEMORY, .maxNumThreads = DATA_MOVER_MAX_NUM_THREADS };
}

AsioDataMoverForRestore::AsioDataMoverForRestore(int fileHandler, std::shared_ptr<IOEngine> vddkHandler,
    uint64_t startDirtyRangePos, std::vector<tag_dirty_range_info> dirtyRanges,
    bool skipWriteZeros) : AsioDataMover(skipWriteZeros)
{
    if (startDirtyRangePos >= dirtyRanges.size()) {
        throw std::runtime_error("Invalid data move, " + std::to_string(startDirtyRangePos) + " > "
                                 + std::to_string(dirtyRanges.size()));
    }
    if (dirtyRanges[0].length != DATA_MOVER_DEFAULT_BLOCK_SIZE) {
        throw std::runtime_error("Invalid dirtyrange block length, " + std::to_string(dirtyRanges[0].length) + " != "
                                 + std::to_string(DATA_MOVER_DEFAULT_BLOCK_SIZE));
    }
    m_dirtyRanges = dirtyRanges;
    Init(fileHandler, vddkHandler, startDirtyRangePos, DEFAULT_DATA_MOVER_CONFIG);
    m_completedBlocksCount.store(0);
    StartThreads(startDirtyRangePos);
};

void AsioDataMoverForRestore::Init(int dmInFd, std::shared_ptr<IOEngine> vddkHandle, uint64_t offsetDirtyRangePos,
    const struct DataMoverConfig& dmConfig)
{
    this->config = dmConfig;
    mp_int32 iRet;
    mp_int32 workerThreadCount = 0;
    mp_int32 memoryInMB = 0;
    uint64_t bytesInMB = 1024 * 1024;
    // parse task pool worker thread count
    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_DATAPROCESS_DATAMOVER_SECTION, CFG_DATAMOVER_THREAD_NUM,
        workerThreadCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get value");
    } else {
        COMMLOG(OS_LOG_INFO, "workerThreadCount: %llu", workerThreadCount);
        this->config.maxNumThreads =  workerThreadCount;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_DATAPROCESS_DATAMOVER_SECTION, CFG_DATAMOVER_MEMORY,
        memoryInMB);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get value");
    } else {
        COMMLOG(OS_LOG_INFO, "memoryInMB: %llu", memoryInMB);
        this->config.maxMemory =  static_cast<uint64_t>(memoryInMB) * bytesInMB;
    }
    this->inFd = dmInFd;
    this->outFd = vddkHandle;
    maxDirtyRangePos = m_dirtyRanges.size();
    // change size to pos by 'maxDirtyRangePos--'
    maxDirtyRangePos--;
    numThreads = 0;
    numDeployedThreads = 0;
    threadOffset.reserve(workerThreadCount);
    transferedBytes.reserve(workerThreadCount);
    submitedEvents.reserve(workerThreadCount);
    handledEvents.reserve(workerThreadCount);
    numAsyncCalls = config.maxMemory / config.blockSize;

    numIOsPerThread = numAsyncCalls / config.maxNumThreads;
    firstBatchDirtyRangePos = offsetDirtyRangePos + std::min<uint64_t>(maxDirtyRangePos, numAsyncCalls);
    ERRLOG("firstBatchDirtyRangePos： %llu, numIOsPerThread: %llu", firstBatchDirtyRangePos, numIOsPerThread);
}

int AsioDataMoverForRestore::SendAsyncRead(
    io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, int fd, off_t offset, size_t ioSize)
{
    io_prep_pread(&asyncReq->iocb, fd, asyncReq->data.get(), ioSize, offset);
    COMMLOG(OS_LOG_DEBUG, "reading pos: %llu", static_cast<uint64_t>(offset));
    auto res = io_submit(ioContext, 1, &asyncReq->iocbPtr);
    submitedEvents[asyncReq->threadId] = submitedEvents[asyncReq->threadId] + 1;
    if (res != 1) {
        std::string errDesc = "Error submitting read IO at offset " + std::to_string(offset) + ",io_submit returned " +
                              std::to_string(res);
        COMMLOG(OS_LOG_ERROR, "SendAsyncRead err: %ls", errDesc.c_str());
    }
    // for reading next offset
    m_dirtyRangesPos++;
    return 0;
}

uint64_t AsioDataMoverForRestore::SendAsyncWrite(
    io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, int fd, off_t offset, size_t ioSize)
{
    COMMLOG(OS_LOG_DEBUG, "writing pos: %llu", static_cast<uint64_t>(offset));
    mp_int32 write_rc = outFd->Write(offset, ioSize, asyncReq->data.get());
    if (write_rc != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Error on sync writing");
        throw std::runtime_error("Error doing sync write from disk");
    }
    transferedBytes[asyncReq->threadId] += ioSize;
    m_completedBlocksCount++;
    uint64_t nextPos = GetNextDirtyRangePos(asyncReq->dirtyRangePos);
    if (nextPos > maxDirtyRangePos) {
        COMMLOG(OS_LOG_ERROR, "finish");
        return nextPos;
    }
    asyncReq->dirtyRangePos = nextPos;
    offset = static_cast<off_t>(m_dirtyRanges[nextPos].start);
    SendAsyncRead(ioContext, asyncReq, inFd, offset, ioSize);
    return nextPos;
}

void AsioDataMoverForRestore::HandleEvent(struct io_event* event, io_context_t ioContext, uint64_t& dirtyRangePos)
{
    auto obj = event->obj;
    auto req = asyncReqMap.at(obj->u.c.buf).get();
    off_t offset = static_cast<uint64_t>(0);
    // We skip write if skipWriteZeros == 1  and the read data is all zeros
    if (obj->aio_lio_opcode == IO_CMD_PWRITE) {
        handledEvents[req->threadId] = handledEvents[req->threadId] + 1;
        COMMLOG(OS_LOG_ERROR, "here", maxDirtyRangePos);
        dirtyRangePos = GetNextDirtyRangePos(req->dirtyRangePos);
        if (dirtyRangePos <= maxDirtyRangePos) {
            size_t ioSize = obj->u.c.nbytes;
            offset = static_cast<off_t>(m_dirtyRanges[dirtyRangePos].start);
            SendAsyncRead(ioContext, req, inFd, offset, ioSize);
            // lan-base is using SyncReadDataMover
            transferedBytes[req->threadId] += ioSize;
        }
    } else if (obj->aio_lio_opcode == IO_CMD_PREAD) {
        handledEvents[req->threadId] = handledEvents[req->threadId] + 1;
        offset = event->obj->u.c.offset;
        size_t ioSize = event->obj->u.c.nbytes;
        dirtyRangePos = SendAsyncWrite(ioContext, req, 0, offset, ioSize);
    }
    threadOffset[req->threadId] = dirtyRangePos > maxDirtyRangePos ? maxDirtyRangePos : dirtyRangePos;
}

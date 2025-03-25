#include "common/ConfigXmlParse.h"
#include "dataprocess/datapath/AsioDataMoverForBackup.h"

namespace {
    static const mp_string CFG_DATAPROCESS_DATAMOVER_SECTION = "DataProcessDataMover";
    static const mp_string CFG_DATAMOVER_MEMORY = "memory";
    static const mp_string CFG_DATAMOVER_THREAD_NUM = "threadNum";
    constexpr int DATA_MOVER_MAX_NUM_THREADS = 1;
    constexpr int DATA_MOVER_DEFAULT_BLOCK_SIZE = 4 * 1024 * 1024;
    constexpr uint64_t DATA_MOVER_DEFAULT_MAX_MEMORY = 1024 * 1024 * 1024;
    constexpr DataMoverConfig DEFAULT_DATA_MOVER_CONFIG = { .blockSize = DATA_MOVER_DEFAULT_BLOCK_SIZE,
        .maxMemory = DATA_MOVER_DEFAULT_MAX_MEMORY, .maxNumThreads = DATA_MOVER_MAX_NUM_THREADS };
    const uint64_t ALL_BLOCKS_ZERO = 99;
    const char ZERO_ARR[4194304] = {0};
}

AsioDataMoverForBackup::AsioDataMoverForBackup(int fileHandler, std::shared_ptr<IOEngine> vddkHandler,
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

void AsioDataMoverForBackup::Init(int dmOutFd, std::shared_ptr<IOEngine> vddkHandle, uint64_t offsetDirtyRangePos,
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

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_DATAPROCESS_DATAMOVER_SECTION,
        CFG_DATAMOVER_MEMORY, memoryInMB);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to get value");
    } else {
        COMMLOG(OS_LOG_INFO, "memoryInMB: %llu", memoryInMB);
        this->config.maxMemory =  static_cast<uint64_t>(memoryInMB) * bytesInMB;
    }
    this->outFd = dmOutFd;
    this->inFd = vddkHandle;
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
    ERRLOG("firstBatchDirtyRangePos: %llu, numIOsPerThread: %llu", firstBatchDirtyRangePos, numIOsPerThread);
}


static inline bool AllZerosNew(struct DataMoverAsyncReq* asyncReq, size_t lenBytes)
{
    if (memcmp(asyncReq->data.get(), ZERO_ARR, lenBytes) == 0) {
        return true;
    }
    return false;
}


static inline bool AllZeros(struct DataMoverAsyncReq* asyncReq, size_t lenBytes)
{
    uint64_t* data = reinterpret_cast<uint64_t*>(asyncReq->data.get());
    for (; data < reinterpret_cast<uint64_t*>(asyncReq->data.get() + lenBytes); data++) {
        if (*data) {
            return false;
        }
    }
    return true;
}

int AsioDataMoverForBackup::SendAsyncRead(
    io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, int fd, off_t offset, size_t ioSize)
{
    COMMLOG(OS_LOG_DEBUG, "reading pos: %llu", static_cast<uint64_t>(offset));
    mp_int32 read_rc = inFd->Read(offset, ioSize, asyncReq->data.get());
    if (read_rc != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Error on sync reading");
        throw std::runtime_error("Error doing sync read from disk");
    }
    while (skipWriteZeros && AllZeros(asyncReq, ioSize) && AllZerosNew(asyncReq, ioSize)) {
        COMMLOG(OS_LOG_DEBUG, "skipping 4MB block from pos %llu cause it is all zero",
            m_dirtyRanges[asyncReq->dirtyRangePos].start);

        uint64_t dirtyRangePos = GetNextDirtyRangePos(asyncReq->dirtyRangePos);
        if (dirtyRangePos <= maxDirtyRangePos) {
            offset = static_cast<off_t>(m_dirtyRanges[dirtyRangePos].start);
            asyncReq->dirtyRangePos = dirtyRangePos;
            read_rc = inFd->Read(offset, ioSize, asyncReq->data.get());
            if (read_rc != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "Error on sync reading");
                throw std::runtime_error("Error doing sync read from disk");
            }
        } else {
            COMMLOG(OS_LOG_INFO, "All rest of data are zeros, will finish datamover");
            return ALL_BLOCKS_ZERO;
        }
    }
    SendAsyncWrite(ioContext, asyncReq, outFd, offset, ioSize);
    return 0;
}

uint64_t AsioDataMoverForBackup::SendAsyncWrite(
    io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, int fd, off_t offset, size_t ioSize)
{
    COMMLOG(OS_LOG_DEBUG, "writing pos: %llu", static_cast<uint64_t>(offset));
    io_prep_pwrite(&asyncReq->iocb, fd, asyncReq->data.get(), ioSize, offset);
    auto res = io_submit(ioContext, 1, &asyncReq->iocbPtr);
    submitedEvents[asyncReq->threadId] = submitedEvents[asyncReq->threadId] + 1;
    if (res != 1) {
        std::string errDesc = "Error submitting write IO at offset " + std::to_string(offset) + ",io_submit returned " +
                              std::to_string(res);
        throw std::runtime_error(errDesc);
    }
    return 0;
}

void AsioDataMoverForBackup::HandleEvent(struct io_event* event, io_context_t ioContext, uint64_t& dirtyRangePos)
{
    auto obj = event->obj;
    auto req = asyncReqMap.at(obj->u.c.buf).get();
    off_t offset = static_cast<uint64_t>(0);
    if (obj->aio_lio_opcode == IO_CMD_PWRITE) {
        handledEvents[req->threadId] = handledEvents[req->threadId] + 1;
        size_t ioSize = obj->u.c.nbytes;
        transferedBytes[req->threadId] += ioSize;
        m_completedBlocksCount++;
        dirtyRangePos = GetNextDirtyRangePos(req->dirtyRangePos);
        COMMLOG(OS_LOG_DEBUG, "write finish, offset: %llu, next dirtyRangePos:%llu, maxDirtyRangePos:%llu",
                m_dirtyRanges[req->dirtyRangePos].start,
                dirtyRangePos,
                maxDirtyRangePos);
        if (dirtyRangePos <= maxDirtyRangePos) {
            offset = static_cast<off_t>(m_dirtyRanges[dirtyRangePos].start);
            req->dirtyRangePos = dirtyRangePos;
            if (SendAsyncRead(ioContext, req, 0, offset, ioSize) == ALL_BLOCKS_ZERO) {
                COMMLOG(OS_LOG_INFO, "no need to read any more since all blocks are zero blocks");
                // maxDirtyRangePos + 1 means no next event is needed
                dirtyRangePos = maxDirtyRangePos + 1;
            } else {
                // refresh dirtyRangePos for skip zeros
                dirtyRangePos = req->dirtyRangePos;
            }
        } else {
            COMMLOG(OS_LOG_INFO, "finish", maxDirtyRangePos);
        }
    } else if (obj->aio_lio_opcode == IO_CMD_PREAD) {
        handledEvents[req->threadId] = handledEvents[req->threadId] + 1;
        COMMLOG(OS_LOG_ERROR, "impossible branch", maxDirtyRangePos);
        throw std::runtime_error("impossible branch");
    }
    threadOffset[req->threadId] = dirtyRangePos > maxDirtyRangePos ? maxDirtyRangePos : dirtyRangePos;
}
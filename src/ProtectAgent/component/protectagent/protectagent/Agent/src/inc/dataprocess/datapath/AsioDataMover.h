#ifndef DATA_MOVER_H
#define DATA_MOVER_H

#include <chrono>
#include <map>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>
#include <atomic>
#include "libaio/include/libaio.h"
#include "common/Log.h"
#include "dataprocess/ioscheduler/VMwareIOEngine.h"
#include "apps/vmwarenative/VMwareDef.h"


struct DataMoverConfig {
    size_t blockSize;
    size_t maxMemory;
    uint32_t maxNumThreads;
};

struct DataMoverAsyncReq {
    std::shared_ptr<uint8_t[]> data;
    struct iocb iocb;
    struct iocb* iocbPtr;
    int threadId = 0;
    uint64_t dirtyRangePos = 0;
};
struct BlockShaData {
    off_t offset;
    std::shared_ptr<uint8_t[]> sha256Value;
};

class AsioDataMover  {
public:
    AsioDataMover(bool skipWriteZeros);
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

    int GetNumThreads() const
    {
        return numThreads;
    }

    void SetNumThreads(int numOfThreads)
    {
        {
            std::lock_guard<std::mutex> lock(numThreadsMtx);
            this->numThreads = numOfThreads;
        }
    }
    uint64_t GetThreadNum();
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
protected:
    void StartThreads(uint64_t startDirtyRangePos);
    std::unique_ptr<std::thread> StartMoveThread(const uint64_t&  startDirtyRangePos);
    virtual void HandleEvent(struct io_event* event, io_context_t ioContext, uint64_t& dirtyRangePos) = 0;
    virtual void Init(int dmInFd, std::shared_ptr<IOEngine> dmOutFd, uint64_t offsetDirtyRangePos,
              const struct DataMoverConfig& dmConfig) = 0;

    void AsyncIOCompletionHandler(io_context_t ioContext, const int& threadId);

    uint64_t GetNextDirtyRangePos(const uint64_t& currentPos);

    virtual int SendAsyncRead(
        io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, int fd, off_t offset, size_t ioSize) = 0;
    virtual uint64_t SendAsyncWrite(
        io_context_t ioContext, struct DataMoverAsyncReq* asyncReq, int fd, off_t offset, size_t ioSize) = 0;

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
    int numDeployedThreads;
    std::vector<uint64_t> threadOffset;
    bool skipWriteZeros = false;
    std::vector<size_t> transferedBytes;
    std::vector<size_t> submitedEvents;
    std::vector<size_t> handledEvents;
    std::vector<tag_dirty_range_info> m_dirtyRanges;
    std::atomic<std::uint64_t> m_dirtyRangesPos;
    std::atomic<std::uint64_t> m_completedBlocksCount;
};
#endif  // DATA_MOVER_H

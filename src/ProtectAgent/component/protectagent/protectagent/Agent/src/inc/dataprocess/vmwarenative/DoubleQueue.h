#ifndef AGENT_VMWARENATIVE_DOUBLEQUEUE_CACHE_H
#define AGENT_VMWARENATIVE_DOUBLEQUEUE_CACHE_H

#include <string>
#include <vector>
#include "common/Types.h"
#include "MutexLock.h"
#include "dataprocess/vmwarenative/Define.h"
#include "dataprocess/vmwarenative/VMwareDiskApi.h"

/*
 * Implement multi read and single write to mounted file or remote vCenter vm
 * Only single read and single write supported currently
 */
namespace AGENT_VMWARENATIVE_DOUBLEQUEUE {
class DoubleQueue {
public:
    DoubleQueue();
    ~DoubleQueue();

    std::vector<st_disk_datablock> GetReadQueue();
    std::vector<st_disk_datablock> GetWriteQueue();

    // enter double queue
    void EnQueue(st_disk_datablock& datablock);

    // pop double queue
    bool DeQueue();
    // write data to remote vCenter vm disk - SAN
    bool DeQueueToVMwareDisk(const std::shared_ptr<VMwareDiskApi>& sPtr, int done, int condition);
    // write data to file mounted - SAN
    bool DeQueueToDataLun(FILE* file, int done, int& condition);
    mp_uint64 GetNumberOfDataBlockCompleted();
    void SetNumberOfDataBlockCompleted(mp_uint64 datablockNum);

private:
    std::vector<st_disk_datablock> m_writeQueue;
    std::vector<st_disk_datablock> m_readQueue;
    AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock m_mutex_write;
    mp_uint64 m_iNumOfBlockCompleted;
};
}  // namespace AGENT_VMWARENATIVE_DOUBLEQUEUE
#endif
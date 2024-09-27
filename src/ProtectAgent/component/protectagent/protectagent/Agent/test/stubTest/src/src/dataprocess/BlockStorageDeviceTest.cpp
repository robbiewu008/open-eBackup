#include "dataprocess/BlockStorageDeviceTest.h"
#include "dataprocess/vmwarenative/MutexLock.h"
#include "dataprocess/vmwarenative/MutexLockGuard.h"
#include "dataprocess/vmwarenative/CountDown.h"
#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubFileBlockStorageDeviceGetValueInt32Return);                                                                      \
    } while (0)

using namespace std;
using namespace AGENT_VMWARENATIVE_MUTEXLOCK;
using namespace AGENT_VMWARENATIVE_COUNTDOWN;
using namespace AGENT_VMWARENATIVE_MUTEXLOCKGUARD;
using namespace AGENT_VMWARENATIVE_DOUBLEQUEUE;

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(BlockStorageDeviceTest, read)
{
    DoubleQueue dQueue;
    vmware_volume_info volumeInfo;
    mp_int32 condition;
    AGENT_VMWARENATIVE_COUNTDOWN::CountDown countdown(1);
    AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock mutex;
    BlockStorageDevice om;

    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    //open lun failed
    {
        om.read(dQueue, volumeInfo, condition, countdown, mutex);
    }
    //ulDiskSize = 0
    {
        volumeInfo.strTargetLunPath = "test";
        volumeInfo.ulDiskSize = 0;
        tag_dirty_range_info(0, 2);
        om.read(dQueue, volumeInfo, condition, countdown, mutex);
    }
    //ulDiskSize > 0
    {
        volumeInfo.strTargetLunPath = "test";
        volumeInfo.ulDiskSize = 41;
        tag_dirty_range_info(0, 2);
        om.read(dQueue, volumeInfo, condition, countdown, mutex);
    }
}

TEST_F(BlockStorageDeviceTest, write)
{
    DoubleQueue dQueue;
    vmware_volume_info volumeInfo;
    mp_int32 condition;
    AGENT_VMWARENATIVE_COUNTDOWN::CountDown countdown(1);
    AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock mutex;
    mp_uint64 backupLevel = 2;
    BlockStorageDevice om;

    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    //open lun failed
    {
        om.write(dQueue, volumeInfo, condition, countdown, mutex, backupLevel);
    }
    
    {
        volumeInfo.strTargetLunPath = "test";
        tag_dirty_range_info(0, 2);
        stub.set(ADDR(DoubleQueue, DeQueueToDataLun), StubDeQueueToDataLun);
        stub.set(ADDR(DoubleQueue, GetNumberOfDataBlockCompleted), StubGetNumberOfDataBlockCompleted);
        om.write(dQueue, volumeInfo, condition, countdown, mutex, backupLevel);
    }
 
    stub.reset(ADDR(DoubleQueue, DeQueueToDataLun));
    stub.reset(ADDR(DoubleQueue, GetNumberOfDataBlockCompleted));
}
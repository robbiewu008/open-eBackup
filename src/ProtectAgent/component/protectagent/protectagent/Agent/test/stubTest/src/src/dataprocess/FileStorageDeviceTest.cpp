#include "dataprocess/FileStorageDeviceTest.h"

using namespace AGENT_VMWARENATIVE_MUTEXLOCK;
using namespace AGENT_VMWARENATIVE_COUNTDOWN;
using namespace AGENT_VMWARENATIVE_MUTEXLOCKGUARD;
using namespace AGENT_VMWARENATIVE_DOUBLEQUEUE;

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(FileStorageDeviceTest, ReadDataBlockToQueue)
{
    DoubleQueue dQueue;
    FILE* file;
    char* ch = new char[VMWARE_DATABLOCK_SIZE];
    std::unique_ptr<char[]> buffer(ch);
    vmware_volume_info volumeInfo;
    mp_uint64 block;
    FileStorageDevice om;

    stub.set(&CLogger::Log, StubCLoggerLog);
    //open file failed
    {
        om.ReadDataBlockToQueue(dQueue, file, buffer, volumeInfo, block);
    }
    //ulDiskSize = 0
    {
        file = fopen("test", "r");
        dirty_range tmp;
        tmp.start = 2;
        tmp.length = 1;
        volumeInfo.vecDirtyRange.push_back(tmp);
        volumeInfo.ulDiskSize = 0;
        tag_dirty_range_info(0, 2);
        om.ReadDataBlockToQueue(dQueue, file, buffer, volumeInfo, block);
    }
    //ulDiskSize > 0
    {
        dirty_range tmp;
        tmp.start = 2;
        tmp.length = 1;
        volumeInfo.vecDirtyRange.push_back(tmp);
        volumeInfo.ulDiskSize = 41;
        tag_dirty_range_info(0, 2);
        om.ReadDataBlockToQueue(dQueue, file, buffer, volumeInfo, block);
    }
}

TEST_F(FileStorageDeviceTest, read)
{
    DoubleQueue dQueue;
    vmware_volume_info volumeInfo;
    mp_int32 condition;
    AGENT_VMWARENATIVE_COUNTDOWN::CountDown countdown(1);
    AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock mutex;
    FileStorageDevice om;
    om.m_strPathSeparatorIdentifier = "/";
    om.m_strNasFilesystemMnt = "/opt/advbackup/vmware/data/";
    om.m_strDiskFileExtension = "-flat.vmdk";
    om.m_strDiskDescFileExtension = ".vmdk";
    om.m_iSectorSize = 512;

    stub.set(&CLogger::Log, StubCLoggerLog);

    om.read(dQueue, volumeInfo, condition, countdown, mutex);
}

TEST_F(FileStorageDeviceTest, PrepareForDiskCopy)
{
    vmware_volume_info volumeInfo;
    mp_string folder = "../dataprocess";
    volumeInfo.strDiskID = "test";
    FILE **file;
    mp_uint64 backupLevel = 1;
    FileStorageDevice om;
    mp_bool iRet = false;

    stub.set(&CLogger::Log, StubCLoggerLog);
    //增量
    {
        iRet = om.PrepareForDiskCopy(volumeInfo, folder, file, backupLevel);
        EXPECT_EQ(false, iRet);
    }
    //全量
    {
        backupLevel = 2;
        iRet = om.PrepareForDiskCopy(volumeInfo, folder, file, backupLevel);
        EXPECT_EQ(false, iRet);
    }
}

TEST_F(FileStorageDeviceTest, GenerateDiskDescFile)
{
    vmware_volume_info volumeInfo;
    mp_string folder = "../dataprocess";
    mp_uint64 backupLevel = 2;
    volumeInfo.strDiskID = "test";
    FileStorageDevice om;
    mp_bool iRet = false;
    stub.set(&CLogger::Log, StubCLoggerLog);

    iRet = om.GenerateDiskDescFile(backupLevel, volumeInfo, folder);
    EXPECT_EQ(false, iRet);
}

TEST_F(FileStorageDeviceTest, write)
{
    DoubleQueue dQueue;
    vmware_volume_info volumeInfo;
    mp_int32 condition;
    AGENT_VMWARENATIVE_COUNTDOWN::CountDown countdown(1);
    AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock mutex;
    mp_uint64 backupLevel = 2;
    FileStorageDevice om;

    stub.set(&CLogger::Log, StubCLoggerLog);
    //open lun failed
    {
        om.write(dQueue, volumeInfo, condition, countdown, mutex, backupLevel);
    }
    
    {
        tag_dirty_range_info(0, 2);
        stub.set(ADDR(FileStorageDevice, PrepareForDiskCopy), StubPrepareForDiskCopy);
        stub.set(ADDR(FileStorageDevice, GenerateDiskDescFile), StubGenerateDiskDescFile);
        stub.set(ADDR(DoubleQueue, DeQueueToDataLun), StubDeQueueToDataLunSucc);
        stub.set(ADDR(DoubleQueue, GetNumberOfDataBlockCompleted), StubGetNumberOfDataBlockCompletedSucc);
        om.write(dQueue, volumeInfo, condition, countdown, mutex, backupLevel);
    }
 
    stub.reset(ADDR(FileStorageDevice, PrepareForDiskCopy));
    stub.reset(ADDR(FileStorageDevice, GenerateDiskDescFile));
    stub.reset(ADDR(DoubleQueue, DeQueueToDataLun));
    stub.reset(ADDR(DoubleQueue, GetNumberOfDataBlockCompleted));
}

TEST_F(FileStorageDeviceTest, GenerateDescFileContent)
{
    vmware_volume_info volumeInfo;
    mp_string strContent;
    FileStorageDevice om;

    om.GenerateDescFileContent(volumeInfo, strContent);
}

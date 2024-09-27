#ifndef __AGENT_VMWARENATIVE_FILESTORAGEDEVICE_TEST_H__
#define __AGENT_VMWARENATIVE_FILESTORAGEDEVICE_TEST_H__

#define private public
#include "dataprocess/vmwarenative/FileStorageDevice.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class FileStorageDeviceTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void FileStorageDeviceTest::SetUp() {}

void FileStorageDeviceTest::TearDown() {}

void FileStorageDeviceTest::SetUpTestCase() {}

void FileStorageDeviceTest::TearDownTestCase() {}

mp_uint64 StubGetNumberOfDataBlockCompletedSucc()
{
    return 0;
}

mp_bool StubPrepareForDiskCopy(const vmware_volume_info &volumeInfo, mp_string &folder, FILE **file,
    mp_uint64 backupLevel)
{
    return true;
}

mp_bool StubDeQueueToDataLunSucc(FILE *file, int done, int &condition)
{
    return true;
}

mp_bool StubGenerateDiskDescFile(
    mp_uint64 backupLevel, const vmware_volume_info& volumeInfo, const mp_string& folder)
{
    return true;
}

#endif
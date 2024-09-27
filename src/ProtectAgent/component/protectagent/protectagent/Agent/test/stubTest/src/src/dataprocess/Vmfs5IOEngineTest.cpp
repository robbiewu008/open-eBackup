#include "dataprocess/Vmfs5IOEngineTest.h"

using namespace NSVmfsIO;
using namespace Vmfs5IO;

#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string &, const mp_string &, mp_int32 &))ADDR(                \
                     CConfigXmlParser, GetValueInt32),                                                                 \
            StubFileIOEngineGetValueInt32Return);                                                                      \
    } while (0)

static mp_void StubCLoggerLog(mp_void)
{
    return;
}

TEST_F(vmfs5IOEngineTest, TEST_Open_Success)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs5IOEngine vmfs5IOEngine(vol, protectType);
    vmfs5IOEngine.m_filename = "target";
    vmfs5IOEngine.m_volInfo.strDiskRelativePath = "target.vmdk";
    stub.set(VmfsDirent::VmfsDirOpenFromBlkid, StubReturnVmfsDirT);
    stub.set(VmfsDirent::VmfsDirLookup, StubReturnVmfsDirentT);
    stub.set(VmfsInode::StatFromBlkid, 0);
    stub.set(VmfsFile::OpenFromBlkid, 1);
    iRet = vmfs5IOEngine.Open();
    // EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(vmfs5IOEngineTest, TEST_Open_Failed_LookupFailed)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs5IOEngine vmfs5IOEngine(vol, protectType);
    vmfs5IOEngine.m_filename = "target";
    vmfs5IOEngine.m_volInfo.strDiskRelativePath = "target.vmdk";
    stub.set(VmfsDirent::VmfsDirOpenFromBlkid, NULL);
    iRet = vmfs5IOEngine.Open();
    // EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(vmfs5IOEngineTest, TEST_Open_Failed_GetInodeInfoFailed)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs5IOEngine vmfs5IOEngine(vol, protectType);
    vmfs5IOEngine.m_filename = "target";
    vmfs5IOEngine.m_volInfo.strDiskRelativePath = "target.vmdk";
    stub.set(VmfsDirent::VmfsDirOpenFromBlkid, StubReturnVmfsDirT);
    stub.set(VmfsDirent::VmfsDirLookup, NULL);
    iRet = vmfs5IOEngine.Open();
    // EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(vmfs5IOEngineTest, TEST_Read_Success)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs5IOEngine vmfs5IOEngine(vol, protectType);
    // stub.set(VMFS_FILEFD, 1);
    stub.set(VmfsFile::Pread, 0);
    iRet = vmfs5IOEngine.Read(0, 0, "");
    // EXPECT_EQ(MP_SUCCESS, iRet);
}
TEST_F(vmfs5IOEngineTest, TEST_Read_Failed_BlockTypeError)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs5IOEngine vmfs5IOEngine(vol, protectType);
    // stub.set(VMFS_FILEFD, 0);
    iRet = vmfs5IOEngine.Read(0, 0, "");
    // EXPECT_EQ(MP_FAILED, iRet);
}
TEST_F(vmfs5IOEngineTest, TEST_Read_Failed_ReadIOFailed)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs5IOEngine vmfs5IOEngine(vol, protectType);
    // stub.set(VMFS_FILEFD, 1);
    stub.set(VmfsFile::Pread, -1);
    iRet = vmfs5IOEngine.Read(0, 0, "");
    // EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(vmfs5IOEngineTest, TEST_Close_Success)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs5IOEngine vmfs5IOEngine(vol, protectType);
    // stub.set(VMFS_FILEFD, 1);
    stub.set(VmfsFile::Close, 0);
    iRet = vmfs5IOEngine.Close();
    // EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(vmfs5IOEngineTest, TEST_Close_Failed)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs5IOEngine vmfs5IOEngine(vol, protectType);
    // stub.set(VMFS_FILEFD, 0);
    iRet = vmfs5IOEngine.Close();
    // EXPECT_EQ(MP_FAILED, iRet);
}
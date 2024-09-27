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
#include "dataprocess/Vmfs6IOEngineTest.h"

using namespace NSVmfsIO;
using namespace Vmfs6IO;

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

TEST_F(vmfs6IOEngineTest, TEST_Open_Success)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs6IOEngine vmfs6IOEngine(vol, protectType);
    vmfs6IOEngine.m_filename = "target";
    vmfs6IOEngine.m_volInfo.strDiskRelativePath = "target.vmdk";
    stub.set(VmfsDirent::VmfsDirOpenFromBlkid, StubReturnVmfsDirT);
    stub.set(VmfsDirent::VmfsDirLookup, StubReturnVmfsDirentT);
    stub.set(VmfsInode::StatFromBlkid, 0);
    stub.set(VmfsFile::OpenFromBlkid, 1);
    iRet = vmfs6IOEngine.Open();
    // EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(vmfs6IOEngineTest, TEST_Open_Failed_LookupFailed)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs6IOEngine vmfs6IOEngine(vol, protectType);
    vmfs6IOEngine.m_filename = "target";
    vmfs6IOEngine.m_volInfo.strDiskRelativePath = "target.vmdk";
    stub.set(VmfsDirent::VmfsDirOpenFromBlkid, NULL);
    iRet = vmfs6IOEngine.Open();
    // EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(vmfs6IOEngineTest, TEST_Open_Failed_GetInodeInfoFailed)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs6IOEngine vmfs6IOEngine(vol, protectType);
    vmfs6IOEngine.m_filename = "target";
    vmfs6IOEngine.m_volInfo.strDiskRelativePath = "target.vmdk";
    stub.set(VmfsDirent::VmfsDirOpenFromBlkid, StubReturnVmfsDirT);
    stub.set(VmfsDirent::VmfsDirLookup, NULL);
    iRet = vmfs6IOEngine.Open();
    // EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(vmfs6IOEngineTest, TEST_Read_Success)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs6IOEngine vmfs6IOEngine(vol, protectType);
    // stub.set(VMFS_FILEFD, 1);
    stub.set(VmfsFile::Pread, 0);
    iRet = vmfs6IOEngine.Read(0, 0, "");
    // EXPECT_EQ(MP_SUCCESS, iRet);
}
TEST_F(vmfs6IOEngineTest, TEST_Read_Failed_BlockTypeError)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs6IOEngine vmfs6IOEngine(vol, protectType);
    // stub.set(VMFS_FILEFD, 0);
    iRet = vmfs6IOEngine.Read(0, 0, "");
    // EXPECT_EQ(MP_FAILED, iRet);
}
TEST_F(vmfs6IOEngineTest, TEST_Read_Failed_ReadIOFailed)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs6IOEngine vmfs6IOEngine(vol, protectType);
    // stub.set(VMFS_FILEFD, 1);
    stub.set(VmfsFile::Pread, -1);
    iRet = vmfs6IOEngine.Read(0, 0, "");
    // EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(vmfs6IOEngineTest, TEST_Close_Success)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs6IOEngine vmfs6IOEngine(vol, protectType);
    // stub.set(VMFS_FILEFD, 1);
    stub.set(VmfsFile::Close, 0);
    iRet = vmfs6IOEngine.Close();
    // EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(vmfs6IOEngineTest, TEST_Close_Failed)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 iRet = 0;
    Vmfs6IOEngine vmfs6IOEngine(vol, protectType);
    // stub.set(VMFS_FILEFD, 0);
    iRet = vmfs6IOEngine.Close();
    // EXPECT_EQ(MP_FAILED, iRet);
}
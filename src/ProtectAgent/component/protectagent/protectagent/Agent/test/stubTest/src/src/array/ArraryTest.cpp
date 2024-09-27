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
#include "array/ArrayTest.h"

//Begin CArrayTest
TEST_F(CArrayTest, Con_Des)
{
    Array arr;
    EXPECT_TRUE(1);
}
/*TEST_F(CArrayTest, OpenDev)
{
    mp_string str = "test";
    mp_int32 iDevFd;
    mp_int32 rst;
    stub.set(&realpath, StubrealpathEq1);
    rst = Array::OpenDev(str, iDevFd);
    EXPECT_EQ(rst, MP_FAILED);
    
    stub.set(&realpath, StubrealpathEq0);
    //open = 0
    {
        stub.set(&open, StubopenEq0);
        rst = Array::OpenDev(str, iDevFd);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
    //open < 0
    {
        stub.set(&open, StubopenLt0);
        rst = Array::OpenDev(str, iDevFd);
        EXPECT_EQ(rst, ERROR_COMMON_OPER_FAILED);
    }
}*/
TEST_F(CArrayTest, GetDiskArrayInfo)
{
    mp_string strDev = "test";
    mp_string strVendor = "test";
    mp_string strProduct = "test";
    mp_int32 rst = 0;
    //GetVendorAndProduct < 0
    {
        stub.set(&open, StubopenEq0);
        stub.set(&ioctl, StubioctlLt0);
        rst = Array::GetDiskArrayInfo(strDev, strVendor, strProduct);
        EXPECT_EQ(rst, -1);
    }
    //open < 0
    {
        stub.set(&open, StubopenLt0);
        rst = Array::GetDiskArrayInfo(strDev, strVendor, strProduct);
        EXPECT_EQ(rst, MP_FAILED);
    }
    // open = 0 and GetVendorAndProduct = 0
    {
        stub.set(&open, StubopenEq0);
        stub.set(&ioctl, StubioctlEq0);
        rst = Array::GetDiskArrayInfo(strDev, strVendor, strProduct);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CArrayTest, GetDisk83Page)
{
    mp_string strDevice = "test";
    mp_string strLunWWN = "test";
    mp_string strLunID = "test";
    mp_int32 rst = 0;
    stub.set(&close, &StubcloseEq0);
    //open < 0
    {
        stub.set(&open, &StubopenLt0);
        rst = Array::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //GetDiskPage < 0
    {
        stub.set(&open, &StubopenEq0);
        stub.set(&ioctl, &StubioctlLt0);
        rst = Array::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, -1);
    }
    //buff > 32
    {
        stub.set(&open, &StubopenEq0);
        stub.set(&ioctl, &StubioctlEq0GetDisk83Page);
        stub.set(&Array::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN);
        rst = Array::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //normal
    {
        strDevice = "/dev/HUAWEI";
        stub.set(&open, &StubopenEq0);
        stub.set(&ioctl, &StubioctlEq0Buf30);
        stub.set(&Array::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN);
        stub.set(&Array::GetDiskWWNAndLUNID, &StubCGetDiskWWNAndLUNIDLt0);
        rst = Array::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_FAILED);
        stub.set(&Array::GetDiskWWNAndLUNID, &StubCGetDiskWWNAndLUNIDEq0);
        rst = Array::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
    //normal
    {
        strDevice = "/dev/Huawei";
        stub.set(&open, &StubopenEq0);
        stub.set(&ioctl, &StubioctlEq0Buf30);
        stub.set(&Array::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN);
        stub.set(&Array::GetFusionStorageWWNAndLUNID, &StubCGetDiskWWNAndLUNIDLt0);
        rst = Array::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_FAILED);
        stub.set(&Array::GetFusionStorageWWNAndLUNID, &StubCGetDiskWWNAndLUNIDEq0);
        rst = Array::GetDisk83Page(strDevice, strLunWWN, strLunID);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CArrayTest, GetDisk80Page)
{
    mp_string strDevice = "test";
    mp_string strSN = "test";
    mp_int32 rst = 0;
    stub.set(&close, &StubcloseEq0);
    //open < 0
    {
        stub.set(&open, &StubopenLt0);
        rst = Array::GetDisk80Page(strDevice, strSN);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //GetDiskPage < 0
    {
        stub.set(&open, &StubopenEq0);
        stub.set(&ioctl, &StubioctlLt0);
        rst = Array::GetDisk80Page(strDevice, strSN);
        EXPECT_EQ(rst, -1);
    }
    //buff > 64
    {
        stub.set(&open, &StubopenEq0);
        stub.set(&ioctl, &StubioctlEq0GetDisk83Page);
        stub.set(&Array::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN);
        mp_string strDev = "/dev/test";
        rst = Array::GetDisk80Page(strDev, strSN);
        EXPECT_EQ(rst, ERROR_COMMON_QUERY_APP_LUN_FAILED);

    }
    //noraml
    {
        strDevice = "/dev/HUAWEI";
        stub.set(&open, &StubopenEq0);
        stub.set(&ioctl, &StubioctlEq0Buf30);
        stub.set(&Array::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN);
        stub.set(&Array::GetDiskSN, &StubCGetDiskSNLt0);
        rst = Array::GetDisk80Page(strDevice, strSN);
        EXPECT_EQ(rst, MP_FAILED);
        stub.set(&Array::GetDiskSN, &StubCGetDiskSNEq0);
        rst = Array::GetDisk80Page(strDevice, strSN);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
    //noraml
    {
        strDevice = "/dev/HUAWEI";
        stub.set(&open, &StubopenEq0);
        stub.set(&ioctl, &StubioctlEq0Buf30);
        stub.set(&Array::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN);
        stub.set(&Array::GetDiskFusionStorageSN, &StubCGetDiskSNLt0);
        rst = Array::GetDisk80Page(strDevice, strSN);
        EXPECT_EQ(rst, MP_SUCCESS);
        stub.set(&Array::GetDiskFusionStorageSN, &StubCGetDiskSNEq0);
        rst = Array::GetDisk80Page(strDevice, strSN);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}

TEST_F(CArrayTest, HextoDec)
{
    mp_int32 rst = Array::HextoDec(NULL, NULL, 1);
    EXPECT_EQ(rst, MP_FAILED);
    mp_char str[] = "test";
    mp_char tmp[] = "1a";
    Array::HextoDec(str, tmp, 2);
    mp_char str1[] = "test";
    mp_char tmp1[] = "jk";
    rst = Array::HextoDec(str1, tmp1, 2);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CArrayTest, HexEncode)
{
    mp_int32 des;
    mp_int32 rst = Array::HexEncode('a', 1, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = Array::HexEncode('b', 0, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = Array::HexEncode('c', 0, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = Array::HexEncode('d', 0, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = Array::HexEncode('e', 0, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = Array::HexEncode('f', 0, des);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = Array::HexEncode('g', 0, des);
    EXPECT_EQ(rst, MP_FAILED);
}

TEST_F(CArrayTest, GetDiskWWNAndLUNID)
{
    mp_string strLunWWN;
    mp_string strLunID;
    mp_int32 buffLen = 256;
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
    aucBuffer[7] = 36;
    mp_int32 rst = Array::GetDiskWWNAndLUNID(aucBuffer, buffLen, strLunWWN, strLunID);
    EXPECT_EQ(rst, MP_FAILED);
    
    aucBuffer[7] = 30;
    stub.set(&Array::BinaryToAscii, &StubCBinaryToAsciiLt0);
    rst = Array::GetDiskWWNAndLUNID(aucBuffer, buffLen, strLunWWN, strLunID);
    EXPECT_EQ(rst, MP_FAILED);
    
    stub.set(&Array::BinaryToAscii, &StubCBinaryToAsciiEq0);
    stub.set(&Array::ConvertLUNIDtoAscii, &StubCConvertLUNIDtoAsciiLt0);
    rst = Array::GetDiskWWNAndLUNID(aucBuffer, buffLen, strLunWWN, strLunID);
    EXPECT_EQ(rst, MP_FAILED);
    
    stub.set(&Array::BinaryToAscii, &StubCBinaryToAsciiEq0);
    stub.set(&Array::ConvertLUNIDtoAscii, &StubCConvertLUNIDtoAsciiEq0);
    rst = Array::GetDiskWWNAndLUNID(aucBuffer, buffLen, strLunWWN, strLunID);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CArrayTest, GetFusionStorageWWNAndLUNID)
{
    mp_string strLunWWN;
    mp_string strLunID;
    mp_int32 buffLen = 256;
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
    aucBuffer[47] = 36;
    mp_int32 rst = Array::GetFusionStorageWWNAndLUNID(aucBuffer, buffLen, strLunWWN, strLunID);
    EXPECT_EQ(rst, MP_FAILED);
    
    aucBuffer[47] = 30;
    stub.set(&Array::BinaryToAscii, &StubCBinaryToAsciiLt0);
    rst = Array::GetFusionStorageWWNAndLUNID(aucBuffer, buffLen, strLunWWN, strLunID);
    EXPECT_EQ(rst, MP_FAILED);
    
    stub.set(&Array::BinaryToAscii, &StubCBinaryToAsciiEq0);
    rst = Array::GetFusionStorageWWNAndLUNID(aucBuffer, buffLen, strLunWWN, strLunID);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CArrayTest, GetDiskSN)
{
    mp_string strSN;
    mp_int32 buffLen = 256;
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
    aucBuffer[3] = 65;
    mp_int32 rst = Array::GetDiskSN(aucBuffer, buffLen, strSN);
    EXPECT_EQ(rst, MP_FAILED);
    
    aucBuffer[3] = 32;
    rst = Array::GetDiskSN(aucBuffer, buffLen, strSN);
    EXPECT_EQ(rst, MP_SUCCESS);
}

/*TEST_F(CArrayTest, GetDiskFusionStorageSN)
{
    mp_string strSN;
    mp_int32 buffLen = 256;
    mp_uchar aucBuffer[DATA_LEN_256] = {0};
    aucBuffer[3] = 65;
    mp_int32 rst = Array::GetDiskFusionStorageSN(aucBuffer, buffLen, strSN);
    EXPECT_EQ(rst, MP_FAILED);
    
    aucBuffer[3] = 32;
    rst = Array::GetDiskFusionStorageSN(aucBuffer, buffLen, strSN);
    EXPECT_EQ(rst, MP_SUCCESS);
}*/

//End CArrayTest
//Begin CDiskTest
TEST_F(CDiskTest, GetAllDiskName)
{
    vector<mp_string> vecDiskName;
    mp_int32 rst = 0;
    //ExecSystemWithEcho < 0
    {
        stub.set(&CSystemExec::ExecSystemWithEcho, &StubCSystemExecExecSystemWithEchoLt0);
        rst = Disk::GetAllDiskName(vecDiskName);
        EXPECT_EQ(rst, -1);
    }
    //IsCmdDevice != 0
    {
        stub.set(&CSystemExec::ExecSystemWithEcho, &StubCSystemExecExecSystemWithEchoEq0IsCmdDevice);
        rst = Disk::GetAllDiskName(vecDiskName);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}

TEST_F(CDiskTest, ClearInvalidLUNPath)
{
    vector<mp_string> vecLUNPaths;
    mp_int32 rst = Disk::ClearInvalidLUNPath(vecLUNPaths);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CDiskTest, ClearInvalidLegacyDSFs)
{
    vector<mp_string> vecLUNPaths;
    mp_int32 rst = Disk::ClearInvalidLegacyDSFs(vecLUNPaths);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CDiskTest, ClearInvalidPersistentDSFs)
{
    vector<mp_string> vecLUNPaths;
    mp_int32 rst = Disk::ClearInvalidPersistentDSFs(vecLUNPaths);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, GetPersistentDSFInfo)
{
    mp_string name, path, type;
    mp_int32 rst = Disk::GetPersistentDSFInfo(name, path, type);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CDiskTest, DeletePersistentDSF)
{
    mp_string name;
    mp_int32 rst = Disk::DeletePersistentDSF(name);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CDiskTest, ExecuteDiskCmdEx)
{
    mp_string cmd, fname;
    vector<mp_string> dName;
    mp_int32 rst = Disk::ExecuteDiskCmdEx(cmd, dName, fname);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, IsDskdisk)
{
    mp_string test = "test";
    mp_bool rst = Disk::IsDskdisk(test);
    EXPECT_TRUE(rst); 
}
TEST_F(CDiskTest, GetHPRawDiskName)
{
    mp_string test = "test";
    mp_int32 rst = Disk::GetHPRawDiskName(test, test);
    EXPECT_EQ(rst, MP_SUCCESS); 
}
TEST_F(CDiskTest, GetSecPvName)
{
    mp_string test = "test";
    mp_int32 rst = Disk::GetSecPvName(test, test, test);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, ClearInvalidDisk)
{
    mp_int32 rst = Disk::ClearInvalidDisk();
    EXPECT_EQ(rst, MP_FAILED); 
}
TEST_F(CDiskTest, GetPersistentDSFByLegacyDSF)
{
    mp_string test = "test";
    mp_int32 rst = Disk::GetPersistentDSFByLegacyDSF(test, test);
    EXPECT_EQ(rst, MP_FAILED);
}

TEST_F(CDiskTest, GetDiskCapacity)
{
    mp_string strDevice = "test";
    mp_string strBuf = "test";
    mp_int32 rst = 0;
    //open < 0
    {
        stub.set(&open, &StubopenLt0);
        rst = Disk::GetDiskCapacity(strDevice, strBuf);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //open = 0 ioctl < 0
    {
        stub.set(&open, &StubopenEq0);
        stub.set(&ioctl, &StubioctlLt0);
        rst = Disk::GetDiskCapacity(strDevice, strBuf);
        EXPECT_EQ(rst, MP_FAILED);
    }
    //ioctl = 0
    {
        stub.set(&open, &StubopenEq0);
        stub.set(&ioctl, &StubioctlOkGetDiskCapacity);
        rst = Disk::GetDiskCapacity(strDevice, strBuf);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
TEST_F(CDiskTest, IsSdisk)
{
    mp_string strDevice = "test";
    mp_bool rst = Disk::IsSdisk(strDevice);
    EXPECT_FALSE(rst);
}
TEST_F(CDiskTest, GetDiskStatus)
{
    stub.set(&CSystemExec::ExecSystemWithEcho, &StubCSystemExecExecSystemWithEchoOk);
    mp_string name, status;
    mp_int32 rst = Disk::GetDiskStatus(name, status);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CDiskTest, IsHdisk)
{
    mp_string strDevice = "test";
    mp_bool rst = Disk::IsHdisk(strDevice);
    EXPECT_FALSE(rst);
}
TEST_F(CDiskTest, GetDevNameByWWN)
{
    mp_string strDevice;
    mp_string strWWN = "test";
    mp_int32 rst = 0;
    //GetAllDiskName < 0
    {
        stub.set(&Disk::GetAllDiskName, &StubCDiskGetAllDiskNameLt0);
        rst = Disk::GetDevNameByWWN(strDevice, strWWN);
        EXPECT_EQ(rst, -1);
    }
    //GetAllDiskName = 0
    {
        stub.set(&Disk::GetAllDiskName, &StubCDiskGetAllDiskNameEq0);
        rst = Disk::GetDevNameByWWN(strDevice, strWWN);
        EXPECT_EQ(rst, ERROR_COMMON_DEVICE_NOT_EXIST);
    }
    //GetAllDiskName ok GetLunInfo < 0
    {
        stub.set(&Disk::GetAllDiskName, &StubCDiskGetAllDiskNameOk);
        stub.set(&Array::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN);
        stub.set(&Array::GetLunInfo, &StubCArrayGetLunInfoLt0);
        rst = Disk::GetDevNameByWWN(strDevice, strWWN);
        EXPECT_EQ(rst, -1);
    }
    //GetLunInfo ok
    {
        stub.set(&Disk::GetAllDiskName, &StubCDiskGetAllDiskNameOk);
        stub.set(&Array::GetArrayVendorAndProduct, &StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN);
        stub.set(&Array::GetLunInfo, &StubCArrayGetLunInfoOk);
        rst = Disk::GetDevNameByWWN(strDevice, strWWN);
        EXPECT_EQ(rst, MP_SUCCESS);
    }
}
//End CDiskTest

TEST_F(CDiskTest, GetHostLunIDList)
{
    mp_string strDevice = "test";
    vector<mp_int32> vecHostLunID;
        
    mp_bool rst = Array::GetHostLunIDList(strDevice,vecHostLunID);
    EXPECT_EQ(MP_FAILED, rst);

    stub.set(&open, StubopenEq0);
    Array::GetHostLunIDList(strDevice,vecHostLunID);
    EXPECT_EQ(MP_FAILED, rst);
}

TEST_F(CDiskTest, GetDisk00Page)
{
    mp_string strDevice = "test";
    vector<mp_string> vecResult;

    mp_bool rst = Array::GetDisk00Page(strDevice,vecResult);
    EXPECT_EQ(MP_FAILED, rst);
    
    stub.set(&open, StubopenEq0);
    rst = Array::GetDisk00Page(strDevice,vecResult);
    EXPECT_EQ(MP_FAILED, rst);
}

TEST_F(CDiskTest, GetDiskC8Page)
{
    mp_string strDevice = "test";
    mp_string strLunID;

    mp_bool rst = Array::GetDiskC8Page(strDevice,strLunID);
    EXPECT_EQ(MP_FAILED, rst);
    
    stub.set(&open, StubopenEq0);
    rst = Array::GetDiskC8Page(strDevice,strLunID);
    EXPECT_EQ(MP_FAILED, rst);
}

TEST_F(CDiskTest, CheckHuaweiLUN)
{
    mp_string strVendor;
    
    mp_bool rst = Array::CheckHuaweiLUN(strVendor);
    EXPECT_FALSE(rst);
}

#ifdef SOLARIS
TEST_F(CDiskTest, GetSolarisRawDiskName)
{
    mp_string strDiskName;
    mp_string strRawDiskName;

    mp_bool rst = Disk::GetSolarisRawDiskName(strDiskName,strRawDiskName);
    EXPECT_FALSE(rst);
}
#endif

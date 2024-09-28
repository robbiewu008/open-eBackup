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
#include "dataprocess/FileIOEngineTest.h"
#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubFileIOEngineGetValueInt32Return);                                                                      \
    } while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(FileIOEngineTest, OpenForRead)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 snapType = 1;
    FileIOEngine om(vol, protectType, snapType);
    mp_int32 iRet = 0;

    stub.set(&CLogger::Log, StubCLoggerLog);
    //文件不存在
    {
        stub.set(ADDR(CMpFile, FileExist), StubFileExistFalse);
        iRet = om.OpenForRead();
        EXPECT_EQ(MP_FAILED, iRet);
    }
    //文件打不开
    {
        stub.set(ADDR(CMpFile, FileExist), StubFileExistTrue);
        iRet = om.OpenForRead();
        EXPECT_EQ(MP_FAILED, iRet);
    }

    stub.reset(ADDR(CMpFile, FileExist));
}

TEST_F(FileIOEngineTest, OpenForWrite)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 snapType = 1;
    FileIOEngine om(vol, protectType, snapType);
    mp_int32 iRet = 0;

    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    //创建文件目录失败
    {
        stub.set(ADDR(CMpFile, CreateDir), StubCreateDirFail);
        iRet = om.OpenForWrite();
        EXPECT_EQ(MP_FAILED, iRet);
    }
    //创建文件目录成功
    stub.set(ADDR(CMpFile, CreateDir), StubCreateDirFail);
    //文件不存在
    {
        stub.set(ADDR(CMpFile, FileExist), StubFileExistFalse);
        iRet = om.OpenForWrite();
        EXPECT_EQ(MP_FAILED, iRet);
    }
    //文件打不开
    {
        stub.set(ADDR(CMpFile, FileExist), StubFileExistTrue);
        iRet = om.OpenForWrite();
        EXPECT_EQ(MP_FAILED, iRet);
    }

    stub.reset(ADDR(CMpFile, FileExist));
    stub.reset(ADDR(CMpFile, CreateDir));
}

TEST_F(FileIOEngineTest, Close)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 snapType = 1;
    FileIOEngine om(vol, protectType, snapType);
    mp_int32 iRet = 0;
    om.m_fp = nullptr;

    stub.set(&CLogger::Log, StubCLoggerLog);
    //文件不存在
    {
        iRet = om.Close();
        EXPECT_EQ(MP_SUCCESS, iRet);
    }
    //文件关闭
    {
        om.m_fp = fopen("test", "r");
        iRet = om.Close();
        EXPECT_EQ(MP_SUCCESS, iRet);
    }
}

TEST_F(FileIOEngineTest, Read)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 snapType = 1;
    const uint64_t offsetInBytes = 1;
    uint64_t bufferSizeInBytes;
    unsigned char* buffer;
    FileIOEngine om(vol, protectType, snapType);
    mp_int32 iRet = 0;
    om.m_fp = nullptr;

    stub.set(&CLogger::Log, StubCLoggerLog);
    //文件不存在
    {
        iRet = om.Read(offsetInBytes, bufferSizeInBytes, buffer);
        EXPECT_EQ(MP_FAILED, iRet);
    }
    //文件存在
    {
        om.m_fp = fopen("test", "w");
        stub.set(ADDR(FileIOEngine, RetryOp), StubRetryOp);
        iRet = om.Read(offsetInBytes, bufferSizeInBytes, buffer);
        EXPECT_EQ(MP_SUCCESS, iRet);
    }

    stub.reset(ADDR(FileIOEngine, RetryOp));
}

TEST_F(FileIOEngineTest, Write)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 snapType = 1;
    const uint64_t offsetInBytes = 1;
    uint64_t bufferSizeInBytes;
    unsigned char* buffer;
    FileIOEngine om(vol, protectType, snapType);
    mp_int32 iRet = 0;
    om.m_fp = nullptr;

    stub.set(&CLogger::Log, StubCLoggerLog);
    //文件不存在
    {
        iRet = om.Write(offsetInBytes, bufferSizeInBytes, buffer);
        EXPECT_EQ(MP_FAILED, iRet);
    }
    //文件存在
    {
        om.m_fp = fopen("test", "w");
        stub.set(ADDR(FileIOEngine, RetryOp), StubRetryOp);
        iRet = om.Write(offsetInBytes, bufferSizeInBytes, buffer);
        EXPECT_EQ(MP_SUCCESS, iRet);
    }

    stub.reset(ADDR(FileIOEngine, RetryOp));
}

TEST_F(FileIOEngineTest, DoRead)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 snapType = 1;
    const uint64_t offsetInBytes = -1;
    uint64_t bufferSizeInBytes = 1;
    unsigned char buffer[] = "tttt";
    FileIOEngine om(vol, protectType, snapType);
    mp_int32 iRet = 0;
    om.m_fp = fopen("test", "r");

    stub.set(&CLogger::Log, StubCLoggerLog);
    //偏移不正确
    {
        iRet = om.DoRead(offsetInBytes, bufferSizeInBytes, buffer);
        EXPECT_EQ(MP_FAILED, iRet);
    }
    //偏移正确
    {
        const uint64_t offsetInBytesSucc = 12;
        iRet = om.DoRead(offsetInBytesSucc, bufferSizeInBytes, buffer);
        EXPECT_EQ(MP_FAILED, iRet);
    }
}

// TEST_F(FileIOEngineTest, DoWrite)
// {
//     vmware_volume_info vol;
//     mp_int32 protectType = 0;
//     mp_int32 snapType = 1;
//     const uint64_t offsetInBytes = -1;
//     uint64_t bufferSizeInBytes = 1;
//     unsigned char buffer[] = "test";
//     FileIOEngine om(vol, protectType, snapType);
//     mp_int32 iRet = 0;
//     om.m_fp = fopen("test", "r");

//     stub.set(&CLogger::Log, StubCLoggerLog);
//     //偏移不正确
//     {
//         iRet = om.DoWrite(offsetInBytes, bufferSizeInBytes, buffer);
//         EXPECT_EQ(MP_FAILED, iRet);
//     }
//     //写错误
//     {
//         const uint64_t offsetInBytesSucc = 1;
//         iRet = om.DoWrite(offsetInBytesSucc, bufferSizeInBytes, buffer);
//         EXPECT_EQ(MP_FAILED, iRet);
//     }
//     //写正确
//     {   
//         om.m_fp = fopen("test", "w");
//         const uint64_t offsetInBytesSucc = 1;
//         iRet = om.DoWrite(offsetInBytesSucc, bufferSizeInBytes, buffer);
//         EXPECT_EQ(MP_SUCCESS, iRet);
//     }
// }

TEST_F(FileIOEngineTest, RetryOp)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 snapType = 1;
    FileIOEngine om(vol, protectType, snapType);
    mp_int32 iRet = 0;

    std::function<mp_int32()> internalOp = []() { return MP_SUCCESS; };
    iRet = om.RetryOp(internalOp);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(FileIOEngineTest, PostBackup)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 snapType = 1;
    FileIOEngine om(vol, protectType, snapType);
    mp_int32 iRet = 0;

    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(FileIOEngine, RetryOp), StubRetryOp);
    iRet = om.PostBackup();
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.reset(ADDR(FileIOEngine, RetryOp));
}

TEST_F(FileIOEngineTest, GenerateDescFileContent)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 snapType = 1;
    FileIOEngine om(vol, protectType, snapType);
    mp_string strContent;

    om.GenerateDescFileContent(strContent);
    EXPECT_NE("", strContent);
}

TEST_F(FileIOEngineTest, GenerateDiskDescFile)
{
    vmware_volume_info vol;
    mp_int32 protectType = 0;
    mp_int32 snapType = 1;
    FileIOEngine om(vol, protectType, snapType);
    mp_int32 iRet = 0;

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = om.GenerateDiskDescFile();
    EXPECT_EQ(MP_FAILED, iRet);
}

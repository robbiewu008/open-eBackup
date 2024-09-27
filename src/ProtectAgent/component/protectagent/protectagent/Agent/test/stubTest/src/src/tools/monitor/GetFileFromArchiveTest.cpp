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
#include "tools/agentcli/GetFileFromArchiveTest.h"

using namespace std;
namespace {

mp_void CLogger_Log_Stub(mp_void* pthis)
{
    return;
}
mp_int32 Connect_SUCCESS(const mp_string &busiIp, mp_int32 busiPort, bool openSsl)
{
    return MP_SUCCESS;
}
mp_int32 PrepareRecovery_SUCCESS(mp_string &metaFileDir)
{
    metaFileDir = "123";
    return MP_SUCCESS;
}
mp_int32 QueryPrepareStatus_SUCCESS(mp_int32 &state)
{
    state = 3;
    return MP_SUCCESS;
}
mp_int32 EndRecover_SUCCESS()
{
    return MP_SUCCESS;
}
mp_int32 QueryPrepareStatus_FAILED(mp_int32 &state)
{
    return MP_FAILED;
}
mp_int32 GetFileData_FAILED(const ArchiveStreamGetFileReq &getFileReq, ArchiveStreamGetFileRsq &getFileRsp)
{
    return MP_FAILED;
}
mp_int32 GetRecoverObjectList_SUCCESS(mp_int64 readCountLimit, mp_string &checkpoint, mp_string &splitFile,
        mp_int64 &objectNum, mp_int32 &status)
{
    status = 3;
    return MP_SUCCESS;

}
mp_int32 GetFileListInfo_SUCCESS(std::unique_ptr<ArchiveStreamService> &m_clientHandler,
    const mp_string &localPath)
{
    return MP_SUCCESS;
}
}

/*
*用例名称：从archive获取文件，获取成功
*前置条件：能正常连接archive、获取数据文件
*check点：能成功获取
*/
TEST_F(CGetFileFromArchiveTest, HandleTest)
{
    mp_int32 iRet = MP_FAILED;
    GetFileFromArchive showStatus;

    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    mp_string backupId = "123456";
    mp_string busiIp = "127.0.0.0:30066";
    mp_string localPath = "/home/";
    mp_string dirList = "";
    stub.set(ADDR(ArchiveStreamService, Connect), Connect_SUCCESS);
    stub.set(ADDR(GetFileFromArchive, GetFileListInfo), GetFileListInfo_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, EndRecover), EndRecover_SUCCESS);

    iRet = showStatus.Handle(backupId, busiIp, localPath, dirList);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
*用例名称：从archive获取文件，获取失败
*前置条件：能正常连接archive、获取文件失败
*check点：获取文件失败
*/
TEST_F(CGetFileFromArchiveTest, HandleTest_failed)
{
    mp_int32 iRet = MP_FAILED;
    GetFileFromArchive showStatus;

    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    mp_string backupId = "123456";
    mp_string busiIp = "127.0.0.0:30066";
    mp_string localPath = "/home/";
    mp_string dirList = "";
    stub.set(ADDR(ArchiveStreamService, Connect), Connect_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, PrepareRecovery), PrepareRecovery_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), QueryPrepareStatus_FAILED);
    stub.set(ADDR(ArchiveStreamService, EndRecover), EndRecover_SUCCESS);

    iRet = showStatus.Handle(backupId, busiIp, localPath, dirList);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
*用例名称：从archive下载文件接口
*前置条件：能正常连接archive、获取数据失败
*check点：获取文件失败
*/
TEST_F(CGetFileFromArchiveTest, DownloadFile_failed)
{
    mp_int32 iRet = MP_FALSE;
    GetFileFromArchive showStatus;

    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    mp_string localPath = "/home/";
    mp_string dirList = "/opt/";
    mp_string fileName = "test.cpp";
    std::unique_ptr<ArchiveStreamService> m_clientHandler = std::make_unique<ArchiveStreamService>();
    stub.set(ADDR(ArchiveStreamService, GetFileData), GetFileData_FAILED);

    iRet = showStatus.DownloadFile(m_clientHandler, dirList, localPath, fileName);
    EXPECT_EQ(iRet, MP_FAILED);
}

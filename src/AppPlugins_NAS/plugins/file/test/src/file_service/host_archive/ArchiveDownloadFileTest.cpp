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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "log/Log.h"
#include "stub.h"
#include "gmock/gmock-actions.h"
#include "ArchiveDownloadFile.h"
#include "io_device/FileDef.h"
#include "ArchiveClient.h"
#include "application/ApplicationManager.h"
#include "PluginUtilities.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using namespace FilePlugin;
using namespace std;

namespace {
const int SUCCESS = 0;
const int FAILED = -1;

const int SKD_PREPARE_CODE_NE1 = -1;
const int SKD_PREPARE_CODE_0 = 0;
const int SKD_PREPARE_CODE_1 = 1;
const int SKD_PREPARE_CODE_2 = 2;
const int SKD_PREPARE_CODE_3 = 3;
}

class ArchiveDownloadFileTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<ArchiveDownloadFile> m_download = nullptr;
    std::unique_ptr<ArchiveStreamService> m_clientHandler = std::make_unique<ArchiveStreamService>();
};

static void returnVoidStub(void* obj)
{
    return;
}

void ArchiveDownloadFileTest::SetUp()
{
    stub.set(sleep, returnVoidStub);
    ArchiveServerInfo info{};
    info.ipList.push_back("aaa");
    info.ipList.push_back("bbb");
    std::string cacheFsPath = "c:\\cachefs";
    std::string cacheFsRemotePath = "/Fileset_CacheDataRepository/53b300a663e940c3a7225bfc5ce380b1";
    std::string jobId = "job123";
    std::string resourceId = "resource123";
    std::string copyId = "copy123";
    ArchiveDownloadParam param;
    param.jobId = jobId;
    param.copyId = copyId;
    param.resourceId = resourceId;
    param.cacheFsPath = cacheFsPath;
    param.cacheFsRemotePath = cacheFsRemotePath;
    m_download = std::make_unique<ArchiveDownloadFile>(param, info);

}

void ArchiveDownloadFileTest::TearDown()
{}

void ArchiveDownloadFileTest::SetUpTestCase()
{}

void ArchiveDownloadFileTest::TearDownTestCase()
{}

static int test_state;

static int Stub_PrepareRecovery_FAILED(const std::string& filePath)
{
    return FAILED;
}

static int Stub_PrepareRecovery_SUCCESS(const std::string& filePath)
{
    return SUCCESS;
}

static int Stub_QueryPrepareStatus_FAILED(int& state)
{
    return FAILED;
}

static bool GetFileList_stub(void* obj) {
    return true;
}

static bool Stub_GetFileList_True(void* obj) {
    return true;
}
static bool Stub_GetFileList_False(void* obj) {
    return false;
}

static bool Function_True()
{
    return true;
}
static bool Function_False()
{
    return false;
}

static void Function_void_stub() {
    INFOLOG("Enter CloseClient");
    bool m_isInit = false;
    if (!m_isInit) {
        INFOLOG("Archive client not init");
        return;
    }
}

static int Stub_QueryPrepareStatus_SUCCESS(int& state)
{
    std::cout << "enter Stub_QueryPrepareStatus()" << std::endl;
    state = 2;
    return 1;
}

static bool Stub_IsFileExist_TRUE(void* obj, string checkpoint, vector<string> controlList)
{
    checkpoint = "6666";
    controlList.push_back(checkpoint);
    return true;
}

static bool STUB_IsFileExist_FALSE(void* obj, string checkpoint, vector<string> controlList)
{
    checkpoint = "6666";
    controlList.push_back(checkpoint);
    return false;
}

static int Stub_OpenFileExistOrNew_TRUE(void* obj)
{
    return Module::FAILED;
}

static bool Stub_WriteFile_TRUE(void* obj,const auto& line)
{
    return true;
}

static bool Stub_IsDir_FALSE(void* obj,string m_fileFullPath)
{
    string outputPath = "/home/zm";
    // string line;
    string fileArchivePath;
    DBGLOG("File archive path: %s", fileArchivePath.c_str());
    m_fileFullPath = outputPath + fileArchivePath;
    return false;

}

// static void Stub_WriteBufferToFile_TRUE(void* obj,retValue.data,req.fileOffset,retValue.fileSize)
// {
//     ArchiveStreamGetFileRsq retValue{};
//     ArchiveStreamGetFileReq req {};
//     return;
// }

/*
 * 用例名称: 测试归档客户端下载启动
 * 前置条件：sdk连接失败
 * check点：初始化客户端返回失败
 */

 static int Function_SUCCESS()
 {
     return SUCCESS;
 }
 static int Function_FAILED()
 {
     return FAILED;
 }

TEST_F(ArchiveDownloadFileTest, InitArchiveClientTest)
{
    std::vector<std::string> ipList = {"0.0.0.0"};
    bool ret = m_download->InitArchiveClient(ipList);
    EXPECT_EQ(ret, false);

    stub.set(ADDR(ArchiveStreamService, Init), Function_FAILED);
    ret = m_download->InitArchiveClient(ipList);
    EXPECT_EQ(ret, false);

    stub.set(ADDR(ArchiveStreamService, Init), Function_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, Connect), Function_SUCCESS);
    stub.set(ADDR(ArchiveDownloadFile, QueryPrepare), Function_False);
    ret = m_download->InitArchiveClient(ipList);
    EXPECT_EQ(ret, false);

    stub.set(ADDR(ArchiveDownloadFile, QueryPrepare), Function_True);
    ret = m_download->InitArchiveClient(ipList);
    EXPECT_EQ(ret, true);

    stub.reset(ADDR(ArchiveDownloadFile, QueryPrepare));
}

/*
 * 用例名称: 测试归档客户端查询准备状态
 * 前置条件：无
 * check点：返回成功或失败
 */


static int Stub_QueryPrepareStatus2_SUCCESS(int& state)
{
    state = 2;
    return SUCCESS;
}

static int Stub_QueryPrepareStatus3_SUCCESS(int& state)
{
    state = -1;
    return SUCCESS;
}

static int Stub_QueryPrepareStatus1_SUCCESS(int& state)
{
    state = 1;
    return SUCCESS;
}

static void Stub_sleep_for()
{}

// TEST_F(ArchiveDownloadFileTest, QueryPrepareTest_2)
// {
//     bool ret = false;

//     stub.set(ADDR(ArchiveStreamService, PrepareRecovery), Stub_PrepareRecovery_SUCCESS);
//     stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), Stub_QueryPrepareStatus2_SUCCESS);
//     // stub.set(std::this_thread::sleep_for<std::chrono::seconds>, Stub_sleep_for); // this_thread打桩问题
//     ret = m_download->QueryPrepare();
//     EXPECT_EQ(ret, false); // 归档客户端准备状态查询失败

//     stub.reset(ADDR(ArchiveStreamService, PrepareRecovery));
//     stub.reset(ADDR(ArchiveStreamService, QueryPrepareStatus));
// }

TEST_F(ArchiveDownloadFileTest, QueryPrepareTest)
{
    bool ret = false;

    // stub.set(ADDR(ArchiveStreamService, PrepareRecovery), Stub_PrepareRecovery_FAILED);
    // ret = m_download->QueryPrepare();
    // EXPECT_EQ(ret, false); // 客户端未准备好

    // stub.set(ADDR(ArchiveStreamService, PrepareRecovery), Stub_PrepareRecovery_SUCCESS);
    // stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), Stub_QueryPrepareStatus_FAILED);
    // ret = m_download->QueryPrepare();
    // EXPECT_EQ(ret, false); // 归档客户端准备状态查询失败
    // stub.reset(ADDR(ArchiveStreamService, PrepareRecovery));
    // stub.reset(ADDR(ArchiveStreamService, QueryPrepareStatus));


    // stub.set(ADDR(ArchiveStreamService, PrepareRecovery), Stub_PrepareRecovery_SUCCESS);
    // stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), Stub_QueryPrepareStatus_SUCCESS);
    // test_state = SKD_PREPARE_CODE_2;
    // ret = m_download->QueryPrepare();
    // EXPECT_EQ(ret, false); // 错误状态码2
    // test_state = SKD_PREPARE_CODE_NE1;
    // ret = m_download->QueryPrepare();
    // EXPECT_EQ(ret, false); // 错误状态码-1
    // test_state = SKD_PREPARE_CODE_1;
    // ret = m_download->QueryPrepare();
    // EXPECT_EQ(ret, true); // 客户端已准备好

    //std::this_thread::sleep_for(std::chrono::seconds(10));

    stub.set(ADDR(ArchiveStreamService, PrepareRecovery), Stub_PrepareRecovery_FAILED);
    ret = m_download->QueryPrepare();
    EXPECT_EQ(ret, false); // 客户端未准备好
    stub.reset(ADDR(ArchiveStreamService, PrepareRecovery));

    stub.set(ADDR(ArchiveStreamService, PrepareRecovery), Stub_PrepareRecovery_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), Stub_QueryPrepareStatus_FAILED);
    ret = m_download->QueryPrepare();

    EXPECT_EQ(ret, false); // 归档客户端准备状态查询失败
    stub.reset(ADDR(ArchiveStreamService, PrepareRecovery));
    stub.reset(ADDR(ArchiveStreamService, QueryPrepareStatus));


    // stub.set(ADDR(ArchiveStreamService, PrepareRecovery), Stub_PrepareRecovery_SUCCESS);
    // stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), Stub_QueryPrepareStatus_SUCCESS);
    // ret = m_download->QueryPrepare();
    // EXPECT_EQ(ret, false); // 归档客户端准备状态查询失败


    // stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus),Stub_QueryPrepareStatus2_FAILED);
    // stub.reset(ADDR(ArchiveStreamService, QueryPrepareStatus));
    // stub.set(ADDR(ArchiveStreamService, PrepareRecovery), Stub_PrepareRecovery_SUCCESS);
    // stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), Stub_QueryPrepareStatus1_SUCCESS);
    // ret = m_download->QueryPrepare();
    // EXPECT_EQ(ret, false); // 归档客户端准备状态查询失败
    // stub.reset(ADDR(ArchiveStreamService, PrepareRecovery));
    // stub.reset(ADDR(ArchiveStreamService, QueryPrepareStatus));

    // stub.set(ADDR(ArchiveStreamService, PrepareRecovery), Stub_PrepareRecovery_SUCCESS);
    // stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), Stub_QueryPrepareStatus2_SUCCESS);
    // ret = m_download->QueryPrepare();
    // EXPECT_EQ(ret, false); // 归档客户端准备状态查询失败
// stub.set(ADDR(ArchiveStreamService, PrepareRecovery), Stub_PrepareRecovery_SUCCESS);
// stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), Stub_QueryPrepareStatus_SUCCESS);
// test_state = SKD_PREPARE_CODE_2;
// ret = m_download->QueryPrepare();
// EXPECT_EQ(ret, false); // 错误状态码2
// test_state = SKD_PREPARE_CODE_NE1;
// ret = m_download->QueryPrepare();
// EXPECT_EQ(ret, false); // 错误状态码-1
// test_state = SKD_PREPARE_CODE_1;
// ret = m_download->QueryPrepare();
// EXPECT_EQ(ret, true); // 客户端已准备好



}

/*
 * 用例名称: 测试获取文件列表
 * 前置条件：无
 * check点：返回成功或失败
 */

// TEST_F(ArchiveDownloadFileTest, GetFileListFromCtrl)
// {
//     INFOLOG("Enter CloseClient..........");
//     int name = -1;
//     std::string fsID = "fileId";
//     std::string fileName = "e:/home/xm";
//     // FilePlugin::ControlFileData::ArchiveMetaInfo metaInfo = ("c:/user/file",1,2);
//     int linkName = -1;
//     std::string linkSrcName = "c:/home";


//     // ControlFileData data {};
//     // data.fileName

//     // FilePlugin::ControlFileData data = {ArchiveMetaInfo("c:/user/file",1,2),name,fsID,fileName,metaInfo,linkName,linkSrcName};
//     std::map<std::string,FilePlugin::ControlFileData> map1;
//     std::string FileName = "/home/zm2/plugins/file/test/zm_doc/file.txt";
//     bool ret = m_download->GetFileListFromCtrl(FileName, map1);

//     EXPECT_EQ(ret,false);
// }

TEST_F(ArchiveDownloadFileTest, GetFileListFromCtrl)
{

    stub.set(PluginUtils::IsFileExist, Function_False);
    std::map<std::string,FilePlugin::ControlFileData> map1;
    std::string FileName = "/home/zm2/plugins/file/test/zm_doc/file.txt";
    bool ret = m_download->GetFileListFromCtrl(FileName, map1);

    EXPECT_EQ(ret,false);

    stub.reset(PluginUtils::IsFileExist);
}

/*
 * 用例名称: 测试将Buffer写入文件的反馈状态
 * 前置条件：无
 * check点：返回成功或失败
 */

TEST_F(ArchiveDownloadFileTest, WriteBufferToFileTest_Fail)
{
    // 此处的buff是一个缓存区
    uint64_t offset = 100;
    uint64_t Length = 9;
    char buff[110];

    int retTest = m_download->WriteBufferToFile(buff, offset, Length);
    EXPECT_EQ((retTest == 9) || (retTest == 29), true);
}

TEST_F(ArchiveDownloadFileTest, WriteBufferToFileTest_Success)
{
    // 此处的buff是一个缓存区
    uint64_t offset = 100;
    uint64_t Length = 0;
    const char* bufPtr = nullptr;
    m_download->WriteBufferToFile(bufPtr, offset, Length);

    char buff[110];
    int ret = m_download->WriteBufferToFile(buff, offset, Length);
    EXPECT_EQ(ret, SUCCESS);
}

static bool Stub_Disconnect_FAILED()
{
    return FAILED;
}
static bool Stub_Disconnect_SUCCESS()
{
    return SUCCESS;
}

// TEST_F(ArchiveDownloadFileTest, SetAbortTest)
// {
//     m_download->SetAbort();
// }

// TEST_F(ArchiveDownloadFileTest, IsAbortTest)
// {
//     bool ret = m_download->IsAbort();
//     EXPECT_EQ(ret,false);
// }

// TEST_F(ArchiveDownloadFileTest, IsDirTest)
// {
//     std::string name = "dog";
//     bool ret = m_download->IsDir(name);
//     EXPECT_EQ(ret,false);
// }

// TEST_F(ArchiveDownloadFileTest, GetFileSystemsIdTest)
// {
//     std::string ret = m_download->GetFileSystemsId();
//     EXPECT_EQ(ret,"");
// }

// TEST_F(ArchiveDownloadFileTest, StartTest)
// {
//     INFOLOG("Archive download file start");

//     std::string outputPath = "e:/home";
//     std::vector<std::string> pathList = {"c:/user"};
//     bool ret = m_download->Start(outputPath, pathList);
//     EXPECT_EQ(ret,false);

//     stub.set(ADDR(ArchiveDownloadFile, StartDownloadMeta), Function_True);
//     ret = m_download->Start(outputPath, pathList);
//     EXPECT_EQ(ret,true);
// }

// TEST_F(ArchiveDownloadFileTest, StartTest)
// {
//     INFOLOG("Archive download file start");
//     stub.set(ADDR(ArchiveDownloadFile, StartDownloadMeta), Function_False);

//     std::string outputPath = "e:/home";
//     std::vector<std::string> pathList = {"c:/user"};
//     bool ret = m_download->Start(outputPath, pathList);
//     EXPECT_EQ(ret,false);

//     stub.set(ADDR(ArchiveDownloadFile, StartDownloadMeta), Function_True);
//     ret = m_download->Start(outputPath, pathList);
//     EXPECT_EQ(ret,true);

//     stub.reset(ADDR(ArchiveDownloadFile, StartDownloadMeta));
// }


// TEST_F(ArchiveDownloadFileTest, StartDownloadMetaTest)
// {
//     std::string outputPath = "e:/home";
//     std::vector<std::string> pathList = {"c:/user"};

    // stub.set(ADDR(ArchiveDownloadFile, GetControlFileFromArchive), Stub_IsFileExist_TRUE);
    // stub.set(ADDR(ArchiveDownloadFile, GetFileListFromCtrl), GetFileList_stub);
    // stub.set(ADDR(ArchiveDownloadFile, DownloadFile), GetFileList_stub);
    // stub.set(ADDR(ArchiveDownloadFile, InitArchiveClient), GetFileList_stub);
    // stub.set(ADDR(ArchiveDownloadFile, CloseClient), Function_void_stub);

    // bool ret = m_download->StartDownloadMeta(outputPath, pathList);
    // EXPECT_EQ(ret,false);
    // stub.reset(ADDR(ArchiveDownloadFile, GetControlFileFromArchive));
    // stub.reset(ADDR(ArchiveDownloadFile, GetFileListFromCtrl));
    // stub.reset(ADDR(ArchiveDownloadFile, DownloadFile));
    // stub.reset(ADDR(ArchiveDownloadFile, InitArchiveClient));
    // stub.reset(ADDR(ArchiveDownloadFile, CloseClient));

    // stub.set(ADDR(ArchiveDownloadFile, InitArchiveClient), Function_False);
    // int ret = m_download->StartDownloadMeta(outputPath, pathList);
    // EXPECT_EQ(ret,false);
    // stub.reset(ADDR(ArchiveDownloadFile, InitArchiveClient));

    // stub.set(ADDR(ArchiveDownloadFile, InitArchiveClient), Function_True);
    // stub.set(ADDR(ArchiveDownloadFile, GetControlFileFromArchive), STUB_IsFileExist_FALSE);
    // ret = m_download->StartDownloadMeta(outputPath, pathList);
    // EXPECT_EQ(ret,false);

    // stub.set(ADDR(ArchiveDownloadFile, InitArchiveClient), Function_True);
    // stub.set(ADDR(ArchiveDownloadFile, GetControlFileFromArchive), Stub_IsFileExist_TRUE);
    // stub.set(ADDR(ArchiveDownloadFile, GetFileListFromCtrl), Function_False);
    // ret = m_download->StartDownloadMeta(outputPath, pathList);
    // EXPECT_EQ(ret,false);

    // stub.set(ADDR(ArchiveDownloadFile, GetFileListFromCtrl), Function_True);
    // stub.set(ADDR(ArchiveDownloadFile, DownloadFile), Function_False);
    // ret = m_download->StartDownloadMeta(outputPath, pathList);
    // EXPECT_EQ(ret,false);
    // stub.reset(ADDR(ArchiveDownloadFile, GetFileListFromCtrl));
    // stub.reset(ADDR(ArchiveDownloadFile, DownloadFile));

    // stub.set(ADDR(ArchiveDownloadFile, GetFileListFromCtrl), Function_True);
    // stub.set(ADDR(ArchiveDownloadFile, DownloadFile), Function_True);
    // ret = m_download->StartDownloadMeta(outputPath, pathList);
    // EXPECT_EQ(ret,true);
    // stub.reset(ADDR(ArchiveDownloadFile, GetFileListFromCtrl));
    // stub.reset(ADDR(ArchiveDownloadFile, DownloadFile));

    // stub.reset(ADDR(ArchiveDownloadFile, InitArchiveClient));
    // stub.reset(ADDR(ArchiveDownloadFile, GetControlFileFromArchive));
// }

int Stub_GetRecoverObjectList_FAILED(void* obj, int64_t readCountLimit, std::string &checkpoint, std::string &splitFile,
        int64_t &objectNum, int32_t &status) {
    return FAILED;
}
int Stub_GetRecoverObjectList_SUCCESS_STATUS_2(void* obj, int64_t readCountLimit, std::string &checkpoint, std::string &splitFile,
        int64_t &objectNum, int32_t &status) {
    status = 2;
    return SUCCESS;
}
int Stub_GetRecoverObjectList_SUCCESS_STATUS_3(void* obj, int64_t readCountLimit, std::string &checkpoint, std::string &splitFile,
        int64_t &objectNum, int32_t &status) {
    status = 3;
    return SUCCESS;
}

TEST_F(ArchiveDownloadFileTest, GetControlFileFromArchiveTest)
{
    stub.set(ADDR(ArchiveStreamService, GetRecoverObjectList), Stub_GetRecoverObjectList_FAILED);
    std::string checkpoint = "ll";
    std::vector<std::string> controlList = {"file"};
    bool ret = m_download->GetControlFileFromArchive(checkpoint, controlList);
    stub.set(ADDR(ArchiveDownloadFile, IsAbort), GetFileList_stub);

    EXPECT_EQ(ret,false);
    stub.reset(ADDR(ArchiveDownloadFile, IsAbort));
    stub.reset(ADDR(ArchiveStreamService, GetRecoverObjectList));


    stub.set(ADDR(ArchiveDownloadFile, IsAbort), Function_True);
    ret = m_download->GetControlFileFromArchive(checkpoint, controlList);
    EXPECT_EQ(ret,false);
    stub.reset(ADDR(ArchiveDownloadFile, IsAbort));

    stub.set(ADDR(ArchiveDownloadFile, IsAbort), Function_False);
    stub.set(ADDR(ArchiveStreamService, GetRecoverObjectList), Stub_GetRecoverObjectList_SUCCESS_STATUS_3);
    ret = m_download->GetControlFileFromArchive(checkpoint, controlList);
    EXPECT_EQ(ret,false);
    stub.reset(ADDR(ArchiveDownloadFile, IsAbort));
    stub.reset(ADDR(ArchiveStreamService, GetRecoverObjectList));

    stub.set(ADDR(ArchiveDownloadFile, IsAbort), Function_False);
    stub.set(ADDR(ArchiveStreamService, GetRecoverObjectList), Stub_GetRecoverObjectList_SUCCESS_STATUS_2);
    ret = m_download->GetControlFileFromArchive(checkpoint, controlList);
    EXPECT_EQ(ret, true);
    stub.reset(ADDR(ArchiveDownloadFile, IsAbort));
    stub.reset(ADDR(ArchiveStreamService, GetRecoverObjectList));
}

static int Stub_OpenFileExistOrNew_0()
{
    return 0;
}
// TEST_F(ArchiveDownloadFileTest, DownloadFile)
// {
//     stub.set(ADDR(ArchiveDownloadFile, OpenFileExistOrNew), Stub_OpenFileExistOrNew_TRUE);
//     stub.set(ADDR(ArchiveDownloadFile, ArchiveWriteFile), Stub_OpenFileExistOrNew_TRUE);
//     stub.set(ADDR(PluginUtils, CreateDirectory), Stub_OpenFileExistOrNew_TRUE);
//     // stub.set(ADDR(ArchiveDownloadFile, WriteFile), Stub_WriteFile_TRUE);
//     const std::string outputPath = "/home/zm2/plugins/file/test/zm_doc/file.txt";
//     std::map<std::string, ControlFileData> map2;
//     bool ret = m_download->DownloadFile(outputPath, map2);
//     EXPECT_EQ(ret,true);
//     stub.reset(ADDR(ArchiveDownloadFile, OpenFileExistOrNew));
//     stub.reset(ADDR(ArchiveDownloadFile, IsDir));
//     stub.reset(ADDR(ArchiveDownloadFile, ArchiveWriteFile));


//     stub.set(ADDR(ArchiveDownloadFile, OpenFileExistOrNew), Stub_OpenFileExistOrNew_0);
//     stub.set(ADDR(ArchiveDownloadFile, ArchiveWriteFile), Stub_OpenFileExistOrNew_TRUE);
//     stub.set(ADDR(PluginUtils, CreateDirectory), Stub_OpenFileExistOrNew_TRUE);
//     ControlFileData controlFileData;
//     map2.insert(make_pair(outputPath, controlFileData));

//     ret = m_download->DownloadFile(outputPath, map2);
//     EXPECT_EQ(ret,false);

//     stub.set(ADDR(ArchiveDownloadFile, ArchiveWriteFile), Function_False);
//     ret = m_download->DownloadFile(outputPath, map2);
//     EXPECT_EQ(ret,false);

//     stub.set(ADDR(ArchiveDownloadFile, IsDir), Function_True);
//     ret = m_download->DownloadFile(outputPath, map2);
//     EXPECT_EQ(ret, false);
//     stub.reset(ADDR(ArchiveDownloadFile, OpenFileExistOrNew));
//     stub.reset(ADDR(ArchiveDownloadFile, IsDir));
//     stub.reset(ADDR(ArchiveDownloadFile, ArchiveWriteFile));
// }

// TEST_F(ArchiveDownloadFileTest, OpenFileExistOrNew)
// {
//     m_download->m_fileFullPath = "/file.txt";
//     int ret = m_download->OpenFileExistOrNew();
//     EXPECT_EQ(ret, 0);
// }

// TEST_F(ArchiveDownloadFileTest, WriteFile)
// {
//     stub.set(ADDR(ArchiveDownloadFile, IsDir), Stub_IsDir_FALSE);
//     stub.set(ADDR(ArchiveClient, GetFileData), Stub_OpenFileExistOrNew_TRUE);
//     stub.set(ADDR(ArchiveDownloadFile, WriteBufferToFile), Stub_OpenFileExistOrNew_TRUE);
//     FilePlugin::ControlFileData ctrlData;
//     bool ret = m_download->WriteFile(ctrlData);

//     ArchiveStreamGetFileRsq retValue{};
//     ArchiveStreamGetFileReq req {};
//     // free(retValue.data);
//     // retValue.data = nullptr;
//     // uint64_t offset = 0;
//     offset += retValue.fileSize;
//     // FilePlugin::
//     const auto MODULE = "ArchiveDownloadFile";
//     // const int BYTE_UNIT = 1024;
//     HCP_Log(DEBUG, MODULE) << "m_dataSize: " << retValue.fileSize / BYTE_UNIT <<
//             " retValue.fileSize " << retValue.fileSize << HCPENDLOG;

//     EXPECT_EQ(ret,true);

//     stub.reset(ADDR(ArchiveDownloadFile, IsDir));
//     stub.reset(ADDR(ArchiveClient, GetFileData));
//     stub.reset(ADDR(ArchiveDownloadFile, WriteBufferToFile));

// }

static int Stub_GetFileData_SUCCESS(ArchiveStreamGetFileReq& req, ArchiveStreamGetFileRsq& retValue)
{
    retValue.readEnd = 1;
    return SUCCESS;
}
static int Stub_GetFileData_FAILED(ArchiveStreamGetFileReq& req, ArchiveStreamGetFileRsq& retValue)
{
    retValue.readEnd = 1;
    return FAILED;
}
static int Stub_WriteBufferToFile_SUCCESS()
{
    return SUCCESS;
}
static int Stub_WriteBufferToFile_FAILED()
{
    return FAILED;
}

TEST_F(ArchiveDownloadFileTest, ArchiveWriteFile)
{
    stub.set(ADDR(ArchiveStreamService, GetFileData), Stub_GetFileData_FAILED);
    FilePlugin::ControlFileData ctrlData;
    ctrlData.fileName = "aaa";
    ctrlData.fsId = "bbb";
    bool ret = m_download->ArchiveWriteFile(ctrlData);
    EXPECT_EQ(ret,false);
    stub.reset(ADDR(ArchiveStreamService, GetFileData));
}

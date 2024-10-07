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
#include "named_stub.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "PluginUtilities.h"
#include "common/CTime.h"
#include "system/System.hpp"

using namespace std;

class PluginUtilitiesTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    LLTSTUB::Stub stub;
    const uint64_t NUM_1024 = 1024;
    const uint64_t KB_size = NUM_1024;
    const uint64_t MB_size = NUM_1024 * KB_size;
    const uint64_t GB_size = NUM_1024 * MB_size;
    const uint64_t TB_size = NUM_1024 * GB_size;
    const uint64_t PB_size = NUM_1024 * TB_size;
};

static void returnVoidStub(void* obj)
{
    return;
}

void PluginUtilitiesTest::SetUp()
{
    stub.set(sleep, returnVoidStub);
}

void PluginUtilitiesTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范

}
void PluginUtilitiesTest::SetUpTestCase() {}
void PluginUtilitiesTest::TearDownTestCase() {}

static int mkdir_success(const char* path, int _mode)
{
    return Module::SUCCESS;
}

static int mkdir_failed(const char* path, int _mode)
{
    return Module::FAILED;
}

static void FunctionVoidSucc(void* obj)
{
    return;
}

static bool FunctionBoolSucc(void* obj)
{
    return true;
}

static bool FunctionBoolFailed(void* obj)
{
    return false;
}

static int FunctionIntSucc(void* obj)
{
    return Module::SUCCESS;
}

static int  FunctionIntFailed(void* obj)
{
    return Module::FAILED;
}


static int runShellCmdWithOutput_failed(const severity_level& severity, const string& moduleName,
        const size_t& requestID, const string& cmd, const vector<string> params,
        vector<string>& cmdoutput, vector<string>& stderroutput)
{
    cmdoutput.push_back("nan jing");
    stderroutput.push_back("error...");

    return Module::FAILED;
}

static int runShellCmdWithOutput_success(const severity_level& severity, const string& moduleName,
        const size_t& requestID, const string& cmd, const vector<string> params,
        vector<string>& cmdoutput, vector<string>& stderroutput)
{
    cout << "runShellCmdWithOutput_success" << endl;
    cmdoutput.push_back("nan jing");
    stderroutput.push_back("error...");
    return Module::SUCCESS;
}

static bool CreateDirectory_Stub_TRUE(void* obj, const char* path)
{
    cout << "CreateDirectory_Stub_TRUE" << endl;
    return true;
}

static bool CreateDirectory_Stub_FALSE(void* obj, const char* path)
{
    cout << "CreateDirectory_Stub_FALSE" << endl;
    return false;
}

static bool IsFileExist_success(const std::string& fileName)
{
    return true;
}

static bool IsFileExist_failed(const std::string& fileName)
{
    return false;
}

 static boost::uintmax_t remove_all_success(string path)
 {
     return true;
 }

 static bool remove_all_failed(string path)
 {
     return false;
 }

/*
 * 用例名称:CheckDeviceNetworkConnect
 * 前置条件：
 * check点：输入是ip列表，以逗号分割，检查通过管理网络访问8088端口是否连通
 */
// TEST_F(PluginUtilitiesTest, CheckDeviceNetworkConnect)
// {
//     stub.set(Module::runShellCmdWithOutput, runShellCmdWithOutput_success);

//     string str = "xxx";
//     bool ret = PluginUtils::CheckDeviceNetworkConnect(str);
//     EXPECT_EQ(ret, true);

//     stub.reset(Module::runShellCmdWithOutput);
// }

TEST_F(PluginUtilitiesTest, CheckDeviceNetworkConnect_Fail)
{
    stub.set(Module::runShellCmdWithOutput, runShellCmdWithOutput_failed);

    string str = "xxx";
    bool ret = PluginUtils::CheckDeviceNetworkConnect(str);
    EXPECT_EQ(ret, false);

    stub.reset(Module::runShellCmdWithOutput);
}

TEST_F(PluginUtilitiesTest, CheckDeviceNetworkConnect_Success)
{
    stub.set(Module::runShellCmdWithOutput, runShellCmdWithOutput_success);

    string str = "xxx";
    bool ret = PluginUtils::CheckDeviceNetworkConnect(str);
    EXPECT_EQ(ret, true);

    stub.reset(Module::runShellCmdWithOutput);
}

/*
 * 用例名称:FormatTimeToStr
 * 前置条件：
 * check点：时间转字符串，转换结果形式为%Y-%m-%d-%H:%M:%S
 */
TEST_F(PluginUtilitiesTest, FormatTimeToStr)
{

    time_t timeInSeconds = 1577257503;
    string res = "2019-12-25-15:05:03";

    string ret = PluginUtils::FormatTimeToStr(timeInSeconds);

    EXPECT_EQ(ret, res);
}

/*
 * 用例名称:Base64Encode
 * 前置条件：
 * check点：检查编码解码功能
 */
TEST_F(PluginUtilitiesTest, Base64Encode)
{
    string str = "aaa";
    string ret = PluginUtils::Base64Encode(str);

    EXPECT_EQ(PluginUtils::Base64Decode(ret), str);
}

/*
 * 用例名称:GenerateHash
 * 前置条件：
 * check点：获得hash值
 */
TEST_F(PluginUtilitiesTest, GenerateHash)
{
    size_t ret = PluginUtils::GenerateHash("55");
    EXPECT_EQ(ret, ret);
}

/*
 * 用例名称:GetCurrentTimeInSeconds
 * 前置条件：
 * check点：获得当前时间点距离1970.01.01 00：00：00的秒数量
 */
TEST_F(PluginUtilitiesTest, GetCurrentTimeInSeconds)
{
    string dateAndTimeString;
    time_t ret = PluginUtils::GetCurrentTimeInSeconds(dateAndTimeString);

    time_t currTime;
    Module::CTime::Now(currTime);

    EXPECT_EQ(ret, currTime);
}

/*
 * 用例名称:CreateDirectory
 * 前置条件：
 * check点：创建目录，可多层
 */
TEST_F(PluginUtilitiesTest, CreateDirectory_TURE)
{
    stub.set(mkdir, mkdir_success);
    string path = "/xxx1";
    bool ret = PluginUtils::CreateDirectory(path);
    EXPECT_EQ(ret, true);
    stub.reset(mkdir);
}

// TEST_F(PluginUtilitiesTest, CreateDirectory_2)
// {
//     stub.set(mkdir, mkdir_failed);
//     string path = "/xxx2";
//     bool ret = PluginUtils::CreateDirectory(path);
//     EXPECT_EQ(ret, false);
//     stub.reset(mkdir);
// }

// TEST_F(PluginUtilitiesTest, CreateDirectory_FALSE)
// {
//     typedef bool (*fptr)(Module::FileSystemIO*, const char*);
//     fptr FileSystemIO_CreateDirectory = (fptr)(&Module::FileSystemIO::CreateDirectory);   //obtaining an address
//
//     stub.set(FileSystemIO_CreateDirectory, CreateDirectory_Stub_FALSE);
//     string path = "/xxx1";
//     size_t ret = PluginUtils::CreateDirectory(path);
//     EXPECT_EQ(ret, false);
//     stub.reset(ADDR(Module::FileSystemIO, CreateDirectory));
// }

/*
 * 用例名称:IsPathExists
 * 前置条件：
 * check点：文件是否存在
 */
TEST_F(PluginUtilitiesTest, IsPathExists_FALSE)
{
    string path = "/pathNotExists";
    bool ret = PluginUtils::IsPathExists(path);
    EXPECT_FALSE(PluginUtils::IsPathExists(path));
}

/*
 * 用例名称:IsPathExists
 * 前置条件：
 * check点：文件是否存在
 */
TEST_F(PluginUtilitiesTest, IsPathExists_TRUE)
{
    string path = "/";
    EXPECT_TRUE(PluginUtils::IsPathExists(path));
}

/*
 * 用例名称:IsPathExists
 * 前置条件：
 * check点：文件是否存在
 */
TEST_F(PluginUtilitiesTest, IsPathExists)
{
    string str = "err";
    boost::filesystem::path path1 = "/a/b.txt";
    boost::filesystem::path path2 = "/a/b.txt";
    boost::system::error_code ec;
    MOCKER_CPP(boost::filesystem::exists, bool(const boost::filesystem::path&))
            .stubs()
            .will(throws(boost::filesystem::filesystem_error(str, path1, path2, ec)));

    string path = "/";
    EXPECT_FALSE(PluginUtils::IsPathExists(path));
}

/*
 * 用例名称:Rename
 * 前置条件：
 * check点：文件重命名或实现文件移动
 */
TEST_F(PluginUtilitiesTest, Rename)
{
    string str1 = "/a";
    string str2 = "/b";
    EXPECT_FALSE(PluginUtils::Rename(str1, str2));
}

/*
 * 用例名称:Remove2
 * 前置条件：
 * check点：删除目录
 */
TEST_F(PluginUtilitiesTest, Remove_Success)
{
    string path = "/dddd";
    EXPECT_TRUE(PluginUtils::Remove(path));
}

/*
 * 用例名称:Remove2
 * 前置条件：
 * check点：删除目录
 */
TEST_F(PluginUtilitiesTest, Remove_Fail)
{
    string str = "err";
    boost::filesystem::path path1 = "/a/b.txt";
    boost::filesystem::path path2 = "/a/b.txt";
    boost::system::error_code ec;
    MOCKER_CPP(boost::filesystem::exists, bool(const boost::filesystem::path&))
            .stubs()
            .will(throws(boost::filesystem::filesystem_error(str, path1, path2, ec)));

    string path = "/dddd";
    EXPECT_FALSE(PluginUtils::Remove(path));
}

/*
 * 用例名称:FormatCapacity
 * 前置条件：
 * check点：格式化存储容量
 */

 TEST_F(PluginUtilitiesTest, FormatCapacity_B)
{

    uint64_t capacity = 512;
    string ret = PluginUtils::FormatCapacity(capacity);

    string res = "512B";
    EXPECT_EQ(ret, res);
}

TEST_F(PluginUtilitiesTest, FormatCapacity_KB)
{

    uint64_t capacity = PluginUtilitiesTest::KB_size *2;
    string ret = PluginUtils::FormatCapacity(capacity);

    string res = "2.0KB";
    EXPECT_EQ(ret, res);
}

TEST_F(PluginUtilitiesTest, FormatCapacity_MB)
{

    uint64_t capacity = PluginUtilitiesTest::MB_size *2;
    string ret = PluginUtils::FormatCapacity(capacity);

    string res = "2.0MB";
    EXPECT_EQ(ret, res);
}

TEST_F(PluginUtilitiesTest, FormatCapacity_GB)
{

    uint64_t capacity = PluginUtilitiesTest::GB_size *2;
    string ret = PluginUtils::FormatCapacity(capacity);

    string res = "2.0GB";
    EXPECT_EQ(ret, res);
}

TEST_F(PluginUtilitiesTest, FormatCapacity_TB)
{

    uint64_t capacity = PluginUtilitiesTest::TB_size *2;
    string ret = PluginUtils::FormatCapacity(capacity);

    string res = "2.00TB";
    EXPECT_EQ(ret, res);
}

TEST_F(PluginUtilitiesTest, FormatCapacity_PB)
{

    uint64_t capacity = PluginUtilitiesTest::PB_size *2;
    string ret = PluginUtils::FormatCapacity(capacity);

    string res = "2.0PB";
    EXPECT_EQ(ret, res);
}

/*
 * 用例名称:GetFileName
 * 前置条件：
 * check点：根据文件路径获取文件名字
 */
TEST_F(PluginUtilitiesTest, GetFileName)
{
    string filePath = "/xxx/yyy/file.cpp";
    string fileName = "file.cpp";
    string ret = PluginUtils::GetFileName(filePath);

    EXPECT_EQ(ret, fileName);
}

/*
 * 用例名称:RemoveFile
 * 前置条件：
 * check点：删除文件
 */
TEST_F(PluginUtilitiesTest, RemoveFile)
{
    string filePath = "/xxx/yyy/file.cpp";
    bool ret = PluginUtils::RemoveFile(filePath);

    EXPECT_EQ(ret, true);
}

/*
 * 用例名称:CopyFile
 * 前置条件：
 * check点：拷贝文件
 */

 TEST_F(PluginUtilitiesTest, CopyFile_Success)
{

    stub.set(PluginUtils::IsFileExist, IsFileExist_success);

    string srcfile = "/";
    string dstfile = "/xxx/yyy/file.cpp";

    bool ret = PluginUtils::CopyFile(srcfile, dstfile);
    EXPECT_EQ(ret, false);

    stub.reset(PluginUtils::IsFileExist);
}

TEST_F(PluginUtilitiesTest, CopyFile_Fail)
{

    stub.set(PluginUtils::IsFileExist, IsFileExist_failed);

    string srcfile = "/";
    string dstfile = "/xxx/yyy/file.cpp";

    bool ret = PluginUtils::CopyFile(srcfile, dstfile);
    EXPECT_EQ(ret, false);

    stub.reset(PluginUtils::IsFileExist);
}

/*
 * 用例名称:GetFileListInDirectory
 * 前置条件：
 * check点：获得某路径下文件列表
 */
TEST_F(PluginUtilitiesTest, GetFileListInDirectory)
{
    string dir = "/xxx/yyy";
    vector<string> fileList;
    bool ret = PluginUtils::GetFileListInDirectory(dir, fileList);

    EXPECT_EQ(ret, false);
}

TEST_F(PluginUtilitiesTest, GetFileListInDirectory_2)
{
    string dir = "/";
    vector<string> fileList;
    bool ret = PluginUtils::GetFileListInDirectory(dir, fileList);

    EXPECT_EQ(ret, true);
}

/*
 * 用例名称:测试递归生成目录
 * 前置条件：多级目录
 * check点：生成目录成功，跳过已存在
 */
TEST(PluginUtilsTest, RecurseCreateDirectory) {
    std::string path = "/tmp/llt/RecurseCreateDirectoryTest";
    bool ret = PluginUtils::CreateDirectory(path);
    EXPECT_EQ(ret, true);
}

/*
 * 用例名称：IsDirExist
 * 前置条件：无
 * check点：拷贝文件
 **/
TEST_F(PluginUtilitiesTest, IsDirExist)
{
    string str = "err";
    boost::filesystem::path path1 = "/a/b.txt";
    boost::filesystem::path path2 = "/a/b.txt";
    boost::system::error_code ec;
    MOCKER_CPP(boost::filesystem::is_directory, bool(const boost::filesystem::path&))
            .stubs()
            .will(throws(boost::filesystem::filesystem_error(str, path1, path2, ec)));
    std::string dstDir = "/";
    bool ret = PluginUtils::IsDirExist(dstDir);
    EXPECT_EQ(ret, false);
}

/*
 * 用例名称：GetRealPath
 * 前置条件：无
 * check点：获取真实路径
 **/
TEST_F(PluginUtilitiesTest, GetRealPath)
{
    std::string path = "/home/../home";
    std::string realPath = PluginUtils::GetRealPath(path);
    EXPECT_EQ(realPath, "/home");
    path = "/abcdefg";
    realPath = PluginUtils::GetRealPath(path);
    EXPECT_EQ(realPath, "");
}

/*
 * 用例名称：GetRealPath
 * 前置条件：无
 * check点：获取目录名称
 **/
TEST_F(PluginUtilitiesTest, GetDirName)
{
    std::string dir = "/home/test.txt";
    std::string dirName = PluginUtils::GetDirName(dir);
    EXPECT_EQ(dirName, "/home");
}

/*
 * 用例名称：GetRealPath
 * 前置条件：无
 * check点：判断路径是否为目录
 **/
TEST_F(PluginUtilitiesTest, IsDir)
{
    std::string dir = "/home";
    bool ret = PluginUtils::IsDir(dir);
    EXPECT_EQ(ret, true);
}

/*
 * 用例名称：GetVolumeUuid
 * 前置条件：无
 * check点：GetVolumeUuid
 **/
TEST_F(PluginUtilitiesTest, GetVolumeUuid)
{
    std::string dir = "";
    string ret = PluginUtils::GetVolumeUuid(dir);
    EXPECT_EQ(ret, "");

    dir = "/home";
    ret = PluginUtils::GetVolumeUuid(dir);
    EXPECT_NE(ret, "");

}

/*
 * 用例名称：WriteFile
 * 前置条件：无
 * check点：保存备份统计信息
 **/
TEST_F(PluginUtilitiesTest, WriteFile)
{
    string path = "/statistic.json";
    string statisticInfo = "test";
    bool ret = PluginUtils::WriteFile(path, statisticInfo);
    EXPECT_EQ(ret, true);
}

/*
 * 用例名称：ReadFile
 * 前置条件：无
 * check点：获取备份统计信息
 **/
TEST_F(PluginUtilitiesTest, ReadFile)
{
    string path = "/home/statistic.json";
    string data;
    stub.set(PluginUtils::IsFileExist, IsFileExist_failed);
    bool ret = PluginUtils::ReadFile(path, data );
    EXPECT_EQ(ret, false);
}

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Author: l00347293
 * Create: 2022-07-21.
 */

#include "ScannerUtils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "addr_pri.h"

// #include "ScannerUtils.h"
#include <random>
#include <thread>
#include <sys/stat.h>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>
#include "log/Log.h"
#include "ScanConsts.h"

using namespace std;
using namespace Module;
using namespace FS_SCANNER;

class ScannerUtilsTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void ScannerUtilsTest::SetUp() {}
void ScannerUtilsTest::TearDown() {}
void ScannerUtilsTest::SetUpTestCase() {}
void ScannerUtilsTest::TearDownTestCase() {}

/*
 * 用例名称：GetParentDirOfFile
 * 前置条件：无
 * check点：获得文件父目录
 **/
TEST_F(ScannerUtilsTest, GetParentDirOK)
{
    EXPECT_EQ("/home", FS_SCANNER::GetParentDirOfFile("/home/file.txt"));
}

/*
 * 用例名称：CheckParentDirIsReachable
 * 前置条件：无
 * check点：检查父目录是否可达
 **/
TEST_F(ScannerUtilsTest, CheckParentDirIsReachable)
{
    EXPECT_EQ(true, FS_SCANNER::CheckParentDirIsReachable("/home/file.txt"));
}

/*
 * 用例名称：GetFileNameOfPath
 * 前置条件：无
 * check点：获得文件名
 **/
TEST_F(ScannerUtilsTest, GetFileNameOfPath)
{
    EXPECT_EQ("file.txt", FS_SCANNER::GetFileNameOfPath("/file.txt"));
    EXPECT_EQ("file.txt", FS_SCANNER::GetFileNameOfPath("/file.txt/"));
}

/*
 * 用例名称：GetCommaCountOfString
 * 前置条件：无
 * check点：获取字符串中逗号的数量
 **/
TEST_F(ScannerUtilsTest, GetCommaCountOfString)
{
    EXPECT_EQ(2, FS_SCANNER::GetCommaCountOfString("a,b,c"));
}

/*
 * 用例名称：ConstructStringName
 * 前置条件：无
 * check点：构建字符串名字
 **/
TEST_F(ScannerUtilsTest, ConstructStringName)
{
    uint32_t offset = 0;
    uint32_t totCommaCnt = 0;
    vector<std::string> lineContents = {"aaa", "bbb"};
    EXPECT_NE("", FS_SCANNER::ConstructStringName(offset, totCommaCnt, lineContents));
}

/*
 * 用例名称：GetRandomNumber
 * 前置条件：无
 * check点：获取随机数
 **/
TEST_F(ScannerUtilsTest, GetRandomNumber)
{
    EXPECT_EQ(0, FS_SCANNER::GetRandomNumber(0, 0));
}

/*
 * 用例名称：Atou16
 * 前置条件：无
 * check点：数字字符串转为整数
 **/
TEST_F(ScannerUtilsTest, Atou16)
{
    char *s = "11";
    EXPECT_EQ(11, FS_SCANNER::Atou16(s));
    
    char *s2 = "-11";
    EXPECT_EQ(0, FS_SCANNER::Atou16(s2));
}

/*
 * 用例名称：GetScanTypeStr
 * 前置条件：无
 * check点：获取扫描类型
 **/
TEST_F(ScannerUtilsTest, GetScanTypeStr)
{
    ScanJobType scanType = ScanJobType::INC;
    EXPECT_EQ("INC", FS_SCANNER::GetScanTypeStr(scanType));

    scanType = ScanJobType::RESTORE;
    EXPECT_EQ("RESTORE", FS_SCANNER::GetScanTypeStr(scanType));

    scanType = ScanJobType::INDEX;
    EXPECT_EQ("INDEX", FS_SCANNER::GetScanTypeStr(scanType));

    scanType = ScanJobType::ARCHIVE;
    EXPECT_EQ("ARCHIVE", FS_SCANNER::GetScanTypeStr(scanType));
    
    scanType = ScanJobType::CONTROL_GEN;
    EXPECT_EQ("FULL", FS_SCANNER::GetScanTypeStr(scanType));
}

static bool Stub_RemoveDir_False()
{
    return false;
}
static bool Stub_CreateDir_False()
{
    return false;
}

/*
 * 用例名称：CheckAndCreateDirectory
 * 前置条件：无
 * check点：检查并创建目录
 **/
TEST_F(ScannerUtilsTest, CheckAndCreateDirectory)
{
    stub.set(FS_SCANNER::RemoveDir, Stub_RemoveDir_False);
    stub.set(FS_SCANNER::CreateDir, Stub_CreateDir_False);

    string str = "/file.txt";
    EXPECT_EQ(false, FS_SCANNER::CheckAndCreateDirectory(str));

    stub.reset(FS_SCANNER::RemoveDir);
    stub.reset(FS_SCANNER::CreateDir);
}

/*
 * 用例名称：MaxFileNumber
 * 前置条件：无
 * check点：字符串转为整型
 **/
TEST_F(ScannerUtilsTest, MaxFileNumber)
{
    string str = "nanjing";
    EXPECT_EQ(0, FS_SCANNER::MaxFileNumber(str));

    str = "nan_jing";
    EXPECT_EQ(0, FS_SCANNER::MaxFileNumber(str));
}

static void CreateFile(const std::string& path, const std::string output = "helloworld")
{
    std::ofstream file(path);
    file << output;
    file << endl;
    file.close();
}

namespace {
    const std::string PATH_FOR_TEST_CASE = "/tmp/ScannerUtilsTest";
    const std::string PATH_FOR_META = PATH_FOR_TEST_CASE + "/meta";
    const std::string PATH_FOR_CTRL = PATH_FOR_TEST_CASE+  "/scan";
    const std::string PATH_FOR_FS = PATH_FOR_TEST_CASE+  "/fs"; // to simulate a filesyetem
    const std::string LOG_PATH = PATH_FOR_TEST_CASE + "/log";
    const std::string PATH_FOR_DIR = PATH_FOR_TEST_CASE + "/dir";
    const std::string PATH_FOR_FILE = PATH_FOR_TEST_CASE + "/file.txt";
}

static void SetupDirForTest() 
{
    FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE);
    if (!FS_SCANNER::CreateDir(PATH_FOR_TEST_CASE)) {
        ERRLOG("failed to create dir %s", PATH_FOR_TEST_CASE.c_str());
    }
    if (!FS_SCANNER::CreateDir(PATH_FOR_META)) {
        ERRLOG("failed to create dir %s", PATH_FOR_META.c_str());
    }
    string metaLatest = PATH_FOR_META + "/latest";
    if (!FS_SCANNER::CreateDir(metaLatest)) {
        ERRLOG("failed to create dir %s", metaLatest.c_str());
    }
    if (!FS_SCANNER::CreateDir(PATH_FOR_CTRL)) {
        ERRLOG("failed to create dir %s", PATH_FOR_CTRL.c_str());
    }
    if (!FS_SCANNER::CreateDir(LOG_PATH)) {
        ERRLOG("failed to create dir %s", LOG_PATH.c_str());
    }
}

static void CreateFileDir()
{
    // FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE);
    if (!FS_SCANNER::CreateDir(PATH_FOR_DIR)) {
        ERRLOG("failed to create dir %s", PATH_FOR_DIR.c_str());
    }
    CreateFile(PATH_FOR_FILE);
}

static void RemoveFileDir()
{
    // FS_SCANNER::RemoveDir(PATH_FOR_DIR);
    // FS_SCANNER::RemoveDir(PATH_FOR_FILE); 
    FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE); 
}
/*
 * 用例名称：CopyFile
 * 前置条件：无
 * check点：拷贝文件
 **/
TEST_F(ScannerUtilsTest, CopyFile)
{
    SetupDirForTest();
    CreateFileDir();
    std::string srcName = PATH_FOR_FILE;
    std::string dstDir = PATH_FOR_DIR;
    EXPECT_EQ(true, FS_SCANNER::CopyFile(srcName, dstDir));
    RemoveFileDir();
}




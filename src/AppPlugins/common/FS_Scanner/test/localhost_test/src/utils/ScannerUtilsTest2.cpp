#include "ScannerUtils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
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


namespace {
    /**
    * tmp/
    *   |--PosixScannerTest/
    */
    const std::string PATH_FOR_TEST_CASE = "/tmp/ScannerUtilsTest2";
    const std::string PATH_FOR_FS = PATH_FOR_TEST_CASE+  "/fs";
}

static void SetupDirForTest() 
{
    FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE);
    if (!FS_SCANNER::CreateDir(PATH_FOR_TEST_CASE)) {
        ERRLOG("failed to create dir %s", PATH_FOR_TEST_CASE.c_str());
    }
}

static void CreateFile(const std::string& path, const std::string output = "helloworld")
{
    std::ofstream file(path);
    file.close();
}

static void SetupFileTree(const std::map<std::string, std::vector<std::string>> &fs)
{
    FS_SCANNER::RemoveDir(PATH_FOR_FS);
    if (!FS_SCANNER::CreateDir(PATH_FOR_FS)) {
        ERRLOG("failed to create dir %s", PATH_FOR_FS.c_str());
    }
    
    for (const auto &entry: fs) {
        string realDirPath = PATH_FOR_FS + entry.first;
        if (!FS_SCANNER::CreateDir(realDirPath)) {
            ERRLOG("failed to create dir %s", realDirPath.c_str());
        }
        for (const std::string& filename: entry.second) {
            string realFilePath = realDirPath + "/" + filename;
            CreateFile(realFilePath);
        }
    }
}

class ScannerUtilsTest2 : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void ScannerUtilsTest2::SetUp() 
{
    string str = "err";
    boost::filesystem::path path1 = "/a/b.txt";
    boost::filesystem::path path2 = "/a/b.txt";
    boost::system::error_code ec;
    MOCKER_CPP(boost::filesystem::remove)
            .stubs()
            .will(throws(boost::filesystem::filesystem_error(str, path1, path2, ec)));
}

void ScannerUtilsTest2::TearDown() 
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}
void ScannerUtilsTest2::SetUpTestCase() {}

void ScannerUtilsTest2::TearDownTestCase() {}

/*
 * 用例名称：RemoveFile
 * 前置条件：无
 * check点：删除文件
 **/
TEST_F(ScannerUtilsTest2, RemoveFile)
{
    std::string path = "/home/file.txt";
    bool ret = FS_SCANNER::RemoveFile(path);
    EXPECT_EQ(ret, false);
}

/*
 * 用例名称：Rename
 * 前置条件：无
 * check点：重命名
 **/
TEST_F(ScannerUtilsTest2, Rename)
{
    std::string oldName = "/home/file.txt";
    std::string newName = "/home/file2.txt";
    bool ret = FS_SCANNER::Rename(oldName, newName);
    EXPECT_EQ(ret, false);
}

/*
 * 用例名称：RemoveDir
 * 前置条件：无
 * check点：移除目录
 **/
TEST_F(ScannerUtilsTest2, RemoveDir)
{
    string str = "err";
    boost::filesystem::path path1 = "/a/b.txt";
    boost::filesystem::path path2 = "/a/b.txt";
    boost::system::error_code ec;
    MOCKER_CPP(boost::filesystem::remove_all)
            .stubs()
            .will(throws(boost::filesystem::filesystem_error(str, path1, path2, ec)));

    SetupDirForTest();
    std::string dirName = "/tmp/ScannerUtilsTest2";
    bool ret = FS_SCANNER::RemoveDir(dirName);
    EXPECT_EQ(ret, false);
}

/*
 * 用例名称：PathExist
 * 前置条件：无
 * check点：路径是否存在
 **/
TEST_F(ScannerUtilsTest2, PathExist)
{
    string str = "err";
    boost::filesystem::path path1 = "/a/b.txt";
    boost::filesystem::path path2 = "/a/b.txt";
    boost::system::error_code ec;
    MOCKER_CPP(boost::filesystem::exists, bool(const boost::filesystem::path&, boost::system::error_code&))
            .stubs()
            .will(throws(boost::filesystem::filesystem_error(str, path1, path2, ec)));
    SetupDirForTest();
    std::string dirName = "/tmp/ScannerUtilsTest2";
    bool ret = FS_SCANNER::PathExist(dirName);
    EXPECT_EQ(ret, false);
}

/*
 * 用例名称：CopyFile
 * 前置条件：无
 * check点：拷贝文件
 **/
TEST_F(ScannerUtilsTest2, CopyFile)
{
    string str = "err";
    boost::filesystem::path path1 = "/a/b.txt";
    boost::filesystem::path path2 = "/a/b.txt";
    boost::system::error_code ec;
    MOCKER_CPP(boost::filesystem::is_directory, bool(const boost::filesystem::path&))
            .stubs()
            .will(throws(boost::filesystem::filesystem_error(str, path1, path2, ec)));

    std::string srcName;
    std::string dstDir = "/tmp/ScannerUtilsTest2";
    bool ret = FS_SCANNER::CopyFile(srcName, dstDir);
    EXPECT_EQ(ret, false);
}

/*
 * 用例名称：GetUniqueId
 * 前置条件：无
 * check点：获取uuid
 **/
/*
TEST_F(ScannerUtilsTest2, GetUniqueId)
{
    string str = "err";
    boost::filesystem::path path1 = "/a/b.txt";
    boost::filesystem::path path2 = "/a/b.txt";
    boost::system::error_code ec;
    MOCKER_CPP(boost::uuids::to_string)
            .stubs()
            .will(throws(boost::filesystem::filesystem_error(str, path1, path2, ec)));

    std::string ret = FS_SCANNER::GetUniqueId();
    EXPECT_NE(ret, "");
}
*/

/*
 * 用例名称：GetFileListInDirectory
 * 前置条件：无
 * check点：获取目录中的文件列表
 **/
// TEST_F(ScannerUtilsTest2, GetFileListInDirectory)
// {
//     string str = "err";
//     boost::filesystem::path path1 = "/a/b.txt";
//     boost::filesystem::path path2 = "/a/b.txt";
//     boost::system::error_code ec;
//     MOCKER_CPP(std::vector<std::string>::emplace_back) // 待解决，标准库模板函数打桩
//             .stubs()
//             .will(throws(boost::filesystem::filesystem_error(str, path1, path2, ec)));
    
//     SetupDirForTest();
//     SetupFileTree({ // 创建文件树
//         {
//             "/home", {}
//         }, {
//             "/home/user1", {"1.txt", "2.txt", "3.txt", "4.txt"}
//         }
//     });

//     std::string path = PATH_FOR_TEST_CASE;
//     std::vector<std::string> fileList;      
//     bool ret = FS_SCANNER::GetFileListInDirectory(path, fileList);
//     EXPECT_EQ(ret, false);
//     for (const string& str : fileList) {
//         cout << str << endl;
//     }
// }
#include "PosixUtils.h"
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/acl.h>
#include <sys/xattr.h>
#include <sys/stat.h>
#include "ScannerUtils.h"
#include "log/Log.h"
#include <fstream> 

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

using namespace std;
using namespace Module;
using namespace FS_SCANNER;

namespace {
    /**
    * tmp/
    *   |--PosixUtilsTest/
    *           |--meta/
    *           |--scan/
    *           |--fs/
    *           |--log/
    */
    const std::string PATH_FOR_TEST_CASE = "/tmp/PosixUtilsTest";
    const std::string PATH_FOR_META = PATH_FOR_TEST_CASE + "/meta";
    const std::string PATH_FOR_CTRL = PATH_FOR_TEST_CASE+  "/scan";
    const std::string PATH_FOR_FS = PATH_FOR_TEST_CASE+  "/fs"; // to simulate a filesyetem
    const std::string LOG_PATH = PATH_FOR_TEST_CASE + "/log";
}

void static SetupDirForTest() 
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

static void CreateFile(const std::string& path, const std::string output = "helloworld")
{
    std::ofstream file(path);
    file << output;
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

class PosixUtilsTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<PosixUtils> m_ins {nullptr};
};

void PosixUtilsTest::SetUp() {
    m_ins = std::make_unique<PosixUtils>();
}

void PosixUtilsTest::TearDown() {}

void PosixUtilsTest::SetUpTestCase() {}

void PosixUtilsTest::TearDownTestCase()
{
    FS_SCANNER::RemoveDir(PATH_FOR_TEST_CASE);
}

static bool Stub_Func_True()
{
    return true;
}

static bool Stub_Func_False()
{
    return false;
}

static void Stub_Func_Void()
{}

/*
 * 用例名称：RemovePathPrefixFromString
 * 前置条件：无
 * check点：删除路径
 **/
TEST_F(PosixUtilsTest, RemovePathPrefixFromString) 
{
    string path = "/a/b/file.txt";
    string prefix = "/a/b";
    string ret = m_ins->RemovePathPrefixFromString(path, prefix);
    EXPECT_EQ(ret, "/file.txt");
    prefix = "/a/b/";
    ret = m_ins->RemovePathPrefixFromString(path, prefix);
    EXPECT_EQ(ret, "/file.txt");
}


/*
 * 用例名称：GetFileSparse
 * 前置条件：无
 * check点：获取文件Sparse
 **/
TEST_F(PosixUtilsTest, GetFileSparse) 
{
    string path = "/a/b/file.txt";
    XMetaField ret = m_ins->GetFileSparse(path);
    EXPECT_EQ(ret.m_xMetaType, XMETA_TYPE::XMETA_TYPE_SPARSE_INFO);

    SetupDirForTest(); // 创建目录用于Test
    SetupFileTree({ // 创建文件树
        {
            "/home", {}
        }, {
            "/home/user1", {"file.txt", "file1.txt"}
        }
    });

    std::string fpath = "/tmp/PosixUtilsTest/fs/home/user1/file.txt"; 
    ret = m_ins->GetFileSparse(fpath);
    EXPECT_EQ(ret.m_xMetaType, XMETA_TYPE::XMETA_TYPE_SPARSE_INFO);

    // m_ins->GetFileSparse(fpath);
}

static string Stub_GetFileOrDirNameFromXMeta()
{
    return "/a/b/file.txt";
}
/*
 * 用例名称：GetFileSparse
 * 前置条件：无
 * check点：获取文件Sparse
 **/
TEST_F(PosixUtilsTest, ReadXattr) 
{
    stub.set(FS_SCANNER::GetFileOrDirNameFromXMeta, Stub_GetFileOrDirNameFromXMeta);
    DirMetaWrapper dirWrapper;
    string prefix = "/a/b/";
    m_ins->RemovePathPrefixInDirectoryWrapper(dirWrapper, prefix);
    stub.reset(FS_SCANNER::GetFileOrDirNameFromXMeta);

    vector<XMetaField> xattrList;
    string path = "/a/b/file.txt";
    char *key = "file";
    ssize_t keylen; 
    ssize_t buflen;
    bool ret = m_ins->ReadXattr(xattrList, path, key, keylen, buflen);
    EXPECT_EQ(ret, true);
}

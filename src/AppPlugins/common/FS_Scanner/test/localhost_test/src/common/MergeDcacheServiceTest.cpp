#include "MergeDcacheService.h"
#include "ScannerUtils.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

using namespace std;
using namespace FS_SCANNER;
using namespace Module;

class MergeDcacheServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<MergeDcacheService> m_ins {nullptr};
    std::unique_ptr<MergeDcache> m_md {nullptr};
};

void MergeDcacheServiceTest::SetUp()
{
    ScanConfig config;
    ScanInfo info;
    config.copyId = "mm";
    config.scanCtrlMaxDataSize = "aa";
    config.scanCtrlMaxEntriesFullBkup = 1;
    config.scanCtrlMaxEntriesIncBkup = 1;
    std::vector<std::string> dcacheList;
    int index = 0;
    m_ins = std::make_unique<MergeDcacheService>(config, info);
    m_md = std::make_unique<MergeDcache>(config);
}

void MergeDcacheServiceTest::TearDown() {}

void MergeDcacheServiceTest::SetUpTestCase() {}

void MergeDcacheServiceTest::TearDownTestCase() {}

static bool Stub_Func_True()
{
    return true;
}

static bool Stub_Func_False()
{
    return false;
}

static void Stub_Func_Void()
{
    return;
}

static CTRL_FILE_RETCODE STUB_Open_FAILED(void* obj)
{
    return CTRL_FILE_RETCODE::FAILED;
}

/*
 * 用例名称：CheckDircachePath
 * 前置条件：无
 * check点：检测dcache路径
 **/
 TEST_F(MergeDcacheServiceTest, CheckDircachePath)
 {
     DirCache dcache1;
     dcache1.m_dirMetaHash.crc = 0;
     dcache1.m_inode = 0;
     dcache1.m_fileId = 0;
     dcache1.m_mdataOffset = 0;
     DirCache dcache2;
     dcache2.m_dirMetaHash.crc = 0;
     dcache2.m_inode = 0;
     dcache2.m_fileId = 0;
     dcache2.m_mdataOffset = 0;
     m_md->m_directory = "AAA";
     string metaFileName = "mm";
     ScanConfig m_config;
     stub.set(ADDR(Module::FileCacheParser, Open), STUB_Open_FAILED);

     SCANNER_STATUS ret = m_md->CheckDircachePath(dcache1, dcache2);
     EXPECT_EQ(ret, SCANNER_STATUS::FAILED);
     stub.reset(ADDR(Module::FileCacheParser, Open));
 }

/*
 * 用例名称：PopDirCaheFileEntry
 * 前置条件：无
 * check点：检测dcache路径
 **/
 /* TEST_F(MergeDcacheServiceTest, PopDirCaheFileEntry)
 {
     m_ins->m_dirCacheFileList.push_back("aa");
     m_ins->m_dirCacheFileList.push_back("bb");
     m_ins->m_dirCacheFileList.push_back("cc");
     string ret = m_ins->PopDirCaheFileEntry();
     EXPECT_EQ(ret, "cc");
 } */

//   TEST_F(MergeDcacheServiceTest, WriteToTmpDirCacheFile)
//   {
//     queue<DirCache> dcQueue;
//     DirCache dirCache;
//     dirCache.m_fileId = 0;
//     dirCache.m_hashTag = 0;
//     dirCache.m_inode = 0;
//     dcQueue.push(dirCache);

//     string fname = "/home";
//     auto dcacheTmpFile = m_ins->InitDcacheObj(fname, CTRL_FILE_OPEN_MODE::WRITE);
//     int count = 1; 
//     DirCache prevHash;
//     prevHash.m_fileId = 1;
//     m_ins->WriteToTmpDirCacheFile(dcQueue, dcacheTmpFile, count, prevHash);

//   }



/*
 * 用例名称：MergeDirCacheFiles
 * 前置条件：无
 * check点：合并dcache文件
 **/
TEST_F(MergeDcacheServiceTest, MergeDirCacheFiles) 
{
    string fileName1 = "";
    string fileName2 = "b"; 
    int index = 0;
    string ret = m_md->MergeDirCacheFiles(fileName1, fileName2, index);
    fileName1 = "a";
    fileName2 = "";
    ret = m_md->MergeDirCacheFiles(fileName1, fileName2, index);

}

/*
 * InitDcacheObj
 * 前置条件：无
 * check点：初始化Dcache
 **/
// TEST_F(MergeDcacheServiceTest, InitDcacheObj)
// {
//     string fname = "mm";
//     CTRL_FILE_OPEN_MODE mode = CTRL_FILE_OPEN_MODE::READ;
//     std::unique_ptr<DirCacheParser> ret = m_ins->InitDcacheObj(fname, mode);
//     EXPECT_EQ(ret, nullptr);
// } 

/*
 * 用例名称：RemoveDcacheFiles
 * 前置条件：无
 * check点：删除dcache文件
 **/
TEST_F(MergeDcacheServiceTest, RemoveDcacheFiles) 
{
    string fileName1;
    string fileName2; 
    stub.set(FS_SCANNER::RemoveFile, Stub_Func_False);
    m_md->RemoveDcacheFiles(fileName1, fileName2);
    stub.reset(FS_SCANNER::RemoveFile);

    stub.set(FS_SCANNER::RemoveFile, Stub_Func_True);
    m_md->RemoveDcacheFiles(fileName1, fileName2);
    stub.reset(FS_SCANNER::RemoveFile);
}

/*
 * 用例名称：CopyToprevHash
 * 前置条件：无
 * check点：拷贝到prevHash
 **/
/* TEST_F(MergeDcacheServiceTest, CopyToprevHash) 
{
    DirCache dcache;
    DirCache prevHash;
    m_ins->CopyToprevHash(dcache, prevHash);
}

/*
 * 用例名称：CompareAndWriteToTmpDirCache
 * 前置条件：无
 * check点：比较并写入到临时dcache
 **/
TEST_F(MergeDcacheServiceTest, CompareAndWriteToTmpDirCache) 
{

    // stub.set(ADDR(DirCache, WriteDirCache), Stub_Func_Void);
    // queue<DirCache> dcQueue1,
    // queue<DirCache> dcQueue2;
    // int count;
    // auto dcacheObj;
    // DirCache prevHash;
    // m_ins->CompareAndWriteToTmpDirCache(dcQueue1, dcQueue2, count, dcacheObj, prevHash);
    // stub.reset(ADDR(DirCache, WriteDirCache));
}    
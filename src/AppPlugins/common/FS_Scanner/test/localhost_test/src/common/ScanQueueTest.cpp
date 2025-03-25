#include "ScanQueue.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "ScannerTime.h"

using namespace std;
using namespace Module;
using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using namespace std;
using namespace FS_SCANNER;
using namespace Module;

namespace {
    constexpr auto MODULE = "COMMON_SERVICE";
    constexpr auto SCANNER_SUCCESS = Module::SUCCESS; // 成功标识
    constexpr auto SCANNER_FAILED = Module::FAILED; // 失败标识
}

class ScanQueueTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::shared_ptr<ScanQueue> m_ins;
};

void ScanQueueTest::SetUp()
{
    shared_ptr<BufferQueue<DirStat>> input;
    string scanOutputDirectory;
    m_ins = make_shared<ScanQueue>(input, scanOutputDirectory, 10000, 8000);
}

void ScanQueueTest::TearDown()
{}

void ScanQueueTest::SetUpTestCase()
{}

void ScanQueueTest::TearDownTestCase()
{}

static CTRL_FILE_RETCODE STUB_OpenWrite_SUCCESS(void* obj)
{
    return CTRL_FILE_RETCODE::SUCCESS;
} 

static bool STUB_RemoveFile_FALSE(void* obj)
{
    return false;
}

static bool STUB_RemoveFile_TRUE(void* obj)
{
    return true;
}

static CTRL_FILE_RETCODE STUB_Open_SUCCESS(void* obj)
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

static void STUB_GetDirStatFileName_VOID(void* obj)
{
    return;
}

static CTRL_FILE_RETCODE STUB_Open_FAILED(void* obj)
{
    return CTRL_FILE_RETCODE::FAILED;
}

static bool STUB_WriteDirStatToBuffer_FALSE(void* obj)
{
    return false;
}

static bool STUB_WriteDirStatToBuffer_TRUE(void* obj)
{
    return true;
}

static string STUB_FS_SCANNER_String(void* obj)
{
    return "";
}

/*
 * 用例名称：OpenWrite
 * 前置条件：无
 * check点：成功打开
 **/
TEST_F(ScanQueueTest, OpenWrite)
{
    CTRL_FILE_RETCODE ret = m_ins->OpenWrite();
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
}

/*
 * 用例名称：CloseWrite
 * 前置条件：无
 * check点：成功关闭
 **/
TEST_F(ScanQueueTest, CloseWrite)
{
    CTRL_FILE_RETCODE ret = m_ins->CloseWrite();
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
}

/*
 * 用例名称：ReadHeader
 * 前置条件：无
 * check点：成功读取头文件
 **/
TEST_F(ScanQueueTest, ReadHeader)
{
    CTRL_FILE_RETCODE ret = m_ins->ReadHeader();
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
}

/*
 * 用例名称：WriteHeader
 * 前置条件：无
 * check点：成功写入头文件
 **/
TEST_F(ScanQueueTest, WriteHeader)
{
    CTRL_FILE_RETCODE ret = m_ins->WriteHeader();
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
}

/*
 * 用例名称：ValidateHeader
 * 前置条件：无
 * check点：成功验证头文件
 **/
TEST_F(ScanQueueTest, ValidateHeader)
{
    CTRL_FILE_RETCODE ret = m_ins->ValidateHeader();
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
}

/*
 * 用例名称：FlushToFile
 * 前置条件：无
 * check点：成功将缓存写入磁盘
 **/
TEST_F(ScanQueueTest, FlushToFile)
{
    CTRL_FILE_RETCODE ret = m_ins->FlushToFile();
    EXPECT_EQ(ret, CTRL_FILE_RETCODE::SUCCESS);
}

/*
 * 用例名称：Pop
 * 前置条件：无
 * check点：取出目标文件
 **/
// TEST_F(ScanQueueTest, Pop)
// {
//     DirStat dirStat;
//     dirStat.m_size = 0;
//     dirStat.m_uid = 1;
//     dirStat.m_isEmpty = true;
//     dirStat.m_path = "aaa";
//     bool ret = m_ins->Pop(dirStat);
//     EXPECT_EQ(ret, true);
// }

/*
 * 用例名称：WriteDirStat
 * 前置条件：无
 * check点：写入DirStat
 **/
TEST_F(ScanQueueTest, WriteDirStat)
{
    DirStat dirStat;
    dirStat.m_size = 0;
    dirStat.m_uid = 1;
    dirStat.m_isEmpty = true;
    dirStat.m_path = "aaa";
    uint32_t lengthWritten = 0;
    // m_ins->m_dirStatqMtx.unlock();

    stub.set(ADDR(SCAN_QUEUE_UTILS, WriteDirStatToBuffer), STUB_WriteDirStatToBuffer_FALSE);
    bool ret = m_ins->WriteDirStat(dirStat);
    EXPECT_EQ(ret, false);
    stub.reset(ADDR(SCAN_QUEUE_UTILS, WriteDirStatToBuffer));

    stub.set(ADDR(SCAN_QUEUE_UTILS, WriteDirStatToBuffer), STUB_WriteDirStatToBuffer_TRUE);
    m_ins->m_totalLengthWritten = 1024*1024;
    m_ins->m_bufferSize = 1;
    m_ins->m_maxSize = 1;
    stub.set(ADDR(ScanQueue, WriteBufferToFile), STUB_WriteDirStatToBuffer_FALSE);
    ret = m_ins->WriteDirStat(dirStat);
    EXPECT_EQ(ret, false);
    stub.reset(ADDR(ScanQueue, WriteBufferToFile));

    stub.set(ADDR(SCAN_QUEUE_UTILS, WriteDirStatToBuffer), STUB_WriteDirStatToBuffer_TRUE);
    stub.set(ADDR(ScanQueue, WriteBufferToFile), STUB_WriteDirStatToBuffer_TRUE);
    ret = m_ins->WriteDirStat(dirStat);
    EXPECT_EQ(ret, true);
    stub.reset(ADDR(SCAN_QUEUE_UTILS, WriteDirStatToBuffer));
    stub.reset(ADDR(ScanQueue, WriteBufferToFile));
}

/*
 * 用例名称：GetDirStatFileName
 * 前置条件：无
 * check点：获得父目录文件名
 **/
TEST_F(ScanQueueTest, GetDirStatFileName)
{
    m_ins->m_scanOutputDirectory = "aa";
    stub.set(ADDR(FS_SCANNER, GetUniqueId), STUB_FS_SCANNER_String);
    stub.set(ADDR(ParserUtils, GetParentDirOfFile), STUB_FS_SCANNER_String);
    m_ins->GetDirStatFileName();
    stub.reset(ADDR(FS_SCANNER, GetUniqueId));
    stub.reset(ADDR(ParserUtils, GetParentDirOfFile));
}

/*
 * 用例名称：WriteBufferToFile
 * 前置条件：无
 * check点：把缓冲区的内容写到文件上
 **/

TEST_F(ScanQueueTest, WriteBufferToFile)
{
    stub.set(ADDR(ScanQueue, GetDirStatFileName), STUB_GetDirStatFileName_VOID);
    stub.set(ADDR(FileParser, Open), STUB_Open_FAILED);
    bool ret = m_ins->WriteBufferToFile();
    EXPECT_EQ(ret, false);
    stub.reset(ADDR(FileParser, Open));

    stub.set(ADDR(FileParser, Open), STUB_Open_SUCCESS);
    stub.set(ADDR(FileParser, WriteToFile), STUB_Open_FAILED);
    ret = m_ins->WriteBufferToFile();
    EXPECT_EQ(ret,false);
    stub.reset(ADDR(FileParser, WriteToFile));

}
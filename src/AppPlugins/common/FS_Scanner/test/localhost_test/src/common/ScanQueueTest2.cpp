#include "ScanQueue.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "ScannerTime.h"

#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

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

class ScanQueueTest2 : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::shared_ptr<ScanQueue> m_ins;
};

void ScanQueueTest2::SetUp()
{
    std::shared_ptr<BufferQueue<DirStat>> input = std::make_shared<BufferQueue<DirStat>>(10);
    string scanOutputDirectory;
    m_ins = make_shared<ScanQueue>(input, scanOutputDirectory, 10000, 8000);
}

void ScanQueueTest2::TearDown()
{}

void ScanQueueTest2::SetUpTestCase()
{}

void ScanQueueTest2::TearDownTestCase()
{}

/*
 * 用例名称：Push
 * 前置条件：无
 * check点：加入队列
 **/
TEST_F(ScanQueueTest2, Push)
{
    MOCKER_CPP(&BufferQueue<DirStat>::GetSize)
            .stubs()
            .will(returnValue(10000000));
    MOCKER_CPP(&ScanQueue::WriteDirStat)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    DirStat dirStat;
    EXPECT_EQ(m_ins->Push(dirStat), false);
    EXPECT_EQ(m_ins->Push(dirStat), true);          
}

/*
 * 用例名称：BlockingPush
 * 前置条件：无
 * check点：加入队列
 **/
TEST_F(ScanQueueTest2, BlockingPush)
{
    MOCKER_CPP(&BufferQueue<DirStat>::GetSize)
            .stubs()
            .will(returnValue(10000000))
            .then(returnValue(10000000))
            .then(returnValue(0))
            .then(returnValue(0));
    MOCKER_CPP(&ScanQueue::WriteDirStat)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&BufferQueue<DirStat>::BlockingPush, bool(BufferQueue<DirStat>::*)(DirStat, uint32_t)) // 模板类中模板函数重载
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    DirStat dirStat;
    EXPECT_EQ(m_ins->BlockingPush(dirStat, 0), true);
    EXPECT_EQ(m_ins->BlockingPush(dirStat, 0), true); 
    EXPECT_EQ(m_ins->BlockingPush(dirStat, 0), true);
    EXPECT_EQ(m_ins->BlockingPush(dirStat, 0), true);        
}

/*
 * 用例名称：Pop
 * 前置条件：无
 * check点：出队
 **/
TEST_F(ScanQueueTest2, Pop)
{
    MOCKER_CPP(&BufferQueue<DirStat>::GetSize)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(100000000));
    MOCKER_CPP(&ScanQueue::ReadDirStatFiles)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&BufferQueue<DirStat>::Pop)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    DirStat dirStat;
    EXPECT_EQ(m_ins->Pop(dirStat), false);    
    EXPECT_EQ(m_ins->Pop(dirStat), true);      
}

/*
 * 用例名称：ReadDirStatFiles
 * 前置条件：无
 * check点：出队
 **/
TEST_F(ScanQueueTest2, ReadDirStatFiles)
{
    MOCKER_CPP(&ScanQueue::WriteBufferToFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&FS_SCANNER::PathExist)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&ScanQueue::ReadFromDirStatFile)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));       
    m_ins->m_bufferSize = 1;
    IterableQueue<std::string> fileList {};
    fileList.push("aaa");
    fileList.push("aaa");
    fileList.push("aaa");
    fileList.push("aaa");
    m_ins->m_fileList = fileList;
    m_ins->ReadDirStatFiles();
    m_ins->ReadDirStatFiles();  
    m_ins->ReadDirStatFiles();
    m_ins->ReadDirStatFiles(); 
//     EXPECT_NE(m_ins->m_numOfFilesCreated, 0);      
}
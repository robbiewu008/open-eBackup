#include <list>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/acl.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "Scanner.h"
#include "ScannerTime.h"
#include "ScannerUtils.h"
#include "NfsMetaProducer.h"

#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace std;
using namespace Module;

static char* DUMMY_ERROR = "DUMMY_ERROR";

class NfsMetaProducerTest2 : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::shared_ptr<NfsMetaProducer> m_ins {nullptr};
    ScanConfig m_config {};
};

void NfsMetaProducerTest2::SetUp()
{
    auto input = std::make_shared<BufferQueue<DirStat>>(1000);

    auto scanQueue = std::make_shared<ScanQueue>(input, "", 0, 0);
    auto output = std::make_shared<BufferQueue<DirectoryScan>>(1000);
    auto statsMgr = std::make_shared<StatisticsMgr>();
    auto scanFilter = std::make_shared<ScanFilter>();

    ScanInfo info {};
    auto chkPntMgr = std::make_shared<FSScannerCheckPoint>(m_config, info);

    m_ins = std::make_shared<NfsMetaProducer>(scanQueue, output, m_config, statsMgr, scanFilter, chkPntMgr);
    m_ins->InitContext(); // nfsClient GetErrors method will be used
}

void NfsMetaProducerTest2::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void NfsMetaProducerTest2::SetUpTestCase()
{}

void NfsMetaProducerTest2::TearDownTestCase()
{}

/*
 * 用例名称：IsRetryMount
 * 前置条件：无
 * check点：检查nfs重试错误
 **/
TEST_F(NfsMetaProducerTest2, IsRetryMount)
{
    int errorCode = -EINTR;
    EXPECT_EQ(m_ins->IsRetryMount(errorCode), true);
}

/*
 * 用例名称：IsRetryMount
 * 前置条件：无
 * check点：检查nfs重试错误
 **/
TEST_F(NfsMetaProducerTest2, CheckAndFillDirStat)
{
    MOCKER_CPP(&Module::NfsContextWrapper::NfsStat64)
            .stubs()
            .will(returnValue(10))
            .then(returnValue(0));
    MOCKER_CPP(&NfsMetaProducer::GetNfs4InodeForNetapp)
            .stubs()
            .will(returnValue(10));
    DirStat dirStat;
    dirStat.m_inode = 0;
    dirStat.m_path = ".";
    dirStat.m_filterFlag = 1;
    EXPECT_EQ(m_ins->CheckAndFillDirStat(dirStat), true);
    dirStat.m_inode = 1;
    dirStat.m_path = "/a/file.txt";
    EXPECT_EQ(m_ins->CheckAndFillDirStat(dirStat), true);     
}

/*
 * 用例名称：SendOpendirReqAsync
 * 前置条件：无
 * check点：发送异步请求
 **/
TEST_F(NfsMetaProducerTest2, SendOpendirReqAsync)
{
    MOCKER_CPP(&NfsMetaProducer::CheckAndFillDirStat)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&NfsContextWrapper::NfsGetError)
            .stubs()
            .will(returnValue(DUMMY_ERROR));
    DirStat dirStat;
    dirStat.m_inode = 1;
    dirStat.m_path = "/a/file.txt";
    dirStat.m_filterFlag = 1;
    EXPECT_EQ(m_ins->SendOpendirReqAsync(dirStat), true);
    int errorCode = -EINTR;
    EXPECT_EQ(m_ins->IsRetriableError(errorCode), true);
}

/*
 * 用例名称：HandleFailedDirectory
 * 前置条件：无
 * check点：处理失败情况
 **/
TEST_F(NfsMetaProducerTest2, HandleFailedDirectory)
{
    MOCKER_CPP(&NfsMetaProducer::IsRetriableError)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&NfsMetaProducer::CopyDirmetaToDirstat)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(0));
    MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&StatisticsMgr::SetCommStatsByType)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&NfsMetaProducer::SendOpendirResumeAsync)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&ScanQueue::BlockingPush)
            .stubs()
            .will(returnValue(true));
    OpendirResData resData;
    resData.m_cbData = new NFSSyncCbData();
    resData.m_cbData->m_isResumeCalled = 1;
    m_ins->HandleFailedDirectory(resData);
    m_ins->HandleFailedDirectory(resData);
    resData.m_cbData->m_isResumeCalled = 0;
    m_ins->HandleFailedDirectory(resData);
    EXPECT_NO_THROW(m_ins->HandleFailedDirectory(resData));
}

/*
 * 用例名称：PostScanDir
 * 前置条件：无
 * check点：后置扫描目录
 **/
TEST_F(NfsMetaProducerTest2, PostScanDir)
{
    MOCKER_CPP(&NfsMetaProducer::IsRetriableError)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&NfsMetaProducer::FreeNFSSyncCbData)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(0));
    MOCKER_CPP(&NfsMetaProducer::SendOpendirResumeAsync)
            .stubs()
            .will(ignoreReturnValue());
    OpendirResData resData;
    resData.m_cbData = new NFSSyncCbData();
    resData.m_cbData->m_isResumeCalled = 1;
    m_ins->PostScanDir(false, resData, 0);
    m_ins->PostScanDir(false, resData, 1);
    m_ins->PostScanDir(false, resData, 1);
    EXPECT_NO_THROW(m_ins->PostScanDir(true, resData, 1));
}

/*
 * 用例名称：SendOpendirResumeAsync
 * 前置条件：无
 * check点：发送异步信息
 **/
TEST_F(NfsMetaProducerTest2, SendOpendirResumeAsync_cbData_NULL)
{
    MOCKER_CPP(&StatisticsMgr::DecrCommStatsByType)
            .stubs()
            .will(ignoreReturnValue());
    OpendirResData opendirObj;
    EXPECT_EQ(m_ins->SendOpendirResumeAsync(opendirObj), SCANNER_STATUS::FAILED);
}

TEST_F(NfsMetaProducerTest2, SendOpendirResumeAsync_cbData_NOT_NULL)
{
    MOCKER_CPP(&StatisticsMgr::DecrCommStatsByType)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Module::NfsContextWrapper::NfsOpendirAsyncScanResume)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(0));
    char* str = "err";
    MOCKER_CPP(&Module::NfsContextWrapper::NfsGetError)
            .stubs()
            .will(returnValue(str));
    OpendirResData opendirObj;
    opendirObj.m_cbData = new NFSSyncCbData();
    opendirObj.m_opendirData = new opendir_cb_data();
    EXPECT_EQ(m_ins->SendOpendirResumeAsync(opendirObj), SCANNER_STATUS::FAILED);
    EXPECT_EQ(m_ins->SendOpendirResumeAsync(opendirObj), SCANNER_STATUS::SUCCESS);
}

/*
 * 用例名称：CopyDirmetaToDirstat
 * 前置条件：无
 * check点：发送异步信息
 **/
TEST_F(NfsMetaProducerTest2, CopyDirmetaToDirstat)
{
    DirStat dirStat {};
    DirMeta dmeta {};
    nfs_fh_scan fh;
    char value[] = "hello word!";
    fh.len = sizeof(value);
    memcpy_s(fh.value, sizeof(fh.value), value, fh.len);
    string basePath = "/test";
    EXPECT_EQ(m_ins->CopyDirmetaToDirstat(dirStat, dmeta, fh, basePath), SCANNER_STATUS::SUCCESS);
}

/*
 * 用例名称：ProcessDirectoryEntry
 * 前置条件：无
 * check点：判断是否跳过目录
 **/
TEST_F(NfsMetaProducerTest2, ProcessDirectoryEntry)
{
    MOCKER_CPP(&NfsMetaProducer::ConvertDirentToDirstat)
            .stubs()
            .will(returnValue(1))
            .then(returnValue(0));
    MOCKER_CPP(&ScanFilter::AcceptDir, bool(ScanFilter::*)(DirStat&, uint8_t))
            .stubs()
            .will(returnValue(false));
    DirectoryScan dirScanObj;
    uint8_t baseFilterFlag;
    struct nfsdirent *nfsdirent;
    string fullPath;
    EXPECT_EQ(m_ins->ProcessDirectoryEntry(dirScanObj, baseFilterFlag, nfsdirent, fullPath), SCANNER_STATUS::FAILED);
    EXPECT_EQ(m_ins->ProcessDirectoryEntry(dirScanObj, baseFilterFlag, nfsdirent, fullPath), SCANNER_STATUS::SUCCESS);
}

/*
 * 用例名称：IsDirSkip
 * 前置条件：无
 * check点：判断是否跳过目录
 **/
TEST_F(NfsMetaProducerTest2, IsDirSkip)
{
    struct nfsdirent nfsdirent1;
    // struct nfsdirent *nfsdirent = &nfsdirent1;
    nfsdirent* ptr = &nfsdirent1;
    ptr->name = "/file.txt";
    string fullPath = "";
    for (int i = 0; i < 5000; i++) {
        fullPath += "a";
    }
    EXPECT_EQ(m_ins->IsDirSkip(ptr, fullPath), true);

    struct ScanDateDetail dateDetails;
    EXPECT_NO_THROW(m_ins->FillDateDetails(dateDetails, ptr));
}
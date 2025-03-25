#include "CommonService.h"
#include "ScannerTime.h"
#include "ScanConsts.h"
#include "ScannerUtils.h"
#include "define/Types.h"
#include "MergeDcacheService.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "ControlFileUtils.h"
#include "ParserStructs.h"
#include "RfiCtrlParser.h"

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
using namespace FS_SCANNER;
using namespace Module;

namespace {
    constexpr auto MODULE = "COMMON_SERVICE";
    constexpr auto SCANNER_SUCCESS = Module::SUCCESS; // 成功标识
    constexpr auto SCANNER_FAILED = Module::FAILED; // 失败标识
}

class CommonServiceTest2 : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::shared_ptr<CommonService> m_ins;
};

void CommonServiceTest2::SetUp()
{

    int threadId = 111;
    ScanConfig config;
    config.copyId = "222";
    ScanInfo info;
    info.m_terminateFlag = false;
    info.m_finalDcacheFile = "file";
    shared_ptr<StatisticsMgr> statsMgr = make_shared<StatisticsMgr>();
    m_ins = make_shared<CommonService>(threadId, config, info, statsMgr);
}

void CommonServiceTest2::TearDown() 
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void CommonServiceTest2::SetUpTestCase()
{}

void CommonServiceTest2::TearDownTestCase()
{}

/*
 * 用例名称：Init
 * 前置条件：无
 * check点：初始化
 **/
// TEST_F(CommonServiceTest2, Init)
// {
//     MOCKER_CPP(&CommonService::GetMetaFileIndex)
//             .stubs()
//             .will(ignoreReturnValue()); 
//     MOCKER_CPP(&CommonService::UpdateMetaFileCountInFile)
//             .stubs()
//             .will(ignoreReturnValue()); 
//     MOCKER_CPP(&CommonService::GetXMetaFileIndex)
//             .stubs()
//             .will(ignoreReturnValue());
//     MOCKER_CPP(&CommonService::GetFcacheFileIndex)
//             .stubs()
//             .will(ignoreReturnValue());
//     MOCKER_CPP(&CommonService::InitMetaFile)
//             .stubs()
//             .will(returnValue(-1))
//             .then(returnValue(0));
//     MOCKER_CPP(&CommonService::InitXMetaFile)
//             .stubs()
//             .will(returnValue(-1))
//             .then(returnValue(0));
//     MOCKER_CPP(&CommonService::InitFcacheFile)
//             .stubs()
//             .will(returnValue(-1))
//             .then(returnValue(0)); 
//     MOCKER_CPP(&CommonService::InitDcacheFile)
//             .stubs()
//             .will(returnValue(-1))
//             .then(returnValue(0));
//     MOCKER_CPP(&CommonService::InitControlFile)
//             .stubs()
//             .will(returnValue(-1))
//             .then(returnValue(0)); 
//     MOCKER_CPP(&CommonService::InitHardLinkControlFile)
//             .stubs()
//             .will(returnValue(-1))
//             .then(returnValue(0)); 
//     MOCKER_CPP(&CommonService::InitMtimeFile)
//             .stubs()
//             .will(returnValue(-1))
//             .then(returnValue(0)); 
//     MOCKER_CPP(&CommonService::InitDeletCtrlFile)
//             .stubs()
//             .will(returnValue(-1))
//             .then(returnValue(0));  
//     m_ins->m_metaFileCount = 2;
//     EXPECT_EQ(m_ins->Init(), SCANNER_FAILED);
//     // EXPECT_EQ(m_ins->Init(), SCANNER_FAILED);        
// }


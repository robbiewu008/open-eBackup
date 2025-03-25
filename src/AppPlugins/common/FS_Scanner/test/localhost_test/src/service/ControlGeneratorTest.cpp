#include "ControlGenerator.h"
#include <dirent.h>
#include <sys/stat.h>
#include "define/Types.h"
#include "ScannerUtils.h"
#include "MergeDcacheService.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

using namespace std;

class ControlGeneratorTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<ControlGenerator> m_ins {nullptr};
};

void ControlGeneratorTest::SetUp() {
    ScanJobType type = ScanJobType::INDEX;
    std::shared_ptr<BufferQueue<DirectoryScan>> buffer;
    std::shared_ptr<StatisticsMgr> statsMgr;
    std::shared_ptr<FSScannerCheckPoint> chkPntMgr;
    std::shared_ptr<ScanFilter> scanFilter;
    m_ins = std::make_unique<ControlGenerator>(type, buffer, statsMgr, chkPntMgr, scanFilter);
}
void ControlGeneratorTest::TearDown() {}
void ControlGeneratorTest::SetUpTestCase() {}
void ControlGeneratorTest::TearDownTestCase() {}

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

static bool Stub_InitCommonService_False(ScanConfig &config, ScanInfo &info)
{
    return false;
}
static void Stub_ReadCheckPointHardLinkFiles(shared_ptr<CommonService> &servicePtr)
{}
/*
 * 用例名称：GenerateMeta
 * 前置条件：无
 * check点：生成meta
 **/
TEST_F(ControlGeneratorTest, GenerateMeta) 
{
    ScanConfig config;
    ScanInfo info;
    m_ins->m_state = SCANNER_STATUS::ABORTED;
    SCANNER_STATUS ret = m_ins->GenerateMeta(config, info);
    EXPECT_EQ(ret, SCANNER_STATUS::FAILED);

    m_ins->m_state = SCANNER_STATUS::INIT;
    ret = m_ins->GenerateMeta(config, info);
    EXPECT_EQ(ret, SCANNER_STATUS::FAILED);

    ScanJobType type = ScanJobType::FULL;
    std::shared_ptr<BufferQueue<DirectoryScan>> buffer;
    std::shared_ptr<StatisticsMgr> statsMgr;
    std::shared_ptr<FSScannerCheckPoint> chkPntMgr;
    std::shared_ptr<ScanFilter> scanFilter;
    std::unique_ptr<ControlGenerator> m_ins2 = std::make_unique<ControlGenerator>(type, buffer, statsMgr, chkPntMgr, scanFilter);
    stub.set(ADDR(ControlGenerator, InitCommonService), Stub_InitCommonService_False); // 打桩失效
    // ret = m_ins2->GenerateMeta(config, info); // 待解决
    // EXPECT_EQ(ret, SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE);
    stub.reset(ADDR(ControlGenerator, InitCommonService));
}

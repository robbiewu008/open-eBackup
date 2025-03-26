#include "StatisticsMgr.h"
#include "PosixStatistics.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

using namespace std;

class StatisticsMgrTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<StatisticsMgr> m_ins {nullptr};
};

void StatisticsMgrTest::SetUp() {
    m_ins = std::make_unique<StatisticsMgr>();
}
void StatisticsMgrTest::TearDown() {}
void StatisticsMgrTest::SetUpTestCase() {}
void StatisticsMgrTest::TearDownTestCase() {}

TEST_F(StatisticsMgrTest, SetCommStatsByType) 
{
    CommStatsType statsType = CommStatsType::TOTAL_DIRS_TO_BACKUP;
    uint64_t newValue = 0;
    m_ins->SetCommStatsByType(statsType, newValue);
    EXPECT_EQ(m_ins->m_commonStatistics[0], 0);
}

TEST_F(StatisticsMgrTest, GetAllCommonStats) 
{
    std::atomic_uint64_t* ret = m_ins->GetAllCommonStats();
    EXPECT_NE(ret, nullptr);
}

TEST_F(StatisticsMgrTest, IncrCommonStatsByRange) 
{
    uint64_t commonStats[3] = {0, 1, 2};
    CommStatsType startType = CommStatsType::TOTAL_DIRS_TO_BACKUP;
    CommStatsType endType = CommStatsType::TOTAL_DIRS_DELETED;
    m_ins->IncrCommonStatsByRange(commonStats, startType, endType);
    EXPECT_EQ(m_ins->m_commonStatistics[0], 0);
}

TEST_F(StatisticsMgrTest, IncrAllCommStats) 
{
    uint64_t commonStats[3] = {0, 1, 2};
    vector<CommStatsType> list = {CommStatsType::TOTAL_DIRS_TO_BACKUP,
     CommStatsType::TOTAL_DIRS_TO_BACKUP};
    m_ins->IncrAllCommStats(commonStats);
    m_ins->IncrCommStatsByIdxList(commonStats, list);
    EXPECT_EQ(m_ins->m_commonStatistics[0], 0);
}

TEST_F(StatisticsMgrTest, IncrementProtoStatsByType) 
{
    int protoStatsType = 0;
    uint64_t incVal = 0;
    m_ins->SetProtoStatsFactoryObject(3);
    m_ins->IncrementProtoStatsByType(protoStatsType, incVal);
    m_ins->SetProtocolStatsByType(protoStatsType, incVal);
    uint64_t ret = m_ins->GetProtoStatsByType(protoStatsType);
    EXPECT_EQ(ret, 0);
}

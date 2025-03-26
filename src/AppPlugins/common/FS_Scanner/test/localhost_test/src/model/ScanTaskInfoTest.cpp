#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"
#include "ScanTaskInfo.h"

using namespace std;

class ScanTaskInfoTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void ScanTaskInfoTest::SetUp()
{}

void ScanTaskInfoTest::TearDown()
{}

void ScanTaskInfoTest::SetUpTestCase()
{}

void ScanTaskInfoTest::TearDownTestCase()
{}

/*
 * 用例名称：获取ScanTaskInfo实例
 * 前置条件：无
 * check点：无异常
 **/
TEST_F(ScanTaskInfoTest, GetInstance)
{
    EXPECT_NO_THROW(ScanTaskInfo::GetInstance());
}

/*
 * 用例名称：插入job信息
 * 前置条件：无
 * check点：是否正常插入
 **/
TEST_F(ScanTaskInfoTest, Insert)
{
    string jobId = "jobId123";
    string metaPathForCtrlFiles = "/dir/ctrol";
    ScanTaskInfo::GetInstance().Insert(jobId, metaPathForCtrlFiles);
    string val = ScanTaskInfo::GetInstance().m_confInfoMap[jobId];
    EXPECT_EQ(val, metaPathForCtrlFiles);
}

/*
 * 用例名称：插入jobId对应的meta path
 * 前置条件：无
 * check点：获取的meta path是否正常
 **/
TEST_F(ScanTaskInfoTest, Query)
{
    string jobId = "jobId1234";
    string metaPathForCtrlFiles = "/dir/ctrol/1234";
    ScanTaskInfo::GetInstance().Insert(jobId, metaPathForCtrlFiles);
    string path = ScanTaskInfo::GetInstance().Query(jobId);
    EXPECT_EQ(path, metaPathForCtrlFiles);
}

/*
 * 用例名称：删除jobId信息
 * 前置条件：无
 * check点：jobId信息是否删除
 **/
TEST_F(ScanTaskInfoTest, Delete)
{
    string jobId = "jobId12345";
    string metaPathForCtrlFiles = "/dir/ctrol/12345";
    ScanTaskInfo::GetInstance().Insert(jobId, metaPathForCtrlFiles);
    uint8_t count = ScanTaskInfo::GetInstance().m_confInfoMap.count(jobId);
    EXPECT_EQ(count, 1);
    ScanTaskInfo::GetInstance().Delete(jobId);
    count = ScanTaskInfo::GetInstance().m_confInfoMap.count(jobId);
    EXPECT_EQ(count, 0);
}
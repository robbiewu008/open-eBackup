#include "PosixMetaProducer.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/acl.h>
#include "log/Log.h"
#include "ScannerTime.h"
#include "ScannerUtils.h"



#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"

using namespace std;
using namespace Module;

class PosixMetaProducerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::shared_ptr<PosixMetaProducer> m_ins;
};

void PosixMetaProducerTest::SetUp()
{
    m_ins = std::make_shared<PosixMetaProducer>();
}

void PosixMetaProducerTest::TearDown()
{}

void PosixMetaProducerTest::SetUpTestCase()
{}

void PosixMetaProducerTest::TearDownTestCase()
{}

/*
 * 用例名称：SkipDirEntry
 * 前置条件：无
 * check点：跳过目录entry
 **/
TEST_F(PosixMetaProducerTest, SkipDirEntry)
{
    std::string name;
    std::string fullPath;
    for (int i = 0; i < 5000; i++) {
        fullPath += "a";
    }
    EXPECT_EQ(m_ins->SkipDirEntry(name, fullPath, ""), true); 

    ScanConfig config;
    std::set<std::string> set {"aaa"};
    config.crossVolumeSkipSet = set;
    m_ins->m_config = config;
    fullPath = "";
    EXPECT_EQ(m_ins->SkipDirEntry(name, fullPath, ""), false);         
}

/*
 * 用例名称：SkipDirEntry
 * 前置条件：无
 * check点：跳过目录entry
 **/
TEST_F(PosixMetaProducerTest, PartiallyOrCompletelyPushDirectoryToWriteQueue)
{
    // MOCKER_CPP(&ScanFilter::DiscardDirectory, bool(ScanFilter::*)(int, uint8_t)const)
    //         .stubs()
    //         .will(returnValue(true));
    // MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
    //         .stubs()
    //         .will(ignoreReturnValue());        
    // DirectoryScan node;
    // DirMetaWrapper dirWrapper;
    // uint8_t baseFilterFlag;
    // string prefix;
    // m_ins->PartiallyOrCompletelyPushDirectoryToWriteQueue(false, node, dirWrapper, baseFilterFlag, prefix);         
}
#include "ObjectInode.h"
#include "log/Log.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"


class ObjectInodeTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<ObjectInode> m_obj {nullptr};
};

void ObjectInodeTest::SetUp() {
    m_obj = std::make_unique<ObjectInode>();
}
void ObjectInodeTest::TearDown() {}
void ObjectInodeTest::SetUpTestCase() {}
void ObjectInodeTest::TearDownTestCase() {}


/*
 * GetInodeValue
 * 前置条件：无
 * check点：获取对象key值的InodeValue
 **/
TEST_F(ObjectInodeTest, GetInodeValueTest)
{
    const char* testStr1 = "Thisisateststringwith1024characters.";
    uint64_t inodeValue = 0xe5b263aecfed0000;

    EXPECT_EQ(inodeValue, m_obj->GetInodeValue(testStr1, 36));
}

/*
 * SaveConflictRecord
 * 前置条件：无
 * check点：保存hash冲突列表到文件
 **/
TEST_F(ObjectInodeTest, SaveConflictRecordTest)
{
    const char* testStr1 = "Thisisateststringwith1024characters.";
    const std::string fileName = "objectconflictrecord.json";
    uint64_t inodeValue = 0xe5b263aecfed0000;

    EXPECT_EQ(inodeValue, m_obj->GetInodeValue(testStr1, 36));
    EXPECT_EQ(inodeValue + 1, m_obj->GetInodeValue(testStr1, 36));
    EXPECT_EQ(true, m_obj->SaveConflictRecord(fileName));
}
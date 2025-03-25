#include "ControlFileUtils.h"
#include "log/Log.h"
#include "ScanFilterUtil.h"
#include "ParserStructs.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

using namespace std;
using namespace Module;

class ControlFileUtilsTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    std::unique_ptr<ControlFileUtils> m_ins {nullptr};
};

void ControlFileUtilsTest::SetUp() {
    m_ins = std::make_unique<ControlFileUtils>();
}
void ControlFileUtilsTest::TearDown() {}
void ControlFileUtilsTest::SetUpTestCase() {}
void ControlFileUtilsTest::TearDownTestCase() {}

static void Stub_Func_Void()
{}

static int Stub_Func_0()
{
    return 0;
}
static int Stub_Func_1()
{
    return 1;
}

static bool Stub_Func_True()
{
    return true;
}

static bool Stub_Func_False()
{
    return false;
}

static CTRL_FILE_RETCODE Stub_Func_SUCCESS()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}
static CTRL_FILE_RETCODE Stub_Func_FAILED()
{
    return CTRL_FILE_RETCODE::FAILED;
}

/*
 * 用例名称：CreateMetaFileObj
 * 前置条件：无
 * check点：创建metafile对象
 **/
TEST_F(ControlFileUtilsTest, CreateMetaFileObj) 
{
    stub.set(ADDR(MetaParser, Open), Stub_Func_SUCCESS);
    string filename = "/a/b/file.txt";
    std::shared_ptr<MetaParser> ret = m_ins->CreateMetaFileObj(filename);
    EXPECT_NE(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));

    stub.set(ADDR(MetaParser, Open), Stub_Func_FAILED);
    ret = m_ins->CreateMetaFileObj(filename);
    EXPECT_EQ(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));
}

TEST_F(ControlFileUtilsTest, CreateMetaFileObj_2) 
{
    stub.set(ADDR(MetaParser, Open), Stub_Func_0);
    string filename = "file.txt";
    Module::CTRL_FILE_OPEN_MODE mode = Module::CTRL_FILE_OPEN_MODE::READ;
    ScanConfig config;
    std::shared_ptr<MetaParser> ret = m_ins->CreateMetaFileObj(filename, mode, config);
    EXPECT_NE(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));
}

/*
 * 用例名称：CreateXMetaFileObj
 * 前置条件：无
 * check点：创建xmetafile对象
 **/
TEST_F(ControlFileUtilsTest, CreateXMetaFileObj) 
{
    stub.set(ADDR(MetaParser, Open), Stub_Func_1);
    string filename = "/a/b/file.txt";
    Module::CTRL_FILE_OPEN_MODE mode = Module::CTRL_FILE_OPEN_MODE::READ;
    ScanConfig config;
    std::shared_ptr<XMetaParser> ret = m_ins->CreateXMetaFileObj(filename, mode, config);
    EXPECT_EQ(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));
}

TEST_F(ControlFileUtilsTest, CreateXMetaFileObj_2) 
{
    stub.set(ADDR(MetaParser, Open), Stub_Func_0);
    string filename = "/a/b/file.txt";
    std::shared_ptr<XMetaParser> ret = m_ins->CreateXMetaFileObj(filename);
    EXPECT_NE(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));

    stub.set(ADDR(MetaParser, Open), Stub_Func_1);
    ret = m_ins->CreateXMetaFileObj(filename);
    EXPECT_EQ(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));
}

/*
 * 用例名称：CreateDcacheObj
 * 前置条件：无
 * check点：创建dcache对象
 **/
TEST_F(ControlFileUtilsTest, CreateDcacheObj) 
{
    stub.set(ADDR(MetaParser, Open), Stub_Func_1);
    std::string fname = "xxx";
    ScanConfig config;
    CTRL_FILE_OPEN_MODE mode = Module::CTRL_FILE_OPEN_MODE::READ;
    std::shared_ptr<DirCacheParser>  ret = m_ins->CreateDcacheObj(fname, mode, config);
    EXPECT_EQ(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));
}

TEST_F(ControlFileUtilsTest, CreateDcacheObj_2) 
{
    stub.set(ADDR(MetaParser, Open), Stub_Func_0);
    std::string fname = "xxx";
    std::shared_ptr<DirCacheParser>  ret = m_ins->CreateDcacheObj(fname);
    EXPECT_NE(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));

    stub.set(ADDR(MetaParser, Open), Stub_Func_1);
    ret = m_ins->CreateDcacheObj(fname);
    EXPECT_EQ(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));
}

/*
 * 用例名称：CreateFileCacheObj
 * 前置条件：无
 * check点：创建filecache对象
 **/
TEST_F(ControlFileUtilsTest, CreateFileCacheObj) 
{
    stub.set(ADDR(MetaParser, Open), Stub_Func_0);
    std::string fname = "xxx";
    std::shared_ptr<FileCacheParser>  ret = m_ins->CreateFileCacheObj(fname);
    EXPECT_NE(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));

    stub.set(ADDR(MetaParser, Open), Stub_Func_1);
    ret = m_ins->CreateFileCacheObj(fname);
    EXPECT_EQ(ret, nullptr);
    stub.reset(ADDR(MetaParser, Open));
}

/*
 * 用例名称：GetXMetaFileName
 * 前置条件：无
 * check点：获取xmetafile文件名字
 **/
TEST_F(ControlFileUtilsTest, GetXMetaFileName) 
{
    string ret = m_ins->GetXMetaFileName(1);
    EXPECT_EQ(ret, "xmeta_file_1");
}

/*
 * 用例名称：GetFileModifiedFlag
 * 前置条件：无
 * check点：获取文件修改标志
 **/
TEST_F(ControlFileUtilsTest, GetFileModifiedFlag) 
{
    FileMetaWrapper fmWrapperOne;
    FileMetaWrapper fmWrapperTwo;
    string ret = m_ins->GetFileModifiedFlag(fmWrapperOne, fmWrapperTwo);
    EXPECT_NE(ret, "");

    stub.set(ADDR(ControlFileUtils, IsFileMetaModified), Stub_Func_True);
    ret = m_ins->GetFileModifiedFlag(fmWrapperOne, fmWrapperTwo);
    EXPECT_EQ(ret, "mm");
    stub.reset(ADDR(ControlFileUtils, IsFileMetaModified));
}

/*
 * 用例名称：IsFileMetaModified
 * 前置条件：无
 * check点：判断文件meta是否被修改
 **/
// TEST_F(ControlFileUtilsTest, IsFileMetaModified) 
// {
//     FileMetaWrapper fmWrapperOne;
//     FileMetaWrapper fmWrapperTwo;
//     bool ret = m_ins->IsFileMetaModified(fmWrapperOne, fmWrapperTwo);
//     EXPECT_EQ(ret, false);

//     fmWrapperOne.m_meta.m_uid = 0;
//     fmWrapperTwo.m_meta.m_uid = 1;
//     ret = m_ins->IsFileMetaModified(fmWrapperOne, fmWrapperTwo);
//     EXPECT_EQ(ret, true);
// }
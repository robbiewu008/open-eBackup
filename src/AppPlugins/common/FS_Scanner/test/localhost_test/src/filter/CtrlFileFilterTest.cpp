#include "gtest/gtest.h"
#include "CtrlFileFilter.h"

class CtrlFileFilterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void CtrlFileFilterTest::SetUp()
{}

void CtrlFileFilterTest::TearDown()
{}

void CtrlFileFilterTest::SetUpTestCase()
{}

void CtrlFileFilterTest::TearDownTestCase()
{}

/*
 * 用例名称：没有开启过滤
 * 前置条件：无
 * check点：接受任何路径
 **/
TEST_F(CtrlFileFilterTest, NoFilters) {
    CtrlFileFilter ctrlFileFilter({}, {});

    EXPECT_FALSE(ctrlFileFilter.Enabled());

    EXPECT_TRUE(ctrlFileFilter.Accept("/"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/home"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/user/bin"));
}

/*
 * 用例名称：有包含规则
 * 前置条件：无
 * check点：对于文件，接受其路径及其父目录路径。对于目录，接受其路径，父目录路径，及子目录理解
 **/
TEST_F(CtrlFileFilterTest, hasFilters) {
    CtrlFileFilter ctrlFileFilter({
        "/home/用户/下载",
        "/root/.config/",
        "/root/.config/", // dulplicated
        "/测试1/测试2/测试3"
    },{
        "/etc/apache/apache.conf",
        "/etc/apache/apache.conf", // dulplicated
        "/目录1/文件1/" // test illegal file path
    });

    EXPECT_TRUE(ctrlFileFilter.Enabled());

    EXPECT_TRUE(ctrlFileFilter.Accept("/"));
    // exception cases
    EXPECT_TRUE(ctrlFileFilter.Accept(""));
    EXPECT_TRUE(ctrlFileFilter.Accept("."));
    EXPECT_TRUE(ctrlFileFilter.Accept("//"));
    EXPECT_TRUE(ctrlFileFilter.Accept("./"));

    EXPECT_TRUE(ctrlFileFilter.Accept("/目录1"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/目录1/文件1"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/目录1"));
    // exception cases
    EXPECT_TRUE(ctrlFileFilter.Accept("目录1"));
    EXPECT_TRUE(ctrlFileFilter.Accept("./目录1"));
    EXPECT_TRUE(ctrlFileFilter.Accept("//目录1"));
    EXPECT_FALSE(ctrlFileFilter.Accept("//目录2"));

    EXPECT_TRUE(ctrlFileFilter.Accept("/home"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/root"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/etc"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/usr"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/home/用户"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/home/用户1"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/home/user"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/home/用户/下载"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/home/用户/下载/folder1"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/home/用户/下载2"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/root/.config"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/root/.vim"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/root/.vim/vimplugin"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/etc/apache"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/etc/nginx"));
    EXPECT_TRUE(ctrlFileFilter.Accept("/etc/apache/apache.conf"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/etc/apache/LICENCE.txt"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/etc/apache/conf.d"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/etc/apache/conf.d/php-fpm.conf"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/etc/nginx/nginx.conf"));
    EXPECT_FALSE(ctrlFileFilter.Accept("/etc/nginx/conf.d/php-fpm.conf"));
}

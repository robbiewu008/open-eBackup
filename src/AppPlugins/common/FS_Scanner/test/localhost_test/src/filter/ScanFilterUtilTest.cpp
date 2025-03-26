#include "gtest/gtest.h"
#include "ScanFilterUtil.h"

namespace {
    const std::string POSIX_SEPARATOR = "/";
    const std::string WIN32_SEPARATOR = "\\";
}

class ScannerFilterUtilTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void ScannerFilterUtilTest::SetUp()
{}

void ScannerFilterUtilTest::TearDown()
{}

void ScannerFilterUtilTest::SetUpTestCase()
{}

void ScannerFilterUtilTest::TearDownTestCase()
{}


/*
 * 用例名称：获取全部父级目录，目录有多个层级
 * 前置条件：无
 * check点：正确返回全部父级目录
 **/
TEST_F(ScannerFilterUtilTest, ParentDirsPosix) {
    std::string path = "/home/xuranus/Download/movie.avi";
    std::vector<std::string> parentDirs = ScanFilterUtil::ParentDirs(path, POSIX_SEPARATOR);
    std::vector<std::string> expectResult {"/", "/home", "/home/xuranus", "/home/xuranus/Download"};

    sort(parentDirs.begin(), parentDirs.end());
    sort(expectResult.begin(), expectResult.end());
    EXPECT_EQ(parentDirs, expectResult);
}

/*
 * 用例名称：获取全部父级目录，目录有多个层级
 * 前置条件：无
 * check点：正确返回全部父级目录
 **/
TEST_F(ScannerFilterUtilTest, ParentDirsWin32) {
    std::string path = R"(C:\User\xuranus\Desktop\movie.avi)";
    std::vector<std::string> parentDirs = ScanFilterUtil::ParentDirs(path, WIN32_SEPARATOR);
    std::vector<std::string> expectResult { R"(C:)", R"(C:\User)", R"(C:\User\xuranus)", R"(C:\User\xuranus\Desktop)" };

    sort(parentDirs.begin(), parentDirs.end());
    sort(expectResult.begin(), expectResult.end());
    EXPECT_EQ(parentDirs, expectResult);
}

/*
 * 用例名称：获取全部父级目录，目录只有一个层级
 * 前置条件：无
 * check点：返回根目录
 **/
TEST_F(ScannerFilterUtilTest, ParentDirsSingleLevelPosix) {
    std::string path = "/LICENCE.txt";
    std::vector<std::string> parentDirs = ScanFilterUtil::ParentDirs(path, POSIX_SEPARATOR);
    std::vector<std::string> expectResult {"/"};

    sort(parentDirs.begin(), parentDirs.end());
    sort(expectResult.begin(), expectResult.end());
    EXPECT_EQ(parentDirs, expectResult);
}

/*
 * 用例名称：获取全部父级目录，目录只有一个层级
 * 前置条件：无
 * check点：返回根目录
 **/
TEST_F(ScannerFilterUtilTest, ParentDirsSingleLevelWin32) {
    std::string path = R"(C:\LICENCE.txt)";
    std::vector<std::string> parentDirs = ScanFilterUtil::ParentDirs(path, WIN32_SEPARATOR);
    std::vector<std::string> expectResult { R"(C:)" };

    sort(parentDirs.begin(), parentDirs.end());
    sort(expectResult.begin(), expectResult.end());
    EXPECT_EQ(parentDirs, expectResult);
}

/*
 * 用例名称：获取全部父级目录，目录为根
 * 前置条件：无
 * check点：返回根目录
 **/
TEST_F(ScannerFilterUtilTest, ParentDirsPosixRoot) {
    std::string path = "/";
    std::vector<std::string> parentDirs = ScanFilterUtil::ParentDirs(path, POSIX_SEPARATOR);
    std::vector<std::string> expectResult {"/"};

    sort(parentDirs.begin(), parentDirs.end());
    sort(expectResult.begin(), expectResult.end());
    EXPECT_EQ(parentDirs, expectResult);
}

/*
 * 用例名称：获取全部父级目录，目录为根
 * 前置条件：无
 * check点：返回根目录
 **/
TEST_F(ScannerFilterUtilTest, ParentDirsWin32Root) {
    std::string path = R"(C:\)";
    std::vector<std::string> parentDirs = ScanFilterUtil::ParentDirs(path, WIN32_SEPARATOR);
    std::vector<std::string> expectResult { R"(C:)" };

    sort(parentDirs.begin(), parentDirs.end());
    sort(expectResult.begin(), expectResult.end());
    EXPECT_EQ(parentDirs, expectResult);
}

/*
 * 用例名称：获取全部父级目录，目录有多个层级，包含unicode
 * 前置条件：无
 * check点：正确返回全部父级目录
 **/
TEST_F(ScannerFilterUtilTest, ParentDirsPosixUnicode) {
    std::string path = "/用户/用户A/下载/电影.avi";
    std::vector<std::string> superDirs = ScanFilterUtil::ParentDirs(path, POSIX_SEPARATOR);
    std::vector<std::string> expectResult {"/", "/用户", "/用户/用户A", "/用户/用户A/下载"};

    sort(superDirs.begin(), superDirs.end());
    sort(expectResult.begin(), expectResult.end());
    EXPECT_EQ(superDirs, expectResult);
}

/*
 * 用例名称：获取文件/目录上层目录
 * 前置条件：无
 * check点：正确返回全部父级目录
 **/
TEST_F(ScannerFilterUtilTest, DirPathPosix) {
    EXPECT_EQ(ScanFilterUtil::DirPath("/home/xuranus/Download", POSIX_SEPARATOR), "/home/xuranus");
    EXPECT_EQ(ScanFilterUtil::DirPath("/home/xuranus/main.cpp", POSIX_SEPARATOR), "/home/xuranus");
    EXPECT_EQ(ScanFilterUtil::DirPath("/main.cpp", POSIX_SEPARATOR), "/");
    EXPECT_EQ(ScanFilterUtil::DirPath("/home/", POSIX_SEPARATOR), "/home");
    EXPECT_EQ(ScanFilterUtil::DirPath("/home/用户/下载", POSIX_SEPARATOR), "/home/用户");
}

/* 用例名称：获取文件/目录上层目录
 * 前置条件：无
 * check点：正确返回全部父级目录
 **/
TEST_F(ScannerFilterUtilTest, DirPathWin32) {
    EXPECT_EQ(ScanFilterUtil::DirPath(R"(C:\User\xuranus\Download)", WIN32_SEPARATOR), R"(C:\User\xuranus)");
    EXPECT_EQ(ScanFilterUtil::DirPath(R"(C:\User\xuranus\main.cpp)", WIN32_SEPARATOR), R"(C:\User\xuranus)");
    EXPECT_EQ(ScanFilterUtil::DirPath(R"(C:\main.cpp)", WIN32_SEPARATOR), R"(C:)");
    EXPECT_EQ(ScanFilterUtil::DirPath(R"(C:\User\)", WIN32_SEPARATOR), R"(C:\User)");
    EXPECT_EQ(ScanFilterUtil::DirPath(R"(C:\用户\下载)", WIN32_SEPARATOR), R"(C:\用户)");
    EXPECT_EQ(ScanFilterUtil::DirPath(R"(C:)", WIN32_SEPARATOR), "C:");
}


/*
 * 用例名称：判断文件(集)能否包含某个目录，可以包含通配
 * 前置条件：无
 * check点：正确返回全部父级目录
 **/
TEST_F(ScannerFilterUtilTest, CanCoverPosix) {
    EXPECT_TRUE(ScanFilterUtil::CanCover("/", "/home/user", POSIX_SEPARATOR));
    EXPECT_TRUE(ScanFilterUtil::CanCover("/", "/LICENCE.txt", POSIX_SEPARATOR));
    EXPECT_TRUE(ScanFilterUtil::CanCover("/home", "/home/user", POSIX_SEPARATOR));
    EXPECT_TRUE(ScanFilterUtil::CanCover("/home", "/home/user/work*", POSIX_SEPARATOR));
    EXPECT_TRUE(ScanFilterUtil::CanCover("/home", "/home/*/Document", POSIX_SEPARATOR));
    EXPECT_TRUE(ScanFilterUtil::CanCover("/home", "/home/user/Download", POSIX_SEPARATOR));
    EXPECT_TRUE(ScanFilterUtil::CanCover("/home", "/home", POSIX_SEPARATOR));
    EXPECT_FALSE(ScanFilterUtil::CanCover("/home", "/etc", POSIX_SEPARATOR));
    EXPECT_FALSE(ScanFilterUtil::CanCover("/home", "/", POSIX_SEPARATOR));

    EXPECT_TRUE(ScanFilterUtil::CanCover(
        std::vector<std::string>{"/home", "/etc"}, 
        "/home/user", POSIX_SEPARATOR));

    EXPECT_TRUE(ScanFilterUtil::CanCover(
        std::vector<std::string>{"/home", "/etc"}, 
        "/home/*.png", POSIX_SEPARATOR));

    EXPECT_TRUE(ScanFilterUtil::CanCover(
        std::vector<std::string>{"/home", "/etc"}, 
        "/etc", POSIX_SEPARATOR));
    
    EXPECT_FALSE(ScanFilterUtil::CanCover(
        std::vector<std::string>{"/home", "/etc"}, 
        "/root/test", POSIX_SEPARATOR));

    EXPECT_FALSE(ScanFilterUtil::CanCover(
        std::vector<std::string>{"/home", "/etc"}, 
        "/home*", POSIX_SEPARATOR));
}

/*
 * 用例名称：判断文件(集)能否包含某个目录，可以包含通配
 * 前置条件：无
 * check点：正确返回全部父级目录
 **/
TEST_F(ScannerFilterUtilTest, CanCoverWin32) {
    EXPECT_TRUE(ScanFilterUtil::CanCover(R"(C:\)", R"(C:\User\Desktop)", WIN32_SEPARATOR));
    EXPECT_TRUE(ScanFilterUtil::CanCover(R"(C:\)", R"(C:\LICENCE.txt)", WIN32_SEPARATOR));
    EXPECT_TRUE(ScanFilterUtil::CanCover(R"(C:\Windows)", R"(C:\Windows\System32)", WIN32_SEPARATOR));
    EXPECT_TRUE(ScanFilterUtil::CanCover(R"(C:\Windows)", R"(C:\Windows\System32\*)", WIN32_SEPARATOR));
    EXPECT_TRUE(ScanFilterUtil::CanCover(R"(C:\)", R"(C:)", WIN32_SEPARATOR));
    EXPECT_FALSE(ScanFilterUtil::CanCover(R"(C:\Windows)", R"(C:)", WIN32_SEPARATOR));

    EXPECT_TRUE(ScanFilterUtil::CanCover(
        std::vector<std::string>{R"(C:\Windows\System32)", R"(D:\Documents)"}, 
        R"(C:\Windows\System32\library)", WIN32_SEPARATOR));
}

/*
 * 用例名称：目录聚合，输入为单个根目录
 * 前置条件：无
 * check点：返回根目录
 **/
TEST_F(ScannerFilterUtilTest, SingleRootDirPathUnionPosix) {
    std::vector<std::string> baseEnqueueList {"/"};

    baseEnqueueList = ScanFilterUtil::DirPathUnion(baseEnqueueList, POSIX_SEPARATOR);
    std::vector<std::string> expectBaseEnqueueList {"/"};

    sort(expectBaseEnqueueList.begin(), expectBaseEnqueueList.end());
    sort(baseEnqueueList.begin(), baseEnqueueList.end());
    EXPECT_EQ(baseEnqueueList, expectBaseEnqueueList);
}

/*
 * 用例名称：目录聚合，输入为单个根目录
 * 前置条件：无
 * check点：返回根目录
 **/
TEST_F(ScannerFilterUtilTest, SingleRootDirPathUnionWin32) {
    std::vector<std::string> baseEnqueueList { R"(C:)" };

    baseEnqueueList = ScanFilterUtil::DirPathUnion(baseEnqueueList, WIN32_SEPARATOR);
    std::vector<std::string> expectBaseEnqueueList { R"(C:)" };

    sort(expectBaseEnqueueList.begin(), expectBaseEnqueueList.end());
    sort(baseEnqueueList.begin(), baseEnqueueList.end());
    EXPECT_EQ(baseEnqueueList, expectBaseEnqueueList);
}

/*
 * 用例名称：目录聚合，输入为单个非根目录
 * 前置条件：无
 * check点：返回非根目录
 **/
TEST_F(ScannerFilterUtilTest, SingleNonRootDirPathUnionPosix) {
    std::vector<std::string> baseEnqueueList {"/home"};

    baseEnqueueList = ScanFilterUtil::DirPathUnion(baseEnqueueList, POSIX_SEPARATOR);
    std::vector<std::string> expectBaseEnqueueList {"/home"};

    sort(expectBaseEnqueueList.begin(), expectBaseEnqueueList.end());
    sort(baseEnqueueList.begin(), baseEnqueueList.end());
    EXPECT_EQ(baseEnqueueList, expectBaseEnqueueList);
}

/*
 * 用例名称：目录聚合，输入为单个非根目录
 * 前置条件：无
 * check点：返回非根目录
 **/
TEST_F(ScannerFilterUtilTest, SingleNonRootDirPathWin32) {
    std::vector<std::string> baseEnqueueList { R"(C:\Users)"};

    baseEnqueueList = ScanFilterUtil::DirPathUnion(baseEnqueueList, WIN32_SEPARATOR);
    std::vector<std::string> expectBaseEnqueueList { R"(C:\Users)" };

    sort(expectBaseEnqueueList.begin(), expectBaseEnqueueList.end());
    sort(baseEnqueueList.begin(), baseEnqueueList.end());
    EXPECT_EQ(baseEnqueueList, expectBaseEnqueueList);
}

/*
 * 用例名称：目录聚合，输入为多个目录，目录中可能包含重复目录
 * 前置条件：无
 * check点：返回正确的聚合结果
 **/
TEST_F(ScannerFilterUtilTest, MultiDirPathUnionPosix) {
    std::vector<std::string> baseEnqueueList {
        "/home/thankod/Download/movie",
        "/home/thankod/Download",
        "/home/thankod",
        "/home/thankod/文档"
        "/home/thankod", // dulplicated
        "/etc",
        "/opt",
        "/etc/nginx"
    };

    baseEnqueueList = ScanFilterUtil::DirPathUnion(baseEnqueueList, POSIX_SEPARATOR);
    std::vector<std::string> expectBaseEnqueueList {"/etc", "/opt", "/home/thankod"};

    sort(expectBaseEnqueueList.begin(), expectBaseEnqueueList.end());
    sort(baseEnqueueList.begin(), baseEnqueueList.end());
    EXPECT_EQ(baseEnqueueList, expectBaseEnqueueList);
}

/*
 * 用例名称：目录聚合，输入为多个目录，目录中可能包含重复目录
 * 前置条件：无
 * check点：返回正确的聚合结果
 **/
TEST_F(ScannerFilterUtilTest, MultiDirPathUnionWin32) {
    std::vector<std::string> baseEnqueueList {
        R"(C:\Users\thankod\Download\movie)",
        R"(C:\Users\thankod\Download)",
        R"(C:\Users\thankod)",
        R"(C:\Users\thankod\文档)"
        R"(C:\Users\thankod)", // dulplicated
        R"(D:\Documents)",
        R"(D:\Downloads)"
    };

    baseEnqueueList = ScanFilterUtil::DirPathUnion(baseEnqueueList, WIN32_SEPARATOR);
    std::vector<std::string> expectBaseEnqueueList { R"(D:\Documents)", R"(D:\Downloads)", R"(C:\Users\thankod)"};

    sort(expectBaseEnqueueList.begin(), expectBaseEnqueueList.end());
    sort(baseEnqueueList.begin(), baseEnqueueList.end());
    EXPECT_EQ(baseEnqueueList, expectBaseEnqueueList);
}

/*
 * 用例名称：测试路径模式匹配，匹配规则不含通配
 * 前置条件：无
 * check点：返回正确匹配结果
 **/
TEST_F(ScannerFilterUtilTest, NonWildcardToRegexMatch) {
    std::string path = "/home/thankod/Download";
    auto pathPattern = ScanFilterUtil::WildcardToRegex(path);

    // match exactly
    EXPECT_TRUE(std::regex_match("/home/thankod/Download", pathPattern));
    EXPECT_FALSE(std::regex_match("/home/thankod/Download/movie.avi", pathPattern));
    EXPECT_FALSE(std::regex_match("/home/thankod/Document", pathPattern));
    EXPECT_FALSE(std::regex_match("/", pathPattern));
    EXPECT_FALSE(std::regex_match("", pathPattern));
}

/*
 * 用例名称：测试路径模式匹配，匹配规则包含通配
 * 前置条件：无
 * check点：返回正确匹配结果
 **/
TEST_F(ScannerFilterUtilTest, WildcardToRegexMatchAsterisk) {
    std::string path = "/home/thankod/*";
    auto pathPattern = ScanFilterUtil::WildcardToRegex(path);

    // match only one level
    EXPECT_TRUE(std::regex_match("/home/thankod/Download", pathPattern));
    EXPECT_TRUE(std::regex_match("/home/thankod/Document", pathPattern));
    EXPECT_TRUE(std::regex_match("/home/thankod/main.cpp", pathPattern));

    EXPECT_FALSE(std::regex_match("/home/thankod/Download/movie.avi", pathPattern));
    EXPECT_FALSE(std::regex_match("/home/thankod", pathPattern));
    EXPECT_FALSE(std::regex_match("/", pathPattern));
    EXPECT_FALSE(std::regex_match("", pathPattern));

    path = "/系统/用户/*";
    pathPattern = ScanFilterUtil::WildcardToRegex(path);

    // match only one level
    EXPECT_TRUE(std::regex_match("/系统/用户/下载", pathPattern));
    EXPECT_TRUE(std::regex_match("/系统/用户/文档", pathPattern));
    EXPECT_TRUE(std::regex_match("/系统/用户/测试.txt", pathPattern));

    EXPECT_FALSE(std::regex_match("/系统/用户/Download/movie.avi", pathPattern));
    EXPECT_FALSE(std::regex_match("/系统/用户", pathPattern));
    EXPECT_FALSE(std::regex_match("/", pathPattern));
    EXPECT_FALSE(std::regex_match("", pathPattern));

    path = "/系统/用户*/下载";
    pathPattern = ScanFilterUtil::WildcardToRegex(path);

    // match only one level
    EXPECT_TRUE(std::regex_match("/系统/用户/下载", pathPattern));
    EXPECT_TRUE(std::regex_match("/系统/用户甲/下载", pathPattern));
    EXPECT_TRUE(std::regex_match("/系统/用户乙/下载", pathPattern));

    EXPECT_FALSE(std::regex_match("/系统/用户/下载/movie.avi", pathPattern));
    EXPECT_FALSE(std::regex_match("/系统/用户/下载/movie.avi", pathPattern));
}

/*
 * 用例名称：测试路径模式匹配，匹配规则包含通配
 * 前置条件：无
 * check点：返回正确匹配结果
 **/
TEST_F(ScannerFilterUtilTest, WildcardToRegexMatchWin32) {
    std::string path = R"(C:\Users\thankod\*)";
    auto pathPattern = ScanFilterUtil::WildcardToRegex(path);

    // match only one level
    EXPECT_TRUE(std::regex_match(R"(C:\Users\thankod\Download)", pathPattern));
    EXPECT_TRUE(std::regex_match(R"(C:\Users\thankod\main.cpp)", pathPattern));
    EXPECT_TRUE(std::regex_match(R"(C:\Users\thankod\测试.txt)", pathPattern));

    EXPECT_FALSE(std::regex_match(R"(C:\Users\thankod\Download\file.txt)", pathPattern));
    EXPECT_FALSE(std::regex_match(R"(C:\Users\thankod)", pathPattern));
    EXPECT_FALSE(std::regex_match(R"(C:)", pathPattern));
    EXPECT_FALSE(std::regex_match("", pathPattern));

    path = R"(C:\系统\用户*\下载)";
    pathPattern = ScanFilterUtil::WildcardToRegex(path);

    // match only one level
    EXPECT_TRUE(std::regex_match(R"(C:\系统\用户\下载)", pathPattern));
    EXPECT_TRUE(std::regex_match(R"(C:\系统\用户甲\下载)", pathPattern));
    EXPECT_TRUE(std::regex_match(R"(C:\系统\用户乙\下载)", pathPattern));

    EXPECT_FALSE(std::regex_match(R"(C:\系统\用户\下载\movie.avi)", pathPattern));
    EXPECT_FALSE(std::regex_match(R"(C:\系统\用户\下载\movie.avi)", pathPattern));
}

/*
 * 用例名称：测试路径模式匹配，匹配规则包含通配，通配不可匹配路径
 * 前置条件：无
 * check点：返回正确匹配结果，*不能匹配路径
 **/
TEST_F(ScannerFilterUtilTest, WildcardToRegexMatchAsterisk2) {
    std::string path = "/home/*/*/work";
    auto pathPattern = ScanFilterUtil::WildcardToRegex(path);

    // match only two level
    EXPECT_TRUE(std::regex_match("/home/user/Desktop/work", pathPattern));
    EXPECT_TRUE(std::regex_match("/home/user1/Document/work", pathPattern));

    EXPECT_FALSE(std::regex_match("/home/work", pathPattern));
    EXPECT_FALSE(std::regex_match("/home/user/work", pathPattern));
    EXPECT_FALSE(std::regex_match("/home/user/Desktop/OcceanProtect/work", pathPattern));

    path = "/系统/*/桌面/*";
    pathPattern = ScanFilterUtil::WildcardToRegex(path);

    // match only one level
    EXPECT_TRUE(std::regex_match("/系统/用户甲/桌面/work", pathPattern));
    EXPECT_TRUE(std::regex_match("/系统/用户/桌面/测试", pathPattern));

    EXPECT_FALSE(std::regex_match("/系统/用户/桌面/桌面/测试", pathPattern));
}

/*
 * 用例名称：测试Nas路径和posix路径之间的转换
 * 前置条件：无
 * check点：返回正确的转换结果
 **/
TEST_F(ScannerFilterUtilTest, NASPathSyntaxUtilTest) {
    EXPECT_EQ(ScanFilterUtil::PosixPathToNasPath(NAS_PROTOCOL::NFS, "/"), ".");
    EXPECT_EQ(ScanFilterUtil::PosixPathToNasPath(NAS_PROTOCOL::NFS, "/dir"), "/dir");
    EXPECT_EQ(ScanFilterUtil::PosixPathToNasPath(NAS_PROTOCOL::SMB, "/"), "");
    EXPECT_EQ(ScanFilterUtil::PosixPathToNasPath(NAS_PROTOCOL::SMB, "/dir"), "dir");
    
    EXPECT_EQ(ScanFilterUtil::NasPathToPosixPath(NAS_PROTOCOL::SMB, ""), "/");
    EXPECT_EQ(ScanFilterUtil::NasPathToPosixPath(NAS_PROTOCOL::SMB, "."), "/");
    EXPECT_EQ(ScanFilterUtil::NasPathToPosixPath(NAS_PROTOCOL::SMB, "./"), "/");
    EXPECT_EQ(ScanFilterUtil::NasPathToPosixPath(NAS_PROTOCOL::SMB, "dir"), "/dir");
    EXPECT_EQ(ScanFilterUtil::NasPathToPosixPath(NAS_PROTOCOL::SMB, "./dir"), "/dir");
    EXPECT_EQ(ScanFilterUtil::NasPathToPosixPath(NAS_PROTOCOL::NFS, ""), "/");
    EXPECT_EQ(ScanFilterUtil::NasPathToPosixPath(NAS_PROTOCOL::NFS, "."), "/");
    EXPECT_EQ(ScanFilterUtil::NasPathToPosixPath(NAS_PROTOCOL::NFS, "./"), "/");
    EXPECT_EQ(ScanFilterUtil::NasPathToPosixPath(NAS_PROTOCOL::NFS, "./dir"), "/dir");
}

/*
 * 用例名称：测试用户输入过滤路径的预处理
 * 前置条件：无
 * check点：返回正确的处理转换结果
 **/
TEST_F(ScannerFilterUtilTest, FormatToStandardPosixPathTest) {
    EXPECT_EQ(ScanFilterUtil::FormatToStandardPosixPath(""), "");
    EXPECT_EQ(ScanFilterUtil::FormatToStandardPosixPath("/////"), "/");
    EXPECT_EQ(ScanFilterUtil::FormatToStandardPosixPath("dir1//dir2///dir3////dir4/////"), "/dir1/dir2/dir3/dir4");
    EXPECT_EQ(ScanFilterUtil::FormatToStandardPosixPath("/dir1///dir2/"), "/dir1/dir2");
    EXPECT_EQ(ScanFilterUtil::FormatToStandardPosixPath("dir1/file1/"), "/dir1/file1");
    
    EXPECT_EQ(ScanFilterUtil::FormatToStandardPosixPath("/"), "/");
    EXPECT_EQ(ScanFilterUtil::FormatToStandardPosixPath("/*"), "/*");
    EXPECT_EQ(ScanFilterUtil::FormatToStandardPosixPath("/file1"), "/file1");
    EXPECT_EQ(ScanFilterUtil::FormatToStandardPosixPath("/dir1/dir2/dir3"), "/dir1/dir2/dir3");
    EXPECT_EQ(ScanFilterUtil::FormatToStandardPosixPath("/dir1/dir2/file*"), "/dir1/dir2/file*");
}

/*
 * 用例名称：测试用户输入过滤路径的预处理
 * 前置条件：无
 * check点：返回正确的处理转换结果
 **/
TEST_F(ScannerFilterUtilTest, FormatToStandardWin32PathTest) {
    EXPECT_EQ(ScanFilterUtil::FormatToStandardWin32Path(""), "");
    EXPECT_EQ(ScanFilterUtil::FormatToStandardWin32Path(R"(C://dir1//dir2///dir3////dir4/////)"), R"(C:\dir1\dir2\dir3\dir4)");
    
    EXPECT_EQ(ScanFilterUtil::FormatToStandardWin32Path(R"(C:)"), R"(C:)");
    EXPECT_EQ(ScanFilterUtil::FormatToStandardWin32Path(R"(C:/dir1/dir2/file*)"), R"(C:\dir1\dir2\file*)");
}

/*
 * 用例名称：测试WIN/SMB大小写不敏感场景
 * 前置条件：无
 * check点：返回正确的匹配结果
 **/
TEST_F(ScannerFilterUtilTest, CaseSensitiveTest) {
    EXPECT_EQ(ScanFilterUtil::LowerCase(""), "");
    EXPECT_EQ(ScanFilterUtil::LowerCase("HeLlO/WorlD114/514"), "hello/world114/514");
    EXPECT_EQ(ScanFilterUtil::LowerCase("/目录1/DiR2/dir3"), "/目录1/dir2/dir3");
 
    FilterItem filterItem1("/目录OnE/fIlETwo", 0, true, POSIX_SEPARATOR);
    EXPECT_FALSE(filterItem1.MatchFile("/目录oNE/FILEtwo"));
 
    FilterItem filterItem2("/目录OnE/fIlETwo", 0, false, POSIX_SEPARATOR);
    EXPECT_TRUE(filterItem2.MatchFile("/目录oNE/FilEtwO"));
}
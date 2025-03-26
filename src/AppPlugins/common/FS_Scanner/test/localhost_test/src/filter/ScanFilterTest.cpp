#include "gtest/gtest.h"
#include "ScanFilter.h"

#define TRAVERSE_ENTRY(PATH) for ( \
    baseFilterFlag = find_if(expectEntryList.begin(), expectEntryList.end(), \
    [](const EnqueueEntry& entry) { return entry.first == PATH;})->second; \
    true ; \
)

#define TRAVERSE_DIR(PATH, FLAG) for ( \
    baseFilterFlag = FLAG; \
    true ; \
)

#define INIT_DIRSTAT(PATH) { \
    dirStat.m_path = PATH; \
    dirStat.m_filterFlag = 0; \
}

#define END_TRAVERSE break;

class ScannerFilterTest : public testing::Test {
public:
    DirStat dirStat {};
    std::shared_ptr<ScanFilter> scanFilter {};
    uint8_t baseFilterFlag;
    std::string tmpDirPath {}; // store lastest parameter of ScanDirAccept() for DiscardDirectoryIfEmpty() to invoke
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    bool ScanDirAccept(const std::string &dirPath, uint8_t targetFilterFlag);
    bool ScanDirNotAccept(const std::string &dirPath);
    bool ScanFileFileAccept(const std::string &filePath);
    bool DiscardDirectoryIfEmpty();
    bool EnqueueEntryListEqual(EnqueueEntryList &expectEntryList);
};

void ScannerFilterTest::SetUp()
{
    dirStat.m_filterFlag = 0;
    baseFilterFlag = FLAG_MAYBE;
}

void ScannerFilterTest::TearDown()
{}

void ScannerFilterTest::SetUpTestCase()
{}

void ScannerFilterTest::TearDownTestCase()
{}

bool ScannerFilterTest::ScanDirAccept(const std::string &dirPath, uint8_t targetFilterFlag)
{
    tmpDirPath = dirPath;
    dirStat.m_path = dirPath;
    dirStat.m_filterFlag = FLAG_MAYBE;
    bool result = scanFilter->AcceptDir(dirStat, baseFilterFlag);
    if (!result) {
        return false;
    }
    if (dirStat.m_filterFlag != targetFilterFlag) {
        return false;
    }
    return true;
}

bool ScannerFilterTest::ScanDirNotAccept(const std::string &dirPath)
{
    dirStat.m_path = dirPath;
    dirStat.m_filterFlag = FLAG_MAYBE;
    bool result = scanFilter->AcceptDir(dirStat, baseFilterFlag);
    return !result;
}

bool ScannerFilterTest::ScanFileFileAccept(const std::string &filePath)
{
    return scanFilter->AcceptFile(filePath, baseFilterFlag);
}

bool ScannerFilterTest::DiscardDirectoryIfEmpty() {
    return scanFilter->DiscardDirectory(0, tmpDirPath, dirStat.m_filterFlag);
}

bool ScannerFilterTest::EnqueueEntryListEqual(EnqueueEntryList &expectEntryList)
{
    scanFilter->InitEnqueueEntryList();
    auto enqueueEntryList = scanFilter->GetEnqueueEntryList();
    sort(enqueueEntryList.begin(), enqueueEntryList.end());
    sort(expectEntryList.begin(), expectEntryList.end());
    return enqueueEntryList == expectEntryList;
}

/**
 * 说明：
 *   NAS作为前缀的用例说明文件集为"/",
 *   Host作为前缀的用例文件集一般不为"/"
 */

/*
 * 用例名称：主机场景，Filter完全关闭
 * 前置条件：无
 * check点：任何文件集包含的文件和目录都能被接受
 **/
TEST_F(ScannerFilterTest, Host_FiltersAllDisabled) {
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::DISABLED, {}},
        ScanFileFilter{FILTER_TYPE::DISABLED, {}});

    scanFilter->Enqueue("/home/thankod/Download");
    scanFilter->Enqueue("/home/thankod");
    scanFilter->Enqueue("/home/thankod/Document");
    scanFilter->Enqueue("/etc");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/home/thankod", FLAG_ACCEPT_ALL},
        {"/etc", FLAG_ACCEPT_ALL},
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    EXPECT_TRUE(scanFilter->ShouldStopTraverse(FLAG_NON_RECURSIVE));
    EXPECT_FALSE(scanFilter->ShouldStopTraverse(FLAG_DIR | FLAG_RECURSIVE));
    EXPECT_FALSE(scanFilter->ShouldStopTraverse(FLAG_DIR | FLAG_FILE_FLTR));

    // accept any dir regardless of input 
    EXPECT_TRUE(scanFilter->AcceptDir(dirStat, FLAG_ACCEPT_ALL));

    // accept any file regardless of input 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/xuranus/hello.txt"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/test.txt"));
    EXPECT_TRUE(scanFilter->AcceptFile("/.vimrc"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/xuranus/hello.jpg", FLAG_ACCEPT_ALL));

    // discard no directory regardless of input 
    EXPECT_FALSE(scanFilter->DiscardDirectory(1, "/home", FLAG_DIR | FLAG_RECURSIVE));
    EXPECT_FALSE(scanFilter->DiscardDirectory(0, "/home", FLAG_NON_RECURSIVE));
    EXPECT_FALSE(scanFilter->DiscardDirectory(0, "/home", FLAG_DIR | FLAG_RECURSIVE));
}

/*
 * 用例名称：NAS场景，Filter完全关闭
 * 前置条件：无
 * check点：任何文件和目录都能被接受
 **/
TEST_F(ScannerFilterTest, NAS_FiltersAllDisabled) {
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::DISABLED, {}},
        ScanFileFilter{FILTER_TYPE::DISABLED, {}});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/", FLAG_ACCEPT_ALL}
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    // accept any dir regardless of input 
    EXPECT_TRUE(scanFilter->AcceptDir(dirStat, FLAG_ACCEPT_ALL));

    // accept any file regardless of input 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/xuranus/hello.txt"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/test.txt"));
    EXPECT_TRUE(scanFilter->AcceptFile("/.vimrc"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/xuranus/hello.jpg", FLAG_ACCEPT_ALL));
}

/*
 * 用例名称：NAS场景，目录过滤<仅包含>
 * 前置条件：无
 * check点：被包含的目录及子目录子文件被接受
 **/
TEST_F(ScannerFilterTest, NAS_DirInclude)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
            "/home/thankod",
            "/home/thankod/Download",
            "/home/thankod/Document", 
            "/etc/trojan/conf*",
            "/opt/*/DataBackup"
        }},
        ScanFileFilter{FILTER_TYPE::DISABLED, {}});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/home", FLAG_NON_RECURSIVE},
        {"/home/thankod", FLAG_ACCEPT_ALL},
        {"/etc", FLAG_NON_RECURSIVE},
        {"/etc/trojan", FLAG_RECURSIVE | FLAG_DIR},
        {"/opt",  FLAG_RECURSIVE | FLAG_DIR},
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    EXPECT_FALSE(scanFilter->AcceptFile("/home/hello.txt")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/hello1.png"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/Download/movie.avi"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/Document/story/story1.docx"));
    EXPECT_FALSE(scanFilter->AcceptFile("/home/xuranus/hello1.png")); 
    EXPECT_FALSE(scanFilter->AcceptFile("/usr/bin/zsh"));
    EXPECT_TRUE(scanFilter->AcceptFile("/etc/trojan/conf1/config.json"));
    EXPECT_FALSE(scanFilter->AcceptFile("/etc/trojan/default/config.json"));  
    EXPECT_TRUE(scanFilter->AcceptFile("/opt/1.0.1/DataBackup/install.sh"));   
    EXPECT_FALSE(scanFilter->AcceptFile("/opt/1.0.1/backup/DataBackup/install.sh"));

    TRAVERSE_ENTRY("/home/thankod") {
        EXPECT_TRUE(ScanFileFileAccept("/home/thankod/1.txt"));
        EXPECT_TRUE(ScanFileFileAccept("/home/thankod/2.png"));

        EXPECT_TRUE(ScanDirAccept("/home/thankod/dir1", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_ENTRY("/etc/trojan") {
        EXPECT_FALSE(ScanFileFileAccept("/etc/trojan/conf.txt"));
        EXPECT_FALSE(ScanFileFileAccept("/etc/trojan/cnf.txt"));

        EXPECT_TRUE(ScanDirAccept("/etc/trojan/conf1", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/etc/trojan/conf", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/etc/trojan/cnf", FLAG_RECURSIVE | FLAG_DIR)); // optimizable: shoule skip        
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/etc/trojan/conf1", FLAG_ACCEPT_ALL) {
        EXPECT_TRUE(ScanFileFileAccept("/etc/trojan/conf1/1.txt"));

        EXPECT_TRUE(ScanDirAccept("/etc/trojan/conf1/dir", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
    
    TRAVERSE_DIR("/etc/trojan/conf", FLAG_ACCEPT_ALL) {
        EXPECT_TRUE(ScanFileFileAccept("/etc/trojan/conf/1.txt"));
        END_TRAVERSE;
    }
    
    TRAVERSE_DIR("/etc/trojan/cnf", FLAG_RECURSIVE | FLAG_DIR) {
        EXPECT_FALSE(ScanFileFileAccept("/etc/trojan/cnf/1.txt"));

        EXPECT_TRUE(ScanDirAccept("/etc/trojan/cnf/dir", FLAG_RECURSIVE | FLAG_DIR)); // optimizable: shoule skip
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_ENTRY("/opt") {
        EXPECT_FALSE(ScanFileFileAccept("/opt/1.txt"));

        EXPECT_TRUE(ScanDirAccept("/opt/eiso", FLAG_RECURSIVE | FLAG_DIR));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/opt/Huawei", FLAG_RECURSIVE | FLAG_DIR));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/opt/eiso", FLAG_RECURSIVE | FLAG_DIR) {
        EXPECT_FALSE(ScanFileFileAccept("/opt/eiso/LICENCE"));

        EXPECT_TRUE(ScanDirAccept("/opt/eiso/install", FLAG_RECURSIVE | FLAG_DIR));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/opt/Huawei", FLAG_RECURSIVE | FLAG_DIR) {
        EXPECT_FALSE(ScanFileFileAccept("/opt/Huawei/LICENCE"));

        EXPECT_TRUE(ScanDirAccept("/opt/Huawei/install", FLAG_RECURSIVE | FLAG_DIR));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/opt/Huawei/DataBackup", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
}

/*
 * 用例名称：NAS场景，文件过滤<仅包含>
 * 前置条件：无
 * check点：被包含的文件及其父目录被接受
 **/
TEST_F(ScannerFilterTest, NAS_FileInclude)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::DISABLED, {}},
        ScanFileFilter{FILTER_TYPE::INCLUDE, {
            "/home/hello.txt",
            "/home/thankod/*.png",
            "/home/thankod/2.png",
            "/etc/*/*.conf"
        }});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/home", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
        {"/home/thankod", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
        {"/etc", FLAG_ACCEPT_ALL | FLAG_FILE_FLTR}
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    EXPECT_FALSE(scanFilter->AcceptFile("/home/hello.png")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/hello.txt"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/scene1.png"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/2.png"));
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/scene2.jpg"));
    EXPECT_FALSE(scanFilter->AcceptFile("/root/.vimrc"));
    EXPECT_FALSE(scanFilter->AcceptFile("/temp.swp"));

    TRAVERSE_ENTRY("/home") {
        EXPECT_TRUE(ScanFileFileAccept("/home/hello.txt"));
        EXPECT_FALSE(ScanFileFileAccept("/home/main.cpp"));

        EXPECT_TRUE(ScanDirNotAccept("/home/thankod")); // scan in next entry
        EXPECT_TRUE(ScanDirNotAccept("/home/xuranus"));
        END_TRAVERSE;
    }

    TRAVERSE_ENTRY("/home/thankod") {
        EXPECT_TRUE(ScanFileFileAccept("/home/thankod/1.png"));
        EXPECT_TRUE(ScanFileFileAccept("/home/thankod/hello.png"));
        EXPECT_FALSE(ScanFileFileAccept("/home/thankod/2.jpg"));

        EXPECT_TRUE(ScanDirNotAccept("/home/thankod/Desktop"));
        END_TRAVERSE;
    }
    
    TRAVERSE_ENTRY("/etc") {
        EXPECT_FALSE(ScanFileFileAccept("/etc/1.conf"));

        EXPECT_TRUE(ScanDirAccept("/etc/apache", FLAG_MAYBE)); // different from dir * case here
        EXPECT_TRUE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/etc/nginx", FLAG_MAYBE));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/etc/apache", FLAG_MAYBE) {
        EXPECT_TRUE(ScanFileFileAccept("/etc/apache/apache.conf"));
        EXPECT_FALSE(ScanFileFileAccept("/etc/apache/LICENCE"));

        EXPECT_TRUE(ScanDirAccept("/etc/apache/conf.d", FLAG_MAYBE)); // different from dir * case here
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/etc/apache/conf.d", FLAG_MAYBE) {
        EXPECT_FALSE(ScanFileFileAccept("/etc/apache/conf.d/php-cli.conf"));

        EXPECT_TRUE(ScanDirAccept("/etc/apache/conf.d/dir", FLAG_MAYBE)); // different from dir * case here
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
}

/*
 * 用例名称：NAS场景，目录过滤<排除>
 * 前置条件：无
 * check点：除了被匹配的目录及其子项接受
 **/
TEST_F(ScannerFilterTest, NAS_DirExclude)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::EXCLUDE, {
            "/home/thankod",
            "/home/thankod/Download",
            "/home/thankod/Document", 
            "/etc/trojan/conf*",
            "/opt/*/DataBackup",
            "/var"
        }},
        ScanFileFilter{FILTER_TYPE::DISABLED, {}});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/", FLAG_EXCLUDE}
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    EXPECT_TRUE(scanFilter->AcceptFile("/home/hello.txt")); 
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/hello1.png"));
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/Download/movie.avi"));
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/Document/story/story1.docx"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/xuranus/hello1.png")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/usr/bin/zsh"));
    EXPECT_FALSE(scanFilter->AcceptFile("/etc/trojan/conf1/config.json"));
    EXPECT_TRUE(scanFilter->AcceptFile("/etc/trojan/default/config.json"));  
    EXPECT_FALSE(scanFilter->AcceptFile("/opt/1.0.1/DataBackup/install.sh"));   
    EXPECT_TRUE(scanFilter->AcceptFile("/opt/1.0.1/backup/DataBackup/install.sh"));

    // traverse / 
    baseFilterFlag = FLAG_EXCLUDE;

    TRAVERSE_ENTRY("/") {
        EXPECT_TRUE(ScanFileFileAccept("/1.txt"));
        EXPECT_TRUE(ScanFileFileAccept("/2.png"));

        EXPECT_TRUE(ScanDirAccept("/home", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/etc", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/opt", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirNotAccept("/var"));
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/home", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanFileFileAccept("/home/1.txt"));
        EXPECT_TRUE(ScanFileFileAccept("/home/2.png"));

        EXPECT_TRUE(ScanDirAccept("/home/xuranus", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirNotAccept("/home/thankod"));
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/etc", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanFileFileAccept("/etc/1.txt"));
        EXPECT_TRUE(ScanFileFileAccept("/etc/2.png"));

        EXPECT_TRUE(ScanDirAccept("/etc/trojan", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/etc/trojan", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanFileFileAccept("/etc/trojan/1.txt"));
        EXPECT_TRUE(ScanFileFileAccept("/etc/trojan/2.png"));

        EXPECT_TRUE(ScanDirNotAccept("/etc/trojan/conf1"));

        EXPECT_TRUE(ScanDirAccept("/etc/trojan/cnf", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/opt", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanFileFileAccept("/opt/main.cpp"));
        EXPECT_TRUE(ScanFileFileAccept("/opt/2.png"));

        EXPECT_TRUE(ScanDirAccept("/opt/huawei", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/opt/huawei", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanFileFileAccept("/opt/huawei/1.cpp"));
        EXPECT_TRUE(ScanFileFileAccept("/opt/huawei/2.png"));

        EXPECT_TRUE(ScanDirAccept("/opt/huawei/OceanStor", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirNotAccept("/opt/huawei/DataBackup"));
        END_TRAVERSE;
    }
}

/*
 * 用例名称：NAS场景，文件过滤<排除>
 * 前置条件：无
 * check点：除了被匹配的目录及其子项接受
 **/
TEST_F(ScannerFilterTest, NAS_FileExclude)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::DISABLED, {}},
        ScanFileFilter{FILTER_TYPE::EXCLUDE, {
            "/home/hello.txt",
            "/hello.cpp",
            "/home/user/*.jpg"
        }});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/", FLAG_EXCLUDE}
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    EXPECT_FALSE(scanFilter->AcceptFile("/hello.cpp"));
    EXPECT_TRUE(scanFilter->AcceptFile("/hello.rb")); 
    EXPECT_FALSE(scanFilter->AcceptFile("/home/hello.txt")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/hello.cpp")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/user/1.png"));
    EXPECT_FALSE(scanFilter->AcceptFile("/home/user/2.jpg"));
    EXPECT_TRUE(scanFilter->AcceptFile("/root/jdk1.8.0/bin/javac"));

    
    TRAVERSE_ENTRY("/") {
        EXPECT_FALSE(ScanFileFileAccept("/hello.cpp"));
        EXPECT_TRUE(ScanFileFileAccept("/hello.txt"));

        EXPECT_TRUE(ScanDirAccept("/home/user", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/home", FLAG_EXCLUDE) {
        EXPECT_FALSE(ScanFileFileAccept("/home/hello.txt"));
        EXPECT_TRUE(ScanFileFileAccept("/home/hello.jpg"));

        EXPECT_TRUE(ScanDirAccept("/home/user", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/home/user", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanFileFileAccept("/home/user/hello.txt"));
        EXPECT_FALSE(ScanFileFileAccept("/home/user/1.jpg"));

        EXPECT_TRUE(ScanDirAccept("/home/user/Desktop", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
}

/*
 * 用例名称：NAS场景，目录过滤<包含>，文件过滤<包含>
 * 前置条件：无
 * check点：任何文件集包含的文件和目录都能被接受
 **/
TEST_F(ScannerFilterTest, NAS_BothInclude)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
            "/home/thankod",
            "/home/thankod/Download",
            "/home/thankod/Document", 
            "/etc/trojan/conf*",
            "/opt/*/DataBackup",
            "/root/Download"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE, {
            "/home/hello.txt",
            "/home/thankod/*.png",
            "/root/start.sh",
            "/var/install.log",
            "/root/Download/1.png"
        }});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/home", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
        {"/home/thankod", FLAG_ACCEPT_ALL}, // optimized, no need FLAG_FILE_FLTR
        {"/etc", FLAG_NON_RECURSIVE},
        {"/etc/trojan", FLAG_RECURSIVE | FLAG_DIR},
        {"/opt", FLAG_RECURSIVE | FLAG_DIR},
        {"/root", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
        {"/root/Download", FLAG_ACCEPT_ALL}, // FLAG_FILE_FLTR moved, optimized 
        {"/var", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR} // optimizable (need Enqueue single file)
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    EXPECT_TRUE(scanFilter->AcceptFile("/home/hello.txt")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/hello1.png")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/hello2.png")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/note.txt")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/Desktop/scene1.png"));
    EXPECT_FALSE(scanFilter->AcceptFile("/usr/bin/zsh"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/Download/movie.avi"));  
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/Document/story.docx"));
    EXPECT_TRUE(scanFilter->AcceptFile("/opt/1.0.1/DataBackup/install.sh"));   
    EXPECT_FALSE(scanFilter->AcceptFile("/opt/1.0.1/backup/DataBackup/install.sh"));

    TRAVERSE_ENTRY("/home") {
        EXPECT_TRUE(ScanFileFileAccept("/home/hello.txt"));
        EXPECT_FALSE(ScanFileFileAccept("/home/word.txt"));

        EXPECT_TRUE(ScanDirNotAccept("/home/thankod")); // scan in next entry
        EXPECT_TRUE(ScanDirNotAccept("/home/xuranus"));
        END_TRAVERSE;
    }

    TRAVERSE_ENTRY("/root") {
        EXPECT_TRUE(ScanFileFileAccept("/root/start.sh"));
        EXPECT_FALSE(ScanFileFileAccept("/root/stop.sh"));

        EXPECT_TRUE(ScanDirNotAccept("/root/Download")); // scan in next entry
        EXPECT_TRUE(ScanDirNotAccept("/root/Deskop")); 
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/home/thankod", FLAG_ACCEPT_ALL | FLAG_FILE_FLTR) {
        EXPECT_FALSE(ScanFileFileAccept("/home/thankod/hello.txt"));
        EXPECT_TRUE(ScanFileFileAccept("/home/thankod/word.png"));

        EXPECT_TRUE(ScanDirAccept("/home/thankod/Desktop", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/home/thankod/Download", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/home/thankod/Document", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
}

/*
 * 用例名称：NAS场景，目录过滤<排除>，文件过滤<排除>
 * 前置条件：无
 * check点：接受没有被任一过滤器匹配的文件和目录
 **/
TEST_F(ScannerFilterTest, NAS_BothExclude)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::EXCLUDE, {
            "/home/thankod",
            "/home/thankod/Download",
            "/home/thankod/Document", 
            "/etc/trojan/conf*",
            "/opt/*/DataBackup",
            "/root/Download"
        }},
        ScanFileFilter{FILTER_TYPE::EXCLUDE, {
            "/home/hello.txt",
            "/home/thankod/*.png",
            "/root/start.sh",
            "/var/install.log"
        }});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/", FLAG_EXCLUDE}
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    EXPECT_FALSE(scanFilter->AcceptFile("/home/hello.txt")); 
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/hello1.png")); 
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/hello2.png")); 
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/note.txt")); 
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/Desktop/scene1.png"));
    EXPECT_TRUE(scanFilter->AcceptFile("/usr/bin/zsh"));
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/Download/movie.avi"));  
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/Document/story.docx"));
    EXPECT_FALSE(scanFilter->AcceptFile("/opt/1.0.1/DataBackup/install.sh"));   
    EXPECT_TRUE(scanFilter->AcceptFile("/opt/1.0.1/backup/DataBackup/install.sh"));

    TRAVERSE_ENTRY("/") {
        EXPECT_TRUE(ScanFileFileAccept("/1.txt"));
        EXPECT_TRUE(ScanFileFileAccept("/2.png"));

        EXPECT_TRUE(ScanDirAccept("/home", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/etc", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/opt", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/root", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/var", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/bin", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/home", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanFileFileAccept("/home/hello.png"));
        EXPECT_FALSE(ScanFileFileAccept("/home/hello.txt"));

        EXPECT_TRUE(ScanDirAccept("/home/xuranus", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirNotAccept("/home/thankod"));
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/root", FLAG_EXCLUDE) {
        EXPECT_FALSE(ScanFileFileAccept("/root/start.sh"));
        EXPECT_TRUE(ScanFileFileAccept("/root/stop.sh"));

        EXPECT_TRUE(ScanDirNotAccept("/root/Download"));

        EXPECT_TRUE(ScanDirAccept("/root/Desktop", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
    
    TRAVERSE_DIR("/var", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanFileFileAccept("/var/uninstall.log"));
        EXPECT_FALSE(ScanFileFileAccept("/var/install.log"));

        EXPECT_TRUE(ScanDirAccept("/var/dir1", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/var/dir2", FLAG_EXCLUDE));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
}


/*
 * 用例名称：NAS场景，目录<仅包含>，文件<排除>
 * 前置条件：无
 * check点：接受被文件过滤器匹配但不被文件过滤器匹配的全部文件和目录
 **/
TEST_F(ScannerFilterTest, NAS_DirIncludeFileExclude)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
            "/home/thankod",
            "/home/thankod/Download",
            "/home/thankod/Document", 
            "/etc/trojan/conf*",
            "/opt/*/DataBackup",
            "/root/Download",
            "/usr"
        }},
        ScanFileFilter{FILTER_TYPE::EXCLUDE, {
            "/home/hello.txt",
            "/home/thankod/*.png",
            "/root/start.sh",
            "/var/tmp/log/install.log",
            "/usr/share/jdk11/*.so"
        }});

    scanFilter->Enqueue("/");

    
    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/home", FLAG_NON_RECURSIVE}, // exclude /home/hello.txt won't affect home since /home is NON_REC
        {"/home/thankod", FLAG_ACCEPT_ALL | FLAG_FILE_FLTR},
        {"/etc", FLAG_NON_RECURSIVE},
        {"/etc/trojan", FLAG_RECURSIVE | FLAG_DIR},
        {"/opt", FLAG_RECURSIVE | FLAG_DIR},
        {"/root", FLAG_NON_RECURSIVE},
        {"/root/Download", FLAG_ACCEPT_ALL},
        {"/usr", FLAG_ACCEPT_ALL | FLAG_FILE_FLTR}
        // /var => files that not in INCLUDE entry can won't be added to EnqueueList
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    EXPECT_FALSE(scanFilter->AcceptFile("/home/hello.txt")); 
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/hello1.png")); 
    EXPECT_FALSE(scanFilter->AcceptFile("/home/thankod/hello2.png")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/note.txt")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/Desktop/scene1.png"));
    EXPECT_TRUE(scanFilter->AcceptFile("/usr/bin/zsh"));
    EXPECT_FALSE(scanFilter->AcceptFile("/usr/share/jdk11/awt.so"));
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/Download/movie.avi"));  
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/Document/story.docx"));
    EXPECT_TRUE(scanFilter->AcceptFile("/opt/1.0.1/DataBackup/install.sh"));   
    EXPECT_FALSE(scanFilter->AcceptFile("/opt/1.0.1/backup/DataBackup/install.sh"));

    TRAVERSE_ENTRY("/home/thankod") {
        EXPECT_FALSE(ScanFileFileAccept("/home/thankod/1.png"));
        EXPECT_TRUE(ScanFileFileAccept("/home/thankod/2.jpg"));

        EXPECT_TRUE(ScanDirAccept("/home/thankod/Desktop", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/home/thankod/demo", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        END_TRAVERSE;
    }
}

/*
 * 用例名称：NAS场景，目录<排除>，文件<仅包含>
 * 前置条件：无
 * check点：接受被文件过滤器匹配但不被目录过滤器匹配的全部文件和目录
 **/
TEST_F(ScannerFilterTest, NAS_DirExcludeFileInclude)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::EXCLUDE, {
            "/etc/apache",
            "/bin",
            "/root"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE, {  // hint: support * in path
            "/home/hello.txt",
            "/home/thankod/*.png",
            "/root/start.sh",
            "/etc/*/*.conf"
        }});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/", FLAG_EXCLUDE},
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));


    EXPECT_TRUE(scanFilter->AcceptFile("/home/hello.txt")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/hello1.png")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/home/thankod/hello2.png")); 
    EXPECT_FALSE(scanFilter->AcceptFile("/root/stop.sh"));
    EXPECT_FALSE(scanFilter->AcceptFile("/root/start.sh")); 
    EXPECT_TRUE(scanFilter->AcceptFile("/etc/nginx/ngin.conf"));
    EXPECT_FALSE(scanFilter->AcceptFile("/etc/apache/apache.conf"));

    TRAVERSE_DIR("/home", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanFileFileAccept("/home/hello.txt"));
        EXPECT_FALSE(ScanFileFileAccept("/home/hello.cpp"));
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/etc", FLAG_EXCLUDE) {
        EXPECT_FALSE(ScanFileFileAccept("/home/thankod/1.txt"));
        EXPECT_TRUE(ScanFileFileAccept("/home/thankod/2.png"));

        EXPECT_TRUE(ScanDirNotAccept("/etc/apache"));

        EXPECT_TRUE(ScanDirAccept("/etc/nginx", FLAG_EXCLUDE));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }

    TRAVERSE_DIR("/etc/nginx", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanFileFileAccept("/etc/nginx/nginx.conf"));
        EXPECT_FALSE(ScanFileFileAccept("/etc/nginx/nginx.doc"));
        END_TRAVERSE;
    }
}

/*
 * 用例名称：ScanFilter支持Enqueue NFS/CIFS风格的路径并以posix风格存储
 * 前置条件：无
 * check点：被包含的文件及其父目录被接受
 **/
TEST_F(ScannerFilterTest, NAS_NFS_PathSyntax_With_Filter_Disabled_Test)
{
    EnqueueEntryList expectEntryList {};

    // For NFS "."
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::DISABLED, {}},
        ScanFileFilter{FILTER_TYPE::DISABLED,{}});
    scanFilter->EnableNasPathSyntax(NAS_PROTOCOL::NFS);
    scanFilter->Enqueue(".");
    expectEntryList = {{".", FLAG_ACCEPT_ALL}};
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    // for CIFS ""
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::DISABLED, {}},
        ScanFileFilter{FILTER_TYPE::DISABLED,{}});
    scanFilter->EnableNasPathSyntax(NAS_PROTOCOL::SMB);
    scanFilter->Enqueue("");
    expectEntryList = {{"", FLAG_ACCEPT_ALL}};
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));
}

/*
 * 用例名称：NAS场景，文件过滤<仅包含>
 * 前置条件：无
 * check点：被包含的文件及其父目录被接受
 **/
TEST_F(ScannerFilterTest, NAS_DTS1)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
            "/opt/fileset3/a/*",
            "/opt/fileset3/b/abc*",
            "/opt/fileset3/c/*abc",
            "/opt/fileset3/d/ab*c"
        }},
        ScanFileFilter{FILTER_TYPE::DISABLED,{}});

    scanFilter->Enqueue("/opt/fileset3");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/opt/fileset3", FLAG_NON_RECURSIVE},
        {"/opt/fileset3/a", FLAG_RECURSIVE | FLAG_DIR},
        {"/opt/fileset3/b", FLAG_RECURSIVE | FLAG_DIR},
        {"/opt/fileset3/c", FLAG_RECURSIVE | FLAG_DIR},
        {"/opt/fileset3/d", FLAG_RECURSIVE | FLAG_DIR}
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    TRAVERSE_ENTRY("/opt/fileset3/b") {
        EXPECT_TRUE(ScanDirAccept("/opt/fileset3/b/aabc", FLAG_RECURSIVE | FLAG_DIR));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/opt/fileset3/b/abc", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/opt/fileset3/b/abcd", FLAG_ACCEPT_ALL));
        EXPECT_FALSE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
}


/*
 * 用例名称：NAS场景，测试根目录文件生成
 * 前置条件：无
 * check点：被包含的文件及其父目录被接受
 **/
TEST_F(ScannerFilterTest, NAS_INCLUDE_ROOT_1)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
            "/test1/test/dir1",
            "/root"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{
            "/*.txt"
        }});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
        {"/test1", FLAG_NON_RECURSIVE},
        {"/test1/test", FLAG_NON_RECURSIVE},
        {"/test1/test/dir1", FLAG_ACCEPT_ALL},
        {"/root", FLAG_ACCEPT_ALL}
    };

    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));
}


/*
 * 用例名称：NAS场景，测试根目录路径生成，文件含*
 * 前置条件：无
 * check点：被包含的文件及其父目录被接受
 **/
TEST_F(ScannerFilterTest, NAS_INCLUDE_ROOT_2)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
            "/L1D*"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{}});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/", FLAG_RECURSIVE | FLAG_DIR},
    };

    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));
}

/*
 * 用例名称：NAS场景，测试根目录路径生成，文件无*
 * 前置条件：无
 * check点：被包含的文件及其父目录被接受
 **/
TEST_F(ScannerFilterTest, NAS_INCLUDE_ROOT_3)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {}},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{
            "/file2.txt"
        }});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
    };

    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));
}

/*
 * 用例名称：NAS场景，测试根目录路径生成，目录无*
 * 前置条件：无
 * check点：被包含的文件及其父目录被接受
 **/
TEST_F(ScannerFilterTest, NAS_INCLUDE_ROOT_4)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
            "/dir1"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{}});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/dir1", FLAG_ACCEPT_ALL},
    };

    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));
}

/*
 * 用例名称：NAS场景，测试根目录路径生成，目录含*
 * 前置条件：无
 * check点：被包含的文件及其父目录被接受
 **/
TEST_F(ScannerFilterTest, NAS_INCLUDE_ROOT_5)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
            "/DIR*"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{}});

    scanFilter->Enqueue("/");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {"/", FLAG_RECURSIVE | FLAG_DIR},
    };

    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));
}

/*
 * 用例名称：NAS场景，文件过滤<双包含>
 * 前置条件：无
 * check点：文件为目录子集，检查文件包含是否会影响到enqueueList生成
 **/
TEST_F(ScannerFilterTest, NAS_BOTH_INCLUDE_2)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE,{
            "/test1/test/dir1/dir22"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{
            "/test1/test/dir1/dir22/dir33/*.txt"
        }});

    scanFilter->Enqueue("/");

    EnqueueEntryList expectEntryList {
        {"/test1", FLAG_NON_RECURSIVE},
        {"/test1/test", FLAG_NON_RECURSIVE},
        {"/test1/test/dir1", FLAG_NON_RECURSIVE},
        {"/test1/test/dir1/dir22", FLAG_ACCEPT_ALL},
    };

    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));
}

/*
 * 用例名称：NAS场景，文件过滤包含，目录排除
 * 前置条件：无
 * check点：文件为目录子集，检查文件包含是否会影响到enqueueList生成。
 * 注：用例来源于每日构建
 **/
TEST_F(ScannerFilterTest, tc_nas_protect_backup_filter_multiple_types_0010)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::EXCLUDE,{
            "/Dir2",
            "/Dir1/Dir1_2/Dir*"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{
            "/1*",
            "/2*",
            "/Dir1/1*",
            "/Dir1/Dir1_1/1*",
            "/Dir1/Dir1_1/Dir1_1_1/1*",
            "/Dir1/Dir1_2/1*",
            "/Dir1/Dir1_2/Dir1_2_1/1*",
            "/Dir1/Dir1_2/Dir1_2_1/Dir1_2_1_1/1*",
            "/Dir2/1*"
        }});
    scanFilter->Enqueue("/");

    EnqueueEntryList expectEntryList {
        {"/", FLAG_EXCLUDE},
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    TRAVERSE_ENTRY("/") {
        EXPECT_TRUE(ScanDirAccept("/Dir1", FLAG_EXCLUDE));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
    TRAVERSE_DIR("/Dir1", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanDirAccept("/Dir1/Dir1_1", FLAG_EXCLUDE));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());

        EXPECT_TRUE(ScanDirAccept("/Dir1/Dir1_2", FLAG_EXCLUDE));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
    TRAVERSE_DIR("/Dir1/Dir1_1", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanDirAccept("/Dir1/Dir1_1/Dir1_1_1", FLAG_EXCLUDE));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
    TRAVERSE_DIR("/Dir1/Dir1_1/Dir1_1_1", FLAG_EXCLUDE) {
        EXPECT_TRUE(ScanDirAccept("/Dir1/Dir1_1/Dir1_1_1/Dir1_1_1_1", FLAG_EXCLUDE));
        EXPECT_TRUE(DiscardDirectoryIfEmpty());
        END_TRAVERSE;
    }
}

/*
 * 用例名称：NAS场景，文件目录双包含，开启SMB语义
 * 前置条件：无
 * check点：测试SMB路径风格转换
 **/
TEST_F(ScannerFilterTest, SmbSyntax_NonRoot)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE,{
            "/vdb.1_2.dir/vdb.2_1.dir/vdb.3_1.dir"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{
            "/vdb.1_1.dir/vdb.2_1.dir/vdb.3_1.dir/vdb_f0012.file"
        }});
    scanFilter->EnableNasPathSyntax(NAS_PROTOCOL::SMB);

    scanFilter->Enqueue(".");
    EnqueueEntryList expectEntryList {
        {"vdb.1_1.dir", FLAG_NON_RECURSIVE},
        {"vdb.1_1.dir/vdb.2_1.dir", FLAG_NON_RECURSIVE},
        {"vdb.1_1.dir/vdb.2_1.dir/vdb.3_1.dir", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
        {"vdb.1_2.dir", FLAG_NON_RECURSIVE},
        {"vdb.1_2.dir/vdb.2_1.dir", FLAG_NON_RECURSIVE},
        {"vdb.1_2.dir/vdb.2_1.dir/vdb.3_1.dir", FLAG_ACCEPT_ALL},
    };

    scanFilter->InitEnqueueEntryList();
    auto enqueueEntryList = scanFilter->GetEnqueueEntryList();
    sort(enqueueEntryList.begin(), enqueueEntryList.end());
    sort(expectEntryList.begin(), expectEntryList.end());
    EXPECT_EQ(enqueueEntryList, expectEntryList);
}

/*
 * 用例名称：NAS场景，文件目录双包含，开启NFS语义
 * 前置条件：无
 * check点：测试NFS路径风格转换
 **/
TEST_F(ScannerFilterTest, NfsSyntax_NonRoot)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE,{
            "/vdb.1_2.dir/vdb.2_1.dir/vdb.3_1.dir"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{
            "/vdb.1_1.dir/vdb.2_1.dir/vdb.3_1.dir/vdb_f0012.file"
        }});
    scanFilter->EnableNasPathSyntax(NAS_PROTOCOL::NFS);

    scanFilter->Enqueue(".");
    EnqueueEntryList expectEntryList {
        {"/vdb.1_1.dir", FLAG_NON_RECURSIVE},
        {"/vdb.1_1.dir/vdb.2_1.dir", FLAG_NON_RECURSIVE},
        {"/vdb.1_1.dir/vdb.2_1.dir/vdb.3_1.dir", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
        {"/vdb.1_2.dir", FLAG_NON_RECURSIVE},
        {"/vdb.1_2.dir/vdb.2_1.dir", FLAG_NON_RECURSIVE},
        {"/vdb.1_2.dir/vdb.2_1.dir/vdb.3_1.dir", FLAG_ACCEPT_ALL},
    };

    scanFilter->InitEnqueueEntryList();
    auto enqueueEntryList = scanFilter->GetEnqueueEntryList();
    sort(enqueueEntryList.begin(), enqueueEntryList.end());
    sort(expectEntryList.begin(), expectEntryList.end());
    EXPECT_EQ(enqueueEntryList, expectEntryList);
}

/*
 * 用例名称：NAS场景，目录排除，开启SMB语义
 * 前置条件：无
 * check点：测试SMB路径风格转换
 **/
TEST_F(ScannerFilterTest, SmbSyntax_Root)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::EXCLUDE,{
            "/vdb.1_2.dir/vdb.2_1.dir/vdb.3_1.dir"
        }},
        ScanFileFilter{FILTER_TYPE::EXCLUDE,{}});
    scanFilter->EnableNasPathSyntax(NAS_PROTOCOL::SMB);

    scanFilter->Enqueue("");
    EnqueueEntryList expectEntryList {
        {"", FLAG_EXCLUDE},
    };
    scanFilter->InitEnqueueEntryList();
    auto enqueueEntryList = scanFilter->GetEnqueueEntryList();
    EXPECT_EQ(enqueueEntryList, expectEntryList);
}

/*
 * 用例名称：NAS场景，目录排除，开启NFS语义
 * 前置条件：无
 * check点：测试Nfs路径风格转换
 **/
TEST_F(ScannerFilterTest, NfsSyntax_Root)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::EXCLUDE,{
            "/vdb.1_2.dir/vdb.2_1.dir/vdb.3_1.dir"
        }},
        ScanFileFilter{FILTER_TYPE::EXCLUDE,{}});
    scanFilter->EnableNasPathSyntax(NAS_PROTOCOL::NFS);

    scanFilter->Enqueue(".");
    EnqueueEntryList expectEntryList {
        {".", FLAG_EXCLUDE},
    };
    scanFilter->InitEnqueueEntryList();
    auto enqueueEntryList = scanFilter->GetEnqueueEntryList();
    EXPECT_EQ(enqueueEntryList, expectEntryList);
}

/*
 * 用例名称：测试部分过滤规则导致的coredump
 * 前置条件：无
 * check点：测试Nfs路径风格转换
 **/
TEST_F(ScannerFilterTest, Coredump)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::DISABLED,{}},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{
            "/file_0~_end.txt",
            "/file_1\\!_end.txt",
            "/file_2@_end.txt",
            "/file_3#_end.txt",
            "/file_4$_end.txt",
            "/file_5 _end.txt",
            "/file_6^_end.txt",
            "/file_7&_end.txt",
            "/file_8(_end.txt"
        }});

    scanFilter->Enqueue("/");
    EnqueueEntryList expectEntryList {
        {"/", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
    };
    scanFilter->InitEnqueueEntryList();
    auto enqueueEntryList = scanFilter->GetEnqueueEntryList();
    EXPECT_EQ(enqueueEntryList, expectEntryList);
}

/*
 * 用例名称：文件目录包含不规范的输入
 * 前置条件：无
 * check点：测试用户输出预处理
 **/
TEST_F(ScannerFilterTest, IllegalUserInputTest)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE,{
            "dir1///dir2/",
            ""
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE,{
            "dir11//dir22///file//",
            ""
        }});

    scanFilter->Enqueue("/");
    EnqueueEntryList expectEntryList {
        {"/dir1", FLAG_NON_RECURSIVE},
        {"/dir1/dir2", FLAG_ACCEPT_ALL},
        {"/dir11", FLAG_NON_RECURSIVE},
        {"/dir11/dir22", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR}
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));
}

/*
 * 用例名称：过滤导致的父目录丢失
 * 前置条件：无
 * check点：测试修复丢失的父目录
 **/
TEST_F(ScannerFilterTest, AmendMissingParentDir)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE,{
            "/*/*/dir3",
        }},
        ScanFileFilter{FILTER_TYPE::DISABLED,{}});

    scanFilter->Enqueue("/");
    EnqueueEntryList expectEntryList {
        {"/", FLAG_RECURSIVE | FLAG_DIR}
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    EXPECT_TRUE(scanFilter->DiscardDirectory(0, "/", FLAG_RECURSIVE | FLAG_FILE | FLAG_DIR | FLAG_FILE_FLTR));
    EXPECT_TRUE(scanFilter->DiscardDirectory(0, "/dir1", FLAG_RECURSIVE | FLAG_FILE | FLAG_DIR | FLAG_FILE_FLTR));
    EXPECT_TRUE(scanFilter->DiscardDirectory(0, "/dir1/dir2", FLAG_RECURSIVE | FLAG_FILE | FLAG_DIR | FLAG_FILE_FLTR));
    EXPECT_FALSE(scanFilter->DiscardDirectory(0, "/dir1/dir2/dir3", FLAG_ACCEPT_ALL));
    std::vector<std::string> expectParentDirs {
         "/",
        "/dir1",
        "/dir1/dir2"
    };
    EXPECT_EQ(scanFilter->CheckMissingParentDirectory("/dir1/dir2/dir3"), expectParentDirs);
}

/*
 * 用例名称：Windows场景，目录过滤<包含>，文件过滤<包含>
 * 前置条件：无
 * check点：任何文件集包含的文件和目录都能被接受
 **/
TEST_F(ScannerFilterTest, Win32_BothInclude)
{
    scanFilter = std::make_shared<ScanFilter>(
        ScanDirectoryFilter{FILTER_TYPE::INCLUDE, {
            R"(C:\Documents)",
            R"(C:\Users\thankod)",
            R"(C:\Users\thankod\Download)",
            R"(C:\Users\thankod\Document)", 
            R"(C:\Downloads\trojan\conf*)",
            R"(C:\opt\*\OceanProtect)",
            R"(C:\Administrator\Download)"
        }},
        ScanFileFilter{FILTER_TYPE::INCLUDE, {
            R"(C:\Users\hello.txt)",
            R"(C:\Users\thankod\*.png)",
            R"(C:\Administrator\start.sh)",
            R"(D:\Software\install.log)",
            R"(D:\Downloads\1.png)"
        }}, true);

    scanFilter->SetCaseSensitive(false);
    scanFilter->Enqueue(R"(C:\)");
    scanFilter->Enqueue(R"(D:\Software)");

    // test enqueue list generate
    EnqueueEntryList expectEntryList {
        {R"(C:\)", FLAG_NON_RECURSIVE},
        {R"(C:\Documents)", FLAG_ACCEPT_ALL},
        {R"(C:\Users)", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
        {R"(C:\Users\thankod)", FLAG_ACCEPT_ALL},
        {R"(C:\Downloads)", FLAG_NON_RECURSIVE},
        {R"(C:\Downloads\trojan)", FLAG_RECURSIVE | FLAG_DIR},
        {R"(C:\opt)", FLAG_RECURSIVE | FLAG_DIR},
        {R"(C:\Administrator\Download)", FLAG_ACCEPT_ALL},
        {R"(C:\Administrator)", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR},
        {R"(D:\Software)", FLAG_RECURSIVE | FLAG_FILE | FLAG_FILE_FLTR}
    };
    EXPECT_TRUE(EnqueueEntryListEqual(expectEntryList));

    EXPECT_TRUE(scanFilter->AcceptFile(R"(C:\Documents\1.txt)"));
    EXPECT_FALSE(scanFilter->AcceptFile(R"(C:\Downloads\2.txt)"));
    EXPECT_FALSE(scanFilter->AcceptFile(R"(C:\Desktop\3.txt)"));

    TRAVERSE_ENTRY(R"(C:\Users)") {
        EXPECT_TRUE(ScanFileFileAccept(R"(C:\Users\hello.txt)"));
        EXPECT_FALSE(ScanFileFileAccept(R"(C:\Users\world.txt)"));

        EXPECT_TRUE(ScanDirNotAccept(R"(C:\Users\xuranus)"));
        EXPECT_TRUE(ScanDirNotAccept(R"(C:\Users\thankod)")); // scan in next enqueue entry
        END_TRAVERSE;
    }

    TRAVERSE_ENTRY(R"(C:\Users)") {
        EXPECT_TRUE(ScanFileFileAccept(R"(C:\Users\hello.txt)"));
        EXPECT_FALSE(ScanFileFileAccept(R"(C:\Users\world.txt)"));

        EXPECT_TRUE(ScanDirNotAccept(R"(C:\Users\xuranus)"));
        EXPECT_TRUE(ScanDirNotAccept(R"(C:\Users\thankod)")); // scan in next enqueue entry
        END_TRAVERSE;
    }
}
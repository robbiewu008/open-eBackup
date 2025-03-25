#include "MergeDcacheService.h"
#include "ScannerUtils.h"

#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"

using namespace std;
using namespace FS_SCANNER;
using namespace Module;

class MergeDcacheServiceTest2 : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    void CreateDcacheFile(string fileName, std::queue<DirCache> &dcQueue);

    ScanConfig m_config  {};
    ScanInfo m_info {};
    ControlFileUtils m_cfu {};
    string m_latestDir {};
    std::unique_ptr<MergeDcacheService> m_ins {nullptr};
};

void MergeDcacheServiceTest2::CreateDcacheFile(string fileName, std::queue<DirCache> &dcQueue)
{
    auto dcacheObj = m_cfu.CreateDcacheObj(fileName, CTRL_FILE_OPEN_MODE::WRITE, m_config);
    if (dcacheObj == nullptr) {
        cout << "Creation of Dcache Object Failed for file, " << fileName << endl;
        return;
    }

    int32_t count = dcacheObj->WriteDirCacheEntries(dcQueue);
    cout << "Total entries written in file, " << fileName << " count: " << count << endl;
}

DirCache GetDcache(uint64_t inode, uint64_t mdataOffset, uint64_t fcacheOffset,
    uint32_t hashTag, uint32_t crc, uint32_t totalFiles, uint16_t fileId,
    uint16_t fcacheFileId, uint16_t metaLength)
{
    DirCache dcache {};
    dcache.m_inode = inode;
    dcache.m_mdataOffset = mdataOffset;
    dcache.m_fcacheOffset = fcacheOffset;
    dcache.m_dirPathHash.crc = hashTag;
    dcache.m_dirMetaHash.crc = crc;
    dcache.m_totalFiles = totalFiles;
    dcache.m_fileId = fileId;
    dcache.m_fcacheFileId = fcacheFileId;
    dcache.m_metaLength = metaLength;

    return dcache;
}

void MergeDcacheServiceTest2::SetUp()
{
    m_config.metaPath = "/opt";
    m_config.scanCtrlMaxDataSize = 102400;
    m_config.jobId = "1234";
    m_config.lastBackupTime = 0;
    m_config.scanCtrlMaxEntriesFullBkup = 10000;
    m_config.scanType = ScanJobType::FULL;

    m_info.m_dirCacheFileIndex = 0;

    m_latestDir = m_config.metaPath + "/latest";
    if (!FS_SCANNER::CheckAndCreateDirectory(m_latestDir)) {
        cout << "creation of latest dir failed" << endl;
    }

    std::queue<DirCache> dcQueue1 {};
    DirCache dcache1 = GetDcache(1234321, 100, 102, 0, 73287382, 2, 0, 0, 52);
    dcQueue1.push(dcache1);
    DirCache dcache2 = GetDcache(1234325, 152, 134, 0, 73283876, 2, 0, 0, 56);
    dcQueue1.push(dcache2);
    string file1 = string("/opt/latest/dircache_") + to_string(m_info.m_dirCacheFileIndex++);
    CreateDcacheFile(file1, dcQueue1);
    m_info.m_dirCacheFileList.push_back(file1);

    std::queue<DirCache> dcQueue2 {};
    DirCache dcache21 = GetDcache(2134321, 101, 103, 0, 73957382, 2, 0, 0, 54);
    dcQueue2.push(dcache21);
    DirCache dcache22 = GetDcache(2134325, 153, 135, 0, 76665376, 2, 0, 0, 58);
    dcQueue2.push(dcache22);
    string file2 = string("/opt/latest/dircache_") + to_string(m_info.m_dirCacheFileIndex++);
    CreateDcacheFile(file2, dcQueue2);
    m_info.m_dirCacheFileList.push_back(file2);

    m_ins = std::make_unique<MergeDcacheService>(m_config, m_info);
}

void MergeDcacheServiceTest2::TearDown()
{
    if (!FS_SCANNER::RemoveDir(m_latestDir)) {
        cout << "removal of latest dir failed" << endl;
    }
}

void MergeDcacheServiceTest2::SetUpTestCase() {}

void MergeDcacheServiceTest2::TearDownTestCase() {}

TEST_F(MergeDcacheServiceTest2, GenerateUnqiueDirCache)
{
    SCANNER_STATUS ret = m_ins->MergeAllDCacheFiles();
    EXPECT_EQ(ret, SCANNER_STATUS::SUCCESS);
    string finalDcacheFile = m_ins->CreateFinalDirCacheFile();
    cout << "finalDcacheFile: " << finalDcacheFile << endl;
}
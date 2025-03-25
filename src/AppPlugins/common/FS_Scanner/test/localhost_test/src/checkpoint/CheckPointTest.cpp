#include "gtest/gtest.h"
#include "stub.h"
#include "FSScannerCheckPoint.h"
#include "HardlinkManager.h"

using namespace Module;
using namespace std;

class CheckPointTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    ScanInfo info {};
    ScanConfig config {};
    Stub stub;
};

void CheckPointTest::SetUp()
{}

void CheckPointTest::TearDown()
{}

void CheckPointTest::SetUpTestCase()
{}

void CheckPointTest::TearDownTestCase()
{}

TEST_F(CheckPointTest, Write)
{
    config.metaPath = "/opt";
    std::string LATEST = "/latest";
    std::string latestDirPath = config.metaPath + LATEST;
    EXPECT_TRUE(FS_SCANNER::CheckAndCreateDirectory(latestDirPath));
    config.jobId = "3003903hk";
    config.lastBackupTime = 0;
    config.scanCtrlMaxDataSize = "102400";
    config.scanCtrlMaxEntriesFullBkup = 5;
    config.scanCheckPointEnable = true;

    std::unique_ptr<FSScannerCheckPoint> chkPnt = std::make_unique<FSScannerCheckPoint>(config, info);
    EXPECT_NE(chkPnt, nullptr);

    CHECKPOINT_STATUS status = chkPnt->GetCheckPointStatus();
    EXPECT_EQ(status, CHECKPOINT_STATUS::INIT);

    status = chkPnt->InitCheckPoint();
    EXPECT_EQ(status, CHECKPOINT_STATUS::SUCCESS);

    status = chkPnt->GetCheckPointStatus();
    EXPECT_EQ(status, CHECKPOINT_STATUS::IN_PROGRESS);

    /* Writing Output Buffer Queue */
    std::shared_ptr<BufferQueue<DirectoryScan>> buffer = std::make_shared<BufferQueue<DirectoryScan>>(10000);

    DirectoryScan dscan1 {};
    XMetaField name1 {};
    DirMetaWrapper dirWrapper1 {};
    name1.m_xMetaType = XMETA_TYPE::XMETA_TYPE_NAME;
    name1.m_value = "/home";
    dirWrapper1.m_xMeta.emplace_back(name1);
    dscan1.m_dmWrapper = dirWrapper1;
    dscan1.m_isDirScanCompleted = true;
    dscan1.m_filterFlag = 0;
    buffer->Push(dscan1);

    DirectoryScan dscan2 {};
    XMetaField name2 {};
    DirMetaWrapper dirWrapper2 {};
    name2.m_xMetaType = XMETA_TYPE::XMETA_TYPE_NAME;
    name2.m_value = "/etc";
    dirWrapper2.m_xMeta.emplace_back(name2);
    dscan2.m_dmWrapper = dirWrapper2;
    dscan2.m_isDirScanCompleted = true;
    dscan2.m_filterFlag = 0;
    buffer->Push(dscan2);
    chkPnt->ProcessOutputQueue(buffer);

    /* Writing ScanInProgressList */
    CheckPointData chkPntDataObj {};
    chkPntDataObj.m_path = "/home/abc";
    chkPntDataObj.m_filterFlag = 0;

    chkPnt->m_scanInProgressList.push_back(chkPntDataObj);
    chkPnt->ProcessScanInProgressList();

    /* Writing ScanQueue */
    std::shared_ptr<BufferQueue<DirStat>> scanQueue = std::make_shared<BufferQueue<DirStat>>(10000);

    DirStat dirStat1 {};
    dirStat1.m_path = "/home/abc/pqr";
    scanQueue->Push(dirStat1);

    DirStat dirStat2 {};
    dirStat2.m_path = "/etc/lmn";
    scanQueue->Push(dirStat2);
    chkPnt->ProcessScanQueue(scanQueue);

    chkPnt->m_isProcessingQueuesDone = true;
    chkPnt->m_ischkPntFlushDone = true;

    status = chkPnt->GetCheckPointStatus();
    EXPECT_EQ(status, CHECKPOINT_STATUS::COMPLETED);

    status = chkPnt->EndCheckPoint();
    EXPECT_EQ(status, CHECKPOINT_STATUS::SUCCESS);

    status = chkPnt->GetCheckPointStatus();
    EXPECT_EQ(status, CHECKPOINT_STATUS::IDLE);
}

TEST_F(CheckPointTest, Read)
{
    std::string LATEST = "/latest";

    config.metaPath = "/oops";

    std::unique_ptr<FSScannerCheckPoint> chkPnt = std::make_unique<FSScannerCheckPoint>(config, info);
    EXPECT_NE(chkPnt, nullptr);

    EXPECT_FALSE(chkPnt->IsCheckPointDirExists());

    chkPnt->m_config.metaPath = "/opt";
    EXPECT_TRUE(chkPnt->IsCheckPointDirExists());

    vector<string> checkPointFiles {};
    CHECKPOINT_STATUS status = chkPnt->ReadCheckPointDirectory(checkPointFiles);
    EXPECT_EQ(status, CHECKPOINT_STATUS::SUCCESS);
    EXPECT_FALSE(checkPointFiles.empty());
    EXPECT_EQ(info.m_dirCacheFileIndex, 1);
    EXPECT_EQ(info.m_metaFileCountIterator, 1);
    EXPECT_EQ(info.m_xMetaFileCountIterator, 1);
    EXPECT_EQ(info.m_fcacheFileCountIterator, 1);

    vector<CheckPointData> chkPntDataList {};
    for (auto fileName: checkPointFiles) {
        status = chkPnt->ReadCheckPointFile(fileName, chkPntDataList);
        EXPECT_EQ(status, CHECKPOINT_STATUS::SUCCESS);
    }
    EXPECT_FALSE(chkPntDataList.empty());

    chkPnt->UpdateCheckPointTime();
    EXPECT_EQ(chkPnt->RemoveCheckPointDir(), CHECKPOINT_STATUS::SUCCESS);

    std::string latestDirPath = config.metaPath + LATEST;
    EXPECT_TRUE(FS_SCANNER::RemoveDir(latestDirPath));
}

static CHECKPOINT_STATUS CreateTempCheckPointDirectory_Fail_Stub(void* obj)
{
    std::cout << "CreateTempCheckPointDirectory fail stub invoked" << std::endl;
    return CHECKPOINT_STATUS::FAILED;
}

static CHECKPOINT_STATUS CreateTempCheckPointDirectory_Stub(void* obj)
{
    std::cout << "CreateTempCheckPointDirectory Success stub invoked" << std::endl;
    return CHECKPOINT_STATUS::SUCCESS;
}

static CHECKPOINT_STATUS CreateCheckPointFile_Fail_Stub(void* obj)
{
    std::cout << "CreateCheckPointFile fail stub invoked" << std::endl;
    return CHECKPOINT_STATUS::FAILED;
}

static CHECKPOINT_STATUS CreateCheckPointFile_Stub(void* obj)
{
    std::cout << "CreateCheckPointFile Success stub invoked" << std::endl;
    return CHECKPOINT_STATUS::SUCCESS;
}


TEST_F(CheckPointTest, InitCheckPoint)
{
    std::unique_ptr<FSScannerCheckPoint> chkPnt = std::make_unique<FSScannerCheckPoint>(config, info);
    stub.set(ADDR(FSScannerCheckPoint, CreateTempCheckPointDirectory), CreateTempCheckPointDirectory_Fail_Stub);
    CHECKPOINT_STATUS status = chkPnt->InitCheckPoint();
    EXPECT_EQ(status, CHECKPOINT_STATUS::FAILED);

    stub.set(ADDR(FSScannerCheckPoint, CreateTempCheckPointDirectory), CreateTempCheckPointDirectory_Stub);
    stub.set(ADDR(FSScannerCheckPoint, CreateCheckPointFile), CreateCheckPointFile_Fail_Stub);
    status = chkPnt->InitCheckPoint();
    EXPECT_EQ(status, CHECKPOINT_STATUS::FAILED);

    stub.set(ADDR(FSScannerCheckPoint, CreateCheckPointFile), CreateCheckPointFile_Stub);
    status = chkPnt->InitCheckPoint();
    EXPECT_EQ(status, CHECKPOINT_STATUS::SUCCESS);
}

static CHECKPOINT_STATUS WriteCheckPointEntryToFile_Success_stub(string chkPntEntry)
{
    cout << "WriteCheckPointEntryToFile Success Stub invoked" << endl;
    return CHECKPOINT_STATUS::SUCCESS;
}

static CHECKPOINT_STATUS WriteCheckPointEntryToFile_Fail_stub(string chkPntEntry)
{
    cout << "WriteCheckPointEntryToFile Fail Stub invoked" << endl;
    return CHECKPOINT_STATUS::FAILED;
}

TEST_F(CheckPointTest, WriteCheckPointEntryToFileSuccess)
{
    std::unique_ptr<FSScannerCheckPoint> chkPnt = std::make_unique<FSScannerCheckPoint>(config, info);
    stub.set(ADDR(FSScannerCheckPoint, WriteCheckPointEntryToFile), WriteCheckPointEntryToFile_Success_stub);

    vector<pair<string, uint8_t>> enqueueEntryList {};
    enqueueEntryList.push_back(make_pair("/home", 14));

    chkPnt->ProcessFilterEnqList(enqueueEntryList);
}

TEST_F(CheckPointTest, WriteCheckPointEntryToFileFail)
{
    std::unique_ptr<FSScannerCheckPoint> chkPnt = std::make_unique<FSScannerCheckPoint>(config, info);
    stub.set(ADDR(FSScannerCheckPoint, WriteCheckPointEntryToFile), WriteCheckPointEntryToFile_Fail_stub);

    vector<pair<string, uint8_t>> enqueueEntryList {};
    enqueueEntryList.push_back(make_pair("/home", 14));

    chkPnt->ProcessFilterEnqList(enqueueEntryList);
}

TEST_F(CheckPointTest, ScanQueueBuffer)
{
    std::stringstream queueBuffer {};
    uint32_t lengthWritten = 0;

    DirStat dirStat {};
    dirStat.m_path = "/root/abc/pqr";

    bool ret = SCAN_QUEUE_UTILS::WriteDirStatToBuffer(queueBuffer, dirStat, lengthWritten);
    if (!ret) {
        cout << "Write DirStat To Buffer Failed, Path: " << dirStat.m_path << endl;
    }

    cout << "lengthWritten: " << lengthWritten << endl;


    std::unique_ptr<FSScannerCheckPoint> chkPnt = std::make_unique<FSScannerCheckPoint>(config, info);
    chkPnt->ProcessScanQueueBuffer(queueBuffer);
}

static CTRL_FILE_RETCODE WriteInodeEntry_stub(HardlinkCtrlInodeEntry &inodeEntry)
{
    cout << "WriteInodeEntry stub invoked" << endl;
    return CTRL_FILE_RETCODE::SUCCESS;
}

TEST_F(CheckPointTest, HardLinkFilesCheckPoint)
{
    OutputStats opStats {};
    HardlinkManager hardLinkManager {};
    HardlinkFileCache hfc {};
    hfc.m_inode = 3283682;
    hfc.m_mdataOffset = 50;
    hfc.m_metaLength = 100;
    hfc.m_dirName = "/dirPath";
    hfc.m_fileName = "fileName";
    hfc.m_aclFlag = 0;
    hfc.m_metafileName = "meta_file_0";
    hardLinkManager.InsertHardLinkFileToMap(hfc);
    hardLinkManager.m_hardlinkFilesCntOfDirPathMap[hfc.m_dirName] = 1;

    config.metaPath = "/opt";
    std::string LATEST = "/latest";
    std::string latestDirPath = config.metaPath + LATEST;
    EXPECT_TRUE(FS_SCANNER::CheckAndCreateDirectory(latestDirPath));
    config.jobId = "3003903hk";
    config.lastBackupTime = 0;
    config.scanCtrlMaxDataSize = "102400";
    config.scanCtrlMinDataSize = "51200";
    config.scanCtrlMaxEntriesFullBkup = 5;
    config.scanCtrlMinEntriesFullBkup = 3;

    stub.set(ADDR(HardlinkCtrlParser, WriteInodeEntry), WriteInodeEntry_stub);
    std::unique_ptr<FSScannerCheckPoint> chkPnt = std::make_unique<FSScannerCheckPoint>(config, info);
    chkPnt->CreateTempCheckPointDirectory();
    chkPnt->InitHardLinkControlFile();
    chkPnt->WriteHardLinkMap(hardLinkManager.m_hardlinkMap, hardLinkManager.m_hardlinkFilesCntOfDirPathMap);

    string tmpChkPntDirPath = latestDirPath + "/checkpoint_tmp";
    string chkPntDirPath = latestDirPath + "/checkpoint";
    if (!FS_SCANNER::Rename(tmpChkPntDirPath, chkPntDirPath)) {
        cout << "Rename Dir Failed, tmpChkPntDirPath: " << tmpChkPntDirPath
            << " chkPntDirPath: " << chkPntDirPath << endl;
    }

    vector<string> hardLinkCtrlFiles {};
    chkPnt->GetHardLinkCtrlFiles(hardLinkCtrlFiles);

    queue<HardlinkFileCache> hardLinkFCacheQue {};
    vector<pair<string, uint32_t>> hardlinkFilesCntList {};
    for (auto fileName: hardLinkCtrlFiles) {
        chkPnt->ReadHardLinkCtrlFile(fileName, hardLinkFCacheQue, hardlinkFilesCntList);
    }

    cout << "hardLinkFCacheQue size: " << hardLinkFCacheQue.size() << endl;
    cout << "hardlinkFilesCntList size: " << hardlinkFilesCntList.size() << endl;
    EXPECT_TRUE(FS_SCANNER::RemoveDir(latestDirPath));
}
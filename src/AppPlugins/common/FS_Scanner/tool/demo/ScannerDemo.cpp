/*
* Copyright (c) Huawei Technologies Co., Ltd. 2021-2023. All rights reserved.
* Author: w30029850
* Create: 2023-01-21
*/
#include <iostream>
#include <chrono>
#include <thread>
#include <fstream>
#include <exception>
#include "Log.h"
#include "File.h"
#include "Path.h"
#include "ScanMgr.h"
#include "ScannerSerializableConfig.h"

#ifdef _WIN32
static std::string LOG_PATH = R"(C:\ScannerDemo\scan\log)";
#else
static std::string LOG_PATH = "/home/compile/shuai/scanner_tool/log"; // coredump if LOG_PATH level is less than 2
#endif

using namespace Module;
using namespace std;

namespace {
    uint64_t beginTime = 0;
    uint32_t cnt = 0;
}

static std::string LOG_NAME = "ScannerDemo.log";
static std::string LOG_LEVEL_STR = "DEBUG";

// used to configure v2 path mapper
std::string PREFIX_PATH_MAPPER_STRING_VALUE;
std::string INFIX_PATH_MAPPER_STRING_VALUE;
std::size_t INFIX_PATH_MAPPER_POS_VALUE;

void InitLog()
{
    std::cout << "Using Log Path " << LOG_PATH << std::endl;
    std::cout << "Using Log Name " << LOG_NAME << std::endl;
    std::cout << "Using Log Level " << LOG_LEVEL_STR << std::endl;
    Module::CPath::GetInstance().Init(LOG_PATH);
    int iLogLevel = 0;
    if (LOG_LEVEL_STR == "DEBUG") {
        iLogLevel = 0;
    } else if (LOG_LEVEL_STR == "INFO") {
        iLogLevel = 1;
    }
    int iLogCount = 100;
    int iLogMaxSize = 30;
    CLogger::GetInstance().Init(LOG_NAME.c_str(), LOG_PATH);
    CLogger::GetInstance().SetLogConf(iLogLevel, iLogCount, iLogMaxSize);
    return;
}

void GeneratedCopyCtrlFileCb(void *usrData, std::string ctrlFile)
{
    cout << "Generated copy ctrl file: " << ctrlFile << endl;
}

void GeneratedHardLinkCtrlFileCb(void *usrData, std::string ctrlFile)
{
    cout << "Generated hard link ctrl file: " << ctrlFile << endl;
}

void GeneratedRfiCtrlFileCb(void *usrData, RfiCbStruct cbParam)
{
    cout << "generate rfi file : " << cbParam.rfiZipFileName << endl;
}

bool ReadScanConfig(const std::string& scanConfigJsonPath, ScanConfig& scanConfig)
{
    /* Path Existence Checked At wmain */
    ScannerSerializableConfig serializableConfig{};
    std::ifstream jsonFile(scanConfigJsonPath);
    if (!jsonFile.is_open()) {
        return false;
    }
    std::string jsonStr;
    std::string bufStr;
    while (jsonFile >> bufStr) {
        jsonStr += bufStr;
    }
    if (!Module::JsonHelper::JsonStringToStruct(jsonStr, serializableConfig)) {
        std::cout << "ScanConfig Deserialization Failed" << std::endl;
        return false;
    }
    serializableConfig.DeserializeLiteralFieldsToOrigin();
    serializableConfig.PrintScanConfig();
    scanConfig = static_cast<ScanConfig>(serializableConfig);
    return true;
}

void FillAdditionalScanConfig(ScanConfig& scanConfig)
{
    INFOLOG("Enter FillAdditionalScanConfig");
    scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
    scanConfig.scanHardlinkResultCb = GeneratedHardLinkCtrlFileCb;
    scanConfig.rfiCtrlCb = GeneratedRfiCtrlFileCb;
}

void FillObjectStorageScanConfig(ScanConfig& scanConfig)
{
#ifdef _OBS
    const string ACCESS_KEY_ID = "KWQB4Q8MJU4B9EKZHI5H";
    const string SECRET_ACCESS_KEY = "ew9d8JdzSueSC4shkguvTR5tK8FdRMgfryanW3Km";
    const string HOST_NAME = "10.160.170.10";
    const string BUCKET_NAME = "hcs-test-10";

    scanConfig.obs.authArgs.storageType = StorageType::HUAWEI;
    scanConfig.obs.authArgs.verifyInfo.endPoint = HOST_NAME;
    scanConfig.obs.authArgs.verifyInfo.accessKey = ACCESS_KEY_ID;
    scanConfig.obs.authArgs.verifyInfo.secretKey = SECRET_ACCESS_KEY;

    ObjectStorageBucket bucket{};
    bucket.bucketName = BUCKET_NAME;
    scanConfig.obs.buckets.emplace_back(bucket);

    scanConfig.enableV2 = false;
#endif
}

/* split the enqueue path with ':' into real enqueuePath and prefix */
std::pair<std::string, std::string> ParseEnqueuePathPair(const std::string& enqueuePath)
{
    size_t pos = enqueuePath.find("::");
    if (pos == std::string::npos) {
        return { enqueuePath, "" };
    }
    return { enqueuePath.substr(0, pos), enqueuePath.substr(pos + 2) };
}

bool StartScanner(const std::string& scanConfigJsonPath,
    std::shared_ptr<Scanner> &scanner, const std::vector<std::string> enqueuePathList)
{
    ScanConfig scanConfig;
    if (!ReadScanConfig(scanConfigJsonPath, scanConfig)) {
        std::cout << "Read ScanConfig From Json File Failed" << std::endl;
        return false;
    }
    FillAdditionalScanConfig(scanConfig);
    // 单线程方便gdb调试
    scanConfig.producerThreadCount = 1;
    scanConfig.diffThreadCount = 1;
    if (scanConfig.scanIO == IOEngine::OBJECTSTORAGE) {
        FillObjectStorageScanConfig(scanConfig);
    }

    if (scanConfig.enableV2) {
        std::cout << "V2 methods enabled!" << std::endl;
        if (!PREFIX_PATH_MAPPER_STRING_VALUE.empty()) {
            scanConfig.pathMapper = ScanMgr::BuildPrefixPathMapper(PREFIX_PATH_MAPPER_STRING_VALUE);
        } else if (!INFIX_PATH_MAPPER_STRING_VALUE.empty()) {
            scanConfig.pathMapper = ScanMgr::BuildInfixPathMapper(
                INFIX_PATH_MAPPER_POS_VALUE, INFIX_PATH_MAPPER_STRING_VALUE);
        }
    }
    scanner = ScanMgr::CreateScanInst(scanConfig);
    if (scanner == nullptr) {
        ERRLOG("create ScannerInst failed");
        return false;
    }
    for (const std::string &enqueuePath : enqueuePathList) {
        std::pair<std::string, std::string> enqueuePathPair = ParseEnqueuePathPair(enqueuePath);
        if (scanConfig.enableV2) {
            scanner->EnqueueV2(enqueuePathPair.first);
        } else {
            scanner->Enqueue(enqueuePathPair.first, enqueuePathPair.second);
        }
    }
    beginTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (SCANNER_STATUS::SUCCESS != scanner->Start()) {
        ERRLOG("Start scanner instance failed!");
        return false;
    }
    return true;
}

void PrintScannerInfo(ScanStatistics &updateStats, ScanStatistics &lastStats)
{
    if (updateStats.mScanDuration == 0) {
        return;
    }
    if (cnt++ % 6 == 0) {
        uint64_t totalDirSpeed = updateStats.mTotDirs / updateStats.mScanDuration * 60;
        uint64_t totalFileSpeed = updateStats.mTotFiles / updateStats.mScanDuration * 60;
        std::cout << "****Scanning totally dirs: " << updateStats.mTotDirs << ", files: " << updateStats.mTotFiles
            << ", speed: " << totalDirSpeed << "/min dirs, " << totalFileSpeed << "/min files." << std::endl;
    }
    uint64_t currentDirCount = updateStats.mTotDirs - lastStats.mTotDirs;
    uint64_t currentFileCount = updateStats.mTotFiles - lastStats.mTotFiles;
    uint64_t currentDirSpeed = currentDirCount / (updateStats.mScanDuration - lastStats.mScanDuration);
    uint64_t currentFileSpeed = currentFileCount / (updateStats.mScanDuration - lastStats.mScanDuration);
    std::cout << "Scanning dirs: " << currentDirCount << ", files: " << currentFileCount
        << ", speed: " << currentDirSpeed << "/s dirs, " << currentFileSpeed << "/s files." << std::endl;
}

void MonitorScanner(std::shared_ptr<Scanner> &scanner)
{
    INFOLOG("Enter Monitor Scanner");
    ScanStatistics lastStats{};
    ScanStatistics updateStats{};
    do {
        auto status = scanner->GetStatus();
        if (status == SCANNER_STATUS::COMPLETED) {
            INFOLOG("Scanner Completed");
            break;
        }
        if (status == SCANNER_STATUS::ABORTED) {
            INFOLOG("Scanner Aborted");
            break;
        }
        uint64_t curTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        updateStats = scanner->GetStatistics();
        updateStats.mScanDuration = curTime - beginTime;
        PrintScannerInfo(updateStats, lastStats);
        lastStats = updateStats;
        std::this_thread::sleep_for(std::chrono::seconds(10));
    } while (true);
    INFOLOG("Exit Monitor Scanner");
    return;
}

#ifndef _WIN32
bool CheckScanConfigPath(const std::string& scanConfigJsonPath)
{
    if (scanConfigJsonPath.empty()) {
        std::cout << "Need To Specify ScanConfigPath!" << std::endl;
        return false;
    }
    std::cout << "Using scanConfig Path: " << scanConfigJsonPath << std::endl;
    if (!Module::CFile::FileExist(scanConfigJsonPath.c_str())) {
        std::cout << "ScanConfigPath Invalid." << std::endl;
        return false;
    }
    return true;
}

int TestReadMeta(std::string metaDir);


struct SerializableScanStatistic {
    uint64_t mScanDuration = 0;                 /* Total scan duration (in seconds) */
    uint64_t mTotDirs = 0;                      /* Total num of dir detected in NAS Share */
    uint64_t mTotFiles = 0;                     /* Total num of files detected in NAS Share */
    uint64_t mTotalSize = 0;                    /* Total size of the NAS Share */
    uint64_t mTotDirsToBackup = 0;              /* Total num of dir (new/modified) to backup */
    uint64_t mTotFilesToBackup = 0;             /* Total num of files(new/modified) to backup */
    uint64_t mTotFilesDeleted = 0;              /* Total num of files to be deleted */
    uint64_t mTotDirsDeleted = 0;               /* Total num of dirs to be deleted */
    uint64_t mTotalSizeToBackup = 0;            /* Total size to backup */
    uint64_t mTotalControlFiles = 0;            /* Total Control Files Generated */
    uint64_t mTotFailedDirs = 0;                /* Total num of Failed dir detected in NAS Share */
    uint64_t mTotFailedFiles = 0;               /* Total num of Failed files detected in NAS Share */
    uint64_t mEntriesMayFailedToArchive = 0;    /* Total num of file/dir with long path/name may failed to archive */

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mScanDuration, mScanDuration)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotDirs, mTotDirs)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotFiles, mTotFiles)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotalSize, mTotalSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotDirsToBackup, mTotDirsToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotFilesToBackup, mTotFilesToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotFilesDeleted, mTotFilesDeleted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotDirsDeleted, mTotDirsDeleted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotalSizeToBackup, mTotalSizeToBackup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotalControlFiles, mTotalControlFiles)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotFailedDirs, mTotFailedDirs)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mTotFailedFiles, mTotFailedFiles)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mEntriesMayFailedToArchive, mEntriesMayFailedToArchive)
    END_SERIAL_MEMEBER
};

void ConvertScanStatisticToSerializableScanStatistic(SerializableScanStatistic& serializableStatistic, const ScanStatistics& statistic)
{
    serializableStatistic.mScanDuration = statistic.mScanDuration;
    serializableStatistic.mTotDirs = statistic.mTotDirs;
    serializableStatistic.mTotFiles = statistic.mTotFiles;
    serializableStatistic.mTotalSize = statistic.mTotalSize;
    serializableStatistic.mTotDirsToBackup = statistic.mTotDirsToBackup;
    serializableStatistic.mTotFilesToBackup = statistic.mTotFilesToBackup;
    serializableStatistic.mTotFilesDeleted = statistic.mTotFilesDeleted;
    serializableStatistic.mTotDirsDeleted = statistic.mTotDirsDeleted;
    serializableStatistic.mTotalSizeToBackup = statistic.mTotalSizeToBackup;
    serializableStatistic.mTotalControlFiles = statistic.mTotalControlFiles;
    serializableStatistic.mTotFailedDirs = statistic.mTotFailedDirs;
    serializableStatistic.mTotFailedFiles = statistic.mTotFailedFiles;
    serializableStatistic.mEntriesMayFailedToArchive = statistic.mEntriesMayFailedToArchive;
}

void WriteScanStatisticJson(const ScanStatistics& statistic, const std::string& statisticDir)
{
    SerializableScanStatistic serializableStatistic {};
    ConvertScanStatisticToSerializableScanStatistic(serializableStatistic, statistic);
    std::string jsonStr;
    if (!Module::JsonHelper::StructToJsonString(serializableStatistic, jsonStr)) {
        std::cout << "SerializableScanStatistic Serialization Failed" << std::endl;
        return;
    }
    std::ofstream fout(statisticDir + "/statistic.json");
    if (!fout.is_open()) {
        ERRLOG("failed to open statistic.json");
        return;
    }
    fout << jsonStr << std::endl;
    fout.close();
}

int main(int argc, char** argv)
{
    std::string scanConfigJsonPath{};
    std::vector<std::string> enqueuePathList{};
    std::shared_ptr<Scanner> scanner = nullptr;
    std::string statisticDir;
    if (argc < 2) {
        std::cout << "usage: ScannerDemo -c scanConfig.json <enqueuePathList...> -l <logpath> -s <statistic outdir>";
    }
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-c" && i + 1 < argc) {
            scanConfigJsonPath = std::string(argv[i + 1]);
            ++i;
        }
        else if (std::string(argv[i]) == "-l" && i + 1 < argc) { // log path
            LOG_PATH = std::string(argv[i + 1]);
            ++i;
        }
        else if (std::string(argv[i]) == "-L" && i + 1 < argc) { // log level
            LOG_LEVEL_STR = std::string(argv[i + 1]);
            ++i;
        }
        else if (std::string(argv[i]) == "-s" && i + 1 < argc) { // statistic
            statisticDir = std::string(argv[i + 1]);
            ++i;
        }
        else if (std::string(argv[i]) == "--prefix" && i + 1 < argc) {
            PREFIX_PATH_MAPPER_STRING_VALUE = std::string(argv[i + 1]);
            ++i;
        }
        else if (std::string(argv[i]) == "--infix" && i + 1 < argc) {
            INFIX_PATH_MAPPER_STRING_VALUE = std::string(argv[i + 1]);
            ++i;
        }
        else if (std::string(argv[i]) == "--infixpos" && i + 1 < argc) {
            INFIX_PATH_MAPPER_POS_VALUE = std::atoi(argv[i + 1]);
            ++i;
        }
        else if (std::string(argv[i]) == "--readmeta" && i + 1 < argc) {
            std::string metaDir = std::string(argv[i + 1]);
            TestReadMeta(metaDir);
            return 0;
        }
        else { /* Recognize As Enqueue List */
            std::string enqueuePath = std::string(argv[i]);
            std::cout << "Enqueue: " << enqueuePath << std::endl;
            enqueuePathList.push_back(enqueuePath);
        }
    }
    std::cout << "Start Scanner Demo" << std::endl;
    InitLog();
    if (!CheckScanConfigPath(scanConfigJsonPath)) {
        return -1;
    }
    if (!StartScanner(scanConfigJsonPath, scanner, enqueuePathList)) {
        ERRLOG("=== Start Scanner Failed ===");
        std::cout << "Start Scanner Failed" << std::endl;
        return -1;
    }
    MonitorScanner(scanner);
    if (scanner != nullptr && !statisticDir.empty()) {
        ScanStatistics stats = scanner->GetStatistics();
        WriteScanStatisticJson(stats, statisticDir);
    }
    if (scanner != nullptr) {
        ScanStatistics stats = scanner->GetStatistics();
        std::cout << "Totally Dirs: " << stats.mTotDirs << ", Files: " << stats.mTotFiles << std::endl;
        INFOLOG("Scanner Destroying!");
        scanner->Destroy();
        scanner.reset();
    }
    INFOLOG("=== Scanner Completed! ===");
    std::cout << "Scan Complete." << std::endl;
    return 0;
}
#endif
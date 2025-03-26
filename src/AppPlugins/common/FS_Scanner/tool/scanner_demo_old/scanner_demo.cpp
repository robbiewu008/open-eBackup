/*
* Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
* Description: 基于快照的挂载克隆文件系统的文件扫描
* Author: z30020916
* Create: 2021-12-21
*/

#include <iostream>
#include <unordered_map>
#include "ScannerInterface.h"
#include "NasConfigReader.h"
#include "common/Log.h"
#include "common/Path.h"
#include "config_parser/ConfigIniReader.h"
#include "ControlFileGenerator.h"
#include "DiffService.h"

const auto MODULE = "scanner_demp";
const std::string META_HEADER_VERSION_V10 = "1.0";
const std::string META_HEADER_VERSION_V20 = "2.0";

using namespace NasScannerNamespace;
using namespace interface;

const std::string LOG_NAME = "scanner_demo.log";
const std::string LOG_PATH = "/home/zhouyang/demo/log/";

void InitLog(int argc, char** argv)
{
    CPath::GetInstance().Init(argv[0]);
    int iLogLevel = 1;
    int iLogCount = ConfigReader::getInt(string(MS_CFG_GENERAL_SECTION), string(MS_CFG_LOG_COUNT));
    int iLogMaxSize = ConfigReader::getInt(string(MS_CFG_GENERAL_SECTION), string(MS_CFG_LOG_MAX_SIZE));
    unsigned int runShellType = ConfigReader::getUint(
        string(MS_CFG_MICROSERVICE_SECTION), string(MS_CFG_RUN_SHELL_BY_BOOST));
    ConfigReader::SetLogLevelAndSslConfigFromK8s(false);
    CLogger::GetInstance().Init(
        LOG_NAME.c_str(), LOG_PATH, iLogLevel, iLogCount, iLogMaxSize, runShellType);
    std::cout << "Log path: " << LOG_PATH << std::endl;
    return;
}

void ReportProgress(int i)
{
    std::cout << "progress: " << i << std::endl;
}

// 全量扫描
void ScanTestFull(std::string nasIp ,std::string nasShare)
{
    ScanDiffInput input;

    input.taskId = "task111";
    // appType is 0 and isFull is true, do full scan
    // appType is 0 and isFull is false, do inc scan
    input.appType = 1;
    input.needIndex = false;           // 是否生成RFI文件
    input.isFull = true;
    input.diffFilePath = "/home/zhouyang/demo/scan";   // 输出文件路径
    input.timestamp = "";              // 可为空
    input.filterFlag = false;
    input.chainId = "f4a49be9-3212-4119-87f5-eb4c1b260c9e";

    BackupCopy previousCopy {};
    BackupCopy currentCopy;
    currentCopy.copyId = "copyId_111";
    currentCopy.serviceIp = nasIp;
    currentCopy.nasSharePath = nasShare;
    input.lastMetaFilePath = "";       // appType=1时使用(归档场景)
    std::cout << "full scan " << nasIp << ":/" << nasShare << std::endl;
    input.currentCopy = currentCopy;

    ScanDiffOutput output;
    ScannerInterface scan;
    ReportFunc callbackFunc;
    callbackFunc = ReportProgress;
 
    scan.ScanDiff(input, output, callbackFunc);
}

// 增量扫描
void ScanTestInc(std::string nasIp, std::string nasShare, std::string nasIp2, std::string nasShare2, std::string timestamp)
{
    ScanDiffInput input;

    input.taskId = "task111";
    // appType is 0 and isFull is true, do full scan
    // appType is 0 and isFull is false, do inc scan
    input.appType = 0;
    input.needIndex = true;            // 是否生成RFI文件
    input.isFull = false;
    input.diffFilePath = "/home/shuai/diffPath";   // 输出文件路径
    input.timestamp = timestamp;          // previousCopy的时间戳

    BackupCopy previousCopy;
    previousCopy.copyId = "copyId_000";
    previousCopy.serviceIp = nasIp;
    previousCopy.nasSharePath = nasShare;
    BackupCopy currentCopy;
    currentCopy.copyId = "5097b20c-5790-40e6-aea9-19254efd34ed";
    currentCopy.serviceIp = nasIp2;
    currentCopy.nasSharePath = nasShare2;
    input.lastMetaFilePath = "";       // appType=1时使用(归档场景)
    std::cout << "inc scan " << nasIp << ":/" << nasShare << " " << nasIp2 << ":/" << nasShare2 << std::endl;
    input.previousCopy = previousCopy;
    input.currentCopy = currentCopy;

    ScanDiffOutput output;
    ScannerInterface scan;
    ReportFunc callbackFunc;
    callbackFunc = ReportProgress;
 
    scan.ScanDiff(input, output, callbackFunc);
}

bool ReportRFICallback(string jobId, string subJobId,
    NasScannerNamespace::RFIGenerationCallBackParam param, double progress, bool isFailed)
{
    // ActionResult result;
    // LogDetail logDetail;
    // logDetail.__set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
    //         std::chrono::system_clock::now().time_since_epoch()).count());
    // logDetail.__set_level(JobLogLevel::TASK_LOG_INFO);
    // logDetail.__set_description("generate a rfi file success");
    // std::vector<LogDetail> logDetailList;
    // logDetailList.push_back(logDetail);
    // SubJobDetails subJobDetails;
    // subJobDetails.__set_jobId(jobId);
    // subJobDetails.__set_subJobId(subJobId);
    // subJobDetails.__set_logDetail(logDetailList);
    // if (static_cast<int>(progress) == 1) {
    //     subJobDetails.__set_progress(FINISHPROGRESS);
    //     subJobDetails.__set_jobStatus(SubJobStatus::COMPLETED);
    // } else {
    //     int32_t iprogress = progress * FINISHPROGRESS;
    //     subJobDetails.__set_progress(iprogress);
    //     subJobDetails.__set_jobStatus(SubJobStatus::RUNNING);
    // }

    // string extendInfo {};
    // if (!param.copyId.empty()) {
    //     if (!JsonHelper::StructToJsonString(param, extendInfo)) {
    //         HCP_Log(ERR, MODULE) << "convert RFI call back param json to string fail" << HCPENDLOG;
    //     }
    //     HCP_Log(INFO, MODULE) << "Report RFI struct :" << WIPE_SENSITIVE(extendInfo) << HCPENDLOG;
    //     subJobDetails.__set_extendInfo(extendInfo);
    // }

    // JobService::ReportJobDetails(result, subJobDetails);
    // log subdetails
    // Json::Value jsonValue;
    // StructToJson(subJobDetails, jsonValue);
    // HCP_Log(DEBUG, MODULE) << "Report job details : " << WIPE_SENSITIVE(jsonValue) << HCPENDLOG;
    return true;
}

// 生成control file 全量GetDirCacheFileName
void TestGenerateControlFileFull()
{
    string metaFilePath = "/home/shuai/test_gen/meta1/latest";
    string preMetaFilePath = "/home/shuai/test_gen/meta/latest";
    string controlFilePath = "/home/shuai/test_gen/rfi";
    DiffService incObj;
    std::vector<string> indexPath {"/home/shuai/test_gen/rfi_out"};
    std::unordered_map<string, string> pathMap {
        {"curMetaPath" , metaFilePath},
        {"preMetaPath" , preMetaFilePath},
        {"workDir" , controlFilePath}
    };
    std::unordered_map<string, string> businessMap {
        {"copy_id" , "copy_id"},
        {"job_id" , "job_id"},
        {"subjob_id" , "subjob_id"}
    };
    ReportRFI rfiCallBack = ReportRFICallback;
    // incObj.Init();
    incObj.DiffScan(pathMap, indexPath, rfiCallBack, businessMap);  // 初始化control file
    // incObj.DiffScan(pathMap);

    // incObj.IncScan();   // 写control file, metaPath
    // incObj.CloseIncrementalScanFiles(); // 
    return;
}

// 生成 control file 增量
void TestGenerateControlFileInc()
{
    
}

void TestZipFile()
{
    // vecor<string> params, outputList, stderroutputList;
    // stringstream outstring;
    // atomic<unsigned int> runShellType {1};
    // string strCmd = " zip -qr /home/shuai/diffPath/scan/previous/metafile.zip /home/shuai/diffPath/scan/previous/*";

    // int ret = BaseRunShellCmdWithOutputWithOutLock("Log", 0, strCmd, params, outputList, stderroutputList, outstring,
    //     runShellType);

    // if (ret != 0) {
    //     HCP_Log(INFO, MODULE) << "run shell fail!" << outstring.str() << HCPENDLOG;
    //     return;
    // }

}

string CheckMetaFileVersion(const string& metaFilePath)
{
    std::vector<string> fileList;
    ScannerUtils::GetFileListInDirectory(metaFilePath, fileList);
    for (string file : fileList) {
        cout << file << endl;
        if (file.find("xmeta") != string::npos) {
            return META_HEADER_VERSION_V20;
        }
    }
    return META_HEADER_VERSION_V10;
}

int main(int argc, char** argv)
{
    HCP_Log(INFO, MODULE) << "Init nas confing" << HCPENDLOG;
    NasScannerNamespace::InitScannerNasConfig();
    std::cout << "start: " << "[" << argv[0] << "]" << "\n";

    InitLog(argc, argv);

    HCP_Log(INFO, MODULE) << "Start Scanner demo." << HCPENDLOG;
    // Log()
    std::string nasIp = std::string(argv[1]);
    std::string nasShare = std::string(argv[2]);
    std::string nasIp2;
    std::string nasShare2;
    std::string timestamp;
    if (argv[3] != nullptr) {
        nasIp2 = std::string(argv[3]);
        nasShare2 = std::string(argv[4]);
        timestamp = std::string(argv[5]);
    }
    if (argv[3] != nullptr) {
        ScanTestInc(nasIp, nasShare, nasIp2, nasShare2, timestamp);
        std::cout << "end\n" << endl;
        return 0;
    }
    ScanTestFull(nasIp, nasShare);

    // std::cout << "end\n";

    // ScanTestFull();

    // zip file

    // TestZipFile();

    // TestGenerateControlFileFull();

    // TestGenerateControlFileInc();

    // string str = CheckMetaFileVersion("/home/shuai/test_gen/meta/latest");
    // cout << str << endl;

    std::cout << "end\n";
}

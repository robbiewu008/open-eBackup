#include "securecom/RootCaller.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common/File.h"
#include "message/curlclient/DmeRestClient.h"
#include "message/curlclient/RestClientCommon.h"
#include "pluginfx/ExternalPluginManager.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "taskmanager/externaljob/PluginSubPostJob.h"

namespace {
    constexpr mp_int32 MAIN_STATUS_ABORT_FAILED = 5;
    constexpr mp_int32 MAIN_STATUS_ABORTED = 4;
    constexpr mp_int32 MAIN_STATUS_ABORTING = 2;
    constexpr mp_int32 MAIN_STATUS_FAILED = 6;
    constexpr mp_int32 SUBJOB_MINI_SIZE = 1;
    constexpr int64_t DATA_REPO_TYPE = 1;
    constexpr int64_t META_REPO_TYPE = 0;
    constexpr int64_t LOG_REPO_TYPE = 3;
    
    // E6000设备即时挂载，需要扫描副本目录用于创建文件克隆，写入扫描结果时，需要提权到root执行
    // root执行会校验目录，修改时需要同步修改ROOT_COMMAND_WRITE_SCAN_RESULT命令字的定义
#ifdef WIN32
    static const mp_int32 WRITE_SCAN_RESULT_PARAM_NUMBER = 3;
    static const mp_int32 PATH_SEPARATOR_COUNT_THREE = 3;
    static const mp_int32 PATH_SEPARATOR_COUNT_TWO = 2;
#endif
    static const mp_string DATA_DIR_NAME = "Data";
    static const mp_string META_DIR_NAME = "Meta";
    static const mp_string RECORD_FILE_NAME = "RecordFile.txt"; // 保存扫描出来的文件
    static const mp_string RECORD_DIR_NAME = "RecordDir.txt";   // 保存扫描出来的目录
    static const mp_string META_NAME = "Context_Global_MD";
    static const mp_string META_DIR = "/meta";
}
namespace AppProtect {
size_t FindStrPos(const mp_string str, char c, int n)
{
    size_t pos = str.find(c);
    for (int i = 1; i < n; i++) {
        pos = str.find(c, pos + 1);
        if (pos == std::string::npos) {
            return std::string::npos;
        }
    }
    return pos;
}

mp_int32 PluginSubPostJob::ExecBackupJob()
{
    ActionResult ret;
    BackupJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Backup post job, jobId=%s, subJobId=%s, wholeResult=%d.",
        subJobParam.jobId.c_str(), subJobParam.subJobId.c_str(), mp_int32(m_wholeJobResult));
    ProtectServiceCall(&ProtectServiceIf::AsyncBackupPostJob, ret, jobParam, subJobParam, m_wholeJobResult);
    mp_int32 deleteQos = DeleteQosStrategy();
    if (deleteQos != MP_SUCCESS) {
        ERRLOG("Failed to delete the qos policy, ret=%d, jobId=%s, subJobId=%s.", deleteQos, m_data.mainID.c_str(),
            m_data.subID.c_str());
    }
    return ret.code;
}

bool PluginSubPostJob::IsPostScanValid()
{
    // 后置任务扫描Data仓库仅非Windows下应用需要
    mp_string pluginName;
    mp_int32 iRet =
        ExternalPluginManager::GetInstance().GetParseManager()->GetPluginNameByAppType(m_data.appType, pluginName);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Fail to get plugin name for apptype=%s.", m_data.appType.c_str());
        return false;
    }

    // E6000设备即时挂载，需要扫描副本目录用于创建文件克隆，写入扫描结果时，需要提权到root执行
    // root执行会校验目录，FilePlugin在AIX和solaris以/mnt开头，如果放开FilePlugin，ROOT_COMMAND_WRITE_SCAN_RESULT需要同步修改检验开头目录
    INFOLOG("The pluginName is %s, app type is %s.", pluginName.c_str(), m_data.appType.c_str());
    if (m_wholeJobResult == JobResult::type::SUCCESS) {
        INFOLOG("The value of the m_wholeJobResult is SUCCESS.");
        if (pluginName == "NasPlugin" || pluginName == "FilePlugin" || pluginName == "" || pluginName == "ObsPlugin") {
            INFOLOG("Plugin no need post scan.");
            return false;
        }
        if (!m_isAgentNeedScan) {
            INFOLOG("No need post scan.");
            return false;
        }
        return true;
    }
    return false;
}

mp_int32 PluginSubPostJob::ScanAndRecordFile()
{
    INFOLOG("ScanAndRecordFile begin");
    AppProtect::ScanRepositories scanRepositories;
    BackupJob backupJob;
    JsonToStruct(m_data.param, backupJob);
    ProtectServiceNormalCall(&ProtectServiceIf::QueryScanRepositories, scanRepositories, backupJob);
    if (scanRepositories.scanRepoList.empty()) {
        ERRLOG("QueryScanRepositories Failed or the path is empty, enter the NormalGetScanRepositories.");
        if (NormalGetScanRepositories(scanRepositories) != MP_SUCCESS) {
            ERRLOG("Can not get the scan plugins path, record file failed");
            return MP_FAILED;
        }
    }
    if (GetScanResult(scanRepositories) != MP_SUCCESS) {
        ERRLOG("Get Scan result failed.");
        return MP_FAILED;
    }
    INFOLOG("Scan finish.");
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::GetScanResult(const AppProtect::ScanRepositories &scanRepositories)
{
    std::vector<mp_string> vecDataFolderPath;
    std::vector<mp_string> vecDataFilePath;
    std::vector<mp_string> vecMetaFolderPath;
    std::vector<mp_string> vecMetaFilePath;
    INFOLOG("Scan result save path is %s", scanRepositories.savePath.c_str());
    for (int i = 0; i < scanRepositories.scanRepoList.size(); ++i) {
        AppProtect::RepositoryPath repoPath = scanRepositories.scanRepoList[i];
        INFOLOG("Will scan repo type %d, scan path %s.", repoPath.repositoryType, repoPath.scanPath.c_str());
        int64_t rType = repoPath.repositoryType;
        if ((rType != META_REPO_TYPE && rType != DATA_REPO_TYPE && rType != LOG_REPO_TYPE) ||
            repoPath.scanPath.size() == 0) {
            continue;
        }

        std::vector<mp_string> vecFolderPath;
        std::vector<mp_string> vecFilePath;
        if (DoScan(repoPath.scanPath, vecFolderPath, vecFilePath) != MP_SUCCESS) {
            ERRLOG("Scan result failed, path %s.", repoPath.scanPath.c_str());
            return MP_FAILED;
        }
        if (vecFolderPath.size() == 0 && vecFilePath.size() != 0) {   // need make sure folder not empy.
            vecFolderPath.push_back(repoPath.scanPath);
        }

        INFOLOG("Scan result: per folder size %d, file size %d.", vecFolderPath.size(), vecFilePath.size());
        if (repoPath.repositoryType == META_REPO_TYPE) {
            vecMetaFolderPath.insert(vecMetaFolderPath.end(), vecFolderPath.begin(), vecFolderPath.end());
            vecMetaFilePath.insert(vecMetaFilePath.end(), vecFilePath.begin(), vecFilePath.end());
        } else {
            vecDataFolderPath.insert(vecDataFolderPath.end(), vecFolderPath.begin(), vecFolderPath.end());
            vecDataFilePath.insert(vecDataFilePath.end(), vecFilePath.begin(), vecFilePath.end());
        }
    }
    INFOLOG("Scan result: Data folder size %d, file size %d; Meta folder size %d, file size %d.",
        vecDataFolderPath.size(), vecDataFilePath.size(), vecMetaFolderPath.size(), vecMetaFilePath.size());
    if (SaveScanResult(scanRepositories.savePath, vecMetaFolderPath, vecMetaFilePath, META_DIR_NAME) == MP_SUCCESS &&
        SaveScanResult(scanRepositories.savePath, vecDataFolderPath, vecDataFilePath, DATA_DIR_NAME) == MP_SUCCESS) {
        INFOLOG("Get scan result and save successfully.");
        return MP_SUCCESS;
    }
    ERRLOG("Failed to save the result.");
    return MP_FAILED;
}

mp_int32 PluginSubPostJob::DoScan(mp_string& scanPath,
    std::vector<mp_string>& vecFolders, std::vector<mp_string>& vecFiles)
{
    std::vector<mp_string> vecResult;
#ifndef WIN32
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCAN_DIR_FILE, scanPath, &vecResult);
#else
    mp_int32 iRet = ScanForInstantlyMountWin(scanPath, vecResult);
#endif
    if (iRet != MP_SUCCESS) {
        WARNLOG("ScanDirAndFile failed, iRet is:%d, scannedPath is:%s.", iRet, scanPath.c_str());
        return MP_FAILED;
    }
    iRet = ExtractScanResult(vecResult, vecFolders, vecFiles);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Extract scan result failed, iRet is:%d.", iRet);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

#ifdef WIN32
mp_string PluginSubPostJob::GetMountPublicPath()
{
    mp_string mountPublicPath = "";
    if (CConfigXmlParser::GetInstance().GetValueString(CFG_MOUNT_SECTION,
        CFG_WIN_MOUNT_PUBLIC_PATH, mountPublicPath) != MP_SUCCESS || mountPublicPath == "") {
        WARNLOG("Failed to get CFG_WIN_MOUNT_PUBLIC_PATH.");
        mountPublicPath = "C:\\mnt\\databackup\\";
    }
    INFOLOG("mountPublicPath from cfg file: %s.", mountPublicPath.c_str());
    if (CMpFile::DirExist(mountPublicPath.c_str())) {
        // 路径存在则直接使用
        return mountPublicPath;
    }
    // 路径不存在，检查对应盘符是否存在，否则转为系统盘符再使用
    mp_string targetDiskName = mountPublicPath.substr(0, 1);
    targetDiskName = targetDiskName + ":\\";
    if (CMpFile::DirExist(targetDiskName.c_str())) {
        // 盘符存在则直接使用
        return mountPublicPath;
    }
    // 盘符不存在，先修改为C盘并改为系统盘符后使用
    mountPublicPath[0] = 'C';
    mountPublicPath = GetSystemDiskChangedPathInWin(mountPublicPath);
    INFOLOG("mountPublicPath is: %s.", mountPublicPath.c_str());
    return mountPublicPath;
}

mp_int32 PluginSubPostJob::ScanForInstantlyMountWin(mp_string& path, std::vector<mp_string>& vecResult)
{
    std::vector<mp_string> vecFolderPath;
    std::vector<mp_string> vecFilePath;
    mp_int32 iRet = ScanDirAndFileWin(path, vecFolderPath, vecFilePath);
    if (iRet != MP_SUCCESS) {
        WARNLOG("ScanDirAndFile failed.iRet is:%d.", iRet);
        return MP_FAILED;
    }
    INFOLOG("Scan result: folder size %d, file size %d.", vecFolderPath.size(), vecFilePath.size());
    vecResult.insert(vecResult.end(), vecFolderPath.begin(), vecFolderPath.end());
    // 目录和文件以INSTANLY_MOUNT_SCAN_RESULT_SPLIT_STR分隔
    vecResult.push_back(INSTANLY_MOUNT_SCAN_RESULT_SPLIT_STR);
    vecResult.insert(vecResult.end(), vecFilePath.begin(), vecFilePath.end());
    return iRet;
}

mp_int32 PluginSubPostJob::ScanDirAndFileWin(mp_string &rootPath,
    std::vector<mp_string> &rootfolderpath, std::vector<mp_string> &rootfilepath)
{
    std::vector<mp_string> vecfilePath;
    int iRet = GetFilePathWin(rootPath, vecfilePath);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Scan file path failed.iRet is:%d.", iRet);
        return iRet;
    }
    rootfilepath.insert(rootfilepath.end(), vecfilePath.begin(), vecfilePath.end());
 
    // 获取当前目录下的所有的子目录全路径
    std::vector<mp_string> vecfolderPath;
    iRet = GetFolderPathWin(rootPath, vecfolderPath);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Scan Folder path failed.iRet is:%d.", iRet);
        return iRet;
    }
    rootfolderpath.insert(rootfolderpath.end(), vecfolderPath.begin(), vecfolderPath.end());
 
    // 递归扫描子目录下的文件和目录
    for (mp_string folderPath : vecfolderPath) {
        iRet = ScanDirAndFileWin(folderPath, rootfolderpath, rootfilepath);
        if (iRet != MP_SUCCESS) {
            ERRLOG("ScanDirAndFile failed.iRet is:%d.", iRet);
            return iRet;
        }
    }
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::GetFolderPathWin(mp_string &strFolder, std::vector<mp_string> &vecFolderPath)
{
    std::vector<mp_string> folderNameList;
    folderNameList.clear();
    mp_int32 iRet = CMpFile::GetFolderDir(strFolder, folderNameList);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    for (mp_string strName : folderNameList) {
        // 快照可见场景下，.snapshot为应用快照目录，非应用目录，过滤掉，不必放到待扫描目录中
        if (strName == ".snapshot") {
            INFOLOG(".snapshot of %s don't need to scan.", strFolder.c_str());
            continue;
        }
        vecFolderPath.push_back(strFolder + PATH_SEPARATOR + strName);
    }
    return MP_SUCCESS;
}
 
mp_int32 PluginSubPostJob::GetFilePathWin(mp_string &strFolder, std::vector<mp_string> &vecFolderPath)
{
    std::vector<mp_string> folderNameList;
    mp_int32 iRet = CMpFile::GetFolderFile(strFolder, folderNameList);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    for (mp_string strName : folderNameList) {
        vecFolderPath.push_back(strFolder + PATH_SEPARATOR + strName);
    }
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::WriteScanResultForInstantlyMountWin(const std::vector<mp_string> &vecParam)
{
    LOGGUARD("");
    if (vecParam.size() < WRITE_SCAN_RESULT_PARAM_NUMBER) {
        ERRLOG("Param is invalid, size:%d.", vecParam.size());
        return MP_FAILED;
    }
    mp_string savePrePath = vecParam.front();
    mp_string savePath = vecParam[1];
    mp_string filePath = vecParam[2];
    DBGLOG("param1: %s, param2: %s param3: %s.", savePrePath.c_str(), savePath.c_str(), filePath.c_str());
    std::vector<mp_string> vecFilePath;
    if (vecParam.size() == WRITE_SCAN_RESULT_PARAM_NUMBER) {
        // 扫描结果为空时，依然生成文件
        WARNLOG("Scan result is empty.");
    } else {
        vecFilePath.assign(vecParam.begin() + WRITE_SCAN_RESULT_PARAM_NUMBER, vecParam.end());
    }
    if (!CMpFile::DirExist(savePrePath.c_str()) && CMpFile::CreateDir(savePrePath.c_str()) != MP_SUCCESS) {
        ERRLOG("Failed to create savePrePath %s.", savePrePath.c_str());
        return MP_FAILED;
    }
    if (!CMpFile::DirExist(savePath.c_str()) && CMpFile::CreateDir(savePath.c_str()) != MP_SUCCESS) {
        ERRLOG("Failed to create savePath %s.", savePath.c_str());
        return MP_FAILED;
    }
    // 检查写入文件路径和目录是否有效
    mp_int32 iRet = CheckPathIsValidWin(filePath);
    if (iRet != MP_SUCCESS) {
        ERRLOG("filepath %s is invalid.", filePath.c_str());
        return MP_FAILED;
    }
    CIPCFile::WriteFile(filePath, vecFilePath);
    INFOLOG("Write %s succ.", filePath.c_str());
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::CheckPathIsValidWin(const mp_string &filePath)
{
    // E6000设备即时挂载，需要扫描副本目录用于创建文件克隆，写入扫描结果时，需要提权到root执行
    // root执行会校验目录，修改时需要同步修改PluginSubPostJob::ScanAndRecordFile()和PrepareFileSystem定义
    mp_string MOUNT_PUBLIC_PATH = GetMountPublicPath();  // 挂载目录的前置
    mp_string realPath = filePath;
    // FormattingPath的目录不存在会返回失败，当前函数只归一化目录，不判断返回值
    (mp_void) CMpString::FormattingPath(realPath);
    DBGLOG("Format path is %s, file ath %s.", realPath.c_str(), filePath.c_str());
    
    if (realPath.find(MOUNT_PUBLIC_PATH) != 0) {
        ERRLOG("realPath %s isn't begin with %s.", realPath.c_str(), MOUNT_PUBLIC_PATH.c_str());
        return MP_FAILED;
    }

    mp_string fileName = BaseFileName(realPath);
    if ((fileName != DATA_DIR_NAME + RECORD_FILE_NAME) && (fileName != DATA_DIR_NAME + RECORD_DIR_NAME) &&
        (fileName != META_DIR_NAME + RECORD_FILE_NAME) && (fileName != META_DIR_NAME + RECORD_DIR_NAME)) {
        ERRLOG("File name %s isn't fix file name.", fileName.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

#endif
mp_int32 PluginSubPostJob::NormalGetScanRepositories(AppProtect::ScanRepositories &scanRepositories)
{
    Json::Value jsonData = m_data.param;
    for (Json::ArrayIndex index = 0; index < jsonData["repositories"].size(); ++index) {
        DBGLOG("NormalScanAndRecordFile, index is %d, the jsondata type is %d, path size is %d",
            index, jsonData["repositories"][index]["type"].asInt(), jsonData["repositories"][index]["path"].size());
        if ((jsonData["repositories"][index]["type"] != META_REPO_TYPE &&
                jsonData["repositories"][index]["type"] != DATA_REPO_TYPE) ||
            jsonData["repositories"][index]["path"].empty()) {
            continue;
        }
        AppProtect::RepositoryPath repoPath;
        for (Json::ArrayIndex index1 = 0; index1 < jsonData["repositories"][index]["path"].size(); ++index1) {
            mp_string path = jsonData["repositories"][index]["path"][index1].asString();
            INFOLOG("Scan path is:%s.", path.c_str());
            if (!CMpFile::DirExist(path.c_str())) {
                continue;
            }
            repoPath.scanPath = path;
            if (jsonData["repositories"][index]["type"] == META_REPO_TYPE) {
                repoPath.repositoryType = RepositoryDataType::type::META_REPOSITORY;
                GetScanSavePath(path, scanRepositories);
            } else {
                repoPath.repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
            }
            break;
        }
        if (!repoPath.scanPath.empty()) {
            scanRepositories.scanRepoList.push_back(repoPath);
            INFOLOG("The repoPath with repositoryType %lld, scanPath %s has been added.",
                repoPath.repositoryType, repoPath.scanPath.c_str());
        } else {
            ERRLOG("The mount point %s can not be accessed.",
                jsonData["repositories"][index]["path"][0].asString().c_str());
        }
    }
    if (scanRepositories.scanRepoList.size() == 0) {
        ERRLOG("Can not get the scan plugins path.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

void PluginSubPostJob::GetScanSavePath(const mp_string &path, AppProtect::ScanRepositories &scanRepositories)
{
    if (scanRepositories.savePath.empty()) {
        scanRepositories.savePath = path;
    }
}

mp_int32 PluginSubPostJob::ExtractScanResult(std::vector<mp_string> &vecResult, std::vector<mp_string> &vecFolderPath,
    std::vector<mp_string> &vecFilePath)
{
    // 解析扫描结果，分隔字符串前的放入vecFolderPath，后面的放入vecFilePath
    if (!vecResult.empty()) {
        auto it = std::find(vecResult.begin(), vecResult.end(), INSTANLY_MOUNT_SCAN_RESULT_SPLIT_STR);
        if (it != vecResult.end()) {
            vecFolderPath.insert(vecFolderPath.end(), vecResult.begin(), it);
            vecFilePath.insert(vecFilePath.end(), it + 1, vecResult.end());
        } else {
            WARNLOG("Scan result is invalid.");
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::SaveScanResult(const mp_string &savePath, std::vector<mp_string> &vecFolderPath,
    std::vector<mp_string> &vecFilePath, const mp_string &type)
{
    mp_string savePreDir = savePath;
    if (Job::IsLogBackupJob()) {
        savePreDir = savePreDir + PATH_SEPARATOR + "meta";
        INFOLOG("Job is Log Backup mode, the savePreDir is %s", savePreDir.c_str());
    }
    mp_string saveDir = savePreDir + PATH_SEPARATOR + m_data.mainID;
    INFOLOG("Scan result save to %s", saveDir.c_str());
    mp_string RecordFolderPath = saveDir + PATH_SEPARATOR + type + RECORD_DIR_NAME;
    mp_string RecordFilePath = saveDir + PATH_SEPARATOR + type + RECORD_FILE_NAME;
    TruncateScanResult(vecFolderPath);
    TruncateScanResult(vecFilePath);

    vecFolderPath.insert(vecFolderPath.begin(), RecordFolderPath);
    vecFolderPath.insert(vecFolderPath.begin(), saveDir);
    vecFolderPath.insert(vecFolderPath.begin(), savePreDir);
#ifdef WIN32
    mp_int32 iRet = WriteScanResultForInstantlyMountWin(vecFolderPath);
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.ExecEx((mp_int32)ROOT_COMMAND_WRITE_SCAN_RESULT, vecFolderPath, nullptr);
#endif
    if (iRet != MP_SUCCESS) {
        WARNLOG("Write dir result failed.iRet is:%d.", iRet);
        return MP_FAILED;
    }

    vecFilePath.insert(vecFilePath.begin(), RecordFilePath);
    vecFilePath.insert(vecFilePath.begin(), saveDir);
    vecFilePath.insert(vecFilePath.begin(), savePreDir);
#ifdef WIN32
    iRet = WriteScanResultForInstantlyMountWin(vecFilePath);
#else
    iRet = rootCaller.ExecEx((mp_int32)ROOT_COMMAND_WRITE_SCAN_RESULT, vecFilePath, nullptr);
#endif
    if (iRet != MP_SUCCESS) {
        WARNLOG("Write file result failed.iRet is:%d.", iRet);
        return MP_FAILED;
    }
    INFOLOG("RecordFolderPath is %s, RecordFilePath is %s", RecordFolderPath.c_str(), RecordFilePath.c_str());
    vecFolderPath.clear();
    vecFilePath.clear();
    return MP_SUCCESS;
}


#ifndef WIN32
mp_int32 PluginSubPostJob::TruncateScanResult(std::vector<mp_string> &vecfolderPath)
{
    SubJob subJobParam;
    JsonToStruct(m_data.param, subJobParam);
    mp_string truncateStr =
        "/mnt/databackup/" + m_data.appType + PATH_SEPARATOR + subJobParam.jobId.c_str() + PATH_SEPARATOR + "data";
    mp_string truncateStrMeta =
        "/mnt/databackup/" + m_data.appType + PATH_SEPARATOR + subJobParam.jobId.c_str() + PATH_SEPARATOR + "meta";
    mp_string truncateStrLog =
        "/mnt/databackup/" + m_data.appType + PATH_SEPARATOR + subJobParam.jobId.c_str() + PATH_SEPARATOR + "log";
    for (mp_string &folderPath : vecfolderPath) {
        size_t size = folderPath.size();
        if (folderPath.find(truncateStr) != 0 && folderPath.find(truncateStrMeta) != 0) {
            if (folderPath.find(truncateStrLog) != 0) {
                ERRLOG("folderPath is invalid:%s.", folderPath.c_str());
                return MP_FAILED;
            }
            folderPath = folderPath.substr(truncateStrLog.length(), size);
            continue;
        }
        folderPath = folderPath.substr(truncateStr.length(), size);
    }
    return MP_SUCCESS;
}
#else
mp_int32 PluginSubPostJob::TruncateScanResult(std::vector<mp_string> &vecfolderPath)
{
    Json::Value jsonData = m_data.param;
    mp_string StorageName = jsonData["repositories"][0]["remotePath"].asString();
    INFOLOG("StorageName before is %s", StorageName.c_str());
    if (!Job::IsLogBackupJob()) {
        size_t pos;
        if (StorageName.find("InnerDirectory") == mp_string::npos) {
            pos = FindStrPos(StorageName, '/', PATH_SEPARATOR_COUNT_THREE);
        } else {
            pos = FindStrPos(StorageName, '/', PATH_SEPARATOR_COUNT_TWO);
        }
        StorageName = StorageName.substr(0, pos);
    }
    INFOLOG("StorageName after is %s", StorageName.c_str());
    for (mp_string &folderPath : vecfolderPath) {
        size_t pathPos = FindStrPos(folderPath, '\\', 4);
        if (pathPos == mp_string::npos) {
            ERRLOG("folderPath is invalid:%s.", folderPath.c_str());
            return MP_FAILED;
        }
        folderPath = folderPath.substr(pathPos, folderPath.size());
        replace(folderPath.begin(), folderPath.end(), '\\', '/');
        folderPath = StorageName + folderPath;
    }
}
#endif

mp_int32 PluginSubPostJob::ExecRestoreJob()
{
    ActionResult ret;
    RestoreJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Restore post job, jobId=%s, subJobId=%s, wholeResult=%d.",
        subJobParam.jobId.c_str(), subJobParam.subJobId.c_str(), mp_int32(m_wholeJobResult));
    ProtectServiceCall(&ProtectServiceIf::AsyncRestorePostJob, ret, jobParam, subJobParam, m_wholeJobResult);
    return ret.code;
}

mp_int32 PluginSubPostJob::ExecInrestoreJob()
{
    ActionResult ret;
    RestoreJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);

    INFOLOG("Instance restore post job, jobId=%s, subJobId=%s, wholeResult=%d.",
        subJobParam.jobId.c_str(), subJobParam.subJobId.c_str(),  mp_int32(m_wholeJobResult));
    ProtectServiceCall(&ProtectServiceIf::AsyncInstantRestorePostJob, ret, jobParam, subJobParam, m_wholeJobResult);
    return ret.code;
}

mp_int32 PluginSubPostJob::ReportCompleted()
{
    SubJob subJobParam;
    JsonToStruct(m_data.param, subJobParam);

    SubJobDetails jobDetail;
    jobDetail.__set_jobId(subJobParam.jobId);
    jobDetail.__set_subJobId(subJobParam.subJobId);
    jobDetail.__set_jobStatus(SubJobStatus::type::COMPLETED);
    INFOLOG("MainJob type=%d, jobId=%s.", mp_int32(m_data.mainType), subJobParam.jobId.c_str());

    ActionResult ret;
    AppProtect::AppProtectJobHandler::GetInstance()->ReportJobDetails(ret, jobDetail);
    return ret.code;
}

mp_int32 PluginSubPostJob::GetJobsExecResult()
{
    Json::Value jTaskId;
    jTaskId["task_id"] = m_data.mainID;
    DmeRestClient::HttpReqParam reqParam = {"POST",
        "/v1/dme-unified/tasks/statistic", jTaskId.toStyledString()};
    HttpResponse response;
    reqParam.mainJobId = m_data.mainID;
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(reqParam, response);
    if (iRet != MP_SUCCESS || response.statusCode != SC_OK) {
        RestClientCommon::RspMsg errMsg;
        RestClientCommon::ConvertStrToRspMsg(response.body, errMsg);
        ERRLOG("Post job GetJobsExecResult failed, jobId=%s, errorCode=%s, errorMessage=%s.",
            m_data.mainID.c_str(), errMsg.errorCode.c_str(), errMsg.errorMessage.c_str());
        return MP_FAILED;
    }
    Json::Value rspValue;
    CHECK_FAIL_EX(CJsonUtils::ConvertStringtoJson(response.body, rspValue));
    iRet = GetJobStatus(rspValue);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get job status failed, ret %d jobId=%s subJobId=%s", iRet,
               m_data.mainID.c_str(), m_data.subID.c_str());
    }
    return iRet;
}
mp_int32 PluginSubPostJob::GetJobStatus(const Json::Value& rspValue)
{
    if (!rspValue.isMember("subTask") || !rspValue.isMember("mainTask")) {
        ERRLOG("DME Job status interface rsp miss key subTask or mainTask");
        return MP_FAILED;
    }
    mp_uint32 subNumber = 0;
    GET_JSON_UINT32(rspValue["subTask"], "total", subNumber);
    // subjob total less than or equal 1 which mean no business job or add post job faild so use mainjob status
    if (subNumber <= SUBJOB_MINI_SIZE) {
        mp_int32 mainStatus = 0;
        GET_JSON_INT32(rspValue, "mainTask", mainStatus);
        DBGLOG("Get maintask status is %d", mainStatus);
        if (mainStatus == MAIN_STATUS_ABORTING ||
            mainStatus == MAIN_STATUS_ABORTED ||
            mainStatus == MAIN_STATUS_ABORT_FAILED) {
            m_wholeJobResult = JobResult::type::ABORTED;
        } else if (mainStatus >= MAIN_STATUS_FAILED) {
            m_wholeJobResult = JobResult::type::FAILED;
        } else {
            m_wholeJobResult = JobResult::type::SUCCESS;
        }
    } else {
        mp_int32 nFailed = 0;
        mp_int32 nAbort = 0;
        GET_JSON_INT32(rspValue["subTask"], "failed", nFailed);
        GET_JSON_INT32(rspValue["subTask"], "aborted", nAbort);
        if (nAbort > 0) {
            m_wholeJobResult = JobResult::type::ABORTED;
        } else if (nFailed > 0) {
            m_wholeJobResult = JobResult::type::FAILED;
        } else {
            m_wholeJobResult = JobResult::type::SUCCESS;
        }
    }
    return MP_SUCCESS;
}
Executor PluginSubPostJob::GetPluginCall()
{
    std::map<MainJobType, Executor> ExcuterMap = {
        { MainJobType::BACKUP_JOB, [this](int32_t)
            {
                return ExecBackupJob();
            }
        },
        { MainJobType::RESTORE_JOB, [this](int32_t)
            {
                return ExecRestoreJob();
            }
        },
        { MainJobType::DELETE_COPY_JOB, [this](int32_t)
            {
                return ReportCompleted();
            }
        },
        { MainJobType::LIVEMOUNT_JOB, [this](int32_t)
            {
                return ReportCompleted();
            }
        },
        { MainJobType::CANCEL_LIVEMOUNT_JOB, [this](int32_t)
            {
                return ReportCompleted();
            }
        },
        { MainJobType::BUILD_INDEX_JOB, [this](int32_t)
            {
                return ReportCompleted();
            }
        },
        { MainJobType::CHECK_COPY_JOB, [this](int32_t)
            {
                return ReportCompleted();
            }
        },
        { MainJobType::INSTANT_RESTORE_JOB, [this](int32_t)
            {
                return ExecInrestoreJob();
            }
        }
    };
    auto it = ExcuterMap.find(m_data.mainType);
    if (it != ExcuterMap.end()) {
        return it->second;
    }
    return GetEmptyExcutor();
}

Executor PluginSubPostJob::ExecPostScript()
{
    return [this](int32_t) {
    mp_int32 iRet = MP_SUCCESS;
    CHECK_FAIL_EX(GetJobsExecResult());
    if ((m_wholeJobResult == JobResult::type::ABORTED) || (m_wholeJobResult == JobResult::type::FAILED)) {
        DBGLOG("Begin to exec job failed post script.");
        iRet = Job::ExecPostScript(KEY_POST_FAIL_SCRIPTS);
    } else if (m_wholeJobResult == JobResult::type::SUCCESS) {
        DBGLOG("Begin to exec job succ post script.");
        iRet = Job::ExecPostScript(KEY_POST_SUCC_SCRIPTS);
    }
    DBGLOG("Exec post script finish.");
    return iRet;
    };
}

bool PluginSubPostJob::NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID)
{
    INFOLOG("NotifyPluginReloadImpl jobId=%s suJobId:%s notify plugin reload m_data.status:%d \
        m_pluginPID:%s newPluginPID:%s.",
        m_data.mainID.c_str(), m_data.subID.c_str(), m_data.status, m_pluginPID.c_str(), newPluginPID.c_str());
    if (m_data.status == mp_uint32(SubJobState::UNDEFINE) ||
        m_data.status == mp_uint32(SubJobState::SubJobComplete) ||
        m_data.status == mp_uint32(SubJobState::SubJobFailed)) {
        DBGLOG("No need redo again, jobId=%s, subJobId=%s, status=%d.", m_data.mainID.c_str(), m_data.subID.c_str(),
            m_data.status);
        return true;
    }
    if (!m_pluginPID.empty() && m_pluginPID != newPluginPID) {
        return false;
    }
    return true;
}

mp_int32 PluginSubPostJob::DeleteQosStrategy()
{
    LOGGUARD("");
    if (!m_data.param.isMember("taskParams") || !m_data.param["taskParams"].isObject()) {
        ERRLOG("Json has no taskParams, jobId=%s, subJobId=%s.", m_data.mainID.c_str(),
            m_data.subID.c_str());
        return MP_FAILED;
    }
    auto taskParams = m_data.param["taskParams"];
    if (!taskParams.isMember("qos") || !taskParams["qos"].isObject() || taskParams["qos"].isNull()) {
        return MP_SUCCESS;
    }
    auto dmeClient = DmeRestClient::GetInstance();
    if (dmeClient == nullptr) {
        ERRLOG("Get dme rest client faield, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        return MP_FAILED;
    }
    mp_string url;
    if (m_data.param["taskId"].isString()) {
        url = "/v1/dme-unified/tasks/qos?task_id=" + m_data.param["taskId"].asString();
    }
    DmeRestClient::HttpReqParam param("DELETE", url, "");
    param.mainJobId = m_data.mainID;
    HttpResponse response;
    mp_int32 iRet = dmeClient->SendRequest(param, response);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Send url %s faield, ret=%d, jobId=%s, subJobId=%s.", url.c_str(), iRet, m_data.mainID.c_str(),
            m_data.subID.c_str());
        return iRet;
    }
    if (response.statusCode != SC_OK) {
        RestClientCommon::RspMsg errMsg;
        RestClientCommon::ConvertStrToRspMsg(response.body, errMsg);
        ERRLOG("Delete qos strategy failed, jobId=%s,  subJobId=%s, statusCode=%d, errorCode=%s, errorMessage=%s.",
            m_data.mainID.c_str(),
            m_data.subID.c_str(),
            response.statusCode,
            errMsg.errorCode.c_str(),
            errMsg.errorMessage.c_str());
        return  CMpString::SafeStoi(errMsg.errorCode);
    }
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::NotifyPauseJob()
{
    INFOLOG("After Pause job, set job failed, jobId=%s subJobId=%s", m_data.mainID.c_str(), m_data.subID.c_str());
    SetJobRetry(true);
    ChangeState(SubJobState::SubJobFailed);
    return MP_SUCCESS;
}

mp_int32 PluginSubPostJob::CanbeRunInLocalNode()
{
    LOGGUARD("");
    ActionResult ret;
    if (m_data.mainType == MainJobType::BACKUP_JOB) {
        SetAgentsToExtendInfo(m_data.param);
        BackupJob backupJob;
        JsonToStruct(m_data.param, backupJob);
        SubJob subJob;
        JsonToStruct(m_data.param, subJob);
        ProtectServiceCall(&ProtectServiceIf::AllowBackupSubJobInLocalNode, ret, backupJob, subJob);
        if (ret.code != MP_SUCCESS) {
            ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
            ERRLOG("Check jobId=%s can be allow backup in local node failed, subJobId=%s, error=%d",
                m_data.mainID.c_str(), m_data.subID.c_str(), ret.code);
        }
    } else if (m_data.mainType == MainJobType::RESTORE_JOB) {
        RestoreJob restoreJob;
        JsonToStruct(m_data.param, restoreJob);
        SubJob subJob;
        JsonToStruct(m_data.param, subJob);
        ProtectServiceCall(&ProtectServiceIf::AllowRestoreSubJobInLocalNode, ret, restoreJob, subJob);
        if (ret.code != MP_SUCCESS) {
            ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
            ERRLOG("Check jobId=%s can be allow restore in local node failed, subJobId=%s, error=%d",
                m_data.mainID.c_str(), m_data.subID.c_str(), ret.code);
        }
    } else if (m_data.mainType == MainJobType::CHECK_COPY_JOB) {
        INFOLOG("Check copy post_job can run,jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        CheckCopyJob checkCopyJob;
        SubJob subJob;
        JsonToStruct(m_data.param, checkCopyJob);
        JsonToStruct(m_data.param, subJob);
        ProtectServiceCall(&ProtectServiceIf::AllowCheckCopySubJobInLocalNode, ret, checkCopyJob, subJob);
        if (ret.code != MP_SUCCESS) {
            ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
            ERRLOG("Check jobId=%s can be allow check copy in local node failed, subJobId=%s, error=%d",
                m_data.mainID.c_str(),
                m_data.subID.c_str(),
                ret.code);
        }
    }
    return ret.code;
}
}  // namespace AppProtect

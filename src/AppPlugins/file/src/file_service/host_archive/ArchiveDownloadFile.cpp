/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "ArchiveDownloadFile.h"
#include <thread>

#ifdef WIN32
#include <fileapi.h>
#include "FileSystemUtil.h"
#endif

#include "log/Log.h"
#include "PluginTypes.h"
#include "PluginUtilities.h"
#include "constant/PluginConstants.h"
#include "common/Thread.h"

/* s3 normal
|-- source_policy_0da76c5b988c40babdf2437047b94083_Context_Global_MD
  |-- filemeta
|-- source_policy_0da76c5b988c40babdf2437047b94083_Context
  |-- ***
*/

/* s3 agg
|-- source_policy_0da76c5b988c40babdf2437047b94083_Context_Global_MD/
  |-- 01f26599-626b-4d3c-8d7c-9cf847407d46/
    |-- sqlite
    |-- filemeta
    |-- backup-copy-meta.json

|-- 01f26599-626b-4d3c-8d7c-9cf847407d46/
  |-- ***
*/
// 0da76c5b988c40babdf2437047b94083 -> source id
// 01f26599-626b-4d3c-8d7c-9cf847407d46 -> backup job id

namespace FilePlugin {
namespace {
const auto MODULE = "ArchiveDownloadFile";

const int SUCCESS = 0;
const int FAILED = -1;
const int TAP_REMIND_ERROR = 17;

const int SKD_PREPARE_CODE_NE1 = -1;
const int SKD_PREPARE_CODE_0 = 0;
const int SKD_PREPARE_CODE_1 = 1;
const int SKD_PREPARE_CODE_2 = 2;
const int SKD_PREPARE_CODE_3 = 3;
const int NUMBER2 = 2;

const int PREPARE_ARCHIVE_CLIENT_INTERVAL = 10;
const int GET_CONTROL_FILE_MAX_COUNT = 360;
const int64_t READ_FILELIST_NUM = 100000; // 10w one control file
const int GET_CONTROL_FILE_INTERVAL = 10;

const int CONTROL_FILE_OFFSET_1 = 1;
const int CONTROL_FILE_OFFSET_2 = 2;
const int CONTROL_FILE_OFFSET_3 = 3;
const int CONTROL_FILE_OFFSET_4 = 4;
const int CONTROL_FILE_OFFSET_5 = 5;
const int CONTROL_FILE_OFFSET_6 = 6;

const int BYTE_UNIT = 1024;
const int BASE64_SIZE_NUM = 2;
const std::string ARCHIVE_DOWNLOAD_FOLDER = "temp_archive_restore";
const std::wstring WPATH_PREFIX = LR"(\\?\)";
}

void ArchiveDownloadFile::SetAbort()
{
    m_isAbort = true;
}

inline bool ArchiveDownloadFile::IsAbort()
{
    return m_isAbort;
}

inline bool ArchiveDownloadFile::IsDir(const std::string& name) const
{
#ifdef WIN32
    return name.back() == '\\';
#else
    return name.back() == '/';
#endif
}

std::string ArchiveDownloadFile::GetFileSystemsId()
{
    return m_fsId;
}

std::string ArchiveDownloadFile::GetParentDir()
{
    return m_parentDir;
}

bool ArchiveDownloadFile::Start(const std::string& outputPath, const std::vector<std::string>& pathList)
{
    m_state = ArchiveDownloadState::RUNNING;
    if (!StartDownloadMeta(outputPath, pathList)) {
        // 没有赋值其他状态则设置为失败
        if (m_state == ArchiveDownloadState::RUNNING) {
            m_state = ArchiveDownloadState::FAILED;
        }
        return false;
    }
    m_state = ArchiveDownloadState::SUCCESS;
    return true;
}

bool ArchiveDownloadFile::StartDownloadMeta(const std::string& outputPath, const std::vector<std::string>& pathList)
{
    INFOLOG("Archive download file start");

    if (!InitArchiveClient(pathList)) {
        ERRLOG("Init archive client failed");
        return false;
    }

    std::string checkpoint;
    INFOLOG("Start to get control file from archive server.");
    std::vector<std::string> controlList;
    if (!GetControlFileFromArchive(checkpoint, controlList)) {
        ERRLOG("GetControlFileFromArchive failed.");
        return false;
    }

    INFOLOG("Start to parser control file");
    for (std::string controlfile : controlList) {
#ifdef WIN32
        controlfile = GetCacheRepoRootPath(m_cacheFsPath) + dir_sep + ARCHIVE_DOWNLOAD_FOLDER + controlfile;
#endif
        INFOLOG("parse control file : %s", controlfile.c_str());
        std::map<std::string, ControlFileData> ctrlFileMap; // record file info for one control file
        if (!GetFileListFromCtrl(controlfile, ctrlFileMap)) {
            return false;
        }
        INFOLOG("ctrlFileMapSize : %d", ctrlFileMap.size());
        if (!DownloadFile(outputPath, ctrlFileMap)) {
            ERRLOG("DownloadFile failed controlfile: %s", controlfile.c_str());
            return false;
        }
    }

    CloseClient();
    return true;
}

#ifdef WIN32
std::string ArchiveDownloadFile::GetCacheRepoRootPath(const std::string& cacheFsPath) const
{
    std::string cacheRootPath = cacheFsPath;
    size_t lastSeparatorPos = cacheRootPath.rfind(dir_sep);
    if (lastSeparatorPos != std::string::npos) {
        size_t secondLastSeparatorPos = cacheRootPath.rfind(dir_sep, lastSeparatorPos - 1);
        if (secondLastSeparatorPos != std::string::npos) {
            cacheRootPath = cacheRootPath.substr(0, secondLastSeparatorPos);
        }
    }
    return cacheRootPath;
}
#endif

bool ArchiveDownloadFile::InitArchiveClient(const std::vector<std::string>& pathList)
{
    INFOLOG("Enter init archive client");
    std::string objList;
    for (std::size_t i = 0; i < pathList.size(); ++i) {
        std::string inObj = pathList[i];
        HCP_Log(DEBUG, MODULE) << "objList: " << inObj << HCPENDLOG;
        std::string encodeObj = PluginUtils::Base64Encode(inObj);
        
        objList.append(encodeObj);
        if (i == pathList.size() - 1) {
            break;
        }
        objList.append(",");
    }
    INFOLOG("Copy id: %s, job id: %s, obj list: %s", m_copyId.c_str(), m_jobId.c_str(), objList.c_str());
    if (m_clientHandler->Init(m_copyId, m_jobId, objList) != SUCCESS) {
        ERRLOG("Client init failed");
        return false;
    }

    const auto& [ipList, port, enableSSL] = m_archiveInfo;
    std::string listString;
    for (std::size_t i = 0; i < ipList.size(); i++) {
        listString.append(ipList[i]);
        if (i == ipList.size() -1) {
            break;
        }
        listString.append(",");
    }
    INFOLOG("Archive client connnect ip [%s], port [%d], ssl [%d]",
        listString.c_str(), port, (enableSSL ? 1 : 0));
    if (m_clientHandler->Connect(listString, port, enableSSL) != SUCCESS) {
        ERRLOG("Archive client connect failed");
        return false;
    }
    m_isInit = true;
    INFOLOG("Archive client connect success");

    if (!QueryPrepare()) {
        ERRLOG("QueryPrepare failed");
        return false;
    }

    return true;
}

bool ArchiveDownloadFile::QueryPrepare()
{
    INFOLOG("Enter QueryPrepare");
#ifdef WIN32
    std::string metaFilePath = "/" + m_resourceId + "/" + ARCHIVE_DOWNLOAD_FOLDER; // /temp_archive_restore
#else
    std::string metaFilePath;
#endif
    // m_cacheFsRemotePath is like /Fileset_CacheDataRepository_su0/53b300a663e940c3a7225bfc5ce380b1
    std::string cacheRepoFsName = m_cacheFsRemotePath;
    if (cacheRepoFsName[0] == '/') {
        cacheRepoFsName = cacheRepoFsName.substr(1);
    }
    // 这里是共享名， windows场景也是 "/"
    size_t pos = cacheRepoFsName.find_first_of("/");
    cacheRepoFsName = cacheRepoFsName.substr(0, pos);
    if (m_clientHandler->PrepareRecovery(metaFilePath, m_parentDir, cacheRepoFsName) != SUCCESS) {
        ERRLOG("Archive client prepare recovery failed.");
        return false;
    }
    INFOLOG("Archive client prepare recovery success.");

    int state = 0;
    while (state == 0) {
        DBGLOG("Archive client query 1, prepare in progress ...");
        if (m_clientHandler->QueryPrepareStatus(state) != SUCCESS) {
            ERRLOG("QueryPrepareStatus failed");
            return false;
        }
        DBGLOG("Archive client query success2, prepare in progress ..., %d", state);
        Module::SleepFor(std::chrono::seconds(PREPARE_ARCHIVE_CLIENT_INTERVAL));
    }

    INFOLOG("Archive client prepare finish, state is %d", state);
    if (state == SKD_PREPARE_CODE_2) {
        HCP_Log(INFO, MODULE) << "QueryPrepareStatus state is failed" << HCPENDLOG;
        m_state = ArchiveDownloadState::EMPTY_COPY;
        return false;
    }

    if (state == SKD_PREPARE_CODE_NE1) {
        ERRLOG("Query prepare failed, state is %d", state);
        return false;
    }

    INFOLOG("Query prepare success");
    return true;
}

bool ArchiveDownloadFile::GetControlFileFromArchive(std::string& checkpoint, std::vector<std::string>& controlList)
{
    INFOLOG("Enter GetControlFileFromArchive");
    int getState = 1; // 默认 1 未完成
    std::string control;
    long long outNum {0};
    while (getState != SKD_PREPARE_CODE_2) { // 非全部取完
        int count = 0;
        while (getState == SKD_PREPARE_CODE_0 || getState == SKD_PREPARE_CODE_1) { // 循环调用
            if (IsAbort()) {
                INFOLOG("GetObjList abort job");
                return false;
            }
            if (count > GET_CONTROL_FILE_MAX_COUNT) {
                ERRLOG("GetObjList timeout, checkpoint: %s", checkpoint.c_str());
                return false;
            }
            if (m_clientHandler->GetRecoverObjectList(
                READ_FILELIST_NUM, checkpoint, control, outNum, getState) != SUCCESS) {
                ERRLOG("GetRecoverObjectList return failed");
                return false;
            }
            INFOLOG("GetObjList getState: [%d], checkpoint: [%s], control: [%s], outNum: [%d]",
                getState, checkpoint.c_str(), control.c_str(), outNum);

            if (getState == 0) {
                break;
            }

            if (getState == SKD_PREPARE_CODE_3) { // 失败
                ERRLOG("GetRecoverObjectList status failed, checkpoint: %s", checkpoint.c_str());
                return false;
            }
            Module::SleepFor(std::chrono::seconds(GET_CONTROL_FILE_INTERVAL));
            count++;
        }
         // 本次已完成
        if (getState == SKD_PREPARE_CODE_2 || getState == SKD_PREPARE_CODE_0) {
            controlList.push_back(control);
            INFOLOG("Get a control file %s", control.c_str());
            continue;
        }
    }
    INFOLOG("Exit GetControlFileFromArchive");
    return true;
}

bool ArchiveDownloadFile::GetFileListFromCtrl(const std::string& ctrlFileName,
    std::map<std::string, ControlFileData>& ctrlFileMap)
{
    INFOLOG("Enter GetFileListFromCtrl");
    if (!PluginUtils::IsFileExist(ctrlFileName)) {
        ERRLOG("control file not exist: %s", ctrlFileName.c_str());
        return false;
    }

    std::ifstream file(ctrlFileName);
    if (!file.is_open()) {
        ERRLOG("open file failed, file name: %s", ctrlFileName.c_str());
        return false;
    }
    std::string line;
    while (!file.eof()) {
        std::vector<std::string> strs;
        std::getline(file, line);
        DBGLOG("GetFileListFromCtrl one line: %s", line.c_str());
        if (line.empty()) {
            continue;
        }
        std::stringstream iss(line);
        std::string word;
        while (std::getline(iss, word, ',')) {
            strs.push_back(word);
        }
        if (strs.size() <= CONTROL_FILE_OFFSET_6) {
            ERRLOG("Get line size is %d.", strs.size());
            continue;
        }
        std::string inFile = strs[CONTROL_FILE_OFFSET_2]; // 解码前归档路径
        if (strs.empty() || inFile == "/") {
            continue;
        }

        ControlFileData data {};
        data.type = std::stoi(strs[0]);
        data.fsId = strs[1];
        m_fsId = strs[1];
        // 解码后归档副本上原始路径
        data.fileName = PluginUtils::Base64Decode(inFile);
        DBGLOG("GetFileListFromCtrl decName: %s", data.fileName.c_str());
        // 跳过根目录和空
        if (data.fileName == "/" || data.fileName.empty()) {
            continue;
        }
        data.fileType = std::stoi(strs[CONTROL_FILE_OFFSET_3]);
        // metafile使用全路径 挂载点+文件系统+metafile名
        data.metaInfo.metaFile = data.fsId + "/" + strs[CONTROL_FILE_OFFSET_4];
        data.metaInfo.offset = std::stoll(strs[CONTROL_FILE_OFFSET_5]);
        data.metaInfo.length = std::stoll(strs[CONTROL_FILE_OFFSET_6]);

        ctrlFileMap.emplace(data.fileName, data);
    }
    file.close();
    return true;
}

/*
choose metafile to download
*/
bool ArchiveDownloadFile::DownloadFile(const std::string& outputPath,
    const std::map<std::string, ControlFileData>& ctrlFileMap)
{
    INFOLOG("Enter DownloadFile");
    // ${cache_repo_path}/source_policy_**_Context_Global_MD
    for (const auto& line : ctrlFileMap) {
        std::string fileArchivePath = line.first;
        DBGLOG("File archive path: %s", fileArchivePath.c_str());
        m_fileFullPath = outputPath + fileArchivePath;
#ifdef WIN32
        m_fileFullPath = PluginUtils::ReverseSlash(m_fileFullPath);
#endif
        // dir
        if (IsDir(m_fileFullPath)) {
            m_fileFullPath.pop_back();
            if (!PluginUtils::CreateDirectory(m_fileFullPath)) {
                ERRLOG("mkdir failed: %s", m_fileFullPath.c_str());
                return false;
            }
            continue;
        }
        // file
        if (!OpenFileExistOrNew()) {
            return false;
        }

        if (!ArchiveWriteFile(line.second)) {
            ERRLOG("Write file failed");
            return false;
        }
        // close
#ifdef WIN32
        if (m_fd != INVALID_HANDLE_VALUE) {
            DBGLOG("close file");
            CloseHandle(m_fd);
            m_fd = INVALID_HANDLE_VALUE;
        }
#endif
    }
    return true;
}

bool ArchiveDownloadFile::OpenFileExistOrNew()
{
    INFOLOG("open file: %s", m_fileFullPath.c_str());
#ifdef WIN32
    std::wstring wFilePullPath = Module::FileSystemUtil::Utf8ToUtf16(m_fileFullPath);
    if (wFilePullPath.find(WPATH_PREFIX) != 0) {
        /* already have prefix */
        wFilePullPath = WPATH_PREFIX + wFilePullPath;
    }
    HANDLE hFile = ::CreateFileW(
        wFilePullPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ || FILE_SHARE_WRITE,
        NULL,
        CREATE_NEW,
        0,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        DWORD lastError = GetLastError();
        ERRLOG("Open file failed: %s, error code: %d", m_fileFullPath.c_str(), lastError);
        return false;
    }
    m_fd = hFile;
#else
    int fd = open(m_fileFullPath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    int err = errno;
    if (fd < 0) {
        ERRLOG("Open file failed: %s, error code: %d", m_fileFullPath.c_str(), err);
        return false;
    }
    m_fd = fd;
#endif
    return true;
}

bool ArchiveDownloadFile::ArchiveWriteFile(const ControlFileData& ctrlData)
{
    INFOLOG("Enter ArchiveWriteFile: %s", ctrlData.fileName.c_str());
    int readEnd = 0;
    uint64_t offset = 0;

    do {
        ArchiveStreamGetFileReq req {};
        req.taskID = m_jobId;
        req.archiveBackupId = m_copyId;
        req.fsID = ctrlData.fsId;
        req.filePath = ctrlData.fileName; // 文件归档的路径
        req.fileOffset = offset;
        req.readSize = ARCHIVESTREAM_READ_SIZE_4M ; // 0
        req.maxResponseSize = ARCHIVESTREAM_RESPONSE_SIZE_512K; // 6

        ArchiveStreamGetFileRsq retValue{};
        HCP_Log(DEBUG, MODULE) << "ArchiveStreamGetFileReq req.taskID: " << req.taskID <<
            " req.archiveBackupId: " << req.archiveBackupId << " req.fsID: " << req.fsID <<
            " req.filePath: " << req.filePath << " req.fileOffset: " << req.fileOffset <<
            " req.readSize: " << req.readSize << " req.maxResponseSize: " << req.maxResponseSize << HCPENDLOG;
        int clientError = 0;
        if ((clientError = m_clientHandler->GetFileData(req, retValue)) != SUCCESS) {
            ERRLOG("GetFileData failed: %s, error: %d", m_fileFullPath.c_str(), clientError);
            m_state = (clientError == TAP_REMIND_ERROR) ?
                ArchiveDownloadState::TAP_REMIND : ArchiveDownloadState::FAILED;
            return false;
        }
        HCP_Log(DEBUG, MODULE) << "retValue.fileSize: " << retValue.fileSize <<
            " offset: " << offset << " retValue.readEnd: " << retValue.readEnd << HCPENDLOG;
        readEnd = retValue.readEnd;
        int ret = WriteBufferToFile(retValue.data, offset, retValue.fileSize);
        if (ret != SUCCESS) {
            HCP_Log(ERR, MODULE) << "Write file failed: " << m_fileFullPath << " offset: " << offset << HCPENDLOG;
            free(retValue.data);
            retValue.data = nullptr;
            return false;
        }
        free(retValue.data);
        retValue.data = nullptr;
        offset += retValue.fileSize;
        HCP_Log(DEBUG, MODULE) << "m_dataSize: " << retValue.fileSize / BYTE_UNIT <<
            " retValue.fileSize " << retValue.fileSize << HCPENDLOG;
    } while (readEnd != 1);

    return true;
}

int ArchiveDownloadFile::WriteBufferToFile(const char* buf, uint64_t offset, uint64_t length) const
{
    HCP_Log(DEBUG, MODULE) << "write file: " << m_fileFullPath <<
        " length: " << length << " offset: " << offset << HCPENDLOG;
    if (buf == nullptr) {
        HCP_Log(ERR, MODULE) << "buf is nullptr failed to write: " << m_fileFullPath << HCPENDLOG;
        return FAILED;
    }
    if (length == 0) {
        HCP_Log(DEBUG, MODULE) << "buf is none to write: " << m_fileFullPath << HCPENDLOG;
        return SUCCESS;
    }
#ifdef WIN32
    DWORD writedSize = 0;
    OVERLAPPED ov {};
    ov.Offset = offset;
    BOOL res = WriteFile(m_fd, (LPVOID)(buf), (DWORD)length, &writedSize, &ov);
    if (!res || writedSize != length) {
        DWORD errorCode = GetLastError();
        HCP_Log(ERR, MODULE) << "Failed to write: " << m_fileFullPath << " Error: " << errorCode << HCPENDLOG;
        return errorCode;
    }
#else
    int ret = lseek(m_fd, offset, SEEK_SET);
    int err = errno;
    if (ret == -1) {
        HCP_Log(ERR, MODULE) << "Failed to lseek: " << m_fileFullPath << " Error: " << err << HCPENDLOG;
        return err;
    }
    ret = write(m_fd, buf, length);
    err = errno;
    if (ret == -1) {
        HCP_Log(ERR, MODULE) << "Failed to write: " << m_fileFullPath << " Error: " << err << HCPENDLOG;
        return err;
    }
#endif
    return SUCCESS;
}

void ArchiveDownloadFile::CloseClient() const
{
    INFOLOG("Enter CloseClient");
    if (!m_isInit) {
        INFOLOG("Archive client not init");
        return;
    }
    if (m_clientHandler->Disconnect() != SUCCESS) {
        WARNLOG("Disconnect to archive server error");
    }
}
}
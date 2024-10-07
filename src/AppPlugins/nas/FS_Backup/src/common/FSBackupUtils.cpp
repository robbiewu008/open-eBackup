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
#include "FSBackupUtils.h"
#include <sys/stat.h>
#include <cstdlib>
#include <random>
#include <ctime>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <unordered_map>

#include <sys/stat.h>
#include <fcntl.h>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>

#include "log/Log.h"
#include "ParserStructs.h"

#ifdef WIN32
#include <filesystem>
#include <windows.h>
#include <fstream>
#include "Win32BackupEngineUtils.h"
#else
#include <malloc.h>
#include "PosixUtils.h"
#endif

using namespace std;
using namespace Module;

namespace {
    const int32_t MAX_RANDOM_STR_SIZE = 16;
    const int32_t MAX_DATE_TIME_STR_SIZE = 32;
    const std::string DOUBLE_BACKSLASH = R"(\\)";
    const std::string BACKSLASH = "\\";
    const std::string SLASH = "/";
    const int NUM2 = 2;
}

std::string FSBackupUtils::GetFileName(std::string filePath)
{
    while (!filePath.empty() && filePath.back() == Module::PATH_SEPARATOR[0]) {
        filePath.pop_back();
    }

    auto pos = filePath.rfind(Module::PATH_SEPARATOR);
    if (pos == std::string::npos) {
        return filePath;
    }

    return filePath.substr(pos + 1);
}

string FSBackupUtils::GetParentDir(const std::string &filePath)
{
    size_t fileoffset = filePath.rfind("/", filePath.length());
    if (fileoffset != string::npos) {
        return (filePath.substr(0, fileoffset));
    }
    return "";
}

string FSBackupUtils::GetParentDirV2(const std::string &filePath)
{
    size_t fileoffset = filePath.rfind(Module::PATH_SEPARATOR, filePath.length());
    if (fileoffset != string::npos) {
        return (filePath.substr(0, fileoffset));
    }
    return "";
}

std::string FSBackupUtils::JoinPath(const std::string& prev, const std::string& trail)
{
    std::string prevPath = prev;
    std::string trailPath = trail;
#ifdef WIN32
    std::replace(prevPath.begin(), prevPath.end(), '/', '\\');
    std::replace(trailPath.begin(), trailPath.end(), '/', '\\');
#else
    std::replace(prevPath.begin(), prevPath.end(), '\\', '/');
    std::replace(trailPath.begin(), trailPath.end(), '\\', '/');
#endif
    while (!prevPath.empty() && prevPath.back() == Module::PATH_SEPARATOR[0]) {
        prevPath.pop_back();
    }
    while (!trailPath.empty() && trailPath.front() == Module::PATH_SEPARATOR[0]) {
        trailPath = trailPath.substr(1);
    }
    return prevPath + Module::PATH_SEPARATOR + trailPath;
}

/*
 * To replace the posix call (stat)
 * Can be replaced by std::filesystem if Linux compile pipeline is updated to CXX17
 */
bool FSBackupUtils::Exists(const std::string& path)
{
#ifdef WIN32
    if (!FileSystemUtil::Stat(path)) {
        return false;
    }
    return true;
#else
    struct stat st;
    return lstat(path.c_str(), &st) == SUCCESS;
#endif
}

/* path like C:\, D: is recongnize as windows root directory */
bool FSBackupUtils::IsWindowDriverRoot(std::string pathNormalized)
{
    std::replace(pathNormalized.begin(), pathNormalized.end(), SLASH[0], BACKSLASH[0]);
    while (!pathNormalized.empty() && pathNormalized.back() == BACKSLASH[0]) {
        pathNormalized.pop_back();
    }
    if (pathNormalized.length() == NUM2 && pathNormalized[1] == ':') {
        /* looks like a windows root path */
        return true;
    }
    return false;
}

void FSBackupUtils::RecurseCreateDirectory(const std::string& path)
{
    DBGLOG("create directory: %s", path.c_str());
#ifdef WIN32
    DWORD errorCode = 0;
    if (!FS_Backup::Win32BackupEngineUtils::CreateDirectoryRecursively(path, "", errorCode)) {
        ERRLOG("failed to create windows directory: %s, errorCode: %u", path.c_str(), errorCode);
    }
#else
    if (path.empty() || path[0] != '/') {
        return;
    }
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
        return;
    }
    RecurseCreateDirectory(path.substr(0, path.find_last_of("/")));
    mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
#endif
}

bool FSBackupUtils::RemoveDir(const std::string& path)
{
    DBGLOG("remove dir:%s", path.c_str());
#ifdef WIN32
    return FS_Backup::Win32BackupEngineUtils::RemovePath(path);
#else
    if (!boost::filesystem::exists(path)) {
        return true;
    }
    try {
        boost::filesystem::remove_all(path);
    } catch (const boost::filesystem::filesystem_error &e) {
        ERRLOG("remove_all() exeption: %s, path: %s", e.code().message().c_str(), path.c_str());
        return false;
    }
    return true;
#endif
}

bool FSBackupUtils::CreateFile(const std::string& path)
{
#ifdef WIN32
    ERRLOG("Create file not implemented on windows!");
    return false;
#else
    if (FSBackupUtils::Exists(path)) {
        return true;
    }
    int fd = ::open(path.c_str(), O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        return false;
    }
    ::close(fd);
    return true;
#endif
}

bool FSBackupUtils::RemoveFile(const std::string& path)
{
#ifdef WIN32
    DBGLOG("remove file:%s", path.c_str());
    return FS_Backup::Win32BackupEngineUtils::RemovePath(path);
#else
    if (!boost::filesystem::exists(path)) {
        return true;
    }
    try {
        boost::filesystem::remove(path);
    } catch (const boost::filesystem::filesystem_error &e) {
        ERRLOG("remove() exeption: %s, path: %s", e.code().message().c_str(), path.c_str());
        return false;
    }
    return true;
#endif
}

/* 创建硬链接 */
bool FSBackupUtils::CreateLink(const std::string& linkName, const std::string& targetFile)
{
#ifdef WIN32
    ERRLOG("Create link not implemented on windows!");
    return false;
#else
    if (link(targetFile.c_str(), linkName.c_str()) != 0) {
        ERRLOG("Create link failed, err:%d, target: %s, link: %s", errno, linkName.c_str(), targetFile.c_str());
        return false;
    }
    return true;
#endif
}

bool FSBackupUtils::CreateSymLink(const std::string& symLinkName, const std::string& targetFileOrFolder)
{
#ifdef WIN32
    /* Hint:
     * used in SQLiteDB::Connect to map the dir if path length is greater than MAX_PATH_LEN_SUPPORED_BY_SQLITE,
     * Same issue may also occur on windows aggr backup.
     * Need to be implemented later.
     */
    ERRLOG("CreateSymLink Not Implemented On Windows!");
    return false;
#else
    if (0 != symlink(targetFileOrFolder.c_str(), symLinkName.c_str())) {
        ERRLOG("Create symlink failed, err:%d, target: %s, symlink: %s",
            errno, symLinkName.c_str(), targetFileOrFolder.c_str());
        return false;
    }
    return true;
#endif
}

void FSBackupUtils::RemoveLeadingSlashes(string &str)
{
    while (true) {
        if ((str.length() > 0) && (str[0] == '/')) {
            str.erase(0, 1);
        } else {
            break;
        }
    }
}

void FSBackupUtils::RemoveTrailingSlashes(string &str)
{
    while (true) {
        if ((str.length() > 0) && (str.at(str.length() - 1) == '/')) {
            str = str.substr(0, str.length() - 1);
        } else {
            break;
        }
    }
}

std::string FSBackupUtils::GenerateRandomStr()
{
    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

    std::random_device rd;
    std::mt19937 generator(rd());
    std::shuffle(str.begin(), str.end(), generator);

    return str.substr(0, MAX_RANDOM_STR_SIZE);
}

time_t FSBackupUtils::GetCurrentTime()
{
    time_t currTime = std::time(nullptr);
    return currTime;
}

int64_t FSBackupUtils::GetMilliSecond()
{
    using namespace std::chrono;
    int64_t milli = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return milli;
}

std::string FSBackupUtils::FormatSpeed(uint64_t speed)
{
    float formatSpeed;

    if ((speed >= MB_SIZE) && (speed < GB_SIZE)) {
        formatSpeed = static_cast<float>(speed) / MB_SIZE;
        return FloatToString(formatSpeed) + "MBps";
    } else if ((speed >= GB_SIZE) && (speed < TB_SIZE)) {
        formatSpeed = static_cast<float>(speed) / GB_SIZE;
        return FloatToString(formatSpeed) + "GBps";
    } else if ((speed >= TB_SIZE) && (speed < PB_SIZE)) {
        formatSpeed = static_cast<float>(speed) / TB_SIZE;
        return FloatToString(formatSpeed) + "TBps";
    } else if ((speed >= KB_SIZE) && (speed < MB_SIZE)) {
        formatSpeed = static_cast<float>(speed) / KB_SIZE;
        return FloatToString(formatSpeed) + "KBps";
    } else if (speed >= PB_SIZE) {
        formatSpeed = static_cast<float>(speed) / PB_SIZE;
        return FloatToString(formatSpeed) + "PBps";
    } else {
        return std::to_string(speed) + "Bytes/s";
    }
}

std::string FSBackupUtils::FloatToString(const float &val, const uint8_t &precisson)
{
    stringstream sstream;
    sstream << setiosflags(ios::fixed) << setprecision(precisson) << val;
    return sstream.str();
}

std::string FSBackupUtils::GetDateTimeString(uint64_t timeValue)
{
    std::time_t tempTime = timeValue;
    struct std::tm *timeInfo;
    char buffer[MAX_DATE_TIME_STR_SIZE];
    timeInfo = std::localtime(&tempTime);
    if (std::strftime(buffer, MAX_DATE_TIME_STR_SIZE, "%Y-%m-%d %H:%M:%S", timeInfo) > 0) {
        return std::string(buffer);
    }
    ERRLOG("get datetime string failed, time value = %u", timeValue);
    return "";
}

#ifndef WIN32
mode_t FSBackupUtils::GetUmask()
{
    mode_t tmpMask = umask(0);
    umask(tmpMask);
    return tmpMask;
}
#endif

int FSBackupUtils::GetFileWithSubDirListInDir(const string& dir, vector<string>& fileList, vector<string>& subDirList)
{
    try {
        if (!boost::filesystem::exists(dir)) {
            ERRLOG("dir: %s is not exist", dir.c_str());
            return Module::FAILED;
        }
        for (const auto& iter: boost::filesystem::directory_iterator(dir)) {
            if (boost::filesystem::is_regular_file(iter.status())) {
                fileList.push_back(iter.path().string());
            } else if (boost::filesystem::is_directory(iter.status())) {
                subDirList.push_back(iter.path().string());
            }
        }
    } catch (const boost::filesystem::filesystem_error &e) {
        ERRLOG("exception: %s, %s", dir.c_str(), e.code().message().c_str());
        return Module::FAILED;
    }

    return Module::SUCCESS;
}

string FSBackupUtils::CheckMetaFileVersion(const string& dir)
{
    vector<string> metaFileList;
    vector<string> subDirList;
    int ret = GetFileWithSubDirListInDir(dir, metaFileList, subDirList);
    if (ret != Module::SUCCESS) {
        return META_VERSION_V20;
    }

    for (auto file : metaFileList) {
        if (file.find("xmeta") != string::npos) {
            return META_VERSION_V20;
        }
    }
    INFOLOG("xmeta not found , metafile v1.0");
    return META_VERSION_V10;
}

/*
 * Special file won't be aggregated:
 * Linux/AIX/NFS: fifo file, char file, block file
 * WIN32/SMB: file is ADS or file has ADS
 */
bool FSBackupUtils::IsSpecialFile(uint32_t mode)
{
    // 1. check ADS (windows/smb)
    if (mode == FILE_IS_ADS_FILE || ((mode & FILE_HAVE_ADS) == FILE_HAVE_ADS)) {
        return true;
    }
    // 2. posix special file
#ifndef WIN32
    if (S_ISFIFO(mode) || S_ISCHR(mode) || S_ISBLK(mode)) {
        return true;
    }
#endif
    return false;
}

#ifdef _NAS
int FSBackupUtils::SetSrcFileHandleForNfs(FileHandle &fileHandle, int length, const char value[])
{
#ifndef WIN32
    fileHandle.m_file->srcIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    if (fileHandle.m_file->srcIOHandle.nfsFh == nullptr) {
        ERRLOG("Failed to allocate Memory for nfsfh");
        return Module::FAILED;
    }
    memset_s(fileHandle.m_file->srcIOHandle.nfsFh, sizeof(struct nfsfh), 0, sizeof(struct nfsfh));
    fileHandle.m_file->srcIOHandle.nfsFh->fh.len = length;
    fileHandle.m_file->srcIOHandle.nfsFh->fh.val = (char *) malloc(length);
    if (fileHandle.m_file->srcIOHandle.nfsFh->fh.val == nullptr) {
        ERRLOG("Failed to allocate Memory for fh val");
        free(fileHandle.m_file->srcIOHandle.nfsFh);
        fileHandle.m_file->srcIOHandle.nfsFh = nullptr;
        return Module::FAILED;
    }
    int ret = memcpy_s(fileHandle.m_file->srcIOHandle.nfsFh->fh.val, length, value, length);
    if (ret != 0) {
        ERRLOG("memcpy of fh failed");
        free(fileHandle.m_file->srcIOHandle.nfsFh->fh.val);
        free(fileHandle.m_file->srcIOHandle.nfsFh);
        fileHandle.m_file->srcIOHandle.nfsFh = nullptr;
        return Module::FAILED;
    }
#endif
    return Module::SUCCESS;
}
#endif

uint16_t FSBackupUtils::GetHashIndexForSqliteTask(const std::string &word)
{
    uint16_t seed = 131;
    unsigned long hash = 0;
    for (uint16_t i = 0; i < word.length(); i++) {
        hash = (hash * seed) + word[i];
    }
    return hash % MAX_JOB_SCHDULER;
}

void FSBackupUtils::SetServerNotReachableErrorCode(const BackupType &backupType,
    BackupPhaseStatus &failReason, bool isReader)
{
    bool isbackup = ((backupType == BackupType::BACKUP_FULL) || (backupType == BackupType::BACKUP_INC)) ? true : false;
    if (isReader) {
        if (isbackup) {
            failReason = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
        } else {
            failReason = BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE;
        }
    } else {
        if (isbackup) {
            failReason = BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE;
        } else {
            failReason = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
        }
    }
    ERRLOG("Server check error reason set failReason = %u", failReason);
}

BackupPhaseStatus FSBackupUtils::GetReaderStatus(
    std::shared_ptr<BackupControlInfo> controlInfo,
    bool abort,
    BackupPhaseStatus failReason)
{
    if (!controlInfo->m_readPhaseComplete) {
        return BackupPhaseStatus::INPROGRESS;
    }
    if (abort) {
        return BackupPhaseStatus::ABORTED;
    }
    if (controlInfo->m_failed || controlInfo->m_controlReaderFailed) {
        return failReason;
    }
    return BackupPhaseStatus::COMPLETED;
}

BackupPhaseStatus FSBackupUtils::GetWriterStatus(
    std::shared_ptr<BackupControlInfo> controlInfo,
    bool abort,
    BackupPhaseStatus failedReason)
{
    if (!controlInfo->m_writePhaseComplete) {
        return BackupPhaseStatus::INPROGRESS;
    }
    if (abort) {
        return BackupPhaseStatus::ABORTED;
    }
    if (controlInfo->m_failed || controlInfo->m_controlReaderFailed) {
        return failedReason;
    }
    return BackupPhaseStatus::COMPLETED;
}

BackupPhaseStatus FSBackupUtils::GetControlFileReaderStatus(std::shared_ptr<BackupControlInfo> controlInfo,
    bool abort)
{
    if (!controlInfo->m_controlReaderPhaseComplete) {
        return BackupPhaseStatus::INPROGRESS;
    }
    if (abort) {
        return BackupPhaseStatus::ABORTED;
    }
    if (controlInfo->m_controlReaderFailed || controlInfo->m_failed) {
        return BackupPhaseStatus::FAILED;
    }
    return BackupPhaseStatus::COMPLETED;
}

BackupPhaseStatus FSBackupUtils::GetAggregateStatus(std::shared_ptr<BackupControlInfo> ctrlInfo,
    bool abort)
{
    if (!ctrlInfo->m_aggregatePhaseComplete) {
        return BackupPhaseStatus::INPROGRESS;
    }
    if (abort) {
        return BackupPhaseStatus::ABORTED;
    }
    if (ctrlInfo->m_controlReaderFailed || ctrlInfo->m_failed) {
        return BackupPhaseStatus::FAILED;
    }
    return BackupPhaseStatus::COMPLETED;
}

bool FSBackupUtils::IsCiticalError(const BackupPhaseStatus &status)
{
    if  (status == BackupPhaseStatus::FAILED_NOACCESS ||
         status == BackupPhaseStatus::FAILED_NOSPACE ||
         status == BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE ||
         status == BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE) {
        return true;
    }

    return false;
}

BackupPhaseStatus FSBackupUtils::GetFailureStatus(const BackupPhaseStatus &readerStatus,
    const BackupPhaseStatus &writerStatus)
{
    if (IsCiticalError(readerStatus)) {
        DBGLOG("Exit GetFailureStatus. Reader failed Status: %d", readerStatus);
        return readerStatus;
    }
    if (IsCiticalError(writerStatus)) {
        DBGLOG("Exit GetFailureStatus. Writer failed Status: %d", writerStatus);
        return writerStatus;
    }

    return BackupPhaseStatus::FAILED;
}

/* check if a fileHandle refer to a symlink file */
bool FSBackupUtils::IsSymLinkFile(const FileHandle& fileHandle)
{
#ifdef WIN32
    return FS_Backup::Win32BackupEngineUtils::IsFileHandleSymbolicLink(fileHandle)
        || FS_Backup::Win32BackupEngineUtils::IsFileHandleJunctionPoint(fileHandle);
#else
    return S_ISLNK(fileHandle.m_file->m_mode);
#endif
}

/* parse error customized error codes defined in BackupConstants.h to there literal value */
std::string FSBackupUtils::ParseCustomizedErrorCode(uint32_t errorCode)
{
    switch (errorCode) {
        case E_BACKUP_READ_LESS_THAN_EXPECTED : return "E_BACKUP_READ_LESS_THAN_EXPECTED";
        case E_BACKUP_FILE_OR_DIR_NOT_EXIST: return "E_BACKUP_FILE_OR_DIR_NOT_EXIST";
        default: return "";
    }
    return "";
}

std::string FSBackupUtils::GetErrMsgByErrCode(uint32_t errorCode)
{
    std::string errorMessage = FSBackupUtils::ParseCustomizedErrorCode(errorCode);
    /* use platform API */
    if (errorMessage.empty()) {
        /* posix errno marco value or win32 error index */
#ifdef WIN32
        namespace ErrMessageUtil = FS_Backup::Win32BackupEngineUtils;
#else
        namespace ErrMessageUtil = FS_Backup::PosixUtils;
#endif
        errorMessage = ErrMessageUtil::ParseErrorMessage(errorCode);
    }
    return errorMessage;
}

void FSBackupUtils::RecordFailureDetail(
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder,
    const std::pair<std::string, uint64_t>& errDetails)
{
    if (failureRecorder == nullptr) {
        DBGLOG("failure recorder is nullptr!");
        return;
    }

    /* put error to failure record queue */
    failureRecorder->RecordFailure(errDetails.first, GetErrMsgByErrCode(errDetails.second));
    return;
}

void FSBackupUtils::RecordFailureDetail(
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder, std::string path, uint32_t errCode)
{
    if (failureRecorder == nullptr) {
        DBGLOG("failure recorder is nullptr!");
        return;
    }
    failureRecorder->RecordFailure(path, GetErrMsgByErrCode(errCode));
    return;
}

std::string FSBackupUtils::PathConcat(const std::string& forwardPath, const std::string& trailPath)
{
#ifdef WIN32
    namespace HostUtil = FS_Backup::Win32BackupEngineUtils;
#else
    namespace HostUtil = FS_Backup::PosixUtils;
#endif
    return HostUtil::PathConcat(forwardPath, trailPath);
}

bool FSBackupUtils::IsHandleMetaModified(const std::string &scannermode, const BackupDataFormat &format)
{
    // 聚合是增量备份，即使只有元数据改变，也需要备份整个文件
    if (scannermode == CTRL_ENTRY_MODE_META_MODIFIED && format != BackupDataFormat::AGGREGATE) {
        return true;
    }
    return false;
}

bool FSBackupUtils::Rename(const std::string &oldName, const std::string &newName)
{
    try {
        boost::filesystem::rename(oldName, newName);
    } catch (const boost::filesystem::filesystem_error &e) {
        ERRLOG("rename() exeption: %s", e.code().message().c_str());
        return false;
    }
    return true;
}

bool FSBackupUtils::OnlyGenerateSqlite(bool genSqlite)
{
    return genSqlite;
}

void FSBackupUtils::MemoryTrim()
{
#if !defined(WIN32) && !defined(SOLARIS) && !defined(_AIX)
    malloc_trim(0);
#endif
}

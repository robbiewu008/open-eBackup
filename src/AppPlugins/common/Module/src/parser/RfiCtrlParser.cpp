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
#include "RfiCtrlParser.h"
#include "Log.h"
#include "system/System.hpp"
#include "Win32PathUtils.h"
#include "JsonHelper.h"

#ifndef _WIN32
#include <sys/stat.h>
#include "system/System.hpp"
#endif

using namespace std;
using namespace Module;

namespace {
    constexpr auto MODULE = "RfiFileGenerator";
    string RFI_FIRST_LINE = R"({"title": "Raw File-system Index Database","version": "2.0","time": )";
    constexpr int MILLION = 1000000;
    constexpr auto FILESYSTEMFILTERHEADER = "/source_policy_";
    constexpr auto FILESYSTEMFILTERTAIL = "_Context";
    const std::string POSIX_PATH_SEPARATOR = "/";
}

/**
 * xmeta file use posix style path both for linux/unix/windows,
 * DEE alse require windows path in RFI file to form like unix path
 * Example: /c/Windows/User represent C:\Windows\User
 * No need to do transformation here
 */

RfiCtrlParser::RfiCtrlParser(RfiCtrlParserParams& params) : FileParser(false)
{
    m_maxEntriesPerFile = params.maxEntriesPerFile;
    m_fileName = params.rfiFileName;
    m_filterFlag = params.filterFlag;
    m_filterKey = params.key;
    m_version = params.version;
}

RfiCtrlParser::~RfiCtrlParser()
{
    if (m_writeFd.is_open()) {
        DBGLOG("close called from Destructor");
        WriteToFile(m_writeBuffer, false);
    }
}

CTRL_FILE_RETCODE RfiCtrlParser::WriteHeader()
{
    WriteStatisticInfo();
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE RfiCtrlParser::WriteRfiHeader()
{
    uint64_t curTime = static_cast<uint64_t>(time(nullptr));
    curTime *= MILLION;
    string firstLine = RFI_FIRST_LINE + "\"" + to_string(curTime) + "\"}" + "\n";
    m_writeBuffer << firstLine;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE RfiCtrlParser::OpenWrite()
{
    WriteRfiHeader();
    CheckZipInstalled();
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE RfiCtrlParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE RfiCtrlParser::FlushToFile()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE RfiCtrlParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE RfiCtrlParser::ReadHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE RfiCtrlParser::WriteDirMeta(const DirCache& dirCache, const DirMetaWrapper& dirMeta,
    RfiEntryStatus status)
{
    lock_guard<std::mutex> lk(m_lock);
    string path = GetFileOrDirNameFromXMeta(dirMeta.m_xMeta);
    if (dirMeta.m_meta.type == static_cast<uint16_t>(MetaType::OBJECT)) {
        path = ParserUtils::ParseObjectPath(path, dirMeta.m_xMeta);
    }
    if (!m_writeFd.is_open()) {
        ERRLOG("Write failed as rfi file not open : %s", m_fileName.c_str());
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (path == ".") {
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    // fix for cifs index
    if (path[0] != '/') {
        path.insert(path.begin(), '/');
    }

    if (NeedCheckMultiSystemRootDirExist()) {
        if (!CheckMultiSystemRootDirExist(path)) {
            DBGLOG("Skip %s", path.c_str());
            return CTRL_FILE_RETCODE::SUCCESS;
        }
    }

    ExecStatisticType();

    RfiDocument document {};
    document.path = path;
    document.mtime = std::to_string(dirMeta.m_meta.m_mtime);
    document.size = std::to_string(dirMeta.m_meta.m_size);
    document.inode = std::to_string(dirMeta.m_meta.m_inode);
    document.type = "d";
    document.status = (status == RfiEntryStatus::RFI_ENTRY_STATUS_NEW) ? "new" : "old";
    if (m_version == RFICTRL_HEADER_VERSION_V20) {
        document.id = std::to_string(dirCache.m_dirMetaHash.crc);
    } else if (m_version == RFICTRL_HEADER_VERSION_V30) {
        m_crc32.Clear();
        m_crc32.Input(reinterpret_cast<const uint8_t *>(dirCache.m_dirMetaHash.sha1), SHA_DIGEST_LENGTH);
        document.id = std::to_string(m_crc32.Result());
    }
    string rfiLine;
    if (!JsonHelper::StructToJsonString(document, rfiLine)) {
        ERRLOG("json struct to string failed");
    }
    m_writeBuffer << rfiLine << "\n";
    m_entries++;
    DBGLOG("Write rfi content:%s to file %s, entries:%d", rfiLine.c_str(), m_fileName.c_str(), m_entries);
    return (m_entries >= m_maxEntriesPerFile) ? CTRL_FILE_RETCODE::LIMIT_REACHED : CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE RfiCtrlParser::WriteEmptyDir(const std::string& dirName)
{
    std::string path = dirName;
    if (NeedCheckMultiSystemRootDirExist()) {
        if (!CheckMultiSystemRootDirExist(path)) {
            DBGLOG("Skip %s", path.c_str());
            return CTRL_FILE_RETCODE::SUCCESS;
        }
    }
    PrepareStatisticType(StatisticType::STATISTIC_TYPE_CREATE_FOLDER, true);
    ExecStatisticType();
    RfiDocument document {};
    document.path = dirName;
    document.mtime = "0";
    document.size = "0";
    document.inode = "0";
    document.type = "d";
    document.status = "new";
    document.id = "0";
    string rfiLine;
    if (!JsonHelper::StructToJsonString(document, rfiLine)) {
        ERRLOG("json struct to string failed");
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_writeBuffer << rfiLine << "\n";
    m_entries++;
    DBGLOG("Write rfi content:%s to file %s, entries:%d", rfiLine.c_str(), m_fileName.c_str(), m_entries);
    return CTRL_FILE_RETCODE::SUCCESS;
}

void RfiCtrlParser::FillRfiFileDoc(const FileCache& fcache, const FileMetaWrapper& fileMeta,
    const string& filePath, RfiEntryStatus status, RfiDocument& document)
{
    document.type = "f";
#ifdef WIN32
    if ((fileMeta.m_meta.m_mode & (FILEMETA_FLAG_WIN32_SYMBOLIC_LINK | FILEMETA_FLAG_WIN32_JUNCTION_LINK)) != 0) {
#else
    if (S_ISLNK(fileMeta.m_meta.m_mode)) {
#endif
        document.type = "l";
    }
    document.path = filePath;
    document.mtime = std::to_string(fileMeta.m_meta.m_mtime);
    document.inode = std::to_string(fileMeta.m_meta.m_inode);
    document.size = std::to_string(fileMeta.m_meta.m_size);
    document.status = (status == RfiEntryStatus::RFI_ENTRY_STATUS_NEW) ? "new" : "old";

    if (m_version == RFICTRL_HEADER_VERSION_V20) {
        document.id = std::to_string(fcache.m_fileMetaHash.crc);
    }  else if (m_version == RFICTRL_HEADER_VERSION_V30) {
        m_crc32.Clear();
        m_crc32.Input(reinterpret_cast<const uint8_t *>(fcache.m_fileMetaHash.sha1), SHA_DIGEST_LENGTH);
        document.id = std::to_string(m_crc32.Result());
    }
}

CTRL_FILE_RETCODE RfiCtrlParser::WriteFileMeta(const FileCache& fcache, const FileMetaWrapper& fileMeta,
    const string& prePath, RfiEntryStatus status)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_writeFd.is_open()) {
        ERRLOG("Write failed as rfi file not open: %s", m_fileName.c_str());
        return CTRL_FILE_RETCODE::FAILED;
    }

    string fullPath;
    if (fileMeta.m_meta.type == static_cast<uint16_t>(MetaType::OBJECT)) {
        fullPath = ParserUtils::ParseObjectPath(prePath, fileMeta.m_xMeta);
    } else {
        fullPath = prePath + POSIX_PATH_SEPARATOR + GetFileOrDirNameFromXMeta(fileMeta.m_xMeta);
    }
    if (fullPath[0] == '.') {
        fullPath = fullPath.substr(1);
    }
    if (fullPath[0] != '/') {
        fullPath.insert(fullPath.begin(), '/');
    }
    if (NeedCheckMultiSystemRootDirExist()) {
        if (!CheckMultiSystemRootDirExist(fullPath)) {
            DBGLOG("Skip %s", fullPath.c_str());
            return CTRL_FILE_RETCODE::SUCCESS;
        }
    }

    ExecStatisticType();
    RfiDocument document {};
    FillRfiFileDoc(fcache, fileMeta, fullPath, status, document);
    string rfiLine;
    if (!JsonHelper::StructToJsonString(document, rfiLine)) {
        ERRLOG("json struct to string failed");
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_writeBuffer << rfiLine << "\n";
    m_entries++;
    DBGLOG("Write rfi content:%s to file %s, entries:%d", rfiLine.c_str(), m_fileName.c_str(), m_entries);
    return (m_entries >= m_maxEntriesPerFile) ?  CTRL_FILE_RETCODE::LIMIT_REACHED : CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE RfiCtrlParser::WriteUpdateDirMeta(const DirCache& dcache1, const DirMetaWrapper& dmeta1,
    const DirCache& dcache2, const DirMetaWrapper& dmeta2)
{
    // 更新的文件写在同一个文件中
    PrepareStatisticType(StatisticType::STATISTIC_TYPE_UPDATE_FOLDER, false);
    WriteDirMeta(dcache1, dmeta1, RfiEntryStatus::RFI_ENTRY_STATUS_NEW);
    PrepareStatisticType(StatisticType::STATISTIC_TYPE_UPDATE_FOLDER, true);
    CTRL_FILE_RETCODE ret = WriteDirMeta(dcache2, dmeta2, RfiEntryStatus::RFI_ENTRY_STATUS_OLD);
    return ret;
}

CTRL_FILE_RETCODE RfiCtrlParser::WriteUpdateFileMeta(const FileCache& fcache1, const FileMetaWrapper& fmeta1,
    const FileCache& fcache2, const FileMetaWrapper& fmeta2, const string& prePath)
{
    PrepareStatisticType(StatisticType::STATISTIC_TYPE_UPDATE_FILE, false);
    WriteFileMeta(fcache1, fmeta1, prePath, RfiEntryStatus::RFI_ENTRY_STATUS_NEW);
    PrepareStatisticType(StatisticType::STATISTIC_TYPE_UPDATE_FILE, true);
    CTRL_FILE_RETCODE ret = WriteFileMeta(fcache2, fmeta2, prePath, RfiEntryStatus::RFI_ENTRY_STATUS_OLD);
    return ret;
}

CTRL_FILE_RETCODE RfiCtrlParser::WriteSingleDirMeta(const DirCache& dcache, const DirMetaWrapper& dmeta,
    RfiEntryStatus status)
{
    if (status == RfiEntryStatus::RFI_ENTRY_STATUS_NEW) {
        PrepareStatisticType(StatisticType::STATISTIC_TYPE_CREATE_FOLDER, true);
    } else {
        PrepareStatisticType(StatisticType::STATISTIC_TYPE_DELETE_FOLDER, true);
    }
    CTRL_FILE_RETCODE ret = WriteDirMeta(dcache, dmeta, status);
    return ret;
}

CTRL_FILE_RETCODE RfiCtrlParser::WriteSingleFileMeta(const FileCache& fcache, const FileMetaWrapper& fmeta,
    const string& prePath, RfiEntryStatus status)
{
    if (status == RfiEntryStatus::RFI_ENTRY_STATUS_NEW) {
        PrepareStatisticType(StatisticType::STATISTIC_TYPE_CREATE_FILE, true);
    } else {
        PrepareStatisticType(StatisticType::STATISTIC_TYPE_DELETE_FILE, true);
    }
    CTRL_FILE_RETCODE ret = WriteFileMeta(fcache, fmeta, prePath, status);
    return ret;
}

void RfiCtrlParser::WriteStatisticInfo()
{
    string statisticStr;
    if (!JsonHelper::StructToJsonString(m_statistic, statisticStr)) {
        ERRLOG("convert statistic struct fail!");
    }
    m_writeBuffer << statisticStr;
    return;
}

string RfiCtrlParser::GetRfiFileName()
{
    return m_fileName;
}

string RfiCtrlParser::GetRfiZipFileName()
{
    // do not compress while zip not installed , return origin filename directly
    if (!m_zipAvailable) {
        return m_fileName;
    }
    return m_fileName.substr(0, m_fileName.find_last_of('.') + 1) + "zip";
}

bool RfiCtrlParser::CheckMultiSystemRootDirExist(string& fileName)
{
    string fileSystemRootPath = FILESYSTEMFILTERHEADER + m_filterKey + FILESYSTEMFILTERTAIL;
    // 如果匹配
    if (fileName.substr(0, fileSystemRootPath.size()) == fileSystemRootPath) {
        fileName.erase(fileName.begin(), fileName.begin() + fileSystemRootPath.size());
        if (*fileName.begin() != '/') {
            // 跳过统计
            return false;
        }
        // 说明是多文件系统目标路径下的文件， 记入rfi
        return true;
    }
    // 跳过统计
    return false;
}

bool RfiCtrlParser::NeedCheckMultiSystemRootDirExist()
{
    if (m_filterFlag) {
        return true;
    }
    return false;
}

void RfiCtrlParser::PrepareStatisticType(StatisticType statisticType, bool takeCharge)
{
    m_statisticType = statisticType;
    m_takeCharge = takeCharge;
}

void RfiCtrlParser::ExecStatisticType()
{
    if (!m_takeCharge) {
        return;
    }
    switch (m_statisticType) {
        case StatisticType::STATISTIC_TYPE_CREATE_FOLDER : {
            m_statistic.createFolderCount++;
            break;
        }
        case StatisticType::STATISTIC_TYPE_CREATE_FILE : {
            m_statistic.createFileCount++;
            break;
        }
        case StatisticType::STATISTIC_TYPE_UPDATE_FILE : {
            m_statistic.updateFileCount++;
            break;
        }
        case StatisticType::STATISTIC_TYPE_UPDATE_FOLDER : {
            m_statistic.updateFolderCount++;
            break;
        }
        case StatisticType::STATISTIC_TYPE_DELETE_FILE : {
            m_statistic.deleteFileCount++;
            break;
        }
        case StatisticType::STATISTIC_TYPE_DELETE_FOLDER : {
            m_statistic.deleteFolderCount++;
            break;
        }
        default:
            ;
    };
}

string RfiCtrlParser::GetFileOrDirNameFromXMeta(const vector<XMetaField> &xMeta)
{
    for (uint32_t i = 0; i < xMeta.size(); i++) {
        if (xMeta[i].m_xMetaType == XMETA_TYPE::XMETA_TYPE_NAME) {
            return xMeta[i].m_value;
        }
    }

    return "";
}

CTRL_FILE_RETCODE RfiCtrlParser::GenerateZip() // Hint: return type should be modify to bool
{
    // do not compress while zip is not installed
    if (!m_zipAvailable) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    string zipFileName = GetRfiZipFileName();

#ifdef WIN32
    string cmd = R"(C:\DataBackup\ProtectClient\ProtectClient-E\bin\7z.exe a -tzip -r )";
    cmd += zipFileName + " " + m_fileName;
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
#else
    string cmd = "zip -qrj " + zipFileName + " " + m_fileName;
    vector<string> output;
    vector<string> errOutput;
    INFOLOG("zip cmd : %s", cmd.c_str());
    (void)runShellCmdWithOutput(INFO, MODULE, 0, cmd, { }, output, errOutput);
    return CTRL_FILE_RETCODE::SUCCESS;
#endif
}

void RfiCtrlParser::CheckZipInstalled()
{
#ifdef WIN32
    string cmd = R"(C:\DataBackup\ProtectClient\ProtectClient-E\bin\7z.exe i)";
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        m_zipAvailable = false;
        return;
    }
    m_zipAvailable = true;
    return;
#else
    string cmd = "zip -v";
    vector<string> output;
    vector<string> errOutput;

    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, { }, output, errOutput);

    if (ret != 0) {
        m_zipAvailable = false;
        return;
    }
    m_zipAvailable = true;
    return;
#endif
}

uint32_t RfiCtrlParser::GetEntries()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_entries;
}
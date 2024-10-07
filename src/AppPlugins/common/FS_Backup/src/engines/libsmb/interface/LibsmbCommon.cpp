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
#include "LibsmbCommon.h"
#include "log/Log.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    constexpr uint8_t MAX_MOUNT_RETRY_CNT = 5;
    const int ONE_THOUSAND_UNIT_CONVERSION = 1000;
    const string SMB_VERSION_3_1_1 = "3.1.1";
    const string SMB_VERSION_3_02 = "3.02";
    const string SMB_VERSION_3_0 = "3.0";
    const string SMB_VERSION_2_1 = "2.1";
    const string SMB_VERSION_2_0 = "2.0";
    const string SMB_ENCRYPTION = "1";
    const char SLASH = '/';
    const char BACK_SLASH = '\\';
    const string DOT_SLASH = "./";
    const string DOT_BACK_SLASH = ".\\";
    const int NUM2 = 2;
}

std::shared_ptr<SmbContextWrapper> SmbConnectContext(const SmbContextArgs &args)
{
    std::shared_ptr<SmbContextWrapper> rootSmb = std::make_shared<SmbContextWrapper>(args);
    bool ret = false;
    if (!rootSmb->Init()) {
        return nullptr;
    }
    uint8_t retryCnt = 0;
    do {
        ret = rootSmb->SmbConnect();
        retryCnt++;
        sleep(retryCnt);
    } while (!ret && (retryCnt <= MAX_MOUNT_RETRY_CNT));
    if (ret) {
        return rootSmb;
    }
    return nullptr;
}

int SmbDisconnectContext(std::shared_ptr<SmbContextWrapper> context)
{
    if (context == nullptr) {
        return FAILED;
    }
    return context->SmbDisconnect();
}

SmbVersion ConvertStringToSmbVersion(std::string versionString)
{
    if (versionString == SMB_VERSION_3_1_1) {
        return SmbVersion::VERSION0311;
    } else if (versionString == SMB_VERSION_3_02) {
        return SmbVersion::VERSION0302;
    } else if (versionString == SMB_VERSION_3_0) {
        return SmbVersion::VERSION0300;
    } else if (versionString == SMB_VERSION_2_1) {
        return SmbVersion::VERSION0210;
    } else if (versionString == SMB_VERSION_2_0) {
        return SmbVersion::VERSION0202;
    } else {
        return SmbVersion::VERSION0300;
    }
}

bool FillContextParams(SmbContextArgs &smbContextArgs, std::shared_ptr<LibsmbBackupAdvanceParams> advParams)
{
    if (advParams->server.empty() || advParams->share.empty()) {
        ERRLOG("Wrong nas share, server: %s, share: %s", advParams->server.c_str(), advParams->share.c_str());
        return false;
    }

    INFOLOG("cifs share %s, authType %s", advParams->share.c_str(), advParams->authType.c_str());
    if (advParams->authType == "krb5") {
        smbContextArgs.authType = SmbAuthType::KRB5;
    } else if (advParams->authType == "ntlmssp") {
        smbContextArgs.authType = SmbAuthType::NTLMSSP;
    } else {
        ERRLOG("Wrong authType for cifs share: %s", advParams->authType.c_str());
        return false;
    }

    smbContextArgs.domain         = advParams->domain;
    smbContextArgs.server         = advParams->server;
    smbContextArgs.share          = advParams->share;
    smbContextArgs.user           = advParams->user;
    smbContextArgs.password       = advParams->password;
    smbContextArgs.krb5CcacheFile = advParams->krb5CcacheFile;
    smbContextArgs.krb5ConfigFile = advParams->krb5ConfigFile;
    smbContextArgs.encryption     = advParams->encryption;
    smbContextArgs.sign           = advParams->sign;
    smbContextArgs.timeout        = advParams->timeout;
    smbContextArgs.version        = ConvertStringToSmbVersion(advParams->version);

    INFOLOG("smbContextArgs.server: %s, %s! ", smbContextArgs.server.c_str(), smbContextArgs.share.c_str());
    return true;
}

int SmbEnqueueToTimer(BackupTimer *timer, FileHandle fileHandle)
{
    fileHandle.m_retryCnt++;
    DBGLOG("SmbEnqueueToTimer: %s, retryTimes: %d", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_retryCnt);
    timer->Insert(fileHandle, fileHandle.m_retryCnt * ONE_THOUSAND_UNIT_CONVERSION);
    return SUCCESS;
}

bool IfNeedRetry(int retryCnt, int maxRetryTimes, int status)
{
    if (retryCnt < maxRetryTimes) {
        if (status == -EIO || status == -ETIMEDOUT || status == -EINTR || status == -EAGAIN ||
            status == -ENETRESET || status == -EBADF || status == -ECANCELED) {
            return true;
        }
    }
    return false;
}

string GetPathName(const string &filePath)
{
    size_t fileOffset = filePath.rfind(PATH_SEPARATOR, filePath.length());
    if (fileOffset != string::npos) {
        return (filePath.substr(0, fileOffset));
    }

    return ("");
}

// /A/B/C/d --> A/B/C/d
string RemoveFirstSeparator(const string &path)
{
    int pathLength = path.size();
    if (pathLength == 0) {
        return "";
    }
    int i = 0;
    int count = 0;
    // remove prefix "./" or ".\"
    if (path.find(DOT_SLASH) == 0 || path.find(DOT_BACK_SLASH) == 0) {
        i = NUM2;
        count = NUM2;
    }
    for (; i < pathLength; ++i) {
        if (path.at(i) == BACK_SLASH || path.at(i) == SLASH) {
            ++count;
        } else {
            break;
        }
    }
    return path.substr(count);
}

string GetLibsmbEventName(LibsmbEvent event)
{
    static std::map<LibsmbEvent, std::string> eventMap = {
        {LibsmbEvent::OPEN_SRC, "OPEN_SRC"},
        {LibsmbEvent::READ, "READ"},
        {LibsmbEvent::CLOSE_SRC, "CLOSE_SRC"},
        {LibsmbEvent::ADS, "ADS"},
        {LibsmbEvent::OPEN_DST, "OPEN_DST"},
        {LibsmbEvent::WRITE, "WRITE"},
        {LibsmbEvent::CLOSE_DST, "CLOSE_DST"},
        {LibsmbEvent::SET_SD, "SET_SD"},
        {LibsmbEvent::STAT_DST, "STAT_DST"},
        {LibsmbEvent::SET_BASIC_INFO, "SET_BASIC_INFO"},
        {LibsmbEvent::MKDIR, "MKDIR"},
        {LibsmbEvent::DELETE, "DELETE"},
        {LibsmbEvent::LINK, "LINK"},
        {LibsmbEvent::UNLINK, "UNLINK"},
        {LibsmbEvent::RESET_ATTR, "RESET_ATTR"}
    };
    auto it = eventMap.find(event);
    if (it == eventMap.end()) {
        return "No Such Event";
    }
    return eventMap[event];
}

void CheckStatusAndIncStat(int status, PKT_TYPE pktType, std::shared_ptr<PacketStats> pktStats)
{
    if (status == -ENOSPC || status == -ERANGE) {
        pktStats->Increment(pktType, PKT_COUNTER::FAILED, 1, PKT_ERROR::NO_SPACE_ERR);
    } else if (status == -EACCES) {
        pktStats->Increment(pktType, PKT_COUNTER::FAILED, 1, PKT_ERROR::NO_ACCESS_ERR);
    } else {
        pktStats->Increment(pktType, PKT_COUNTER::FAILED);
    }
}

int HandleConnectionException(std::shared_ptr<SmbContextWrapper> &smbContext,
    SmbContextArgs &contextArgs, int connectRetryTimes)
{
    if (smbContext->SmbEcho() == SUCCESS) {
        INFOLOG("Send Echo succeeded, socket available!");
        return SUCCESS;
    }
    INFOLOG("Send Echo failed, socket unavailable!");

    smbContext->RemoveErrorPdus();
    smbContext.reset();

    smbContext = std::make_shared<SmbContextWrapper>(contextArgs);
    if (!smbContext->Init()) {
        return FAILED;
    }
    int retryCnt = 0;
    bool ret;
    do {
        ret = smbContext->SmbConnect();
        if (ret) {
            INFOLOG("Reconnect to smb server success.");
            return SUCCESS;
        }
        ++retryCnt;
        sleep(retryCnt);
    } while (!ret && (retryCnt < connectRetryTimes));

    INFOLOG("Reconnect to connect smb server failed.");
    return FAILED;
}

PKT_TYPE LibsmbEventToPKT_TYPE(LibsmbEvent event)
{
    switch (event) {
        case LibsmbEvent::OPEN_SRC:
            return PKT_TYPE::OPEN;
        case LibsmbEvent::OPEN_DST:
            return PKT_TYPE::CREATE;
        case LibsmbEvent::READ:
            return PKT_TYPE::READ;
        case LibsmbEvent::CLOSE_SRC:
        case LibsmbEvent::CLOSE_DST:
            return PKT_TYPE::CLOSE;
        case LibsmbEvent::ADS:
        case LibsmbEvent::STAT_DST:
            return PKT_TYPE::LSTAT;
        case LibsmbEvent::WRITE:
            return PKT_TYPE::WRITE;
        case LibsmbEvent::SET_SD:
        case LibsmbEvent::SET_BASIC_INFO:
            return PKT_TYPE::SETMETA;
        case LibsmbEvent::MKDIR:
            return PKT_TYPE::MKDIR;
        case LibsmbEvent::DELETE:
            return PKT_TYPE::LINKDELETE;
        case LibsmbEvent::LINK:
            return PKT_TYPE::HARDLINK;
        default:
            return PKT_TYPE::OPEN;
    }
    return PKT_TYPE::OPEN;
}

bool IsFileReadOrWriteFailed(FileHandle &fileHandle)
{
    return (fileHandle.m_file->GetDstState() == FileDescState::WRITE_FAILED ||
            fileHandle.m_file->GetSrcState() == FileDescState::READ_FAILED);
}

bool IsBackupTask(BackupType type)
{
    return (type == BackupType::BACKUP_FULL || type == BackupType::BACKUP_INC);
}

int64_t GetActualOpenedFileHandleCount(std::shared_ptr<PacketStats> pktStats)
{
    int64_t openCount =
        pktStats->GetValue(PKT_TYPE::OPEN, PKT_COUNTER::SENT) -
        pktStats->GetValue(PKT_TYPE::OPEN, PKT_COUNTER::FAILED) -
        pktStats->GetValue(PKT_TYPE::OPEN, PKT_COUNTER::RETRIABLE_ERR);
    int64_t closeCount =
        pktStats->GetValue(PKT_TYPE::CLOSE, PKT_COUNTER::RECVD) -
        pktStats->GetValue(PKT_TYPE::CLOSE, PKT_COUNTER::RETRIABLE_ERR);
    
    return openCount - closeCount;
}

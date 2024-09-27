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
#include "SmbContextWrapper.h"

namespace {
    constexpr auto MODULE = "SMBCONTEXT_WRAPPER";
    const std::string KRB5_CCNAME_ENV_PREFIX = "FILE:";
    const int MAX_SHARE_SIZE = 1024;
    const int NUMBER_ZERO = 0;
    const int NUMBER_ONE = 1;
    const int NUMBER_TWO = 2;
    std::mutex krbMtx {};
    constexpr int DEFAULT_POLL_EXPIRED_TIME = 100;
}

using namespace std;
namespace Module {
SmbContextWrapper::SmbContextWrapper(SmbContextArgs args): m_args(args) {}

SmbContextWrapper::~SmbContextWrapper()
{
    HCP_Log(INFO, MODULE) << "Context " << SmbGetClientGuid() << " destructed!" << HCPENDLOG;
    if (m_smbContext != nullptr) {
        smb2_destroy_context(m_smbContext);
        m_smbContext = nullptr;
    }
}

bool SmbContextWrapper::Init()
{
    timeval curTime {};
    gettimeofday(&curTime, nullptr);
    m_lastPollTime = static_cast<uint64_t>(curTime.tv_sec);
    if (IsInitailized()) {
        return true;
    }
    if (!SmbInitContext()) {
        HCP_Log(ERR, MODULE) << "Failed to init context." << HCPENDLOG;
        return false;
    }
    if (!SmbValidateArgs()) {
        HCP_Log(ERR, MODULE) << "Arguments are invalid." << HCPENDLOG;
        return false;
    }
    SmbConfigure();
    HCP_Log(INFO, MODULE) << "Context " << SmbGetClientGuid() << " constructed!" << HCPENDLOG;
    return true;
}

bool SmbContextWrapper::IsInitailized()
{
    return m_initiated;
}

string SmbContextWrapper::SmbGetClientGuid()
{
    return string(smb2_get_client_guid(m_smbContext));
}

SmbContextArgs SmbContextWrapper::SmbGetArgs()
{
    return m_args;
}

bool SmbContextWrapper::SmbConnect()
{
    if (m_args.authType == SmbAuthType::KRB5) {
        lock_guard<std::mutex> krbLock(krbMtx);
        SmbSetKrbEnvironment();
    }
    HCP_Log(INFO, MODULE) << "smb2_connect_share params, server: " << m_args.server <<
        ", share: " << m_args.share << HCPENDLOG;
    int ret = smb2_connect_share(m_smbContext, m_args.server.c_str(), m_args.share.c_str(), nullptr);
    if (ret != SUCCESS) {
        HCP_Log(ERR, MODULE) << "Failed to connect share." <<
            " err: " << smb2_get_error(m_smbContext) << ", ret:" << ret << HCPENDLOG;
        return false;
    }

    SmbSetLogCb(SmbLogCb, this);

    HCP_Log(INFO, MODULE) << "SUCCESS to connect share." << HCPENDLOG;
    return true;
}

struct smb2_stats SmbContextWrapper::SmbGetStats()
{
    return smb2_get_stats(m_smbContext);
}

void SmbLogCb(void *jobData, const char* logString, int logLevel)
{
    auto *smbContextWrapper = static_cast<SmbContextWrapper *>(jobData);
    HCP_Log(logLevel, MODULE) << "Smb Log: " << logString << HCPENDLOG;
}

void SmbContextWrapper::SmbSetLogCb(smb2_log_cb cb, void *privateData)
{
    smb2_set_log_cb(m_smbContext, cb, privateData);
}

int SmbContextWrapper::SmbEcho()
{
    return smb2_echo(m_smbContext);
}

struct smb2dirent *SmbContextWrapper::SmbReadDir(struct smb2dir *smb2dir)
{
    return smb2_readdir(m_smbContext, smb2dir);
}

void SmbContextWrapper::SmbCloseDir(struct smb2dir *smb2dir)
{
    smb2_closedir(m_smbContext, smb2dir);
}

bool SmbContextWrapper::SmbProcess()
{
    struct pollfd pfd;
    int revents = 0;
    int ret = 0;
    while (true) {
        pfd.fd = smb2_get_fd(m_smbContext);
        pfd.events = smb2_which_events(m_smbContext);
        pfd.revents = 0;
        ret = poll(&pfd, 1, DEFAULT_POLL_EXPIRED_TIME);
        if (ret < 0) {
            ERRLOG("Poll Error, errno: %d", errno);
            return false;
        } else if (ret == 0) {
            break;
        } else {
            revents = pfd.revents;
        }
        if (pfd.fd == -1) {
            ERRLOG("Timeout expired and no connection exists");
            return false;
        }
        if (pfd.revents == 0) {
            break;
        }
        if ((ret = smb2_service(m_smbContext, revents)) < 0) {
            ERRLOG("SmbService Error, ret=%d, error: %s", ret, smb2_get_error(m_smbContext));
            return false;
        }
    }
    return true;
}

bool SmbContextWrapper::SmbService(int revents)
{
    int ret = smb2_service(m_smbContext, revents);
    if (ret < 0) {
        HCP_Log(DEBUG, MODULE) << "SmbService Error, ret=" << ret << ", error:"
            << smb2_get_error(m_smbContext) << HCPENDLOG;
        return false;
    }
    return true;
}

int SmbContextWrapper::SmbStat64(const char *path, struct smb2_stat_64 *st)
{
    return smb2_stat(m_smbContext, path, st);
}

int SmbContextWrapper::SmbMkdir(const char *path)
{
    return smb2_mkdir(m_smbContext, path);
}

int SmbContextWrapper::SmbOpendirAsync(const char *path, smb2_command_cb cb, void *privateData)
{
    return smb2_opendir_async(m_smbContext, path, cb, privateData);
}

struct smb2_query_directory_data *SmbContextWrapper::SmbAccessDir(const char* path)
{
    return smb2_accessdir(m_smbContext, path);
}

int SmbContextWrapper::SmbAccessDirAsync(const char *path, smb2_command_cb cb, void *privateData)
{
    return smb2_accessdir_async(m_smbContext, path, cb, privateData);
}

int SmbContextWrapper::SmbQueryDir(struct smb2_query_directory_data *qd)
{
    return smb2_querydir(m_smbContext, qd);
}

int SmbContextWrapper::SmbQueryDirAsync(struct smb2_query_directory_data *qd, smb2_command_cb cb, void *privateData)
{
    return smb2_querydir_async(m_smbContext, qd, cb, privateData);
}

int SmbContextWrapper::SmbCloseQueryDir(struct smb2_query_directory_data *qd)
{
    return smb2_close_querydir(m_smbContext, qd);
}

int SmbContextWrapper::SmbCloseQueryDirAsync(struct smb2_query_directory_data *qd, smb2_command_cb cb, void *privateData)
{
    return smb2_close_querydir_async(m_smbContext, qd, cb, privateData);
}

struct smb2dir *SmbContextWrapper::SmbGetDir(struct smb2_query_directory_data *qd)
{
    return smb2_getdir(m_smbContext, qd);
}

string SmbContextWrapper::SmbGetError()
{
    return string(smb2_get_error(m_smbContext));
}

string SmbContextWrapper::SmbGetUrl()
{
    string url = "smb://" + m_args.server + "/" + m_args.share;
    return url;
}

uint32_t SmbContextWrapper::SmbGetMaxReadSize()
{
    return smb2_get_max_read_size(m_smbContext);
}

uint32_t SmbContextWrapper::SmbGetMaxWriteSize()
{
    return smb2_get_max_write_size(m_smbContext);
}

int SmbContextWrapper::SmbLstat64(const char *path, struct smb2_stat_64 *st)
{
    return smb2_stat(m_smbContext, path, st);
}

int SmbContextWrapper::SmbGetSdAsync(const char *path, wchar_t **sdstr, smb2_command_cb cb, void *privateData)
{
    return smb2_getsd_async(m_smbContext, path, sdstr, cb, privateData);
}

int SmbContextWrapper::SmbGetSd(const char *path, wchar_t **sdstr)
{
    return smb2_getsd(m_smbContext, path, sdstr);
}

int SmbContextWrapper::SmbSetSdAsync(const char *path, wchar_t *sdstr, smb2_command_cb cb, void *privateData)
{
    return smb2_setsd_async(m_smbContext, path, sdstr, cb, privateData);
}
int SmbContextWrapper::SmbSetSd(const char *path, wchar_t *sdstr)
{
    return smb2_setsd(m_smbContext, path, sdstr);
}
int SmbContextWrapper::SmbSetBasicInfoAsync(const char *path, struct SMB2_BASIC_INFO *basicInfo,
    smb2_command_cb cb, void *privateData)
{
    return smb2_set_basic_info_async(m_smbContext, path, basicInfo, cb, privateData);
}

int SmbContextWrapper::SmbSetBasicInfo(const char *path, struct SMB2_BASIC_INFO *basicInfo)
{
    return smb2_set_basic_info(m_smbContext, path, basicInfo);
}

int SmbContextWrapper::SmbStatAsync(const char *path, struct smb2_stat_64 *st,
    smb2_command_cb cb, void *privateData)
{
    return smb2_stat_async(m_smbContext, path, st, cb, privateData);
}

int SmbContextWrapper::SmbGetStreamInfoAsync(const char *path, struct smb2_filestream_info **info,
    smb2_command_cb cb, void *privateData)
{
    return smb2_get_streaminfo_async(m_smbContext, path, info, cb, privateData);
}

int SmbContextWrapper::SmbGetStreamInfo(const char *path, struct smb2_filestream_info **info)
{
    return smb2_get_streaminfo(m_smbContext, path, info);
}

struct smb2_url *SmbContextWrapper::SmbParseUrlFull(const char *url)
{
    return smb2_parse_url(m_smbContext, url);
}

int SmbContextWrapper::SmbUnlinkAsync(const char *path, smb2_command_cb cb, void *privateData)
{
    return smb2_unlink_async(m_smbContext, path, cb, privateData);
}

int SmbContextWrapper::SmbUnlink(const char *path)
{
    return smb2_unlink(m_smbContext, path);
}

struct smb2dir *SmbContextWrapper::SmbOpendir(const char *path)
{
    return smb2_opendir(m_smbContext, path);
}

int SmbContextWrapper::SmbRmdir(const char *path)
{
    return smb2_rmdir(m_smbContext, path);
}

int SmbContextWrapper::SmbWriteFileAsync(struct smb2fh *fh, struct SMBWriteAsyncS writeAsyncS, smb2_command_cb cb,
    void *privateData)
{
    return smb2_pwrite_async(m_smbContext, fh, writeAsyncS.buf, writeAsyncS.count, writeAsyncS.offset,
        cb, privateData);
}

int SmbContextWrapper::SmbOpenAsync(const char *path, int flags, smb2_command_cb cb, void *privateData)
{
    return smb2_open_async(m_smbContext, path, flags, cb, privateData);
}

int SmbContextWrapper::SmbMkdirAsync(const char *path, smb2_command_cb cb, void *privateData)
{
    return smb2_mkdir_async(m_smbContext, path, cb, privateData);
}

int SmbContextWrapper::SmbCloseAsync(struct smb2fh *smbfh, smb2_command_cb cb, void *privateData)
{
    return smb2_close_async(m_smbContext, smbfh, cb, privateData);
}

int SmbContextWrapper::SmbWriteCompoundAsync(const char* path, uint32_t openFlag, uint64_t offset, uint64_t count,
    const uint8_t *buf, smb2_command_cb cb, void *privateData)
{
    return smb2_pwrite_plus_async(m_smbContext, path, openFlag, buf, count, offset,
        cb, privateData);
}

int SmbContextWrapper::SmbReadCompoundAsync(const char* path, uint64_t offset, uint64_t count, uint8_t *buf,
    smb2_command_cb cb, void *privateData)
{
    return smb2_pread_plus_async(m_smbContext, path, buf, count, offset,
        cb, privateData);
}

int SmbContextWrapper::SmbReadAsync(struct smb2fh *fh, uint8_t *buf, uint64_t count, uint64_t offset,
    smb2_command_cb cb, void *privateData)
{
    return smb2_pread_async(m_smbContext, fh, buf, count, offset,
        cb, privateData);
}

int SmbContextWrapper::SmbGetInfo(const char *path, struct SMB2_ALL_INFO *allInfo)
{
    return smb2_get_info(m_smbContext, path, allInfo);
}

int SmbContextWrapper::SmbRename(const char *oldPath, const char *newPath)
{
    std::lock_guard<std::mutex> lk(mtx);
    return smb2_rename(m_smbContext, oldPath, newPath);
}

int SmbContextWrapper::SmbGetInfoAsync(const char *path, struct SMB2_ALL_INFO *allInfo,
    smb2_command_cb cb, void *privateData)
{
    return smb2_get_info_async(m_smbContext, path, allInfo, cb, privateData);
}

void SmbContextWrapper::InitPfd(struct pollfd &pfd)
{
    pfd.fd = smb2_get_fd(m_smbContext);
    pfd.events = smb2_which_events(m_smbContext);
    pfd.revents = 0;
}

int SmbContextWrapper::GetTimeout()
{
    return smb2_get_timeout(m_smbContext);
}

void SmbContextWrapper::RemoveTimeoutPdus()
{
    smb2_remove_timeout_pdus(m_smbContext);
}

void SmbContextWrapper::RemoveErrorPdus()
{
    smb2_remove_error_pdus(m_smbContext);
}

uint64_t SmbContextWrapper::GetLastPollTime()
{
    return m_lastPollTime;
}

void SmbContextWrapper::SetLastPollTime(uint64_t time)
{
    m_lastPollTime = time;
}

int SmbContextWrapper::SmbDisconnect()
{
    return smb2_disconnect_share(m_smbContext);
}

int SmbContextWrapper::SmbWriteAsync(struct smb2fh *smb2fh, const uint8_t *buf, uint32_t count,
    uint64_t offset, smb2_command_cb cb, void *privateData)
{
    return smb2_pwrite_async(m_smbContext, smb2fh, buf, count, offset, cb, privateData);
}

int SmbContextWrapper::SmbWrite(struct smb2fh *smb2fh, const uint8_t *buf, uint32_t count, uint64_t offset)
{
    return smb2_pwrite(m_smbContext, smb2fh, buf, count, offset);
}

int SmbContextWrapper::SmbHardLinkAsync(const char *oldpath, const char *newpath, smb2_command_cb cb, void *privatedata)
{
    return smb2_createhardlink_async(m_smbContext, oldpath, newpath, cb, privatedata);
}
struct smb2fh *SmbContextWrapper::SmbOpen(const char *path, int flags)
{
    return smb2_open(m_smbContext, path, flags);
}

int SmbContextWrapper::SmbReadFileAsync(struct smb2fh *fh, struct SMBWriteAsyncS writeAsyncS, smb2_command_cb cb,
    void *privateData)
{
    return smb2_pread_async(m_smbContext, fh, writeAsyncS.buf, writeAsyncS.count, writeAsyncS.offset,
        cb, privateData);
}

int SmbContextWrapper::SmbClose(struct smb2fh *fh)
{
    return smb2_close(m_smbContext, fh);
}

int SmbContextWrapper::SmbFsync(struct smb2fh *fh)
{
    return smb2_fsync(m_smbContext, fh);
}

int SmbContextWrapper::SmbFsyncAsync(struct smb2fh *fh, smb2_command_cb cb, void *privateData)
{
    return smb2_fsync_async(m_smbContext, fh, cb, privateData);
}

void SmbContextWrapper::SmbFreeData(smb2_filestream_info *ptr)
{
    return smb2_free_data(m_smbContext, ptr);
}

bool SmbContextWrapper::SmbInitContext()
{
    m_smbContext = smb2_init_context();
    if (m_smbContext == nullptr) {
        return false;
    }
    m_initiated = true;
    return m_initiated;
}

bool SmbContextWrapper::SmbValidateArgsKrbInfo()
{
    if (m_args.krb5CcacheFile.empty()) {
        HCP_Log(ERR, MODULE) << "Invalid krb5CcacheFile args."<< HCPENDLOG;
        return false;
    }
    if (m_args.krb5ConfigFile.empty()) {
        HCP_Log(ERR, MODULE) << "Invalid krb5ConfigFile args."<< HCPENDLOG;
        return false;
    }
    struct stat info;
    if (stat(m_args.krb5CcacheFile.c_str(), &info) != 0) {
        HCP_Log(ERR, MODULE) << "There is no krb5CcacheFile."<< HCPENDLOG;
        return false;
    }
    if (stat(m_args.krb5ConfigFile.c_str(), &info) != 0) {
        HCP_Log(ERR, MODULE) << "There is no krb5ConfigFile."<< HCPENDLOG;
        return false;
    }
    return true;
}

bool SmbContextWrapper::SmbValidateArgs()
{
    if (m_args.server.empty() || m_args.share.empty()) {
        HCP_Log(ERR, MODULE) << "Server and share cannot be empty."<< HCPENDLOG;
        return false;
    }
    if (m_args.share[0] == '/') {
        m_args.share = m_args.share.substr(1);
    }
    string::size_type pos = m_args.share.find("/");
    if (pos != m_args.share.npos) {
        m_args.share = m_args.share.substr(0, pos);
    }
    if (m_args.share.size() > MAX_SHARE_SIZE) {
        HCP_Log(ERR, MODULE) << "Share name too long."<< HCPENDLOG;
        return false;
    }
    if (m_args.user.empty()) {
        HCP_Log(ERR, MODULE) << "Username cannot be empty."<< HCPENDLOG;
        return false;
    }
    if (m_args.authType == SmbAuthType::KRB5) {
        if (!SmbValidateArgsKrbInfo()) {
            return false;
        }
    }
    if (m_args.authType == SmbAuthType::NTLMSSP) {
        if (m_args.password.empty()) {
            HCP_Log(ERR, MODULE) << "Password cannot be empty when in NTLMSSP authentication."<< HCPENDLOG;
            return false;
        }
    }
    if (m_args.encryption) {
        if (m_args.version == SmbVersion::VERSION0202 || m_args.version == SmbVersion::VERSION0210) {
            HCP_Log(ERR, MODULE) << "Invalid SMB version 2.0/2.1 when encryption enable." << HCPENDLOG;
            return false;
        }
    }
    return true;
}

void SmbContextWrapper::SmbConfigure()
{
    if (!m_args.domain.empty()) {
        smb2_set_domain(m_smbContext, m_args.domain.c_str());
    }

    if (m_args.encryption) {
        smb2_set_seal(m_smbContext, NUMBER_ONE);
    } else {
        smb2_set_seal(m_smbContext, NUMBER_ZERO);
    }

    if (m_args.sign) {
        smb2_set_sign(m_smbContext, NUMBER_ONE);
    } else {
        smb2_set_sign(m_smbContext, NUMBER_ZERO);
    }

    if (m_args.authType == SmbAuthType::NTLMSSP) {
        smb2_set_authentication(m_smbContext, NUMBER_ONE);
        smb2_set_password(m_smbContext, m_args.password.c_str());
    } else if (m_args.authType == SmbAuthType::KRB5) {
        smb2_set_authentication(m_smbContext, NUMBER_TWO);
    }

    switch (m_args.version) {
        case SmbVersion::VERSION0311:
            smb2_set_version(m_smbContext, SMB2_VERSION_0311);
            break;
        case SmbVersion::VERSION0302:
            smb2_set_version(m_smbContext, SMB2_VERSION_0302);
            break;
        case SmbVersion::VERSION0300:
            smb2_set_version(m_smbContext, SMB2_VERSION_0300);
            break;
        case SmbVersion::VERSION0210:
            smb2_set_version(m_smbContext, SMB2_VERSION_0210);
            break;
        case SmbVersion::VERSION0202:
            smb2_set_version(m_smbContext, SMB2_VERSION_0202);
            break;
        default:
            smb2_set_version(m_smbContext, SMB2_VERSION_0300);
            break;
    }

    smb2_set_user(m_smbContext, m_args.user.c_str());
    smb2_set_timeout(m_smbContext, m_args.timeout);
    smb2_set_security_mode(m_smbContext, SMB2_NEGOTIATE_SIGNING_ENABLED);
}

void SmbContextWrapper::SmbSetKrbEnvironment()
{
    if (m_args.authType == SmbAuthType::KRB5) {
        HCP_Log(INFO, MODULE) << "Start to set krb5 environment variable." << HCPENDLOG;
        string krb5ConfigEnv = m_args.krb5ConfigFile;
        string krb5CcnameEnv = KRB5_CCNAME_ENV_PREFIX + m_args.krb5CcacheFile;
        setenv("KRB5_CONFIG", krb5ConfigEnv.c_str(), NUMBER_ONE);
        setenv("KRB5CCNAME", krb5CcnameEnv.c_str(), NUMBER_ONE);
    }
    return;
}

uint64_t SmbContextWrapper::GetCurTimeSecond()
{
    timeval curTime {};
    gettimeofday(&curTime, nullptr);
    return curTime.tv_sec;
}

int SmbContextWrapper::Poll(int expireTime)
{
    struct pollfd pfd = {0};
    short revents = 0;
    int ret = 0;

    InitPfd(pfd);
    ret = poll(&pfd, 1, expireTime);
    if (ret < 0) {
        ERRLOG("Poll failed, errno: %d, ret: %d", errno, ret);
        return FAILED;
    } else if (ret == 0) {
        return SUCCESS;
    } else {
        revents = pfd.revents;
    }

    if (pfd.fd == -1) {
        ERRLOG("Timeout expired and no connection exists");
        return FAILED;
    }
    if (pfd.revents == 0) {
        return SUCCESS;
    }
    ret = smb2_service(m_smbContext, revents);
    if (ret < 0) {
        ERRLOG("Poll failed, errno: %d, ret: %d", errno, ret);
        return ret;
    }

    return SUCCESS;
}
}
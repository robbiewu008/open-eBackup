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
#ifndef LIBSMB_CONTEXT_WRAPPER_H
#define LIBSMB_CONTEXT_WRAPPER_H
#include <mutex>
#include <poll.h>
#include <sys/stat.h>
#include "smb2/smb2.h"
#include "smb2/libsmb2.h"
#include "smb2/libsmb2-raw.h"
#include "SmbContextArgs.h"
#include "log/Log.h"

namespace Module {

struct SMBWriteAsyncS {
    uint64_t offset = 0;
    uint64_t count = 0;
    uint8_t *buf = nullptr;
};

class SmbContextWrapper {
public:
    explicit SmbContextWrapper(SmbContextArgs args);
    SmbContextWrapper(const SmbContextWrapper &obj) = delete;
    SmbContextWrapper &operator=(const SmbContextWrapper &obj) = delete;
    ~SmbContextWrapper();

    bool Init();
    bool IsInitailized();
    std::string SmbGetClientGuid();
    SmbContextArgs SmbGetArgs();
    bool SmbConnect();
    struct smb2_stats SmbGetStats();
    void SmbSetLogCb(smb2_log_cb cb, void *privateData);
    int SmbEcho();

    struct smb2dirent *SmbReadDir(struct smb2dir *smb2dir);
    void SmbCloseDir(struct smb2dir *smb2dir);
    bool SmbProcess();
    bool SmbService(int revents);
    int SmbStat64(const char *path, struct smb2_stat_64 *st);
    int SmbMkdir(const char *path);

    int SmbOpendirAsync(const char *path, smb2_command_cb cb, void *privateData);
    struct smb2_query_directory_data *SmbAccessDir(const char* path);
    int SmbAccessDirAsync(const char *path, smb2_command_cb cb, void *privateData);
    int SmbQueryDir(struct smb2_query_directory_data *qd);
    int SmbQueryDirAsync(struct smb2_query_directory_data *qd, smb2_command_cb cb, void *privateData);
    int SmbCloseQueryDir(struct smb2_query_directory_data *qd);
    int SmbCloseQueryDirAsync(struct smb2_query_directory_data *qd, smb2_command_cb cb, void *privateData);
    struct smb2dir *SmbGetDir(struct smb2_query_directory_data *qd);
    std::string SmbGetError();
    std::string SmbGetUrl();

    uint32_t SmbGetMaxReadSize();
    uint32_t SmbGetMaxWriteSize();

    int SmbLstat64(const char *path, struct smb2_stat_64 *st);
    int SmbUnlinkAsync(const char *path, smb2_command_cb cb, void *privateData);
    int SmbUnlink(const char *path);
    struct smb2dir *SmbOpendir(const char *path);
    int SmbRmdir(const char *path);

    int SmbWriteFileAsync(struct smb2fh *fh, struct SMBWriteAsyncS writeAsyncS, smb2_command_cb cb,  void *privateData);
    struct smb2_url *SmbParseUrlFull(const char *url);
    struct smb2fh *SmbOpen(const char *path, int flags);
    int SmbReadFileAsync(struct smb2fh *fh, struct SMBWriteAsyncS writeAsyncS, smb2_command_cb cb, void *privateData);
    int SmbClose(struct smb2fh *fh);
    void SmbFreeData(smb2_filestream_info *ptr);
    int SmbOpenAsync(const char *path, int flags, smb2_command_cb cb, void *privateData);
    int SmbMkdirAsync(const char *path, smb2_command_cb cb, void *privateData);
    int SmbCloseAsync(struct smb2fh *smbfh, smb2_command_cb cb, void *privateData);
    void InitPfd(struct pollfd &pfd);
    int GetTimeout();
    void RemoveTimeoutPdus();
    void RemoveErrorPdus();
    uint64_t GetLastPollTime();
    void SetLastPollTime(uint64_t time);
    int SmbDisconnect();
    int SmbWriteAsync(struct smb2fh *smb2fh, const uint8_t *buf, uint32_t count,
        uint64_t offset, smb2_command_cb cb, void *privateData);
    int SmbWrite(struct smb2fh *smb2fh, const uint8_t *buf, uint32_t count, uint64_t offset);
    int SmbHardLinkAsync(const char *oldpath, const char *newpath, smb2_command_cb cb, void *privatedata);
    int SmbGetSdAsync(const char *path, wchar_t **sdstr, smb2_command_cb cb, void *privateData);
    int SmbGetSd(const char *path, wchar_t **sdstr);
    int SmbSetSdAsync(const char *path, wchar_t *sdstr, smb2_command_cb cb, void *privateData);
    int SmbSetSd(const char *path, wchar_t *sdstr);
    int SmbSetBasicInfoAsync(const char *path, struct SMB2_BASIC_INFO *basicInfo,
        smb2_command_cb cb, void *privateData);
    int SmbSetBasicInfo(const char *path, struct SMB2_BASIC_INFO *basicInfo);
    int SmbStatAsync(const char *path, struct smb2_stat_64 *st, smb2_command_cb cb, void *privateData);
    int SmbGetStreamInfoAsync(const char *path, struct smb2_filestream_info **info,
        smb2_command_cb cb, void *privateData);
    int SmbGetStreamInfo(const char *path, struct smb2_filestream_info **info);

    int SmbWriteCompoundAsync(const char* path, uint32_t openFlag, uint64_t offset, uint64_t count,
        const uint8_t *buf, smb2_command_cb cb, void *privateData);
    int SmbReadCompoundAsync(const char* gitpath, uint64_t offset, uint64_t count, uint8_t *buf,
        smb2_command_cb cb, void *privateData);

    int SmbReadAsync(struct smb2fh *fh, uint8_t *buf, uint64_t count, uint64_t offset,
        smb2_command_cb cb, void *privateData);

    int SmbGetInfo(const char *path, struct SMB2_ALL_INFO *allInfo);
    int SmbRename(const char *oldPath, const char *newPath);
    int SmbGetInfoAsync(const char *path, struct SMB2_ALL_INFO *allInfo, smb2_command_cb cb, void *privateData);

    int SmbFsync(struct smb2fh *fh);
    int SmbFsyncAsync(struct smb2fh *fh, smb2_command_cb cb, void *privateData);

    void SmbSetKrbEnvironment();
    int Poll(int expireTime);
    static uint64_t GetCurTimeSecond();

private:
    bool SmbInitContext();
    bool SmbValidateArgsKrbInfo();
    bool SmbValidateArgs();
    void SmbConfigure();

private:
    struct smb2_context *m_smbContext = nullptr;
    std::mutex mtx {};
    SmbContextArgs m_args {};
    bool m_initiated = false;
    uint64_t m_lastPollTime = 0;
    uint64_t m_ioFailures = 0;
};

void SmbLogCb(void *jobData, const char* logString, int logLevel);
}

#endif // LIBSMB_CONTEXT_WRAPPER_H
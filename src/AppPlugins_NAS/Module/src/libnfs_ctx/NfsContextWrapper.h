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
#ifndef MODULE_NFS_CONTEXT_WRAPPER_H
#define MODULE_NFS_CONTEXT_WRAPPER_H

#include <mutex>
#include <poll.h>
#include <utime.h>
#include "nfsc/libnfs.h"
#include "nfsc/libnfs-raw.h"
#include "nfsc/libnfs-raw-nfs.h"
#include "log/Log.h"
#include "ThreadPool.h"

namespace Module {

const int MNT_TIMEOUT_VALUE = 120000;
constexpr uint8_t DME_NAS_SCAN_CMP_PSTN = 6;
constexpr uint16_t MAX_CNT = 1000;
constexpr uint8_t ONE_1 = 1;
constexpr uint8_t TWO_2 = 2;
constexpr uint8_t THREE_3 = 3;
constexpr uint8_t FOUR_4 = 4;
constexpr uint8_t FIVE_5 = 5;
constexpr uint8_t SIX_6 = 6;
constexpr uint8_t SEVEN_7 = 7;
constexpr uint8_t ERR_ENOENT = 2;
constexpr uint8_t ERR_EEXIST = 17;
constexpr uint8_t ERR_NOTDIR = 20;
constexpr uint8_t ERR_EISDIR = 21;
constexpr uint8_t ERR_STALE = 70;
constexpr uint8_t ERR_NOTEMPTY = 39;
constexpr uint32_t NUM_OF_DIR_ENTRY_TO_READ_PER_REQ = 1024;

struct WriteAsyncS {
    uint64_t offset = 0;
    uint64_t count = 0;
    const void *buf = nullptr;
};

struct LibNfsClient {
    char *server = nullptr;
    char *exportcpp = nullptr;
    uint32_t mountPort = 0;
    int isfinished = 0;
};

struct RpcClient {
    char *server = nullptr;
    char *exportcpp = nullptr;
    uint32_t mountPort = 0;
    uint32_t rquotaPort = 0;
    int isfinished = false;
    struct nfs_fh3 rootfh {};
    struct sync_cb_data cb_data;
    std::string aclText = "";
};

class NfsContextWrapper {
public:
    explicit NfsContextWrapper(struct nfs_context *nfs);
    explicit NfsContextWrapper(std::string url, std::string nfsMntArgs);
    NfsContextWrapper(const NfsContextWrapper &obj);
    NfsContextWrapper &operator=(const NfsContextWrapper &obj);
    ~NfsContextWrapper();
    static char* GetLibNfsVersion();
    int NfsMount(bool scanByRoot = false);
    struct nfsdirent *NfsReadDir(struct nfsdir *nfsdir);
    struct nfsdirent *NfsReadDirLock(struct nfsdir *nfsdir);
    void NfsCloseDir(struct nfsdir *nfsdir);
    void NfsCloseDirLock(struct nfsdir *nfsdir);
    void NfsService();
    int NfsStat64(const char *path, struct nfs_stat_64 *st);
    int NfsStat64Lock(const char *path, struct nfs_stat_64 *st);
    int NfsMkdir(const char *path, int mode);
    int NfsMkdirLock(const char *path, int mode);
    int NfsMkdirGetFh(const char *path, int mode, struct nfsfh **nfsfh);
    int NfsMkdirGetFhLock(const char *path, int mode, struct nfsfh **nfsfh);
    int NfsMkdirGetFhWithParentFh(const char *path, const char *dirName, int mode, struct nfsfh **nfsfh,
        struct nfsfh *parentDirFh);
    int NfsMkdirGetFhWithParentFhLock(const char *path, const char *dirName, int mode,
        struct nfsfh **nfsfh, struct nfsfh *parentDirFh);
    int NfsLookupGetFh(const char *path, struct nfsfh **nfsfh);
    int NfsLookupGetFhLock(const char *path, struct nfsfh **nfsfh);
    int NfsLookupGetFhWithParentFh(const char *path, const char *dirName, struct nfsfh **nfsfh,
        struct nfsfh *parentDirFh);
    int NfsLookupGetFhWithParentFhLock(const char *path, const char *dirName, struct nfsfh **nfsfh,
        struct nfsfh *parentDirFh);
    int NfsChown(const char *path, int uid, int gid);
    int NfsChownLock(const char *path, int uid, int gid);

    int NfsOpendirAsync(const char *path, nfs_cb cb, void *privateData, uint32_t dirEntryReadCount);
    int NfsOpendirAsyncLock(const char *path, nfs_cb cb, void *privateData, uint32_t dirEntryReadCount);
    int NfsOpendirAsyncScan(const char *path, nfs_cb cb, void *privateData, nfs_fh_scan &fh,
        uint32_t dirEntryReadCount);
    int NfsOpendirAsyncScanLock(const char *path, nfs_cb cb, void *privateData, nfs_fh_scan &fh,
        uint32_t dirEntryReadCount);
    int NfsOpendirAsyncScanResume(void *privateData);
    int NfsOpendirAsyncScanResumeLock(void *privateData);
    char *NfsGetError();
    char *NfsGetErrorLock();
    struct rpc_context *NfsGetRpcContext();
    void NfsV3GetAcl(struct RpcClient* client, struct nfsdirent* &nfsdirent, std::string &aclValue);
    void NfsV3GetAclLock(struct RpcClient* client, struct nfsdirent* &nfsdirent, std::string &aclValue);

    // backup_sync
    int NfsLstat64(const char *path, struct nfs_stat_64 *st);
    int NfsLstat64Lock(const char *path, struct nfs_stat_64 *st);
    int NfsLstat64WithParentFh(const char *path, struct nfsfh *fh, struct nfs_stat_64 *st);
    int NfsLstat64WithParentFhLock(const char *path, struct nfsfh *fh, struct nfs_stat_64 *st);
    int NfsUnlink(const char *path);
    int NfsUnlinkLock(const char *path);
    int NfsOpendir(const char *path, struct nfsdir **nfsdir, uint32_t dirEntryReadCount);
    int NfsOpendirLock(const char *path, struct nfsdir **nfsdir);
    int NfsRmdir(const char *path);
    int NfsRmdirLock(const char *path);
    int NfsDirMtime(const char *path, struct utimbuf *times);
    int NfsDirMtimeLock(const char *path, struct utimbuf *times);

    // backup async
    int NfsWriteFileAsync(struct nfsfh *nfsfh, nfs_cb cb, struct WriteAsyncS writeAsyncS, void *privateData);
    int NfsWriteFileAsyncLock(struct nfsfh *nfsfh, nfs_cb cb, struct WriteAsyncS writeAsyncS, void *privateData);
    struct nfs_url* NfsParseUrlFull(const char *url);
    int NfsOpen(const char *path, int flags, struct nfsfh **nfsfh);
    int NfsOpenLock(const char *path, int flags, struct nfsfh **nfsfh);
    int NfsReadFileAsync(struct nfsfh *nfsfh, const char *path, uint64_t offset, uint64_t count, nfs_cb cb,
        void *privateData);
    int NfsReadFileAsyncLock(struct nfsfh *nfsfh, const char *path, uint64_t offset, uint64_t count, nfs_cb cb,
        void *privateData);
    int NfsClose(struct nfsfh *nfsfh);
    int NfsCloseLock(struct nfsfh *nfsfh);
    int NfsCreateAsyncWithDirHandle(struct nfsfh *nfsfh, const char *filename, const char *pathName, int flags,
        int mode, nfs_cb cb, void *privateData);
    int NfsCreateAsyncWithDirHandleLock(struct nfsfh *nfsfh, const char *filename, const char *pathName, int flags,
        int mode, nfs_cb cb, void *privateData);
    int NfsCreateAsync(const char *path, int flags, int mode, nfs_cb cb, void *privateData);
    int NfsCreateAsyncLock(const char *path, int flags, int mode, nfs_cb cb, void *privateData);
    int NfsMknodAsync(const char *path, int mode, int dev, nfs_cb cb, void *privateData);
    int NfsMknodAsyncLock(const char *path, int mode, int dev, nfs_cb cb, void *privateData);
    int NfsOpenAsync(const char *path, int flags, nfs_cb cb, void *privateData);
    int NfsOpenAsyncLock(const char *path, int flags, nfs_cb cb, void *privateData);
    int NfsMkdirAsync(const char *path, int mode, nfs_cb cb, void *privateData);
    int NfsMkdirAsyncLock(const char *path, int mode, nfs_cb cb, void *privateData);
    int NfsCloseAsync(struct nfsfh *nfsfh, nfs_cb cb, void *privateData);
    int NfsCloseAsyncLock(struct nfsfh *nfsfh, nfs_cb cb, void *privateData);
    bool IsNfsServiceTaskRunning();
    void InitPfd(struct pollfd &pfd);
    nfs_context *GetNfsContext();
    void NfsDestroyContext();
    int NfsWriteAsync(struct nfsfh *nfsfh, uint64_t offset, uint64_t count, const void *buf, nfs_cb cb,
        void *privateData);
    int NfsWriteAsyncLock(struct nfsfh *nfsfh, uint64_t offset, uint64_t count, const void *buf, nfs_cb cb,
        void *privateData);
    int NfsChmodAsync(const char *path, int mode, nfs_cb cb, void *privateData);
    int NfsChmodAsyncLock(const char *path, int mode, nfs_cb cb, void *privateData);
    int NfsChownAsync(const char *path, int uid, int gid, nfs_cb cb, void *privateData);
    int NfsChownAsyncLock(const char *path, int uid, int gid, nfs_cb cb, void *privateData);
    int NfsChmodChownUtime(struct nfsfh *fh, const char *path, struct nfs_stat_64 allMetaData);
    int NfsChmodChownUtimeLock(struct nfsfh *fh, const char *path, struct nfs_stat_64 allMetaData);
    int NfsFchmodChownUtime(const char *path, int mode, int uid, int gid, struct utimbuf *times);
    int NfsFchmodChownUtimeLock(const char *path, int mode, int uid, int gid, struct utimbuf *times);
    int NfsChmodChownUtimeAsync(struct nfsfh *fh, const char *path, struct nfs_stat_64 allMetaData, nfs_cb cb,
        void *privateData);
    int NfsChmodChownUtimeAsyncLock(struct nfsfh *fh, const char *path, struct nfs_stat_64 allMetaData, nfs_cb cb,
        void *privateData);
    int NfsFchmodChownUtimeAsync(const char *path, struct nfs_stat_64 allMetaData, nfs_cb cb, void *privateData);
    int NfsFchmodChownUtimeAsyncLock(const char *path, struct nfs_stat_64 allMetaData, nfs_cb cb, void *privateData);
    int NfsReadLinkAsync(const char *path, nfs_cb cb, void *privatedata);
    int NfsReadLinkAsyncLock(const char *path, nfs_cb cb, void *privatedata);
    int NfsHardLinkAsync(const char *oldpath, const char *newpath, nfs_cb cb, void *privatedata);
    int NfsHardLinkAsyncLock(const char *oldpath, const char *newpath, nfs_cb cb, void *privatedata);
    int NfsSymLinkAsync(const char *target, const char *newpath, nfs_cb cb, void *privatedata);
    int NfsSymLinkAsyncLock(const char *target, const char *newpath, nfs_cb cb, void *privatedata);
    int NfsLutimeAsync(const char *path, struct timeval *times, nfs_cb cb, void *privateData);
    int NfsLutimeAsyncLock(const char *path, struct timeval *times, nfs_cb cb, void *privateData);
    int NfsFWriteMtimeAsync(const char *path, struct utimbuf *times, nfs_cb cb, void *privateData);
    int NfsFWriteMtimeAsyncLock(const char *path, struct utimbuf *times, nfs_cb cb, void *privateData);
    int NfsFWriteMtimeAsyncWithFH(const char *path, struct nfsfh *fh, struct utimbuf *times, nfs_cb cb,
        void *privateData);
    int NfsFWriteMtimeAsyncWithFHLock(const char *path, struct nfsfh *fh, struct utimbuf *times, nfs_cb cb,
        void *privateData);
    int NfsFchownAsync(struct nfsfh *nfsfh, int uid, int gid, nfs_cb cb, void *privateData);
    int NfsFchownAsyncLock(struct nfsfh *nfsfh, int uid, int gid, nfs_cb cb, void *privateData);
    int NfsFchmodAsync(struct nfsfh *nfsfh, int mode, nfs_cb cb, void *privateData);
    int NfsFchmodAsyncLock(struct nfsfh *nfsfh, int mode, nfs_cb cb, void *privateData);
    int NfsFchownUtimeAsync(struct nfsfh *nfsfh, int uid, int gid, struct utimbuf *times, nfs_cb cb,
        void *privateData);
    int NfsFchownUtimeAsyncLock(struct nfsfh *nfsfh, int uid, int gid, struct utimbuf *times, nfs_cb cb,
        void *privateData);
    int NfsLstatAsync(const char *path, nfs_cb cb, void *privateData);
    int NfsLstatAsyncLock(const char *path, nfs_cb cb, void *privateData);
    int NfsLstatAsyncWithDirHandle(const char *path, struct nfsfh *fh, nfs_cb cb, void *privateData);
    int NfsLstatAsyncWithDirHandleLock(const char *path, struct nfsfh *fh, nfs_cb cb, void *privateData);
    int NfsFsync(struct nfsfh *nfsfh);
    int NfsFsyncLock(struct nfsfh *nfsfh);
    int NfsFsyncAsync(struct nfsfh *nfsfh, nfs_cb cb, void *privateData);
    int NfsFsyncAsyncLock(struct nfsfh *nfsfh, nfs_cb cb, void *privateData);
    int NfsUnlinkAsync(const char *path, nfs_cb cb, void *privateData);
    int NfsUnlinkAsyncLock(const char *path, nfs_cb cb, void *privateData);
    uint64_t GetNfsReadMaxSize();
    uint64_t GetNfsReadMaxSizeLock();
    uint64_t GetNfsWriteMaxSize();
    uint64_t GetNfsWriteMaxSizeLock();
    void NfsSetLogCb(nfs_log_cb cb, void *privateData);
    int NfsNullSync();
    int NfsNullSyncLock();

    char* GetRidOfSepInPathName(char *rawPath);
    void PrintRpcCounters(const std::string containerType);
    int64_t GetCurTime();
    void Poll(int expireTime);
public:
    struct nfs_context *m_nfsContext = nullptr;
    std::mutex mtx {};
    std::string m_url {};
    bool m_isNfsServiceTaskRunning = false;

private:
    int NfsValidateUrl(struct LibNfsClient &client);
    int NfsInitCtx(bool scanByRoot = false);
    static std::string NfsV3AclTranslatePermToMaskString(u_int perm);
    static void NfsV3GetAclCb(const struct rpc_context* rpc, int status, const GETACL3res* res,
        struct RpcClient* client);

    std::atomic<bool> m_isDestroy = false;
};

class NfsServiceTask : public Module::ExecutableItem {
public:
    NfsServiceTask(std::shared_ptr<NfsContextWrapper> wrapper, int32_t revents, std::size_t reqId)
        : m_reqId(reqId), m_wrapper(wrapper), m_revents(revents) {};
    virtual ~NfsServiceTask() {};
    virtual void Exec() override;

private:
    std::size_t m_reqId = 0;
    std::shared_ptr<NfsContextWrapper> m_wrapper = nullptr;
    int32_t m_revents = 0;
};

void NasLogCb(void *jobData, const char* logString, int logLevel);
}
#endif // MODULE_NFS_CONTEXT_WRAPPER_H

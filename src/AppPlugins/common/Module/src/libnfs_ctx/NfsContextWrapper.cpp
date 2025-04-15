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
#include "NfsContextWrapper.h"
#include "define/Types.h"
#include "define/Defines.h"
#include "common/MpString.h"
#include "securec.h"
namespace {
    constexpr auto MODULE = "NFSCONTEXT_WRAPPER";
    const int MP_FAILED = 1;
}
using namespace std;

namespace Module {

NfsContextWrapper::NfsContextWrapper(struct nfs_context *nfs): m_nfsContext(nfs)
{
}

NfsContextWrapper::NfsContextWrapper(std::string url, std::string nfsMntArgs)
{
    // Codex Issue fix. Self assign
    nfsMntArgs = nfsMntArgs;
    m_url = url;
}

NfsContextWrapper::~NfsContextWrapper()
{
}

NfsContextWrapper::NfsContextWrapper(const NfsContextWrapper &obj)
{
    m_nfsContext = obj.m_nfsContext;
}

NfsContextWrapper& NfsContextWrapper::operator=(const NfsContextWrapper &obj)
{
    if (this != &obj) {
        m_nfsContext = obj.m_nfsContext;
    }
    return *this;
}

char* NfsContextWrapper::GetLibNfsVersion()
{
    return get_libnfs_version();
}

void NasLogCb(void *jobData, const char* logString, int logLevel)
{
    auto *nfsContextWrapper = static_cast<NfsContextWrapper *>(jobData);
    HCP_Log(logLevel, MODULE) << "Nfs Log: " << logString << HCPENDLOG;
}

void NfsContextWrapper::NfsSetLogCb(nfs_log_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return;
    }
    nfs_set_log_cb(m_nfsContext, cb, privateData);
}

int NfsContextWrapper::NfsNullSync()
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_null_sync(m_nfsContext);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsNullSync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsNullSyncLock()
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsNullSync();
}

int NfsContextWrapper::NfsMount(bool scanByRoot)
{
    std::string localUrl = m_url;
    std::lock_guard<std::mutex> lk(mtx);
    struct LibNfsClient client {};
    std::string nfsMntArgs = "auto-traverse-mounts=0";

    HCP_Log(INFO, MODULE) << "URL check: " << localUrl << HCPENDLOG;

    /* Before parse , extract the server and path */
    if (NfsValidateUrl(client) != SUCCESS) {
        return FAILED;
    }

    if (NfsInitCtx(scanByRoot) != SUCCESS) {
        free(client.server);
        free(client.exportcpp);
        return FAILED;
    }

    /* Now replace "?" with escape char */
    localUrl = CMpString::StrReplace(localUrl, "?", "%3F");
    localUrl.append("?");
    localUrl.append(nfsMntArgs);

    /* nfs_parse_url_dir will set the above mount option into m_nfsContext */
    struct nfs_url *nfsURL = nfs_parse_url_dir(m_nfsContext, localUrl.c_str());
    if (nfsURL == nullptr) {
        HCP_Log(ERR, MODULE) << "Failed to parse nfs url :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
        if (m_nfsContext != nullptr) {
            nfs_destroy_context(m_nfsContext);
        }
        free(client.server);
        free(client.exportcpp);
        return FAILED;
    }
    nfs_destroy_url(nfsURL);

    HCP_Log(INFO, MODULE) << "server: " << client.server << "Path: " << client.exportcpp << HCPENDLOG;

    NfsSetLogCb(NasLogCb, this);

    nfs_set_version(m_nfsContext, NFS_V3);
    nfs_set_dircache(m_nfsContext, 0);
    nfs_set_timeout(m_nfsContext, MNT_TIMEOUT_VALUE);
    nfs_set_autoreconnect(m_nfsContext, -1);
    int ret = nfs_mount(m_nfsContext, client.server, client.exportcpp);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "Failed to mount nfs share: " << client.server <<
            " absPath: " << client.exportcpp << " Nfs Error: " << nfs_get_error(m_nfsContext) << HCPENDLOG;
        free(client.server);
        free(client.exportcpp);
        if (m_nfsContext != nullptr) {
            nfs_destroy_context(m_nfsContext);
        }
        return ret;
    }

    HCP_Log(DEBUG, MODULE) << "mounting success." << HCPENDLOG;
    free(client.server);
    free(client.exportcpp);
    return SUCCESS;
}

// 输入"//path/，"去掉路径前后的'/'， 返回"/path"这样的字符串
char* NfsContextWrapper::GetRidOfSepInPathName(char *rawPath)
{
    int len = strlen(rawPath);
    if (len <= 1) {
        return nullptr;
    }
    int lPos = 0;
    int rPos = len - 1;
    for (int i = 0; i < len; ++i) {
        if (rawPath[i] != '/') {
            lPos = i;
            break;
        }
    }
    for (int i = len - 1; i > 0; --i) {
        if (rawPath[i] != '/') {
            rPos = i;
            break;
        }
    }

    // rPos - lPos + 1是path长度，再加一个斜杠的长度
    int pathNameLen = (rPos - lPos + 1) + 1;
    if (pathNameLen <= 0 || pathNameLen > len) {
        return nullptr;
    }
    char *pathName = (char *)malloc((pathNameLen + 1) * sizeof(char));
    errno_t err = strncpy_s(pathName, pathNameLen + 1, rawPath + lPos - 1, pathNameLen);
    if (err != EOK) {
        free(pathName);
        HCP_Log(ERR, MODULE) << "strncpy_s failed." << HCPENDLOG;
        return nullptr;
    }
    pathName[pathNameLen] = 0;
    DBGLOG("Get Rid Of Sep In PathName: %d %d, Len: %d, get new path: %s, old path: %s",
        lPos, rPos, pathNameLen, pathName, rawPath);
    return pathName;
}

int NfsContextWrapper::NfsValidateUrl(struct LibNfsClient &client)
{
    char *pServer = nullptr;
    char *pPath = nullptr;
    char *pStrp = nullptr;
    std::string localUrl = m_url;

    if (localUrl.empty()) {
        HCP_Log(ERR, MODULE) << "No URL specified." << HCPENDLOG;
        return FAILED;
    }
    if (strncmp(localUrl.c_str(), "nfs://", DME_NAS_SCAN_CMP_PSTN)) {
        HCP_Log(ERR, MODULE) << "Invalid URL specified." << localUrl << HCPENDLOG;
        return FAILED;
    }
    pServer = strdup(localUrl.c_str() + DME_NAS_SCAN_CMP_PSTN);
    if (pServer == nullptr) {
        HCP_Log(ERR, MODULE) << "Failed to strdup server string." << HCPENDLOG;
        return FAILED;
    }
    if (pServer[0] == '/' || pServer[0] == '\0') {
        HCP_Log(ERR, MODULE) << "Invalid server string." << HCPENDLOG;
        free(pServer);
        return FAILED;
    }
    pStrp = strchr(pServer, '/');
    if (pStrp == nullptr) {
        HCP_Log(ERR, MODULE) << "Invalid URL specified:" << localUrl << HCPENDLOG;
        free(pServer);
        return FAILED;
    }
    pPath = GetRidOfSepInPathName(pStrp);
    if (pPath == nullptr) {
        HCP_Log(ERR, MODULE) << "Failed to get server string" << HCPENDLOG;
        free(pServer);
        return FAILED;
    }
    if (pPath[0] != '/') {
        HCP_Log(ERR, MODULE) << "Invalid path." << HCPENDLOG;
        free(pServer);
        free(pPath);
        return FAILED;
    }
    *pStrp = 0;
    client.server = pServer;
    client.exportcpp = pPath;
    client.isfinished = 0;
    return SUCCESS;
}

int NfsContextWrapper::NfsInitCtx(bool scanByRoot)
{
    HCP_Log(DEBUG, MODULE) << "Initialise the context started." << HCPENDLOG;
    m_nfsContext = scanByRoot ? nfs_init_context_root() : nfs_init_context();
    if (m_nfsContext == nullptr) {
        HCP_Log(ERR, MODULE) << "failed to init context." << HCPENDLOG;
        return FAILED;
    }
    return SUCCESS;
}

struct nfsdirent *NfsContextWrapper::NfsReadDir(struct nfsdir *nfsdir)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return nullptr;
    }
    /**
     * This API works only on the nfsdir pointer passed which is not shared.
     * Hence there is no need to lock via std::lock_guard<std::mutex> lk(mtx);
     */
    return nfs_readdir(m_nfsContext, nfsdir);
}

struct nfsdirent *NfsContextWrapper::NfsReadDirLock(struct nfsdir *nfsdir)
{
    std::lock_guard<std::mutex> lk(mtx);
    return nfs_readdir(m_nfsContext, nfsdir);
}

void NfsContextWrapper::NfsCloseDir(struct nfsdir *nfsdir)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return;
    }
    /**
     * This API works only on the nfsdir pointer passed which is not shared.
     * Hence there is no need to lock via std::lock_guard<std::mutex> lk(mtx);
     */
    nfs_closedir(m_nfsContext, nfsdir);
}

void NfsContextWrapper::NfsCloseDirLock(struct nfsdir *nfsdir)
{
    std::lock_guard<std::mutex> lk(mtx);
    nfs_closedir(m_nfsContext, nfsdir);
}

void NfsContextWrapper::NfsService()
{
    struct pollfd pfd = {0};
    int revents = 0;
    int ret;
    std::unique_lock<std::mutex> lk(mtx);
    pfd.fd = nfs_get_fd(m_nfsContext);
    pfd.events = nfs_which_events(m_nfsContext);
    lk.unlock();
    pfd.revents = 0;
    ret = poll(&pfd, 1, 0);
    if (ret < 0) {
        revents = -1;
    } else if (ret > 0) {
        revents = pfd.revents;
    }
    lk.lock();
    if (nfs_service(m_nfsContext, revents) < 0) {
        if (revents != -1) {
        }
    }
    lk.unlock();
}

int NfsContextWrapper::NfsOpendirAsync(const char *path, nfs_cb cb, void *privateData, uint32_t dirEntryReadCount)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_opendir_async(m_nfsContext, path, cb, privateData, 0, dirEntryReadCount);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsOpendirAsync failed, ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsOpendirAsyncLock(const char *path, nfs_cb cb, void *privateData, uint32_t dirEntryReadCount)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsOpendirAsync(path, cb, privateData, dirEntryReadCount);
}

int NfsContextWrapper::NfsOpendirAsyncScanResume(void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_opendir_async_scan_resume(m_nfsContext, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsOpendirAsyncScanResume failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsOpendirAsyncScanResumeLock(void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsOpendirAsyncScanResume(privateData);
}

int NfsContextWrapper::NfsStat64(const char *path, struct nfs_stat_64 *st)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_stat64(m_nfsContext, path, st);
    if (ret != 0 && ret != -ERR_ENOENT) {
        HCP_Log(ERR, MODULE) << "NfsStat64 failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsStat64Lock(const char *path, struct nfs_stat_64 *st)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsStat64(path, st);
}

int NfsContextWrapper::NfsMkdir(const char *path, int mode)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_mkdir2(m_nfsContext, path, mode);
    if (ret != 0 && ret != -ERR_EEXIST) {
        HCP_Log(ERR, MODULE) << "NfsMkdir failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsMkdirLock(const char *path, int mode)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsMkdir(path, mode);
}

int NfsContextWrapper::NfsMkdirGetFh(const char *path, int mode, struct nfsfh **nfsfh)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_mkdir3(m_nfsContext, path, mode, nfsfh);
    if (ret != 0 && ret != -ERR_ENOENT && ret != -ERR_NOTDIR &&
        ret != -EINTR && ret != -ERR_EEXIST) {
        HCP_Log(ERR, MODULE) << "NfsMkdirGetFh failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsMkdirGetFhLock(const char *path, int mode, struct nfsfh **nfsfh)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsMkdirGetFh(path, mode, nfsfh);
}

int NfsContextWrapper::NfsMkdirGetFhWithParentFh(const char *path, const char *dirName, int mode,
    struct nfsfh **nfsfh, struct nfsfh *parentDirFh)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_mkdir_with_parent_fh(m_nfsContext, path, dirName, mode, nfsfh, parentDirFh);
    if (ret != 0 && ret != -ERR_ENOENT && ret != -ERR_NOTDIR &&
        ret != -EINTR && ret != -ERR_EEXIST) {
        HCP_Log(ERR, MODULE) << "NfsMkdirGetFhWithParentFh failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsMkdirGetFhWithParentFhLock(const char *path, const char *dirName, int mode,
    struct nfsfh **nfsfh, struct nfsfh *parentDirFh)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsMkdirGetFhWithParentFh(path, dirName, mode, nfsfh, parentDirFh);
}

int NfsContextWrapper::NfsLookupGetFh(const char *path, struct nfsfh **nfsfh)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_lookup_get_fh(m_nfsContext, path, nfsfh);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsLookupGetFh failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsLookupGetFhLock(const char *path, struct nfsfh **nfsfh)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsLookupGetFh(path, nfsfh);
}

int NfsContextWrapper::NfsLookupGetFhWithParentFh(const char *path, const char *dirName,
    struct nfsfh **nfsfh, struct nfsfh *parentDirFh)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_lookup_get_fh_with_parent_fh(m_nfsContext, path, dirName, nfsfh, parentDirFh);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsLookupGetFhWithParentFh failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsLookupGetFhWithParentFhLock(const char *path, const char *dirName,
    struct nfsfh **nfsfh, struct nfsfh *parentDirFh)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsLookupGetFhWithParentFh(path, dirName, nfsfh, parentDirFh);
}

int NfsContextWrapper::NfsChown(const char *path, int uid, int gid)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_chown(m_nfsContext, path, uid, gid);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsChown failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsChownLock(const char *path, int uid, int gid)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsChown(path, uid, gid);
}

int NfsContextWrapper::NfsOpendirAsyncScan(const char *path, nfs_cb cb, void *privateData, nfs_fh_scan &fh,
    uint32_t dirEntryReadCount)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_opendir_async_scan(m_nfsContext, path, cb, privateData, &fh, dirEntryReadCount);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsOpendirAsyncScan failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsOpendirAsyncScanLock(const char *path, nfs_cb cb, void *privateData, nfs_fh_scan &fh,
    uint32_t dirEntryReadCount)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsOpendirAsyncScan(path, cb, privateData, fh, dirEntryReadCount);
}

char *NfsContextWrapper::NfsGetError()
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return "Nfscontext destroyed";
    }
    return nfs_get_error(m_nfsContext);
}

char *NfsContextWrapper::NfsGetErrorLock()
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsGetError();
}

struct rpc_context *NfsContextWrapper::NfsGetRpcContext()
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return nullptr;
    }
    return nfs_get_rpc_context(m_nfsContext);
}

void NfsContextWrapper::NfsV3GetAcl(struct RpcClient* client, struct nfsdirent* &nfsdirent, std::string &aclValue)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return;
    }
    GETACL3args args;
    args.dir.data.data_val = nfsdirent->fh.value;
    args.dir.data.data_len = (u_int) nfsdirent->fh.len;
    client->cb_data.is_finished = 0;
    args.mask = NFSACL_MASK_ACL_ENTRY | NFSACL_MASK_ACL_COUNT | NFSACL_MASK_ACL_DEFAULT_ENTRY
                | NFSACL_MASK_ACL_DEFAULT_COUNT;
    struct rpc_context* rpc = NfsGetRpcContext();
    if (rpc_nfsacl_getacl_async(rpc, reinterpret_cast<rpc_cb>(NfsV3GetAclCb), &args, client) != 0) {
        HCP_Log(ERR, MODULE) << "failed to get acl " << HCPENDLOG;
    }
    wait_for_rpc_reply(rpc, &client->cb_data);
    if (client->aclText.size() > 0) {
        aclValue = client->aclText;
        client->aclText = "";
    }
}

void NfsContextWrapper::NfsV3GetAclLock(struct RpcClient* client, struct nfsdirent* &nfsdirent, std::string &aclValue)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsV3GetAcl(client, nfsdirent, aclValue);
}

int NfsContextWrapper::NfsReadFileAsync(struct nfsfh *nfsfh, const char *path, uint64_t offset,
    uint64_t count, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_pread_async2(m_nfsContext, nfsfh, path, offset, count, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsReadFileAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsReadFileAsyncLock(struct nfsfh *nfsfh, const char *path, uint64_t offset,
    uint64_t count, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsReadFileAsync(nfsfh, path, offset, count, cb, privateData);
}

int NfsContextWrapper::NfsWriteFileAsync(struct nfsfh *nfsfh, nfs_cb cb, struct WriteAsyncS writeAsyncS,
    void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_pwrite_async(m_nfsContext, nfsfh, writeAsyncS.offset, writeAsyncS.count, writeAsyncS.buf, cb,
        privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsWriteFileAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsWriteFileAsyncLock(struct nfsfh *nfsfh, nfs_cb cb, struct WriteAsyncS writeAsyncS,
    void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsWriteFileAsync(nfsfh, cb, writeAsyncS, privateData);
}

int NfsContextWrapper::NfsWriteAsync(struct nfsfh *nfsfh, uint64_t offset, uint64_t count, const void *buf,
    nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_pwrite_async(m_nfsContext, nfsfh, offset, count, buf, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsWriteAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsWriteAsyncLock(struct nfsfh *nfsfh, uint64_t offset, uint64_t count, const void *buf,
    nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsWriteAsync(nfsfh, offset, count, buf, cb, privateData);
}

struct nfs_url* NfsContextWrapper::NfsParseUrlFull(const char *url)
{
    std::lock_guard<std::mutex> lk(mtx);
    return nfs_parse_url_full(m_nfsContext, url);
}

int NfsContextWrapper::NfsOpen(const char *path, int flags, struct nfsfh **nfsfh)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_open(m_nfsContext, path, flags, nfsfh);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsOpen failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsOpenLock(const char *path, int flags, struct nfsfh **nfsfh)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsOpen(path, flags, nfsfh);
}

int NfsContextWrapper::NfsLstat64(const char *path, struct nfs_stat_64 *st)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_lstat64(m_nfsContext, path, st);
    if (ret != 0 && ret != -ERR_ENOENT) {
        HCP_Log(ERR, MODULE) << "NfsLstat64 failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsLstat64Lock(const char *path, struct nfs_stat_64 *st)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsLstat64(path, st);
}

int NfsContextWrapper::NfsLstat64WithParentFh(const char *path, struct nfsfh *fh, struct nfs_stat_64 *st)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_lstat64_with_parentdir_fh(m_nfsContext, path, fh, st);
    if (ret != 0 && ret != -ERR_ENOENT) {
        HCP_Log(ERR, MODULE) << "NfsLstat64WithParentFh failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsLstat64WithParentFhLock(const char *path, struct nfsfh *fh, struct nfs_stat_64 *st)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsLstat64WithParentFh(path, fh, st);
}

int NfsContextWrapper::NfsUnlink(const char *path)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_unlink(m_nfsContext, path);
    if (ret != 0 && ret != -ERR_ENOENT && ret != -ERR_EISDIR &&
        ret != -ERR_EEXIST && ret != -ERR_NOTEMPTY && ret != -EINTR && ret != -EAGAIN) {
        HCP_Log(ERR, MODULE) << "NfsUnlink failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsUnlinkLock(const char *path)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsUnlink(path);
}

int NfsContextWrapper::NfsRmdir(const char *path)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_rmdir(m_nfsContext, path);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsRmdir failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsRmdirLock(const char *path)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsRmdir(path);
}

int NfsContextWrapper::NfsOpendir(const char *path, struct nfsdir **nfsdir, uint32_t dirEntryReadCount)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_opendir(m_nfsContext, path, nfsdir, dirEntryReadCount);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsOpendir failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsOpendirLock(const char *path, struct nfsdir **nfsdir)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsOpendir(path, nfsdir, NUM_OF_DIR_ENTRY_TO_READ_PER_REQ);
}

int NfsContextWrapper::NfsDirMtime(const char *path, struct utimbuf *times)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_utime(m_nfsContext, path, times);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsDirMtime failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsDirMtimeLock(const char *path, struct utimbuf *times)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsDirMtime(path, times);
}

int NfsContextWrapper::NfsClose(struct nfsfh *nfsfh)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_close(m_nfsContext, nfsfh);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsClose failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsCloseLock(struct nfsfh *nfsfh)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsClose(nfsfh);
}

int NfsContextWrapper::NfsCreateAsync(const char *path, int flags, int mode, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_create_async(m_nfsContext, path, flags, mode, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsCreateAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsCreateAsyncLock(const char *path, int flags, int mode, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsCreateAsync(path, flags, mode, cb, privateData);
}

int NfsContextWrapper::NfsCreateAsyncWithDirHandle(struct nfsfh *nfsfh, const char *filename, const char *pathName,
    int flags, int mode, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_create_async2(m_nfsContext, filename, pathName, flags, mode, cb, privateData, nfsfh);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsCreateAsyncWithDirHandle failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsCreateAsyncWithDirHandleLock(struct nfsfh *nfsfh, const char *filename, const char *pathName,
    int flags, int mode, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsCreateAsyncWithDirHandle(nfsfh, filename, pathName, flags, mode, cb, privateData);
}

int NfsContextWrapper::NfsMknodAsync(const char *path, int mode, int dev, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_mknod_async(m_nfsContext, path, mode, dev, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsMknodAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsMknodAsyncLock(const char *path, int mode, int dev, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsMknodAsync(path, mode, dev, cb, privateData);
}

int NfsContextWrapper::NfsOpenAsync(const char *path, int flags, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_open_async(m_nfsContext, path, flags, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsOpenAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsOpenAsyncLock(const char *path, int flags, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsOpenAsync(path, flags, cb, privateData);
}

int NfsContextWrapper::NfsCloseAsync(struct nfsfh *nfsfh, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_close_async(m_nfsContext, nfsfh, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsCloseAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsCloseAsyncLock(struct nfsfh *nfsfh, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsCloseAsync(nfsfh, cb, privateData);
}

int NfsContextWrapper::NfsMkdirAsync(const char *path, int mode, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_mkdir2_async(m_nfsContext, path, mode, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsMkdirAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsMkdirAsyncLock(const char *path, int mode, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsMkdirAsync(path, mode, cb, privateData);
}

int NfsContextWrapper::NfsChmodAsync(const char *path, int mode, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_chmod_async(m_nfsContext, path, mode, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsChmodAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsChmodAsyncLock(const char *path, int mode, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsChmodAsync(path, mode, cb, privateData);
}

int NfsContextWrapper::NfsChownAsync(const char *path, int uid, int gid, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_chown_async(m_nfsContext, path, uid, gid, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsChownAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsChownAsyncLock(const char *path, int uid, int gid, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsChownAsync(path, uid, gid, cb, privateData);
}

int NfsContextWrapper::NfsFchmodChownUtime(const char *path, int mode, int uid, int gid, struct utimbuf *times)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_chmod_chown_utime(m_nfsContext, path, mode, uid, gid, times);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsFchmodChownUtime failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsFchmodChownUtimeLock(const char *path, int mode, int uid, int gid, struct utimbuf *times)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsFchmodChownUtime(path, mode, uid, gid, times);
}

int NfsContextWrapper::NfsChmodChownUtimeAsync(struct nfsfh *fh, const char *path,
    struct nfs_stat_64 allMetaData, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    struct utimbuf futimes {};
    futimes.modtime = (time_t) allMetaData.nfs_mtime;
    futimes.actime = (time_t) allMetaData.nfs_atime;

    int ret = nfs_chmod_chown_utime_async_with_fh(m_nfsContext, fh, path, allMetaData.nfs_mode,
                                                  allMetaData.nfs_uid, allMetaData.nfs_gid, &futimes,
                                                  cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsChownChmodUtimeAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsChmodChownUtimeAsyncLock(struct nfsfh *fh, const char *path,
    struct nfs_stat_64 allMetaData, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsChmodChownUtimeAsync(fh, path, allMetaData, cb, privateData);
}

int NfsContextWrapper::NfsFchmodChownUtimeAsync(const char *path, struct nfs_stat_64 allMetaData, nfs_cb cb,
    void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    struct utimbuf futimes {};
    futimes.modtime = (time_t) allMetaData.nfs_mtime;
    futimes.actime = (time_t) allMetaData.nfs_atime;

    int ret = nfs_chmod_chown_utime_async(m_nfsContext, path, allMetaData.nfs_mode, allMetaData.nfs_uid,
        allMetaData.nfs_gid, &futimes, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsFchmodChownUtimeAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsFchmodChownUtimeAsyncLock(const char *path, struct nfs_stat_64 allMetaData, nfs_cb cb,
    void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsFchmodChownUtimeAsyncLock(path, allMetaData, cb, privateData);
}

int NfsContextWrapper::NfsChmodChownUtime(struct nfsfh *fh, const char *path,
    struct nfs_stat_64 allMetaData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    struct utimbuf futimes {};
    futimes.modtime = (time_t) allMetaData.nfs_mtime;
    futimes.actime = (time_t) allMetaData.nfs_atime;

    int ret = nfs_chmod_chown_utime_with_parent_fh(m_nfsContext, path, fh, allMetaData.nfs_mode, allMetaData.nfs_uid,
        allMetaData.nfs_gid, &futimes);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsChownChmodUtimeAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsChmodChownUtimeLock(struct nfsfh *fh, const char *path,
    struct nfs_stat_64 allMetaData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsChmodChownUtime(fh, path, allMetaData);
}

int NfsContextWrapper::NfsFWriteMtimeAsync(const char *path, struct utimbuf *times, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_utime_async(m_nfsContext, path, times, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsFWriteMtimeAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsFWriteMtimeAsyncLock(const char *path, struct utimbuf *times, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsFWriteMtimeAsync(path, times, cb, privateData);
}

int NfsContextWrapper::NfsFWriteMtimeAsyncWithFH(const char *path, struct nfsfh *fh, struct utimbuf *times,
    nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_futime_async(m_nfsContext, path, fh, times, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsFWriteMtimeAsyncWithFH failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsFWriteMtimeAsyncWithFHLock(const char *path, struct nfsfh *fh, struct utimbuf *times,
    nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsFWriteMtimeAsyncWithFH(path, fh, times, cb, privateData);
}

int NfsContextWrapper::NfsFchownAsync(struct nfsfh *nfsfh, int uid, int gid, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_fchown_async(m_nfsContext, nfsfh, uid, gid, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsFchownAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsFchownAsyncLock(struct nfsfh *nfsfh, int uid, int gid, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsFchownAsync(nfsfh, uid, gid, cb, privateData);
}

int NfsContextWrapper::NfsFchownUtimeAsync(struct nfsfh *nfsfh, int uid, int gid, struct utimbuf *times, nfs_cb cb,
    void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_fchown_utime_async(m_nfsContext, nfsfh, uid, gid, times, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsFchownUtimeAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsFchownUtimeAsyncLock(struct nfsfh *nfsfh, int uid, int gid, struct utimbuf *times, nfs_cb cb,
    void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsFchownUtimeAsync(nfsfh, uid, gid, times, cb, privateData);
}

int NfsContextWrapper::NfsFchmodAsync(struct nfsfh *nfsfh, int mode, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_fchmod_async(m_nfsContext, nfsfh, mode, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsFchmodAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsFchmodAsyncLock(struct nfsfh *nfsfh, int mode, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsFchmodAsync(nfsfh, mode, cb, privateData);
}

int NfsContextWrapper::NfsReadLinkAsync(const char *path, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_readlink_async(m_nfsContext, path, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsReadLinkAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsReadLinkAsyncLock(const char *path, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsReadLinkAsync(path, cb, privateData);
}

int NfsContextWrapper::NfsHardLinkAsync(const char *oldpath, const char *newpath, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_link_no_follow_async(m_nfsContext, oldpath, newpath, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsHardLinkAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsHardLinkAsyncLock(const char *oldpath, const char *newpath, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsHardLinkAsync(oldpath, newpath, cb, privateData);
}

int NfsContextWrapper::NfsSymLinkAsync(const char *target, const char *newpath, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_symlink_async(m_nfsContext, target, newpath, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsSymLinkAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsSymLinkAsyncLock(const char *target, const char *newpath, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsSymLinkAsync(target, newpath, cb, privateData);
}

int NfsContextWrapper::NfsLutimeAsync(const char *path, struct timeval *times, nfs_cb cb,
    void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_lutimes_async(m_nfsContext, path, times, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsLutimeAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsLutimeAsyncLock(const char *path, struct timeval *times, nfs_cb cb,
    void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsLutimeAsync(path, times, cb, privateData);
}

int NfsContextWrapper::NfsLstatAsyncWithDirHandle(const char *path, struct nfsfh *fh, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_lstat64_async_with_parentdir_fh(m_nfsContext, path, fh, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsLstatAsyncWithDirHandle failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsLstatAsyncWithDirHandleLock(const char *path, struct nfsfh *fh, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsLstatAsyncWithDirHandle(path, fh, cb, privateData);
}

int NfsContextWrapper::NfsLstatAsync(const char *path, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_lstat64_async(m_nfsContext, path, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsLstatAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsLstatAsyncLock(const char *path, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsLstatAsync(path, cb, privateData);
}

int NfsContextWrapper::NfsFsync(struct nfsfh *nfsfh)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_fsync(m_nfsContext, nfsfh);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsFsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsFsyncLock(struct nfsfh *nfsfh)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsFsync(nfsfh);
}

int NfsContextWrapper::NfsFsyncAsync(struct nfsfh *nfsfh, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_fsync_async(m_nfsContext, nfsfh, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsFsyncAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsFsyncAsyncLock(struct nfsfh *nfsfh, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsFsyncAsync(nfsfh, cb, privateData);
}

int NfsContextWrapper::NfsUnlinkAsync(const char *path, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_unlink_async(m_nfsContext, path, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsUnlinkAsync failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

int NfsContextWrapper::NfsUnlinkAsyncLock(const char *path, nfs_cb cb, void *privateData)
{
    std::lock_guard<std::mutex> lk(mtx);
    return NfsUnlinkAsync(path, cb, privateData);
}

int NfsContextWrapper::NfsUnlinkAsyncWithParentFh(const char *path, struct nfsfh *fh, nfs_cb cb, void *privateData)
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return MP_FAILED;
    }
    int ret = nfs_unlink_async2(m_nfsContext, fh, path, cb, privateData);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "NfsUnlinkAsync2 failed , ret :" << ret
            << ", error :" << nfs_get_error(m_nfsContext) << HCPENDLOG;
    }
    return ret;
}

uint64_t NfsContextWrapper::GetNfsReadMaxSize()
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return 0;
    }
    uint64_t readMaxSize = nfs_get_readmax(m_nfsContext);
    HCP_Log(DEBUG, MODULE) << "NfsReadMaxSize :" << readMaxSize << HCPENDLOG;
    return readMaxSize;
}

uint64_t NfsContextWrapper::GetNfsReadMaxSizeLock()
{
    std::lock_guard<std::mutex> lk(mtx);
    return GetNfsReadMaxSize();
}

uint64_t NfsContextWrapper::GetNfsWriteMaxSize()
{
    if (m_isDestroy) {
        HCP_Log(ERR, MODULE) << "Nfscontext destroyed" << HCPENDLOG;
        return 0;
    }
    uint64_t writeMaxSize = nfs_get_writemax(m_nfsContext);
    HCP_Log(DEBUG, MODULE) << "NfsWriteMaxSize :" << writeMaxSize << HCPENDLOG;
    return writeMaxSize;
}

uint64_t NfsContextWrapper::GetNfsWriteMaxSizeLock()
{
    std::lock_guard<std::mutex> lk(mtx);
    return GetNfsWriteMaxSize();
}

bool NfsContextWrapper::IsNfsServiceTaskRunning()
{
    lock_guard<std::mutex> lk(mtx);
    if (m_isNfsServiceTaskRunning) {
        return true;
    }
    m_isNfsServiceTaskRunning = true;
    return false;
}

void NfsContextWrapper::InitPfd(struct pollfd &pfd)
{
    std::lock_guard<std::mutex> lk(mtx);
    pfd.fd = nfs_get_fd(m_nfsContext);
    pfd.events = nfs_which_events(m_nfsContext);
    pfd.revents = 0;
}

nfs_context *NfsContextWrapper::GetNfsContext()
{
    return m_nfsContext;
}

void NfsContextWrapper::NfsDestroyContext()
{
    std::lock_guard<std::mutex> lk(mtx);
    m_isDestroy = true;
    nfs_umount(m_nfsContext);
    nfs_destroy_context(m_nfsContext);
    m_nfsContext = nullptr;
}

void NfsServiceTask::Exec()
{
    lock_guard<std::mutex> lk(m_wrapper->mtx);
    // TODO set the requestID
    if (m_wrapper->m_nfsContext == nullptr) {
        HCP_Log(ERR, MODULE) << "Context destroyed, ignore tasks" << HCPENDLOG;
        m_wrapper->m_isNfsServiceTaskRunning = false;
        m_result = SUCCESS;
        return;
    }

    try {
        if (nfs_service(m_wrapper->GetNfsContext(), m_revents) < 0) {
            if (m_revents != -1) {
                m_result = FAILED;
                m_wrapper->m_isNfsServiceTaskRunning = false;
                return;
            }
        }
    }
    catch (std::exception& e) {
        HCP_Logger_noid(ERR, MODULE)
            << " Exception, what=" << e.what() << ", result=" << m_result
            << ", m_revents=" << m_revents << HCPENDLOG;
        m_result = FAILED;
        m_wrapper->m_isNfsServiceTaskRunning = false;
        return;
    }
    m_wrapper->m_isNfsServiceTaskRunning = false;
    m_result = SUCCESS;
}

std::string NfsContextWrapper::NfsV3AclTranslatePermToMaskString(u_int perm)
{
    std::string mask;
    switch (perm) {
        case ONE_1: mask = "--x";
            break;
        case TWO_2: mask = "-w-";
            break;
        case THREE_3: mask = "-wx";
            break;
        case FOUR_4: mask = "r--";
            break;
        case FIVE_5: mask = "r-x";
            break;
        case SIX_6: mask = "rw-";
            break;
        case SEVEN_7: mask = "rwx";
            break;
        default: mask = "---";
            break;
    }
    return mask;
}

void NfsContextWrapper::NfsV3GetAclCb(const struct rpc_context* rpc, int status, const GETACL3res* res,
    struct RpcClient* client)
{
    rpc = rpc;
    if (status == RPC_STATUS_SUCCESS && res->status == NFS3_OK) {
        int aclSize = (int) res->GETACL3res_u.resok.ace.ace_len;
        int32_t userMask = 0;
        int32_t groupMask = 0;
        int32_t mask = 0;
        int32_t otherMask = 0;
        for (int i = 0; i < aclSize; i++) {
            if (NFSACL_TYPE_USER == res->GETACL3res_u.resok.ace.ace_val[i].type) {
                u_int id = (u_int) res->GETACL3res_u.resok.ace.ace_val[i].id;
                u_int perm = (u_int) res->GETACL3res_u.resok.ace.ace_val[i].perm;
                client->aclText = client->aclText + "user:" + std::to_string(id) + ":"
                    + NfsV3AclTranslatePermToMaskString(perm) + "\n";
            }
            if (NFSACL_TYPE_USER_OBJ == res->GETACL3res_u.resok.ace.ace_val[i].type) {
                userMask = (int32_t) res->GETACL3res_u.resok.ace.ace_val[i].perm;
            }
            if (NFSACL_TYPE_GROUP_OBJ == res->GETACL3res_u.resok.ace.ace_val[i].type) {
                groupMask = (int32_t) res->GETACL3res_u.resok.ace.ace_val[i].perm;
            }
            if (NFSACL_TYPE_GROUP == res->GETACL3res_u.resok.ace.ace_val[i].type) {
                u_int id = (u_int) res->GETACL3res_u.resok.ace.ace_val[i].id;
                u_int perm = (u_int) res->GETACL3res_u.resok.ace.ace_val[i].perm;
                client->aclText = client->aclText + "group:" + std::to_string(id) + ":"
                    + NfsV3AclTranslatePermToMaskString(perm) + "\n";
            }
            if (NFSACL_TYPE_CLASS_OBJ == res->GETACL3res_u.resok.ace.ace_val[i].type) {
                mask = (int32_t) res->GETACL3res_u.resok.ace.ace_val[i].perm;
            }
            if (NFSACL_TYPE_CLASS == res->GETACL3res_u.resok.ace.ace_val[i].type) {
                otherMask = (int32_t) res->GETACL3res_u.resok.ace.ace_val[i].perm;
            }
        }
        client->aclText = "user::" + NfsV3AclTranslatePermToMaskString(userMask) + "\n" + "group::"
            + NfsV3AclTranslatePermToMaskString(groupMask) + "\n" + client->aclText + "mask::"
            + NfsV3AclTranslatePermToMaskString(mask) + "\n" + "other::"
            + NfsV3AclTranslatePermToMaskString(otherMask) + "\n";
    }
    client->cb_data.is_finished = 1;
    client->cb_data.status = status;
    client->cb_data.return_data = NULL;
}

void NfsContextWrapper::PrintRpcCounters(const std::string containerType)
{
    std::lock_guard<std::mutex> lk(mtx);
    nfs_print_rpc_counters(m_nfsContext, containerType.c_str());
}

int64_t NfsContextWrapper::GetCurTime()
{
    timeval curTime {};
    gettimeofday(&curTime, nullptr);
    int64_t milli = (curTime.tv_sec * uint64_t(MAX_CNT)) + (curTime.tv_usec / MAX_CNT);
    return milli;
}

void NfsContextWrapper::Poll(int expireTime)
{
    struct pollfd pfd = {0};
    int revents = 0;
    int ret = 0;
    static int64_t time = GetCurTime();
    bool isEventPresent = true;

    InitPfd(pfd);
    ret = poll(&pfd, 1, expireTime);
    if (ret < 0) {
        ERRLOG("Poll failed, errno:%d", errno);
        revents = -1;
    } else if (ret > 0) {
        revents = pfd.revents;
    } else {
        if ((GetCurTime() - time) < 1000) {
            isEventPresent = false;
        }
    }
    if (isEventPresent) {
        time = GetCurTime();
        if (revents == 0) {
            DBGLOG("No socket events. Calling nfs_service to process timeout requests.");
        }
        std::unique_lock<std::mutex> lk(mtx);
        if (nfs_service(m_nfsContext, revents) < 0) {
            if (revents != -1) {
            }
        }
        lk.unlock();
    }
}

}

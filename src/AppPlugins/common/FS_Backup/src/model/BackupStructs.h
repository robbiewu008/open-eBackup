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
#ifndef BACKUP_STRUCTS_H
#define BACKUP_STRUCTS_H

#include <string>
#include <shared_mutex>
#include <memory>
#include <map>
#include <vector>
#include <atomic>
#include <algorithm>
#include <unordered_set>
#include "Backup.h"
#ifdef _NAS
#include "nfsc/libnfs.h"
#include "smb2/libsmb2.h"
#endif

constexpr uint32_t     TRUNCATE             = 0x00000001;          /* File must be tuncated */
constexpr uint32_t     SRC_CLOSED           = 0x00000002;          /* Src File is already closed */
constexpr uint32_t     DST_CLOSED           = 0x00000004;          /* Dst File is already closed */
constexpr uint32_t     ACL_EXIST            = 0x00000008;          /* ACL Attribute exist for file */
constexpr uint32_t     PROCESSED_BY_AGGR    = 0x00000010;          /* File is processed by aggregator */
constexpr uint32_t     AGGREGATE_GEN_FILE   = 0x00000020;          /* During restore enable this flag for zipfile */
constexpr uint32_t     IS_DIR               = 0x00000040;          /* Set 1 For Directory */
constexpr uint32_t     BADHANDLE_ERR_HIT    = 0x00000080;          /* BadHandle error encountered during read request */
constexpr uint32_t     ATTR_RESETED         = 0x00000100;          /* file handle attr has been reseted */
constexpr uint32_t     HUGE_OBJECT_FILE     = 0x00000200;          /* Only for object storage */
constexpr uint32_t     READ_FAILED_DISCARD  = 0x01000000;          /* Filehandle read failed but discarded */

#ifdef WIN32
/*
 * Introduced since 1.3.0, used as KEY value for m_xattr in FileHandle for Windows Backup
 * for the purpose of reusing this field
 */
const std::string EXTEND_ATTR_KEY_WIN32_SYMBOLIC_TARGET = "XMETA_TYPE_SYMBOLIC_TARGET";
const std::string EXTEND_ATTR_KEY_WIN32_JUNCTION_TARGET = "XMETA_TYPE_JUNCTION_TARGET";
#endif

namespace {
constexpr auto ENTRY_MODE_META_MODIFIED = "mm";
}

#define IS_VALID_FILEHANDLE(fileHandle) (fileHandle.m_file != nullptr)

enum class FileDescState {
    INIT = 0,               /* 0 init */
    LSTAT,                  /* 1 lstat req sent for the file to obtain the attributes */
    SRC_OPENED,             /* 2 file is opened (at source) */
    DST_OPENED,             /* 3 file is opened (at destination) */
    PARTIAL_READED,         /* 4 file data read from src is in-progress, partially read */
    READED,                 /* 5 file data read from src is completed, all blocks are read */
    AGGREGATED,             /* 6 used in aggregate restore */
    META_READED,            /* 7 File meta read from src is completed */
    PARTIAL_WRITED,         /* 8 file data write to src is in-progress, partially written */
    WRITED,                 /* 9 file data write to src is completed, all blocks are written */
    META_WRITED,            /* 10 File metadata write to src is completed */
    SRC_CLOSED,             /* 11 file is closed (at source) */
    DST_CLOSED,             /* 12 file is closed (at destination) */
    READ_OPEN_FAILED,       /* 13 file open at source is failed */
    WRITE_OPEN_FAILED,      /* 14 file open at destination is failed */
    LINK,                   /* 15 */
    DIR_DEL,                /* 16 directory with same name as file, to be deleted */
    DIR_DEL_RESTORE,        /* directory with same name as file, to be deleted. Comes in link delete flow of restore */
    LINK_DEL,               /* 18 symlink/hardlink/failed file to be deleted */
    LINK_DEL_FAILED,        /* 19 normal file to be deleted after copy failure */
    LINK_DEL_FOR_RESTORE,   /* 20 non zero files in restore to be deleted */
    READ_FAILED,            /* 21 file data read at source is failed */
    WRITE_FAILED,           /* 22 file data write at destination is failed */
    WRITE_SKIP,             /* 23 */
    META_WRITE_FAILED,      /* 24 file metadata write at destination is failed */
    REPLACE_DIR,            /* 25 need to remove the dir and send the fileHandle to copy again */
    FILEHANDLE_INVALID,     /* 26 file handle became invalid, need to reopen */
    END                     /* 27 end */
};

enum class LINK_TYPE {
    SYM = 0,        // src -> protected share
    HARD = 1,       // dst -> secondary share
    REGULAR = 2,    // Create flow
    DEVICE_TYPE = 3 // Mknod flow
};

enum class FileType { // 与 Module 中的enum class MetaType对应
    NFS = 1,
    CIFS,
    UNIX,
    WINDOWS,
    OBJECT
};

struct ParentInfo {
    std::string dirName;
    std::string metaFileName;
    uint64_t nlink { 0 };
};

union IOHandle {
#ifdef _NAS
    struct nfsfh* nfsFh;
    struct smb2fh* smbFh;
#endif
#ifdef WIN32
    HANDLE win32Fd;
#else
    int posixFd;
#endif
};

class BlockStats {
public:
    std::atomic<uint32_t> m_totalCnt        {0};
    std::atomic<uint32_t> m_readReqCnt      {0};
    std::atomic<uint32_t> m_readRespCnt     {0};
    std::atomic<uint32_t> m_writeReqCnt     {0};
    std::atomic<uint32_t> m_writeRespCnt    {0};
};

class FileDesc {
public:
    void LockCommonMutex()
    {
        m_commonMutex.lock();
    }

    void UnlockCommonMutex()
    {
        m_commonMutex.unlock();
    }

    void SetSrcState(FileDescState state)
    {
        std::lock_guard<std::shared_mutex> exclusiveLock(m_srcMutex);
        m_srcState = state;
    }

    FileDescState GetSrcState()
    {
        std::shared_lock<std::shared_mutex> sharedLock(m_srcMutex);
        return m_srcState;
    }

    void SetDstState(FileDescState state)
    {
        std::lock_guard<std::shared_mutex> exclusiveLock(m_dstMutex);
        m_dstState = state;
    }

    FileDescState GetDstState()
    {
        std::shared_lock<std::shared_mutex> sharedLock(m_dstMutex);
        return m_dstState;
    }

    void SetFlag(uint32_t flagBit)
    {
        std::lock_guard<std::shared_mutex> exclusiveLock(m_flgMtx);
        m_flag |= flagBit;
    }

    void ClearFlag(uint32_t flagBit)
    {
        std::lock_guard<std::shared_mutex> exclusiveLock(m_flgMtx);
        m_flag &= ~(flagBit);
    }

    bool IsFlagSet(uint32_t flagBit)
    {
        std::shared_lock<std::shared_mutex> sharedLock(m_flgMtx);
        return (m_flag & flagBit);
    }

    void IncUnAggTaskCnt()
    {
        std::lock_guard<std::shared_mutex> exclusiveLock(m_AggrMtx);
        m_UnAggrTasks += 1;
    }

    void DecUnAggTaskCnt()
    {
        std::lock_guard<std::shared_mutex> exclusiveLock(m_AggrMtx);
        if (m_UnAggrTasks > 0) {
            m_UnAggrTasks -= 1;
        }
    }

    uint32_t GetUnAggTaskCnt()
    {
        std::lock_guard<std::shared_mutex> exclusiveLock(m_AggrMtx);
        return m_UnAggrTasks;
    }

public:

    FileDesc(BackupIOEngine srcIoEngine, BackupIOEngine dstIoEngine)
    {
        switch (srcIoEngine) {
#ifdef _NAS
            case BackupIOEngine::LIBNFS:
                srcIOHandle.nfsFh = nullptr;
                break;
            case BackupIOEngine::LIBSMB:
                srcIOHandle.smbFh = nullptr;
                break;
#endif
#ifdef WIN32
            case BackupIOEngine::WIN32_IO:
                srcIOHandle.win32Fd = nullptr;
                break;
#else
            case BackupIOEngine::POSIX:
                srcIOHandle.posixFd = -1;
                break;
#endif
            default:
                break;
        };

        switch (dstIoEngine) {
#ifdef _NAS
            case BackupIOEngine::LIBNFS:
                dstIOHandle.nfsFh = nullptr;
                break;
            case BackupIOEngine::LIBSMB:
                dstIOHandle.smbFh = nullptr;
                break;
#endif
#ifdef WIN32
            case BackupIOEngine::WIN32_IO:
                dstIOHandle.win32Fd = nullptr;
                break;
#else
            case BackupIOEngine::POSIX:
                dstIOHandle.posixFd = -1;
                break;
#endif
            default:
                break;
        };
    }
#ifdef _NAS
    nfs_fh_scan m_fh {};                  /* File Handle */
#endif
    uint64_t m_fileCount { 0 };           /* No of files under this directory entry */
    uint64_t m_size { 0 };                /* total size, in bytes */
    uint64_t m_originalFileCount { 1 };   /* to record original file count inside current fh */
    uint64_t m_rdev { 0 };                /* device ID (if special file) */
    uint64_t m_inode { 0 };               /* inode number */
    uint64_t m_mtime { 0 };               /* time of last modification */
    uint64_t m_atime { 0 };               /* time of last access */
    uint64_t m_ctime { 0 };               /* time of change */
    uint64_t m_btime { 0 };               /* time of birth */
    uint64_t m_metaFileOffset { 0 };
    uint32_t m_uid { 0 };                 /* user ID of owner */
    uint32_t m_gid { 0 };                 /* group ID of owner */
    uint32_t m_nlink { 0 };               /* number of hard links */
    uint32_t m_fileAttr { 0 };
    uint64_t m_aggregateFileOffset { 0 }; /* Offset of small file content inside the aggregate file */

    uint32_t m_mode { 0 };                /* mode_t for Linux, Also used as reserved fields for Windows since 1.3.0 */
    FileType m_type { 0 };                /* 元数据类型：1.nfs 2.cifs 3.UNIX 4.Windows */
    uint16_t m_metaFileIndex { 0 };
    uint16_t m_metaFileReadLen { 0 };
    
    std::string m_scannermode;            /* Mode to denote the change happened to file */
    std::string m_aclText;                /* Acl text stored nfsv3 acl/posix access acl/cifs security descriptor */
    std::string m_defaultAclText;         /* Acl text only store posix default acl */
    std::string m_securityDescriptor;     /* Securty descriptor ACE string for Windows/SMB */

    /* Extended attribute (xttr) for Posix, Also used to store some XMeta pair for Windows since 1.3.0 */
    std::vector<std::pair<std::string, std::string>> m_xattr;

    std::vector<std::pair<uint64_t, uint64_t>> m_sparse;        /* Sparse info */

    std::string m_fileName;               /* Absolute path of file */
    std::string m_dirName;                /* Absolute path of parent directory */
    std::string m_onlyFileName;           /* file/dirctory name */
    std::string m_obsKey;  /* 对象的key */

    IOHandle srcIOHandle { 0 };
    IOHandle dstIOHandle { 0 };

    BlockStats m_blockStats {};
    uint32_t m_flag { 0 };

#ifdef _OBS
    std::string m_uploadId;
#endif

private:
    FileDescState m_srcState { FileDescState::INIT };
    FileDescState m_dstState { FileDescState::INIT };

    uint32_t m_UnAggrTasks   { 0 };        /* during restore unaggregated task of blobfile */

    std::shared_mutex m_flgMtx;
    std::shared_mutex m_AggrMtx;
    std::shared_mutex m_srcMutex;
    std::shared_mutex m_dstMutex;
    std::mutex m_commonMutex;
};

class BlockBuffer {
public:
    uint32_t m_size { 0 };
    uint64_t m_offset { 0 };
    uint64_t m_seq { 0 };
    uint8_t *m_buffer = nullptr;
};

class FileHandle {
public:
    std::shared_ptr<FileDesc>   m_file              { nullptr };
    BlockBuffer                 m_block             {};
    uint32_t                    m_retryCnt          { 0 };
    uint32_t                    m_errNum            { 0 };
    std::string                 m_errMessage        {};

    bool operator< (const FileHandle & fileHandleObj) const
    {
        if (this->m_file == nullptr || fileHandleObj.m_file == nullptr) {
            return (this->m_block.m_seq < fileHandleObj.m_block.m_seq);
        }
        if (this->m_file->m_inode < fileHandleObj.m_file->m_inode) {
            return true;
        }
        if (this->m_file->m_fileName < fileHandleObj.m_file->m_fileName) {
            return true;
        }
        if (this->m_block.m_seq < fileHandleObj.m_block.m_seq) {
            return true;
        }
        return false;
    }

    bool operator== (const FileHandle & fileHandleObj) const
    {
        if ((this->m_block.m_seq != fileHandleObj.m_block.m_seq)
            || (this->m_file == nullptr || fileHandleObj.m_file == nullptr)
            || (this->m_file->m_inode != fileHandleObj.m_file->m_inode)
            || (this->m_file->m_fileName != fileHandleObj.m_file->m_fileName)) {
            return false;
        }
        return true;
    }

    bool IsFile() const
    {
        return !m_file->IsFlagSet(IS_DIR);
    }

    bool IsDir() const
    {
        return m_file->IsFlagSet(IS_DIR);
    }

    bool IsOnlyMetaModified() const
    {
        return m_file->m_scannermode == ENTRY_MODE_META_MODIFIED;
    }

    bool IsAdsFile() const
    {
        return m_file->m_mode == FILE_IS_ADS_FILE;
    }

    bool HasAdsFile() const
    {
        return (m_file->m_mode & FILE_HAVE_ADS) == FILE_HAVE_ADS;
    }
};

class DeleteInfo {
public:
    uint32_t m_isDir = 0;
    std::string m_fileName {};
};

enum class BACKUP_DIRECTION {
    SRC = 0, // src -> protected share
    DST = 1, // dst -> secondary share
};

class HardLinkDesc {
public:
    uint32_t refCount { 0 };
    uint32_t linkCount { 0 };
    bool isTargetCopied { false };
    std::string targetPath;
    std::vector<FileHandle> links;
};

class HardLinkMap {
public:
    bool Empty()
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_hardlinkMap.empty();
    }

    void Erase()
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_hardlinkMap.clear();
        return;
    }

    bool IsTargetCopied(uint64_t inode)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_hardlinkMap.find(inode);
        if (it == m_hardlinkMap.end()) {
            return false;
        } else {
            return it->second.isTargetCopied;
        }
    }

    bool SetTargetCopied(uint64_t inode)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_hardlinkMap.find(inode);
        if (it == m_hardlinkMap.end()) {
            return false;
        } else {
            it->second.isTargetCopied = true;
            return true;
        }
    }

    int IncreaseRef(uint64_t inode)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_hardlinkMap.find(inode);
        if (it == m_hardlinkMap.end()) {
            return -1;
        }
        it->second.refCount++;
        if (it->second.refCount == it->second.linkCount) {
            m_hardlinkMap.erase(inode);
        }
        return 0;
    }

    void Insert(uint64_t inode, const HardLinkDesc &hardLinkDesc)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_hardlinkMap.emplace(std::make_pair(inode, hardLinkDesc));
    }

    void InsertLinks(uint64_t inode, const FileHandle &fileHandle)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_hardlinkMap.find(inode);
        if (it != m_hardlinkMap.end()) {
            it->second.links.push_back(fileHandle);
        }
        return;
    }

    void RemoveLink(const FileHandle& fileHandle)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        if (fileHandle.m_file == nullptr) { return; }
        auto it = m_hardlinkMap.find(fileHandle.m_file->m_inode);
        if (it == m_hardlinkMap.end()) {
            return;
        }
        std::vector<FileHandle>& links = it->second.links;
        for (auto it = links.begin(); it != links.end(); it++) {
            if (*it == fileHandle) {
                links.erase(it);
            }
        }
    }

    std::vector<FileHandle> GetLinks(uint64_t inode)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        std::vector<FileHandle> links;
        auto it = m_hardlinkMap.find(inode);
        if (it != m_hardlinkMap.end()) {
            links = it->second.links;
        }
        return links;
    }

    std::vector<FileHandle> GetLinksAndClear(uint64_t inode)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        std::vector<FileHandle> links;
        auto it = m_hardlinkMap.find(inode);
        if (it != m_hardlinkMap.end()) {
            swap(links, it->second.links);
        }
        return links;
    }

    bool Exist(uint64_t inode)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_hardlinkMap.find(inode);
        if (it == m_hardlinkMap.end()) {
            return false;
        }
        return true;
    }

    int GetTargetPath(uint64_t inode, std::string &targetPath)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_hardlinkMap.find(inode);
        if (it == m_hardlinkMap.end()) {
            return -1;
        }
        targetPath = it->second.targetPath;
        return 0;
    }

    uint32_t GetLinkCount(uint64_t inode)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_hardlinkMap.find(inode);
        if (it == m_hardlinkMap.end()) {
            return 0;
        }
        return it->second.linkCount;
    }

    int RemoveElement(uint64_t inode)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_hardlinkMap.find(inode);
        if (it == m_hardlinkMap.end()) {
            return -1;
        }

        m_hardlinkMap.erase(inode);
        return 0;
    }

    std::vector<std::vector<FileHandle>> GetAllFileHandlesAndClear()
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        std::vector<std::vector<FileHandle>> result;
        for (auto it = m_hardlinkMap.begin(); it != m_hardlinkMap.end(); ++it) {
            if (it->second.isTargetCopied) {
                std::vector<FileHandle> tmp;
                swap(tmp, it->second.links);
                result.push_back(tmp);
            }
        }
        return result;
    }

    void InsertFailedInode(uint64_t inode)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        m_failedInodeSet.insert(inode);
        return;
    }

    bool CheckInodeFailed(uint64_t inode)
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        return m_failedInodeSet.count(inode) > 0;
    }

private:
    std::map<uint64_t, HardLinkDesc> m_hardlinkMap;
    std::unordered_set<uint64_t> m_failedInodeSet;
    std::mutex m_mtx;
};
#endif // BACKUP_STRUCTS_H
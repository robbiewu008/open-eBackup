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
#ifndef ARCHIVE_DOWNLOAD_FILE_H
#define ARCHIVE_DOWNLOAD_FILE_H

#include <string>
#include <memory>
#include <atomic>
#include <map>
#include "ArchiveClient.h"
#include "message/archivestream/ArchiveStreamService.h"

namespace FilePlugin {
enum class ArchiveDownloadState {
    SUCCESS,
    FAILED,
    FINISH,
    RUNNING,
    EMPTY_COPY,
    TAP_REMIND
};

struct ArchiveServerInfo {
    std::vector<std::string> ipList;
    int port {0};
    bool enableSSL {true};
};

/*
操作类型(0-恢复，1-删除),文件系统ID,文件名,文件类型（0-非聚合文件，1-聚合文件，2-细粒度恢复文件）,
元数据信息(metafile，offset，length),1(软连接),/lib/libscheduler.so.1（链接的原始文件）
*/
struct ControlFileData {
    struct ArchiveMetaInfo {
        std::string metaFile;
        uint64_t offset {0};
        uint16_t length {0};
    };

    int type { -1 };
    std::string fsId;
    std::string fileName;
    int fileType { -1 };
    ArchiveMetaInfo metaInfo {}; // metafile名-offset-length
    int linkType { -1 };
    std::string linkSrcFile;
};

struct ArchiveDownloadParam {
    ArchiveDownloadParam() {}
    ArchiveDownloadParam(const std::string& pJobId, const std::string& pCopyId, const std::string& pResourceId,
        const std::string& pCacheFsPath, const std::string& pCacheFsRemotePath) : jobId(pJobId), copyId(pCopyId),
        resourceId(pResourceId), cacheFsPath(pCacheFsPath), cacheFsRemotePath(pCacheFsRemotePath) {}
    std::string jobId;
    std::string copyId;
    std::string resourceId;
    std::string cacheFsPath;
    std::string cacheFsRemotePath;
};

class ArchiveDownloadFile {
public:
    ArchiveDownloadFile(ArchiveDownloadParam& param, ArchiveServerInfo& archiveInfo)
        : m_jobId(param.jobId), m_copyId(param.copyId), m_resourceId(param.resourceId),
        m_cacheFsPath(param.cacheFsPath), m_cacheFsRemotePath(param.cacheFsRemotePath),
        m_archiveInfo(archiveInfo) {}
    ~ArchiveDownloadFile() {}
    bool Start(const std::string& outputPath, const std::vector<std::string>& pathList);
    bool StartDownloadMeta(const std::string& outputPath, const std::vector<std::string>& pathList);
    std::string GetFileSystemsId();
    std::string GetParentDir();
    void SetAbort();

    ArchiveDownloadState m_state {ArchiveDownloadState::RUNNING};
private:
    inline bool IsAbort();
    bool InitArchiveClient(const std::vector<std::string>& pathList);
    bool QueryPrepare();
    bool GetControlFileFromArchive(std::string& checkpoint, std::vector<std::string>& controlList);
    bool GetFileListFromCtrl(const std::string& ctrlFileName,
        std::map<std::string, ControlFileData>& ctrlFileMap);
    bool DownloadFile(const std::string& outputPath, const std::map<std::string, ControlFileData>& ctrlFileMap);
    void CloseClient() const;

    bool OpenFileExistOrNew();
    inline bool IsDir(const std::string& name) const;
    bool ArchiveWriteFile(const ControlFileData& ctrlData);
    int WriteBufferToFile(const char* buf, uint64_t offset, uint64_t length) const;
#ifdef WIN32
    std::string GetCacheRepoRootPath(const std::string& cacheFsPath) const;
#endif
private:
    std::string m_jobId;
    std::string m_copyId;
    std::string m_resourceId;
    std::string m_fsId;
    std::string m_cacheFsPath;
    std::string m_cacheFsRemotePath;
    std::string m_parentDir;
    bool m_isInit {false};
    ArchiveServerInfo m_archiveInfo;
    std::unique_ptr<ArchiveStreamService> m_clientHandler = std::make_unique<ArchiveStreamService>();
    std::string m_controlFile;

    std::string m_fileFullPath;
#ifdef WIN32
    HANDLE m_fd;
#else
    int m_fd;
#endif
    std::atomic<bool> m_isAbort {false};
};
}
#endif // ARCHIVE_DOWNLOAD_FILE_H
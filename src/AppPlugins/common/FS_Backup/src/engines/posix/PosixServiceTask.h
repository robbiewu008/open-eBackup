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
#ifndef POSIX_SERVICE_TASK_H
#define POSIX_SERVICE_TASK_H

#include <memory>
#include <unistd.h>
#include <fcntl.h>
#include <utility>
#include "ThreadPool.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "HostServiceTask.h"

class PosixServiceTask : public HostServiceTask {
public:
    PosixServiceTask() : HostServiceTask() {}
    
    PosixServiceTask(
        HostEvent event,
        std::shared_ptr<BlockBufferMap> bufferMapPtr,
        FileHandle& fileHandle,
        const HostParams& params)
        : HostServiceTask(event, bufferMapPtr, fileHandle, params) {}

private:
    void SetCriticalErrorInfo(uint64_t err) override;

    /* template methods win32 sepcification implement */
    void HandleOpenSrc() override;
    void HandleOpenDst() override;
    void HandleReadData() override;
    void HandleReadMeta() override;
    void HandleWriteData() override;
    void HandleWriteMeta() override;
    void HandleLink() override;
    void HandleCloseSrc() override;
    void HandleCloseDst() override;
    void HandleDelete() override;
    void HandleCreateDir() override;
    void HandleReadSpecialData();

    /* methods defined only for posix backup task */
    int ProcessOpenDst(const std::string& dstFile);
    int ProcessOpenDstInteraction(const std::string &dstFile);
    int ProcessRestorePolicy(const std::string& dstFile);
    int ProcessOverwriteOlderPolicy(const std::string &dstFile, int flag);
    int ProcessOverwritePolicy(const std::string &dstFile, int flag);
    int ProcessOverrideRenamePolicy(const std::string &dstFile, int flag);

    int ProcessReadSoftLinkData();
    int ProcessReadSpecialFileData();
    int ProcessWriteSoftLinkData();
    int ProcessWriteSpecialFileData();

    bool RemoveFile(const std::string &filePath);
    bool SetUtime(const std::string &dstPath);
    bool SetAcl(const std::string &dstPath);
#if !defined(_AIX) && !defined(SOLARIS)
    bool SetAcl4Posix(const std::string &dstPath);
#endif
#ifdef _AIX
    bool SetAcl4Aix(const std::string &dstPath);
#endif
#ifdef SOLARIS
    bool SetAcl4Solaris(const std::string &dstPath);
#endif
    bool SetXattr(const std::string &dstPath);
    bool ShouldWriteMode();
    int ProcessWriteSpecialFileReplacePolicy(const std::string& dstFile, std::string& newDstFile, bool &isContinue);
    void CloseSmallFileSrcFd();
    void CloseSmallFileDstFd();
    bool CreateDirectory(const std::string &path);
    ssize_t PwriteWithRetry(const int fd, const uint8_t *buf, const size_t size, const off_t offset);
private:
    uint64_t m_offset { 0 };
    uint64_t m_length { 0 };
    uint32_t m_seq { 0 };
};

#endif // POSIX_SERVICE_TASK_H
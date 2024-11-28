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
#ifndef WIN32_SERVICE_TASK_H
#define WIN32_SERVICE_TASK_H

#include <memory>
#include <fcntl.h>
#include <utility>
#include "ThreadPool.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "BackupQueue.h"
#include "HostServiceTask.h"
#include "StreamHostFilePendingMap.h"

using namespace FS_Backup;
using namespace Module;

/*
 * compounded params used to build Win32ServiceTask for ADS ReadMeta stage
 */
struct Win32TaskExtendContext {
    std::shared_ptr<BackupQueue<FileHandle>>        readQueuePtr { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>>        aggregateQueuePtr { nullptr };
    std::shared_ptr<BackupControlInfo>              controlInfo { nullptr };

    /* used to count sub stream count in ReadMeta stage */
    uint64_t                                        subStreamCount = 0;
};

class Win32ServiceTask : public HostServiceTask {
public:
    Win32ServiceTask() : HostServiceTask() {}
    
    Win32ServiceTask(
        HostEvent event,
        std::shared_ptr<BlockBufferMap> bufferMapPtr,
        FileHandle& fileHandle,
        const HostParams& params)
        : HostServiceTask(event, bufferMapPtr, fileHandle, params) {}
    
    /* only used to build ADS ReadMeta task  */
    Win32ServiceTask(
        HostEvent event,
        std::shared_ptr<BlockBufferMap> bufferMapPtr,
        FileHandle& fileHandle,
        const HostParams& params,
        const Win32TaskExtendContext& extendContext)
        : HostServiceTask(event, bufferMapPtr, fileHandle, params), m_extendContext(extendContext) {}

    bool IsCriticalError() const override;

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

    /* methods defined only for posix backup task */
    int ProcessOpenDst(const std::string& dstFile);
    int ProessOpenDstFailed(const std::string& dstFile, const std::wstring wDstFile, DWORD lastError);
    int ReopenDstNoAccessFile(const std::string& dstFile, const std::wstring wDstFile);
    int ProcessRestorePolicy(const std::string& dstFile);
    int ProcessOverwriteOlderPolicy(const std::string& dstFile);
    int ProcessOverwritePolicy(const std::string& dstFile);
    int ProcessOverrideRenamePolicy(const std::string& dstFile);

    bool IsValidWin32Handle(const HANDLE& handle) const;
    bool CheckWin32HandleValid(HANDLE hFile, const std::string& filePath);

    bool SetSecurityDescriptorW(const std::wstring& wPath, DWORD& errorCode) const;
    bool InitSparseFile(const std::string& filepath, HANDLE hFile, uint64_t size);
    bool ProcessCreateSymlink(const std::string& dstFile);
    void CloseSmallFileDstFd();
    void CloseSmallFileSrcFd();
    bool Win32DeleteFile(const std::string& filePath);
    bool DeleteReadOnlyFile(const std::string& filePath);
    bool SetFileInformation();

    bool NeedCreateSymlink(const std::string& dstFile) const;
    void PushSubStreamFileHandleToReadQueue(const std::wstring& wStreamName);
    void HandleFindStreamFailed(const std::string& srcFile, DWORD lastError);
    void PostReadMetaRoutine() const;
    void HandleWriteMetaForADS();
    bool WriteFileWithRetry(const std::string& dstFile);
private:

    Win32TaskExtendContext m_extendContext {};    /* used to store ADS context in ReadMeta stage */
};
#endif  // WIN32_SERVICE_TASK_H
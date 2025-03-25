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
#ifndef FS_SCANNER_SMB_TRAVERSAL_H
#define FS_SCANNER_SMB_TRAVERSAL_H
#include "FolderTraversal.h"
#include "SmbProducerThread.h"

class SmbFolderTraversal : public FolderTraversal {
public:
    explicit SmbFolderTraversal(std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<BufferQueue<DirectoryScan>> buffer,
        Module::SmbContextArgs args,
        ScanConfig config);
    ~SmbFolderTraversal() override;

    SCANNER_STATUS Enqueue(const std::string& directory,
        const std::string& prefix = "", uint8_t filterFlag = 0) override;
    SCANNER_STATUS Start() override;
    SCANNER_STATUS Poll() override;
    SCANNER_STATUS Suspend() override;
    SCANNER_STATUS Resume() override;
    SCANNER_STATUS Abort() override;
    SCANNER_STATUS Destroy() override;
    void ProcessCheckPointContainers() override;

private:
    std::vector<std::shared_ptr<SmbProducerThread>> m_threads;
    Module::SmbContextArgs m_args {};
    ScanConfig m_config {};
};

#endif
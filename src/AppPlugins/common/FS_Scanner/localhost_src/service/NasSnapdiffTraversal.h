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
#ifdef NAS_SNAPDIFF

#ifndef FS_SCANNER_NAS_SNAPDIFF_TRAVERSAL_H
#define FS_SCANNER_NAS_SNAPDIFF_TRAVERSAL_H

#include "FolderTraversal.h"
#include <memory>
#include <map>
#include <list>
#include <thread>
#include "NfsContextWrapper.h"
#include "SmbContextWrapper.h"
#include "SnapdiffBackupEntry.h"
#include "ParserUtils.h"
#include "ControlDevice.h"
#include "CommonService.h"

class NasSnapdiffTraversal : public FolderTraversal {
public:
    explicit NasSnapdiffTraversal(std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<BufferQueue<SnapdiffResultMap>> m_snapdiffBuffer,
        ScanConfig &config);

    ~NasSnapdiffTraversal();

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

    SCANNER_STATUS SnapdiffScan();
    void PerformSnapdiffRequest(std::unique_ptr<Module::ControlDevice> &oceanstorDevice, const std::string &sessionID);
    int FilterSnapdiffResultMap(SnapdiffResultMap& snapdiffResultMap);
    void ReadDiffInfoIntoMap(Module::SnapdiffMetadataInfo diffInfo[],
        SnapdiffResultMap& snapdiffResultMap, int numChanges);
    void CheckDeviceResourceName();
    std::unique_ptr<Module::ControlDevice> InitOceanStorDevice();
    void CloseSnapdiffSession(const std::string &sessionID, std::unique_ptr<Module::ControlDevice> &oceanstorDevice);

private:
    ScanConfig &m_config;
    std::shared_ptr<BufferQueue<SnapdiffResultMap>> m_buffer;

    bool m_snapdiffRequestDone {false};
};

#endif
#endif
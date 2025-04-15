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
#ifndef NFS_ANTI_RANSOMWARE_WORM_READER_H
#define NFS_ANTI_RANSOMWARE_WORM_READER_H

#include <memory>
#include "ThreadPoolFactory.h"
#include "AntiRansomwareReader.h"
#include "NfsContextContainer.h"
#include "PacketStats.h"
#include "LibnfsCommonMethods.h"
#include "LibnfsInterface.h"

class NfsWormReader : public AntiRansomwareReader {
public:
    explicit NfsWormReader(const AntiReaderParams &antiReaderParams);
    ~NfsWormReader();

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupPhaseStatus GetStatus() override;
    
    std::shared_ptr<NfsAntiRansomwareAdvanceParams> m_advParams;

private:
    bool IsComplete() const;
    bool IsAbort() const;
    void HandleComplete();
    void ThreadFunc() override;

    int OpenFile(FileHandle &fileHandle) override;
    int ReadData(FileHandle &fileHandle) override;
    int ReadMeta(FileHandle &fileHandle) override;
    int CloseFile(FileHandle &fileHandle) override;
    
    Module::NfsContextContainer m_nfsContextContainer;             /* nfs contexts  (used for async calls) */

    std::mutex mtx {};
    std::thread m_thread;
};

#endif // NFS_ANTI_RANSOMWARE_WORM_READER_H

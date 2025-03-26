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
#ifndef ANTIRANSOMWARE_H
#define ANTIRANSOMWARE_H

#include <memory>
#include <string>
#include <unordered_map>

#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "AntiControlFileReader.h"
#include "BackupQueue.h"
#include "AntiRansomwareReader.h"
#include "AntiRansomwareWriter.h"

namespace FS_Backup {
class AntiRansomware : public Backup {
public:
    explicit AntiRansomware(const BackupParams& backupParams);
    ~AntiRansomware() override;

    BackupRetCode Start() override;
    BackupRetCode Abort() override;
    BackupRetCode Destroy() override;
    BackupRetCode Enqueue(std::string contrlFile) override;
    BackupPhaseStatus GetStatus() override;
    BackupStats GetStats() override;
    std::unordered_set<FailedRecordItem, FailedRecordItemHash> GetFailedDetails() override;

private:
    void CreateBackupStatistic();
    void CreateBackupQueue();
    void CreateBackupEngine(const BackupParams& backupParams);

    bool IsMemberNull() const;
    bool IsCompleted(const BackupPhaseStatus &controlFileReaderStatus, const BackupPhaseStatus &readerStatus,
        const BackupPhaseStatus &writerStatus) const;
    bool IsFailed(const BackupPhaseStatus &controlFileReaderStatus, const BackupPhaseStatus &readerStatus,
        const BackupPhaseStatus &writerStatus) const;
    bool IsAborted(const BackupPhaseStatus &controlFileReaderStatus, const BackupPhaseStatus &readerStatus,
        const BackupPhaseStatus &writerStatus) const;
    bool IsStatusFailed(const BackupPhaseStatus &antiStatus) const;
    void FillReaderParams(AntiReaderParams &antiReaderParams) const;
    void FillWriterParams(AntiWriterParams &antiWriterParams) const;

private:
    std::shared_ptr<BackupControlInfo> m_controlInfo           = nullptr;
    std::shared_ptr<BlockBufferMap> m_blockBufferMap           = nullptr;
    std::unique_ptr<AntiControlFileReader> m_controlFileReader = nullptr;    /* anti-ransomware control file reader main thread object */
    std::unique_ptr<AntiRansomwareReader> m_reader             = nullptr;    /* anti-ransomware reader main thread object */
    std::unique_ptr<AntiRansomwareWriter> m_writer             = nullptr;    /* anti-ransomware writer main thread object */
    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue       = nullptr;    /* queue used by anti-ransomware control file reader and anti-ransomware reader */
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = nullptr;    /* queue used by anti-ransomware reader and anti-ransomware writer */

    bool m_abort { false };
};
}

#endif
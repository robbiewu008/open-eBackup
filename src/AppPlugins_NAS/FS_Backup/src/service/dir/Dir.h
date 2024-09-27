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
#ifndef DIR_H
#define DIR_H

#include <memory>
#include <string>

#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "DirAggregator.h"
#include "DirControlFileReader.h"
#include "BackupQueue.h"
#include "ReaderBase.h"
#include "WriterBase.h"
#include "CommonServiceBase.h"

namespace FS_Backup {

class Dir
    : public BackupServiceBase<DirControlFileReader, DirAggregator> {
public:
    explicit Dir(const BackupParams& backupParams);

    /* This method is only used to build backup engine for host backup */
    Dir(const std::string& source, const std::string& destination, const std::string& metaPath, bool writeMeta);

private:
    void InitReaderEngine(BackupIOEngine srcEngine, const ReaderParams& dirReaderParams);
    void InitWriterEngine(BackupIOEngine dstEngine, const WriterParams& dirWriterParams);

    /* select host OS platform to create corresponding writer/reader engine */
    void InitHostReaderEngine(const ReaderParams& readerParams) override;
    void InitHostWriterEngine(const WriterParams& writerParams) override;
};

}

#endif // DIR_H
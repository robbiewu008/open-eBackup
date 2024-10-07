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
#ifndef DELETE_H
#define DELETE_H

#include <memory>
#include <string>

#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "DeleteAggregator.h"
#include "DeleteControlFileReader.h"
#include "ReaderBase.h"
#include "WriterBase.h"
#include "BackupQueue.h"
#include "CommonServiceBase.h"

namespace FS_Backup {

class Delete
    : public BackupServiceBase<DeleteControlFileReader, DeleteAggregator> {
public:
    explicit Delete(const BackupParams& backupParams);

    /* This method is only used to build backup engine for host backup */
    Delete(const std::string& source, const std::string& destination, const std::string& metaPath, bool writeMeta);

private:
    void InitReaderEngine(BackupIOEngine srcEngine, const ReaderParams& copyReaderParams) override;
    void InitWriterEngine(BackupIOEngine dstEngine, const WriterParams& copyWriterParams) override;

    /* select host OS platform to create corresponding writer/reader engine */
    void InitHostReaderEngine(const ReaderParams& readerParams) override;
    void InitHostWriterEngine(const WriterParams& writerParams) override;
};

}

#endif // DELETE_H
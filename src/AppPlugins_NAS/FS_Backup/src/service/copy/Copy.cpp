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
#include "Copy.h"
#include "log/Log.h"
#include "FSBackupUtils.h"
#include "IOEngines.h"

using namespace std;
using namespace FS_Backup;

Copy::Copy(const BackupParams& backupParams)
    : BackupServiceBase<CopyControlFileReader, CopyAggregator>(backupParams)
{
    ReaderParams readerParams = WrapReaderParams();
    WriterParams writerParams = WrapWriterParams();
    InitReaderEngine(backupParams.srcEngine, readerParams);
    InitWriterEngine(backupParams.dstEngine, writerParams);
}

/* This method is only used to build backup engine for host backup */
Copy::Copy(const string& source, const string& destination, const string& metaPath, bool writeMeta)
    : BackupServiceBase<CopyControlFileReader, CopyAggregator>(
        source, destination, metaPath, writeMeta)
{
    ReaderParams readerParams = WrapReaderParams();
    WriterParams writerParams = WrapWriterParams();
    InitHostReaderEngine(readerParams);
    InitHostWriterEngine(writerParams);
}

/* Public APIs */

void Copy::InitReaderEngine(BackupIOEngine srcEngine, const ReaderParams& copyReaderParams)
{
    switch (srcEngine) {
#ifndef WIN32
        case BackupIOEngine::POSIX: {
            m_reader = mem::make_unique<PosixCopyReader>(copyReaderParams, m_failureRecorder);
            break;
        }
#endif

#ifdef WIN32
        case BackupIOEngine::WIN32_IO: {
            m_reader = mem::make_unique<Win32CopyReader>(copyReaderParams, m_failureRecorder);
            break;
        }
#endif

#ifdef _NAS
        case BackupIOEngine::LIBSMB: {
            m_reader = mem::make_unique<LibsmbCopyReader>(copyReaderParams, m_failureRecorder);
            break;
        }
        case BackupIOEngine::LIBNFS: {
            m_reader = mem::make_unique<LibnfsCopyReader>(copyReaderParams, m_failureRecorder);
            break;
        }
#endif

#ifdef _OBS
        case BackupIOEngine::OBJECTSTORAGE: {
            m_reader = mem::make_unique<ObjectCopyReader>(copyReaderParams, m_failureRecorder);
            break;
        }
#endif

        case BackupIOEngine::ARCHIVE_CLIENT: {
            auto client = dynamic_pointer_cast<ArchiveRestoreAdvanceParams>(
                copyReaderParams.backupParams.srcAdvParams)->archiveClient;
            unique_ptr<ArchiveCopyReader> reader = mem::make_unique<ArchiveCopyReader>(copyReaderParams);
            reader->SetArchiveClient(client);
            m_reader = std::move(reader);
            break;
        }
        default: {
            ERRLOG("unknown backup srcEngine: %u", srcEngine);
            break;
        }
    }
    return;
}

void Copy::InitWriterEngine(BackupIOEngine dstEngine, const WriterParams& copyWriterParams)
{
    switch (dstEngine) {
#ifndef WIN32
        case BackupIOEngine::POSIX: {
            m_writer = mem::make_unique<PosixCopyWriter>(copyWriterParams, m_failureRecorder);
            break;
        }
#endif

#ifdef WIN32
        case BackupIOEngine::WIN32_IO: {
            m_writer = mem::make_unique<Win32CopyWriter>(copyWriterParams, m_failureRecorder);
            break;
        }
#endif

#ifdef _NAS
        case BackupIOEngine::LIBSMB: {
            m_writer = mem::make_unique<LibsmbCopyWriter>(copyWriterParams, m_failureRecorder);
            break;
        }
        case BackupIOEngine::LIBNFS: {
            m_writer = mem::make_unique<LibnfsCopyWriter>(copyWriterParams, m_failureRecorder);
            break;
        }
#endif
#ifdef _OBS
        case BackupIOEngine::OBJECTSTORAGE: {
            m_writer = mem::make_unique<ObjectCopyWriter>(copyWriterParams, m_failureRecorder);
            break;
        }
#endif
        default: {
            ERRLOG("unknown backup dstEngine: %u", dstEngine);
            break;
        }
    }
    return;
}

/* select host OS platform to create corresponding writer/reader engine */
void Copy::InitHostReaderEngine(const ReaderParams& readerParams)
{
#ifdef WIN32
    m_reader = mem::make_unique<Win32CopyReader>(readerParams, m_failureRecorder);
#else
    m_reader = mem::make_unique<PosixCopyReader>(readerParams, m_failureRecorder);
#endif
}

void Copy::InitHostWriterEngine(const WriterParams& writerParams)
{
#ifdef WIN32
    m_writer = mem::make_unique<Win32CopyWriter>(writerParams, m_failureRecorder);
#else
    m_writer = mem::make_unique<PosixCopyWriter>(writerParams, m_failureRecorder);
#endif
}
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
#include "Delete.h"
#include "log/Log.h"
#include "FSBackupUtils.h"
#include "IOEngines.h"

using namespace std;
using namespace FS_Backup;

Delete::Delete(const BackupParams& backupParams)
    : BackupServiceBase<DeleteControlFileReader, DeleteAggregator>(backupParams)
{
    ReaderParams readerParams = WrapReaderParams();
    WriterParams writerParams = WrapWriterParams();
    InitReaderEngine(backupParams.srcEngine, readerParams);
    InitWriterEngine(backupParams.dstEngine, writerParams);
}

/* This method is only used to build backup engine for host backup */
Delete::Delete(const string& source, const string& destination, const string& metaPath, bool writeMeta)
    : BackupServiceBase<DeleteControlFileReader, DeleteAggregator>(
        source, destination, metaPath, writeMeta)
{
    ReaderParams readerParams = WrapReaderParams();
    WriterParams writerParams = WrapWriterParams();
    InitHostReaderEngine(readerParams);
    InitHostWriterEngine(writerParams);
}

void Delete::InitReaderEngine(BackupIOEngine srcEngine, const ReaderParams& readerParams)
{
    switch (srcEngine) {
        case BackupIOEngine::ARCHIVE_CLIENT: {
            auto client = dynamic_pointer_cast<ArchiveRestoreAdvanceParams>(
                readerParams.backupParams.srcAdvParams)->archiveClient;
            unique_ptr<ArchiveDeleteReader> reader = mem::make_unique<ArchiveDeleteReader>(readerParams);
            reader->SetArchiveClient(client);
            m_reader = move(reader);
            break;
        }
#ifndef WIN32
        case BackupIOEngine::POSIX: {
            m_reader = mem::make_unique<PosixDeleteReader>(readerParams, m_failureRecorder);
            break;
        }
#endif

#ifdef WIN32
        case BackupIOEngine::WIN32_IO: {
            m_reader = mem::make_unique<Win32DeleteReader>(readerParams, m_failureRecorder);
            break;
        }
#endif

#ifdef _NAS
        case BackupIOEngine::LIBSMB: {
            m_reader = mem::make_unique<LibsmbDeleteReader>(readerParams);
            break;
        }
        case BackupIOEngine::LIBNFS: {
            m_reader = mem::make_unique<LibnfsDeleteReader>(readerParams);
            break;
        }
#endif

#ifdef _OBS
        case BackupIOEngine::OBJECTSTORAGE: {
            m_reader = mem::make_unique<ObjectDeleteReader>(readerParams);
            break;
        }
#endif

        default: {
            ERRLOG("unknown backup srcEngine: %u", srcEngine);
            break;
        }
    }
    return;
}

void Delete::InitWriterEngine(BackupIOEngine dstEngine, const WriterParams& writerParams)
{
    switch (dstEngine) {
#ifndef WIN32
        case BackupIOEngine::POSIX: {
            m_writer = mem::make_unique<PosixDeleteWriter>(writerParams, m_failureRecorder);
            break;
        }
#endif

#ifdef WIN32
        case BackupIOEngine::WIN32_IO: {
            m_writer = mem::make_unique<Win32DeleteWriter>(writerParams, m_failureRecorder);
            break;
        }
#endif

#ifdef _NAS
        case BackupIOEngine::LIBSMB: {
            m_writer = mem::make_unique<LibsmbDeleteWriter>(writerParams, m_failureRecorder);
            break;
        }
        case BackupIOEngine::LIBNFS: {
            m_writer = mem::make_unique<LibnfsDeleteWriter>(writerParams, m_failureRecorder);
            break;
        }
#endif

#ifdef _OBS
        case BackupIOEngine::OBJECTSTORAGE: {
            m_writer = mem::make_unique<ObjectDeleteWriter>(writerParams, m_failureRecorder);
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
void Delete::InitHostReaderEngine(const ReaderParams& readerParams)
{
#ifdef WIN32
    m_reader = mem::make_unique<Win32DeleteReader>(readerParams, m_failureRecorder);
#else
    m_reader = mem::make_unique<PosixDeleteReader>(readerParams, m_failureRecorder);
#endif
}

void Delete::InitHostWriterEngine(const WriterParams& writerParams)
{
#ifdef WIN32
    m_writer = mem::make_unique<Win32DeleteWriter>(writerParams, m_failureRecorder);
#else
    m_writer = mem::make_unique<PosixDeleteWriter>(writerParams, m_failureRecorder);
#endif
}
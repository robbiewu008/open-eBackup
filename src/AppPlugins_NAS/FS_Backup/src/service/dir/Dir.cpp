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
#include "Dir.h"
#include "log/Log.h"
#include "FSBackupUtils.h"
#include "IOEngines.h"

using namespace std;
using namespace FS_Backup;

Dir::Dir(const BackupParams& backupParams)
    : BackupServiceBase<DirControlFileReader, DirAggregator>(backupParams)
{
    ReaderParams readerParams = WrapReaderParams();
    WriterParams writerParams = WrapWriterParams();
    InitReaderEngine(backupParams.srcEngine, readerParams);
    InitWriterEngine(backupParams.dstEngine, writerParams);
}

/* This method is only used to build backup engine for host backup */
Dir::Dir(const string& source, const string& destination, const string& metaPath, bool writeMeta)
    : BackupServiceBase<DirControlFileReader, DirAggregator>(
        source, destination, metaPath, writeMeta)
{
    ReaderParams readerParams = WrapReaderParams();
    WriterParams writerParams = WrapWriterParams();
    InitHostReaderEngine(readerParams);
    InitHostWriterEngine(writerParams);
}

void Dir::InitReaderEngine(BackupIOEngine srcEngine, const ReaderParams& dirReaderParams)
{
    switch (srcEngine) {
#ifndef WIN32
        case BackupIOEngine::POSIX: {
            m_reader = mem::make_unique<PosixDirReader>(dirReaderParams, m_failureRecorder);
            break;
        }
#endif

#ifdef WIN32
        case BackupIOEngine::WIN32_IO: {
            m_reader = mem::make_unique<Win32DirReader>(dirReaderParams, m_failureRecorder);
            break;
        }
#endif

#ifdef _NAS
        case BackupIOEngine::LIBSMB: {
            m_reader = mem::make_unique<LibsmbDirReader>(dirReaderParams);
            break;
        }
        case BackupIOEngine::LIBNFS: {
            m_reader = mem::make_unique<LibnfsDirMetaReader>(dirReaderParams);
            break;
        }
#endif

        case BackupIOEngine::ARCHIVE_CLIENT: {
            m_reader = mem::make_unique<ArchiveDirReader>(dirReaderParams);
            break;
        }
    
        default: {
            ERRLOG("unknown backup srcEngine: %u", srcEngine);
            break;
        }
    }
    return;
}

void Dir::InitWriterEngine(BackupIOEngine dstEngine, const WriterParams& dirWriterParams)
{
    switch (dstEngine) {
#ifndef WIN32
        case BackupIOEngine::POSIX: {
            m_writer = mem::make_unique<PosixDirWriter>(dirWriterParams, m_failureRecorder);
            break;
        }
#endif

#ifdef WIN32
        case BackupIOEngine::WIN32_IO: {
            m_writer = mem::make_unique<Win32DirWriter>(dirWriterParams, m_failureRecorder);
            break;
        }
#endif

#ifdef _NAS
        case BackupIOEngine::LIBSMB: {
            m_writer = mem::make_unique<LibsmbDirWriter>(dirWriterParams, m_failureRecorder);
            break;
        }
        case BackupIOEngine::LIBNFS: {
            m_writer = mem::make_unique<LibnfsDirMetaWriter>(dirWriterParams, m_failureRecorder);
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
void Dir::InitHostReaderEngine(const ReaderParams& readerParams)
{
#ifdef WIN32
    m_reader = mem::make_unique<Win32DirReader>(readerParams, m_failureRecorder);
#else
    m_reader = mem::make_unique<PosixDirReader>(readerParams, m_failureRecorder);
#endif
}

void Dir::InitHostWriterEngine(const WriterParams& writerParams)
{
#ifdef WIN32
    m_writer = mem::make_unique<Win32DirWriter>(writerParams, m_failureRecorder);
#else
    m_writer = mem::make_unique<PosixDirWriter>(writerParams, m_failureRecorder);
#endif
}

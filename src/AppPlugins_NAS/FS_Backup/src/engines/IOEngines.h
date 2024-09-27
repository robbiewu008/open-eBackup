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
#ifndef IO_ENGINES_H
#define IO_ENGINES_H

#ifndef WIN32
#include "PosixCopyReader.h"
#include "PosixCopyWriter.h"
#include "PosixDeleteReader.h"
#include "PosixDeleteWriter.h"
#include "PosixHardlinkReader.h"
#include "PosixHardlinkWriter.h"
#include "PosixDirReader.h"
#include "PosixDirWriter.h"
#ifdef _NAS
#include "LibnfsCopyReader.h"
#include "LibnfsCopyWriter.h"
#include "LibnfsDeleteReader.h"
#include "LibnfsDeleteWriter.h"
#include "LibnfsDirMetaReader.h"
#include "LibnfsDirMetaWriter.h"
#include "LibnfsHardlinkReader.h"
#include "LibnfsHardlinkWriter.h"
#include "LibsmbCopyReader.h"
#include "LibsmbCopyWriter.h"
#include "LibsmbDeleteReader.h"
#include "LibsmbDeleteWriter.h"
#include "LibsmbHardlinkReader.h"
#include "LibsmbHardlinkWriter.h"
#include "LibsmbDirReader.h"
#include "LibsmbDirWriter.h"
#include "NfsWormReader.h"
#include "NfsWormWriter.h"
#endif // NAS

#ifdef _OBS
#include "ObjectCopyReader.h"
#include "ObjectCopyWriter.h"
#include "ObjectDeleteReader.h"
#include "ObjectDeleteWriter.h"
#endif

#include "ArchiveCopyReader.h"
#include "ArchiveDirReader.h"
#include "ArchiveHardlinkReader.h"
#else
#include "ArchiveCopyReader.h"
#include "ArchiveDirReader.h"
#include "ArchiveHardlinkReader.h"
#include "Win32CopyReader.h"
#include "Win32CopyWriter.h"
#include "Win32DeleteReader.h"
#include "Win32DeleteWriter.h"
#include "Win32DirReader.h"
#include "Win32DirWriter.h"
#include "Win32HardlinkReader.h"
#include "Win32HardlinkWriter.h"
#endif

#endif // IO_ENGINES_H
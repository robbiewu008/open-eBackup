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
#ifndef FS_SCANNER_SCAN_QUEUE_UTILS_H
#define FS_SCANNER_SCAN_QUEUE_UTILS_H

#include "ScanConsts.h"
#include "ScanStructs.h"

namespace SCAN_QUEUE_UTILS {
    /* map partial member of DirStat to DirStatRW */
    bool PartialMapDirStatToDirStatRW(const DirStat &dirStat, DirStatReadWrite &dirStatRw);

    /* recover partial member of DirStat from DirStatRW */
    bool PartialRecoverDirStatFromDirStatRW(DirStat &dirStat, const DirStatReadWrite &dirStatRw);

    bool ReadDirStatFromBuffer(std::stringstream &file, DirStat &dirStat);
    
    bool WriteDirStatToBuffer(std::stringstream &buffer, const DirStat &dirStat, uint32_t &lengthWritten);
}
#endif // FS_SCANNER_SCAN_QUEUE_UTILS_H
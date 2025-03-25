/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 * Author: a00587389
 * Create: 8/30/2022.
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
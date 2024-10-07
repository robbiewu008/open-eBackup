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
#ifndef FILE_DEF_H
#define FILE_DEF_H
#include <string>

namespace Module {
enum OBJECT_TYPE {
    OBJECT_CACHE_DATA,
    OBJECT_FILE_DATA,
    OBJECT_NO_CACHE_DATA,
    OBJECT_EXTERNAL_DATA,
};

typedef struct EBKFileHandle {
    EBKFileHandle():partNum(0), fileOffset(0){}
    std::string handle;
    uint64_t partNum;
    uint64_t fileOffset;
} EBKFileHandle;

enum FileCloseStatus {
    FILE_CLOSE_STATUS_PAUSE,
    FILE_CLOSE_STATUS_FINISH,
};
} // namespace Module
#endif // FILE_DEF_H

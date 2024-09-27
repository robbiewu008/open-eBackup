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
#include <string>
#include <mutex>
#include "FileParser.h"
#include "ParserStructs.h"
#include "define/Defines.h"

namespace Module {

class BackupFailedCtrlParser : public FileParser {

public:
    BackupFailedCtrlParser();
    ~BackupFailedCtrlParser();

    CTRL_FILE_RETCODE OpenWrite() override;

    CTRL_FILE_RETCODE CloseWrite() override;

    CTRL_FILE_RETCODE FlushToFile() override;
    
    CTRL_FILE_RETCODE ReadHeader() override;

    CTRL_FILE_RETCODE ValidateHeader() override;

    CTRL_FILE_RETCODE WriteHeader() override;

    CTRL_FILE_RETCODE WriteFailedFile(std::string fileName);

    CTRL_FILE_RETCODE ReadFailedFile(std::string& fileName);
};
};
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
#ifndef PARSE_CONFIG_FILE_H
#define PARSE_CONFIG_FILE_H

#include <memory>
#include "define/Types.h"
#include "common/JsonUtils.h"
#include "common/Defines.h"
#include "LocalCmdExector.h"

namespace GeneralDB {
class ParseConfigFile {
public:
    static std::shared_ptr<ParseConfigFile> GetInstance();
    mp_int32 GetExectueCmd(const Param &comParam, mp_string &actionConf, mp_string &progessConf);
    mp_int32 GetGenJobConfHandle(Json::Value &retVal);

private:
    mp_int32 LoadAppConfFile(const mp_string &appType, Json::Value &retVal);
    mp_int32 GetJsonFileContent(const mp_string &filePath, Json::Value &retVal);
    mp_void ReplaceScriptPath(const mp_string &scriptDir, mp_string &fullPath);
    std::map<mp_string, Json::Value> m_appConfMap;
    Json::Value m_genJobConfHandle;
};
}
#endif
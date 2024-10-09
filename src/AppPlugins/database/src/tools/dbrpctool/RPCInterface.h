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
#ifndef PRC_INTERFACE_H
#define PRC_INTERFACE_H
#include <functional>
#include <map>
#include "common/JsonUtils.h"
#include "common/Defines.h"

namespace GeneralDB {
class RPCInterface {
public:
    RPCInterface();
    ~RPCInterface();

    mp_int32 Call(mp_string& interfaceName, mp_string& inputFilePath, mp_string& outputFilePat);
private:
    mp_int32 CreateResource(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 QueryResource(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 UpdateResource(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 DeleteResource(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 LockResource(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 UnLockResource(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 ReportJobDetails(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 ReportCopyAdditionalInfo(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 QueryPreviousCopy(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 MountRepositoryByPlugin(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 UnMountRepositoryByPlugin(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 AddIpWhiteList(const mp_string &inputFilePath, mp_string &outputFilePath);
    mp_int32 LockJobResource(const mp_string &inputFilePath, mp_string &outputFilePath);

private:
    using RPCFunc = std::function<mp_int32(mp_string&, mp_string&)>;
    std::map<mp_string, RPCFunc> m_rpcFunMap;
    mp_int32 GetInputFileContent(const mp_string &filePath, Json::Value &jsValue);
    mp_int32 WriteOutputFile(mp_string &filePath, Json::Value &outputValue);
};
}
#endif
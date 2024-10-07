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
#ifndef DORADO_API_OPERATOR_H
#define DORADO_API_OPERATOR_H

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <utility>
#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "common/CleanMemPwd.h"
#include "volume_handlers/common/ControlDevice.h"
#include "json/json.h"
#include "common/Constants.h"
#include "SessionCache.h"
#include "ApiOperator.h"

namespace VirtPlugin {
class DoradoApiOperator : public ApiOperator {
public:
    explicit DoradoApiOperator(ControlDeviceInfo deviceInfo) : ApiOperator(deviceInfo)
    {}
    ~DoradoApiOperator() override
    {}

    int32_t CreateMappingSet(const std::string& objId, MO_TYPE objType) override;
    int32_t DeleteMappingSet(const std::string& objId, MO_TYPE objType) override;

private:
    int32_t CreateHostLunMapping(const std::string &hostName, const std::string &lunName);
    int32_t QueryHostLunMapping(const std::string &hostName, const std::string &lunName);
    int32_t DeleteHostLunMapping(const std::string &hostName, const std::string &lunName);
    int32_t NameLegalization(std::string &name);
    int32_t GetObjName(const std::string& objId, MO_TYPE objType, std::string &objName);
    int32_t CreateHostLunGroupMapping(const std::string &hostName, const std::string &lunGroupName);
    int32_t QueryHostLunGroupMapping(const std::string &hostName, const std::string &lunGroupName);
    int32_t DeleteHostLunGroupMapping(const std::string &hostName, const std::string &lunGroupName);
};
}

#endif
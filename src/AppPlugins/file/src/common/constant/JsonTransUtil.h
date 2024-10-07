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
#ifndef TR_JSON_WITH_STRUCT_H
#define TR_JSON_WITH_STRUCT_H

#include "define/Types.h"
#include "Module/src/common/JsonHelper.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"

static void StructToJson(const Authentication& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(int(st.authType), jsonValue["authType"]);
    Module::JsonHelper::TypeToJsonValue(st.authkey, jsonValue["authKey"]);
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const Application& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    Module::JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.parentId, jsonValue["parentId"]);
    Module::JsonHelper::TypeToJsonValue(st.parentName, jsonValue["parentName"]);
    StructToJson(st.auth, jsonValue["auth"]);
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

static void StructToJson(const ApplicationEnvironment& st, Json::Value& jsonValue)
{
    Module::JsonHelper::TypeToJsonValue(st.id, jsonValue["id"]);
    Module::JsonHelper::TypeToJsonValue(st.name, jsonValue["name"]);
    Module::JsonHelper::TypeToJsonValue(st.type, jsonValue["type"]);
    Module::JsonHelper::TypeToJsonValue(st.subType, jsonValue["subType"]);
    Module::JsonHelper::TypeToJsonValue(st.endpoint, jsonValue["endpoint"]);
    Module::JsonHelper::TypeToJsonValue(st.port, jsonValue["port"]);
    StructToJson(st.auth, jsonValue["auth"]);
    if (!st.extendInfo.empty())
        Module::JsonHelper::JsonStringToJsonValue(st.extendInfo, jsonValue["extendInfo"]);
}

#endif
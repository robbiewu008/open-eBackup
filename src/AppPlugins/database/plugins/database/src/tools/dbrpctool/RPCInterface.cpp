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
#include "RPCInterface.h"
#include <fstream>
#include "log/Log.h"
#include "common/File.h"
#include "client/ClientInvoke.h"
#include "trjsontostruct.h"
#include "trstructtojson.h"
#include "ParamHandler.h"

using namespace GeneralDB;
namespace {
static const mp_string MODULE = "RPCInterface";
}

RPCInterface::RPCInterface()
{
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("CreateResource",
        std::bind(&RPCInterface::CreateResource, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("QueryResource",
        std::bind(&RPCInterface::QueryResource, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("UpdateResource",
        std::bind(&RPCInterface::UpdateResource, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("DeleteResource",
        std::bind(&RPCInterface::DeleteResource, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("LockResource",
        std::bind(&RPCInterface::LockResource, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("UnLockResource",
        std::bind(&RPCInterface::UnLockResource, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("ReportJobDetails",
        std::bind(&RPCInterface::ReportJobDetails, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("ReportCopyAdditionalInfo",
        std::bind(&RPCInterface::ReportCopyAdditionalInfo, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("QueryPreviousCopy",
        std::bind(&RPCInterface::QueryPreviousCopy, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("MountRepositoryByPlugin",
        std::bind(&RPCInterface::MountRepositoryByPlugin, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("UnMountRepositoryByPlugin",
        std::bind(&RPCInterface::UnMountRepositoryByPlugin, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("AddIpWhiteList",
        std::bind(&RPCInterface::AddIpWhiteList, this, std::placeholders::_1, std::placeholders::_2)));
    m_rpcFunMap.insert(std::make_pair<mp_string, RPCFunc>("LockJobResource",
        std::bind(&RPCInterface::LockJobResource, this, std::placeholders::_1, std::placeholders::_2)));
}

RPCInterface::~RPCInterface()
{
}

mp_int32 RPCInterface::Call(mp_string& interfaceName, mp_string& inputFilePath, mp_string& outputFilePath)
{
    auto iter = m_rpcFunMap.find(interfaceName);
    if (iter != m_rpcFunMap.end()) {
        return iter->second(inputFilePath, outputFilePath);
    }
    ERRLOG("The interface %s is not defined.", interfaceName.c_str());
    return MP_FAILED;
}

mp_int32 RPCInterface::CreateResource(const mp_string &inputFilePath, mp_string &outputFilePath)
{
#ifndef WIN32
    AppProtect::ActionResult returnValue;
    AppProtect::Resource resource;
    Json::Value inputValue;
    mp_string mainJobId;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue, resource);
        mainJobId = inputValue["jobId"].asString();
        ShareResource::CreateResource(returnValue, resource, mainJobId);
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
#else
    ERRLOG("Windows does not support CreateResource interface.");
    return MP_FAILED;
#endif
}

mp_int32 RPCInterface::QueryResource(const mp_string &inputFilePath, mp_string &outputFilePath)
{
#ifndef WIN32
    AppProtect::ResourceStatus returnValue;
    AppProtect::Resource resource;
    Json::Value inputValue;
    mp_string mainJobId;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue, resource);
        mainJobId = inputValue["jobId"].asString();
        ShareResource::QueryResource(returnValue, resource, mainJobId);
    } else {
        return MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
#else
    ERRLOG("Windows does not support QueryResource interface.");
    return MP_FAILED;
#endif
}

mp_int32 RPCInterface::UpdateResource(const mp_string &inputFilePath, mp_string &outputFilePath)
{
#ifndef WIN32
    AppProtect::ActionResult returnValue;
    AppProtect::Resource resource;
    Json::Value inputValue;
    mp_string mainJobId;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue, resource);
        mainJobId = inputValue["jobId"].asString();
        ShareResource::UpdateResource(returnValue, resource, mainJobId);
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
#else
    ERRLOG("Windows does not support UpdateResource interface.");
    return MP_FAILED;
#endif
}

mp_int32 RPCInterface::DeleteResource(const mp_string &inputFilePath, mp_string &outputFilePath)
{
#ifndef WIN32
    AppProtect::ActionResult returnValue;
    AppProtect::Resource resource;
    Json::Value inputValue;
    mp_string mainJobId;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue, resource);
        mainJobId = inputValue["jobId"].asString();
        ShareResource::DeleteResource(returnValue, resource, mainJobId);
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
#else
    ERRLOG("Windows does not support DeleteResource interface.");
    return MP_FAILED;
#endif
}

mp_int32 RPCInterface::LockResource(const mp_string &inputFilePath, mp_string &outputFilePath)
{
#ifndef WIN32
    AppProtect::ActionResult returnValue;
    AppProtect::Resource resource;
    Json::Value inputValue;
    mp_string mainJobId;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue, resource);
        mainJobId = inputValue["jobId"].asString();
        ShareResource::LockResource(returnValue, resource, mainJobId);
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
#else
    ERRLOG("Windows does not support LockResource interface.");
    return MP_FAILED;
#endif
}

mp_int32 RPCInterface::UnLockResource(const mp_string &inputFilePath, mp_string &outputFilePath)
{
#ifndef WIN32
    AppProtect::ActionResult returnValue;
    AppProtect::Resource resource;
    Json::Value inputValue;
    mp_string mainJobId;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue, resource);
        mainJobId = inputValue["jobId"].asString();
        ShareResource::UnLockResource(returnValue, resource, mainJobId);
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
#else
    ERRLOG("Windows does not support UnLockResource interface.");
    return MP_FAILED;
#endif
}

mp_int32 RPCInterface::ReportJobDetails(const mp_string &inputFilePath, mp_string &outputFilePath)
{
    AppProtect::ActionResult returnValue;
    AppProtect::SubJobDetails jobdetails;
    Json::Value inputValue;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue, jobdetails);
        JobService::ReportJobDetails(returnValue, jobdetails);
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
}

mp_int32 RPCInterface::ReportCopyAdditionalInfo(const mp_string &inputFilePath, mp_string &outputFilePath)
{
    AppProtect::ActionResult returnValue;
    AppProtect::Copy copy;
    mp_string jobId;
    Json::Value inputValue;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        if (inputValue.isObject() && inputValue["jobId"].isString()) {
            JsonToStruct(inputValue["copy"], copy);
            jobId = inputValue["jobId"].asString();
            JobService::ReportCopyAdditionalInfo(returnValue, jobId, copy);
        } else {
            returnValue.code = MP_FAILED;
        }
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
}

mp_int32 RPCInterface::QueryPreviousCopy(const mp_string &inputFilePath, mp_string &outputFilePath)
{
    AppProtect::Copy copy;
    AppProtect::Application application;
    std::set<AppProtect::CopyDataType> types;
    mp_string copyId;
    Json::Value inputValue;
    mp_string mainJobId;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue["application"], application);
        for (Json::ArrayIndex index = 0; index < inputValue["types"].size(); ++index) {
            AppProtect::CopyDataType dataType;
            if (!inputValue["types"][index].isString()) {
                continue;
            }
            auto it = TransferMap_CopyDataType.find(inputValue["types"][index].asString());
            if (it != TransferMap_CopyDataType.end()) {
                dataType = it->second;
                types.insert(dataType);
            }
        }
        if (inputValue["copyId"].isString()) {
            copyId = inputValue["copyId"].asString();
        }
        mainJobId = inputValue["jobId"].asString();
        try {
            JobService::QueryPreviousCopy(copy, application, types, copyId, mainJobId);
        } catch (AppProtect::AppProtectFrameworkException &ex) {
            ERRLOG("Catch exception, code=%d, message=%s.", ex.code, ex.message.c_str());
            return MP_FAILED;
        }
    } else {
        return MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(copy, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 RPCInterface::MountRepositoryByPlugin(const mp_string &inputFilePath, mp_string &outputFilePath)
{
    AppProtect::ActionResult returnValue;
    AppProtect::PrepareRepositoryByPlugin mountinfo;
    Json::Value inputValue;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue, mountinfo);
        JobService::MountRepositoryByPlugin(returnValue, mountinfo);
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
}

mp_int32 RPCInterface::UnMountRepositoryByPlugin(const mp_string &inputFilePath, mp_string &outputFilePath)
{
    AppProtect::ActionResult returnValue;
    AppProtect::PrepareRepositoryByPlugin mountinfo;
    Json::Value inputValue;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue, mountinfo);
        JobService::UnMountRepositoryByPlugin(returnValue, mountinfo);
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
}

mp_int32 RPCInterface::AddIpWhiteList(const mp_string &inputFilePath, mp_string &outputFilePath)
{
    mp_string jobId;
    mp_string ipListStr;
    AppProtect::ActionResult returnValue;
    Json::Value inputValue;
    INFOLOG("AddIpWhiteList.");
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        jobId = inputValue["jobId"].asString();
        ipListStr = inputValue["ipListStr"].asString();
        JobService::AddIpWhiteList(returnValue, jobId, ipListStr);
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
}

mp_int32 RPCInterface::LockJobResource(const mp_string &inputFilePath, mp_string &outputFilePath)
{
    AppProtect::ActionResult returnValue;
    AppProtect::Resource resource;
    Json::Value inputValue;
    mp_string mainJobId;
    if (GetInputFileContent(inputFilePath, inputValue) == MP_SUCCESS) {
        JsonToStruct(inputValue, resource);
        mainJobId = inputValue["jobId"].asString();
        ShareResource::LockJobResource(returnValue, resource, mainJobId);
    } else {
        returnValue.code = MP_FAILED;
    }
    Json::Value outputValue;
    StructToJson(returnValue, outputValue);
    if (WriteOutputFile(outputFilePath, outputValue) != MP_SUCCESS) {
        returnValue.code = MP_FAILED;
    }
    return returnValue.code;
}

mp_int32 RPCInterface::GetInputFileContent(const mp_string &filePath, Json::Value &jsValue)
{
    mp_string fileContent;
    if (Module::CFile::ReadFile(filePath, fileContent) != MP_SUCCESS) {
        ERRLOG("Read input file content failed.");
        return MP_FAILED;
    }
    if (!Module::JsonHelper::JsonStringToJsonValue(fileContent, jsValue)) {
        ERRLOG("Failed to convert string into json.");
        return MP_FAILED;
    }
    if (!jsValue.isObject()) {
        ERRLOG("Json value is not object.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 RPCInterface::WriteOutputFile(mp_string &filePath, Json::Value &outputValue)
{
    ParamHandler handler;
    std::map<mp_string, mp_string> sensitiveInfo;
    if (handler.Exec(outputValue, sensitiveInfo) != MP_SUCCESS) {
        ERRLOG("Failed to filter sensitive info.");
        return MP_FAILED;
    }
    sensitiveInfo.clear();
    mp_string inputStr = outputValue.toStyledString();
    std::ofstream file(filePath, std::ios::out | std::ios::binary);
    file.write(inputStr.c_str(), inputStr.size());
    if (!file.good()) {
        ERRLOG("Write file failed, path: %s.", filePath.c_str());
        return MP_FAILED;
    }
    file.close();
    return MP_SUCCESS;
}

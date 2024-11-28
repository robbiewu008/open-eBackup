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
#ifndef APPPLUGIN_NAS_APPLICATIONMANAGER_H
#define APPPLUGIN_NAS_APPLICATIONMANAGER_H
#include <iostream>
#include <vector>
#include <memory>
#include "ApplicationServiceDataType.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"

#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
namespace FilePlugin {
 
class ApplicationManager {
public:
 
    virtual ~ApplicationManager() = default;
 
    // 浏览资源入口
    void ListApplicationResource(
        AppProtect::ResourceResultByPage& returnValue,
        const FilePlugin::ListResourceParam& listResourceParam);
    
protected:
    virtual void ListNativeResource(
        FilePlugin::FileResourceInfo& resourceInfo,
        const FilePlugin::ListResourceParam& listResourceParam) = 0;
 
    virtual void ListAggregateResource(
        FilePlugin::FileResourceInfo& resourceInfo,
        const FilePlugin::ListResourceParam& listResourceParam) = 0;
 
    virtual void ListVolumeResource(
        FilePlugin::FileResourceInfo& resourceInfo,
        const FilePlugin::ListResourceParam& listResourceParam) = 0;
 
    inline void ThrowAppException(int errCode, const std::string& message) const
    {
        AppProtect::AppProtectPluginException appException;
        appException.__set_code(errCode);
        appException.__set_message(message);
        throw appException;
    }
    virtual void TransformResultForVolume(AppProtect::ResourceResultByPage& returnValue,
        const FilePlugin::FileResourceInfo& resourceInfo);
private:
    void TransformResultForNative(AppProtect::ResourceResultByPage& returnValue,
        const FilePlugin::FileResourceInfo& resourceInfo);
    void TransformResult(AppProtect::ResourceResultByPage& returnValue,
        const FilePlugin::FileResourceInfo& resourceInfo);
};
 
using AppManagerPtr = std::unique_ptr<ApplicationManager>;
 
}
#endif
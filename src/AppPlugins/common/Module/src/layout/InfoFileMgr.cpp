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
#include "InfoFileMgr.h"

#include <string>

#include "common/JsonUtils.h"

using namespace std;
using namespace Module;

LayoutRetCode InfoFileMgr::CreateInfoFile(const ImageSetInfo &imageSetInfo, string &infoFileBuffer)
{
    ImageSetInfo tmpSetInfo(imageSetInfo);
    if (!JsonHelper::StructToJsonString(tmpSetInfo, infoFileBuffer)) {
        return LayoutRetCode::FAILED;
    }
    return LayoutRetCode::SUCCESS;
}

LayoutRetCode InfoFileMgr::UpdateInfoFile(const ImageSetInfo &imageSetInfo, string &infoFileBuffer)
{
    if (CreateInfoFile(imageSetInfo, infoFileBuffer) != LayoutRetCode::SUCCESS) {
        return LayoutRetCode::FAILED;
    }
    return LayoutRetCode::SUCCESS;
}

LayoutRetCode InfoFileMgr::AnalyzeInfoFile(const string &infoFileBuffer, ImageSetInfo &imageSetInfo)
{
    if (!JsonHelper::JsonStringToStruct(infoFileBuffer, imageSetInfo)) {
        return LayoutRetCode::FAILED;
    }
    return LayoutRetCode::SUCCESS;
}

LayoutRetCode InfoFileMgr::CreateInfoFile(const ImageInfo &imageInfo, string &infoFileBuffer)
{
    ImageInfo tmpImageInfo(imageInfo);
    if (!JsonHelper::StructToJsonString(tmpImageInfo, infoFileBuffer)) {
        return LayoutRetCode::FAILED;
    }
    return LayoutRetCode::SUCCESS;
}

LayoutRetCode InfoFileMgr::UpdateInfoFile(const ImageInfo &imageInfo, string &infoFileBuffer)
{
    if (CreateInfoFile(imageInfo, infoFileBuffer) != LayoutRetCode::SUCCESS) {
        return LayoutRetCode::FAILED;
    }
    return LayoutRetCode::SUCCESS;
}

LayoutRetCode InfoFileMgr::AnalyzeInfoFile(const string &infoFileBuffer, ImageInfo &imageInfo)
{
    if (!JsonHelper::JsonStringToStruct(infoFileBuffer, imageInfo)) {
        return LayoutRetCode::FAILED;
    }
    return LayoutRetCode::SUCCESS;
}


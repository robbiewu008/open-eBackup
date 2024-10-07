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
#ifndef INFO_FILE_MGR_H
#define INFO_FILE_MGR_H

#include <string>
#include "LayoutCommon.h"

namespace Module {
class InfoFileMgr {
public:
    InfoFileMgr() {};
    ~InfoFileMgr() = default;

    /* imageset info */
    LayoutRetCode CreateInfoFile(const ImageSetInfo &imageSetInfo, std::string &infoFileBuffer);
    LayoutRetCode UpdateInfoFile(const ImageSetInfo &imageSetInfo, std::string &infoFileBuffer);
    LayoutRetCode AnalyzeInfoFile(const std::string &infoFileBuffer, ImageSetInfo &imageSetInfo);

    /* image info */
    LayoutRetCode CreateInfoFile(const ImageInfo &imageInfo, std::string &infoFileBuffer);
    LayoutRetCode UpdateInfoFile(const ImageInfo &imageInfo, std::string &infoFileBuffer);
    LayoutRetCode AnalyzeInfoFile(const std::string &infoFileBuffer, ImageInfo &imageInfo);
};
} // namespace Module
#endif // INFO_FILE_MGR_H
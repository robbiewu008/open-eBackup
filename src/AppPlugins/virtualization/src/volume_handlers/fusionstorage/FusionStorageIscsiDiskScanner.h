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
#ifndef FUSIONSTORAGE_DISK_SCANNER_H
#define FUSIONSTORAGE_DISK_SCANNER_H

#include "volume_handlers/oceanstor/DiskScannerHandler.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

class FusionStorageIscsiDiskScanner {
public:
    static FusionStorageIscsiDiskScanner *GetInstance();

    virtual ~FusionStorageIscsiDiskScanner();

    FusionStorageIscsiDiskScanner(const FusionStorageIscsiDiskScanner &) = delete;
    FusionStorageIscsiDiskScanner &operator=(const FusionStorageIscsiDiskScanner &) = delete;

    // 映射建立后读取路径
    int32_t DoScanAfterMapped(
        const std::string &volName, const std::string &volWwn, std::string &diskDevicePath, bool isInternalScence);
    // 通过wwn查找到路径
    std::string GetDiskPathForWWN(const std::string &volWwn);
    // 登出Target
    int32_t DoLogOut(const std::string &targetIp);
    // 查看已登录
    int32_t GetLoginedTargetIP(std::vector<std::string> &loginedIps);
    // 在登出前删除不用路径
    int32_t DeleteDiskFromPathSet(const std::string &volName, const std::string &volWwn);
    // 获得路径集合
    int32_t GetIscsiDiskPathSet(std::set<std::string> &diskPathSet);
    // 从路径集合中删除指定路径
    int32_t DoDeleteDiskFromPathSet(
        std::set<std::string> &diskPathSet, const std::string &targetDiskPath, bool isExist);
    int32_t GetLoginedNodeIP(std::vector<std::string> &nodeIPs);

private:
    FusionStorageIscsiDiskScanner();
};

VIRT_PLUGIN_NAMESPACE_END

#endif
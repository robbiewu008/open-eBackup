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
#include "FsDevice.h"

using namespace std;
namespace FilePlugin {
FsDevice::FsDevice() : devNo(0), mountPoint(""), fsType(""),
    deviceName(""), lvPath(""), supportSnapCalled(false), isSupportSnapshot(false)
{
}
FsDevice::FsDevice(uint64_t idevNo, std::string imountPoint,
    std::string ifsType, std::string ideviceName)
    : devNo(idevNo), mountPoint(imountPoint), fsType(ifsType), deviceName(ideviceName)
{
}

FsDevice::FsDevice(uint64_t idevNo, std::set<std::string> imountPoints, std::string ifsType, std::string ideviceName)
    : devNo(idevNo), mountPoints(imountPoints), fsType(ifsType), deviceName(ideviceName)
{
}
}

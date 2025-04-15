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
#ifndef __AGENT_VMWARENATIVE_STORAGEDEVICEFACTORY_H__
#define __AGENT_VMWARENATIVE_STORAGEDEVICEFACTORY_H__

#include <memory>
#include "common/Types.h"
#include "AbstractStorageDevice.h"

class StorageDeviceFactory {
public:
    StorageDeviceFactory();
    virtual ~StorageDeviceFactory();
    static std::shared_ptr<AbstractStorageDevice> CreateStorageDevice(mp_int32 storageProtocol);
};

#endif
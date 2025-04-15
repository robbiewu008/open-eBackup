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
#include "dataprocess/vmwarenative/StorageDeviceFactory.h"
#include "dataprocess/vmwarenative/FileStorageDevice.h"
#include "dataprocess/vmwarenative/BlockStorageDevice.h"

using namespace std;
namespace STORAGEPROTOCOL {
const mp_int32 PROTOCOL_ISCSI = 1;
const mp_int32 PROTOCOL_NAS = 2;
}  // namespace STORAGEPROTOCOL

StorageDeviceFactory::StorageDeviceFactory()
{}

StorageDeviceFactory::~StorageDeviceFactory()
{}

std::shared_ptr<AbstractStorageDevice> StorageDeviceFactory::CreateStorageDevice(mp_int32 storageProtocol)
{
    switch (storageProtocol) {
        case STORAGEPROTOCOL::PROTOCOL_ISCSI:
            COMMLOG(OS_LOG_INFO, "The block storage device will be created, storage type: '%d'.", storageProtocol);
            return make_shared<BlockStorageDevice>();
            break;
        case STORAGEPROTOCOL::PROTOCOL_NAS:
            COMMLOG(OS_LOG_INFO, "The NAS storage device will be created, storage type: '%d'.", storageProtocol);
            return make_shared<FileStorageDevice>();
            break;
        default:
            COMMLOG(OS_LOG_ERROR, "Unknown storage type: '%d'.", storageProtocol);
            break;
    }
    return NULL;
}
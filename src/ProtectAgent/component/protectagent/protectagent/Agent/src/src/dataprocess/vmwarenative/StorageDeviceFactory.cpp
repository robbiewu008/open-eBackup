/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file StorageDeviceFactory.cpp
 * @brief  Implementation of the Class StorageDeviceFactory
 * @version 1.0.0
 * @date 2019-11-15
 * @author wangguitao 00510599
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
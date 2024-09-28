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
#include "dataprocess/StorageDeviceFactoryTest.h"
#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubStorageDeviceFactoryGetValueInt32Return);                                                                      \
    } while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}
TEST_F(StorageDeviceFactoryTest, CreateStorageDevice)
{
    mp_int32 storageProtocol = 1;
    StorageDeviceFactory om;

    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    //storageProtocol = 1
    {
        om.CreateStorageDevice(storageProtocol);
    }
    //storageProtocol = 1
    {
        storageProtocol = 2;
        om.CreateStorageDevice(storageProtocol);
    }
    //storageProtocol = 3
    {
        storageProtocol = 3;
        om.CreateStorageDevice(storageProtocol);
    }
}

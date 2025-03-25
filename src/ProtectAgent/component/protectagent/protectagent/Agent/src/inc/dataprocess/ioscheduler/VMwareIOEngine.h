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
#ifndef __VMWARE_IO_ENGINE_H__
#define __VMWARE_IO_ENGINE_H__

#include <cstdint>
#include <memory>
#include "IOEngine.h"
#include "dataprocess/vmwarenative/VMwareDiskApi.h"

class VMwareIOEngine : public IOEngine {
public:
    VMwareIOEngine(std::shared_ptr<VMwareDiskApi> sp) : m_vixDiskApi(sp){};
    ~VMwareIOEngine() {}
    mp_int32 Read(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer) override;
    mp_int32 Write(const uint64_t& offsetInBytes, uint64_t& bufferSizeInBytes, unsigned char* buffer) override;
    mp_string GetFileName() override { return "";}
private:
    std::shared_ptr<VMwareDiskApi> m_vixDiskApi;
};

#endif
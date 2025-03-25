/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file VMwareIOEngine.h
 * @author t00302329
 * @brief VMware IO读写接口
 * @version 0.1
 * @date 2021-01-14
 *
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
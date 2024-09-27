/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file DataContext.h
 * @brief  Contains function declarations for DataContext
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __DATA_CONTEXT_H__
#define __DATA_CONTEXT_H__

#include <map>
#include "common/Log.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "common/Types.h"
#include "message/tcp/CSocket.h"
class DataContext {
public:
    mp_void SetSockFd(mp_socket sFd);
    mp_socket GetSockFd();
    mp_int32 GetDiskFdByName(const mp_string &diskName);
    mp_int32 SetDiskFdByName(const mp_string &diskName, mp_int32 diskFd);

private:
    mp_socket sockFd;
    std::map<mp_string, mp_int32> mapOfDisks;
};
#endif
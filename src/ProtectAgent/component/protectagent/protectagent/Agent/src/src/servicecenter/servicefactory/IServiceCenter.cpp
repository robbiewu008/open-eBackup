/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file IServiceCenter.cpp
 * @brief  implement for IServiceCenter
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#include "servicefactory/include/IServiceCenter.h"

namespace servicecenter {
std::shared_ptr<IServiceCenter> IServiceCenter::g_instance = nullptr;

std::shared_ptr<IServiceCenter> IServiceCenter::GetInstance()
{
    return g_instance;
}
}
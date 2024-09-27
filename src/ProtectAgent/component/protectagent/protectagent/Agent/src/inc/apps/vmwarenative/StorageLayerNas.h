/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */
#ifndef __STORAGELAYERNAS_H__
#define __STORAGELAYERNAS_H__

#include <vector>
#include "common/Types.h"

class StorageLayerNas {
public:
    static mp_int32 NasMount(const std::ostringstream &cmdParam);
    static mp_int32 NasUnMount(const std::ostringstream &cmdParam);
};

#endif
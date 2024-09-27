/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file PartitionFactory.h
 * @brief AFS - Partition factory class.
 *
 */

#ifndef PARTITIONFACTORY_H
#define PARTITIONFACTORY_H

#include "afs/PartitionHandler.h"
#include "afs/MBRHandler.h"
#include "afs/GPTHandler.h"

class PartitionFacotry : public afsObjectFacotry {
public:
    PartitionFacotry();
};
#endif

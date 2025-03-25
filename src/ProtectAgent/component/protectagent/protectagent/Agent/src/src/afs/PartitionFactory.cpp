/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file partitionfactory.cpp
 * @brief AFS - Partition factory class.
 *
 */

#include "afs/PartitionFactory.h"
#include "afs/RawReader.h"
// 分区工厂
PartitionFacotry::PartitionFacotry()
{
    registerClass("mbr", rawReader::CreateObject);
}

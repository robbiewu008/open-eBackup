/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Object storage statistics.
* Author: w00444223
* Create: 2023-12-04
*/

#include "ObjectStatistics.h"
#include "ScanConfig.h"

namespace {
    constexpr int MAX_FILES_NUM_OF_DIR = 100000;
}

ObjectStatistics::ObjectStatistics()
{
    for (int statsType = 0; statsType < MAX_OBJECT_STATS_TYPE; statsType++) {
        m_protoStatistics[statsType] = 0;
    }
}

ObjectStatistics::~ObjectStatistics()
{
    for (int statsType = 0; statsType < MAX_OBJECT_STATS_TYPE; statsType++) {
        m_protoStatistics[statsType] = 0;
    }
}

void ObjectStatistics::IncrProtoStatsByType(const int& idx, const uint64_t& incVal)
{
    m_protoStatistics[idx] += incVal;
}

void ObjectStatistics::SetProtoStatsByType(const int& idx, const uint64_t& newVal)
{
    m_protoStatistics[idx] = newVal;
}

uint64_t ObjectStatistics::GetProtoStatsByType(const int& idx)
{
    return m_protoStatistics[idx];
}

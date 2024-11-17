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
#ifndef CONSISTENT_HASHRING_H
#define CONSISTENT_HASHRING_H
#include <map>
#include "common/Types.h"
#include "common/Defines.h"

constexpr uint32_t NODE_REPLICA = 128;
constexpr uint32_t UINT256_BIT_NUM = 4;

struct uint256_t {
    uint64_t bit1 = 0;
    uint64_t bit2 = 0;
    uint64_t bit3 = 0;
    uint64_t bit4 = 0;

    bool operator< (const uint256_t& other) const
    {
        for (int nOffset = 0; nOffset < UINT256_BIT_NUM; nOffset++) {
            uint64_t lh = *(uint64_t*)(uint64_t(this) + sizeof(uint64_t) * nOffset);
            uint64_t rh = *(uint64_t*)(uint64_t(&other) + sizeof(uint64_t) * nOffset);
            if (lh == rh) {
                continue;
            } else if (lh > rh) {
                return false;
            } else {
                return true;
            }
        }
        return false;
    }
};

mp_int32 CalculationHash(const std::string& in, uint256_t& out);

class  AGENT_API ConsistentHashRing {
public:
    using NodeMap = std::map<uint256_t, std::string>;

    ConsistentHashRing(unsigned int replicas = NODE_REPLICA) : m_replicas(replicas)
    {}

    ~ConsistentHashRing()
    {}

    mp_bool AddNode(const std::string& node);

    mp_bool RemoveNode(const std::string& node);

    mp_bool AssignNode(const std::string& data, std::string& node) const;

private:
    NodeMap m_ring;
    const unsigned int m_replicas;
};

#endif
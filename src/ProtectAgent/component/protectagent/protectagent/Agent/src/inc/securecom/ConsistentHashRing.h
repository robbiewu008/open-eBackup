/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 *
 * @file ConsistentHashRing.h
 * @brief Consistent Has Ring
 * @version 0.1
 * @date 2021-12-25
 * @author
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
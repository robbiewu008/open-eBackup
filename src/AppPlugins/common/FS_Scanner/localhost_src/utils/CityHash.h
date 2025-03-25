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
#ifndef SCANNER_CITYHASH_H
#define SCANNER_CITYHASH_H

#include <cstdint>
#include <cstddef>
#include <utility>

#define STATIC_INLINE static inline
using Uint128T = std::pair<uint64_t, uint64_t>;

class CityHash {
public:
    explicit CityHash() {}
    virtual ~CityHash() {}
    static uint64_t CityHash64(const char *s, size_t len);
private:
    STATIC_INLINE uint32_t Uint32InExpectedOrder(uint32_t x);
    STATIC_INLINE uint64_t Uint64InExpectedOrder(uint64_t x);
    STATIC_INLINE uint64_t Uint128Low64(const Uint128T& x);
    STATIC_INLINE uint64_t Uint128High64(const Uint128T& x);
    STATIC_INLINE uint64_t UNALIGNED_LOAD64(const char *p);
    STATIC_INLINE uint32_t UNALIGNED_LOAD32(const char *p);
    STATIC_INLINE uint32_t Bswap32(const uint32_t x);
    STATIC_INLINE uint64_t Bswap64(const uint64_t x);
    STATIC_INLINE uint64_t Hash128to64(const Uint128T& x);
    STATIC_INLINE uint64_t Get64(const char *p);
    STATIC_INLINE uint32_t Get32(const char *p);
    STATIC_INLINE uint64_t Rotate(uint64_t val, int shift);
    STATIC_INLINE uint64_t ShiftMix(uint64_t value);
    STATIC_INLINE uint64_t HashLen16(uint64_t u, uint64_t v);
    STATIC_INLINE uint64_t HashLen16(uint64_t u, uint64_t v, uint64_t mul);
    STATIC_INLINE uint64_t HashLen0to16(const char *s, size_t len);
    STATIC_INLINE uint64_t HashLen17to32(const char *s, size_t len);
    STATIC_INLINE std::pair<uint64_t, uint64_t> WeakHashLen32WithSeeds(const char* s, uint64_t a, uint64_t b);
    STATIC_INLINE uint64_t HashLen33to64(const char *s, size_t len);
    
    STATIC_INLINE uint64_t CityHash64WithSeed(const char *buf, size_t len, uint64_t seed);
    STATIC_INLINE uint64_t CityHash64WithSeeds(const char *buf, size_t len, uint64_t seed0, uint64_t seed1);
};


#endif // DME_NAS_SCANNER_CITYHASH_H

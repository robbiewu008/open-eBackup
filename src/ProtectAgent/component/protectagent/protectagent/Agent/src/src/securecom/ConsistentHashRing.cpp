#include "securecom/ConsistentHashRing.h"
#include <openssl/evp.h>
#include <vector>
#include <iomanip>
#include "common/Defines.h"
#include "common/Log.h"

constexpr uint32_t HEX_SZIE = 2;
constexpr uint32_t EVP_HASH_SIZE = 32;

mp_int32 CalculationHash(const std::string& in, uint256_t& out)
{
    const EVP_MD* md = EVP_sha256();
    unsigned char mdvalue[EVP_MAX_MD_SIZE];
    unsigned int mdlen = 0;
    EVP_MD_CTX* mdctx = EVP_MD_CTX_new();
    CHECK_POINTER_NULL(mdctx);
    EVP_DigestInit_ex(mdctx, md, nullptr);
    EVP_DigestUpdate(mdctx, in.c_str(), in.size());
    EVP_DigestFinal_ex(mdctx, mdvalue, &mdlen);
    EVP_MD_CTX_free(mdctx);
    if (mdlen != EVP_HASH_SIZE) {
        ERRLOG("CalculationHash failed, mdlen=%d.", mdlen);
        return MP_FAILED;
    }
    for (int i = 0; i < UINT256_BIT_NUM; i++) {
        unsigned int nStart = i * (mdlen / UINT256_BIT_NUM);
        unsigned int nEnd = (i + 1) * (mdlen / UINT256_BIT_NUM);
        std::stringstream ss;
        uint64_t lngHex = 0;
        for (unsigned int pos = nStart; pos < nEnd; ++pos) {
            ss << std::hex << std::setw(HEX_SZIE) << std::setfill('0') << int(mdvalue[pos]);
        }
        ss >> std::hex >> lngHex;
        CHECK_NOT_OK(memcpy_s((void*)(uint64_t(&out) + sizeof(uint64_t) * i),
            sizeof(uint64_t), &lngHex, sizeof(uint64_t)));
    }

    return MP_SUCCESS;
}

mp_bool ConsistentHashRing::AddNode(const std::string& node)
{
    for (unsigned int i = 0; i < m_replicas; i++) {
        uint256_t hash;
        if (CalculationHash(node + "#" + std::to_string(i), hash) != MP_SUCCESS) {
            return MP_FALSE;
        }
        m_ring[hash] = node;
    }
    return MP_TRUE;
}

mp_bool ConsistentHashRing::RemoveNode(const std::string& node)
{
    for (unsigned int i = 0; i < m_replicas; i++) {
        uint256_t hash;
        if (CalculationHash(node + "#" + std::to_string(i), hash) != MP_SUCCESS) {
            return MP_FALSE;
        }
        m_ring.erase(hash);
    }
    return MP_TRUE;
}

mp_bool ConsistentHashRing::AssignNode(const std::string& data, std::string& node) const
{
    uint256_t hash;
    if (CalculationHash(data, hash) != MP_SUCCESS) {
        return MP_FALSE;
    }

    // Find the first node >= hash
    auto it = m_ring.lower_bound(hash);
    if (it == m_ring.end()) {
        // If wrap around, get the first node
        it = m_ring.begin();
    }
    node = it->second;
    return MP_TRUE;
}

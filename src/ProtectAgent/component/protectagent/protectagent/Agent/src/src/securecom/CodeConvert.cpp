#include "securecom/CodeConvert.h"
#include <securec.h>
#include <openssl/ssl.h>
#include <memory>
#include "common/Log.h"
#include "common/Types.h"

namespace {
const mp_int32 MAX_ENCODE_BASE64_LENGTH = 10*1024*1024; // 10MB
}
bool CodeConvert::EncodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out)
{
    if (bufferSize <= 0 || bufferSize > MAX_ENCODE_BASE64_LENGTH) {
        ERRLOG("bufferSize abnormal: %d", bufferSize);
        return false;
    }
    std::unique_ptr<unsigned char[]> base64_array = std::make_unique<unsigned char[]>(bufferSize);
    
    memset_s(base64_array.get(), bufferSize, 0, bufferSize);

    std::size_t len = in.length();
    EVP_EncodeBlock(base64_array.get(), (unsigned char*)const_cast<char*>(in.c_str()), (int)len);
    
    out = std::string((char*)base64_array.get());

    return true;
}

bool CodeConvert::DecodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out)
{
    if (bufferSize <= 0 || bufferSize > MAX_ENCODE_BASE64_LENGTH) {
        ERRLOG("bufferSize abnormal: %d", bufferSize);
        return false;
    }
    std::unique_ptr<unsigned char[]> base64_array = std::make_unique<unsigned char[]>(bufferSize);
    
    memset_s(base64_array.get(), bufferSize, 0, bufferSize);

    std::size_t len = in.length();
    int decodeLen = EVP_DecodeBlock(base64_array.get(), (unsigned char*)const_cast<char*>(in.c_str()), (int)len);
    if (decodeLen < 0) {
        return false;
    }
    
    while (in[--len] == '=') decodeLen--;
    out = std::string((char*)base64_array.get(), decodeLen);

    return true;
}

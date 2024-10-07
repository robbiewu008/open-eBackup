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
#include <iostream>
#include <securec.h>
#include <openssl/ssl.h>
#include <openssl/evp.h>
#include "log/Log.h"
#include "curl_http/CodeConvert.h"

namespace Module {
namespace {
const int32_t DEFAULT_NUM_ZERO = 0;
const int32_t DOUBLE_SIZE = 2;
const int32_t MAX_CERT_SIZE = 1024 * 10;
const int32_t MAX_ENCODE_BASE64_LENGTH = 10 * 1024 * 1024; // 10MB
const std::string MODULE_NAME = "CodeConvert";
}

void CodeConvert::ConvertBinary2ASCII(const std::string& in, std::string& out)
{
    size_t len = in.length();
    HCP_Logger_noid(DEBUG, MODULE_NAME) <<"Before encode size:"<<len<< HCPENDLOG;
    unsigned char* base64Array = nullptr;
    base64Array = new (std::nothrow) unsigned char [MAX_CERT_SIZE * DOUBLE_SIZE];
    if (nullptr == base64Array) {
        out = "";
        HCP_Logger_noid(ERR, MODULE_NAME) <<"Memory malloc failed."<< HCPENDLOG;
        return;
    }
    memset_s(base64Array, MAX_CERT_SIZE * DOUBLE_SIZE, DEFAULT_NUM_ZERO, MAX_CERT_SIZE * DOUBLE_SIZE);

    EVP_EncodeBlock(base64Array, (unsigned char *)const_cast<char *>(in.c_str()), (int)len);
    out =std::string((char*)base64Array);

    delete [] base64Array;
#ifdef _DEBUG
    HCP_Logger_noid(DEBUG, MODULE_NAME) <<"After encode:"<< WIPE_SENSITIVE(out) << HCPENDLOG;
#endif
}

void CodeConvert::ConvertASCII2Binary(const std::string& in, std::string& out)
{
    size_t len = in.length();
    HCP_Logger_noid(DEBUG, MODULE_NAME) <<"Before decode size:"<<len<< HCPENDLOG;
    unsigned char* base64Array = nullptr;
    base64Array = new (std::nothrow) unsigned char [MAX_CERT_SIZE * DOUBLE_SIZE];
    if (base64Array == nullptr) {
        out = "";
        HCP_Logger_noid(ERR, MODULE_NAME) <<"Memory malloc failed."<< HCPENDLOG;
        return;
    }
    memset_s(base64Array, MAX_CERT_SIZE * DOUBLE_SIZE, DEFAULT_NUM_ZERO, MAX_CERT_SIZE * DOUBLE_SIZE);

    int decodeLen = EVP_DecodeBlock(base64Array, (unsigned char *)const_cast<char *>(in.c_str()), (int)len);
    HCP_Logger_noid(DEBUG, MODULE_NAME) <<"DecodeLen:"<<decodeLen<< HCPENDLOG;

    while (in[--len] == '=') {
        decodeLen--;
    }

    out =std::string((char*)base64Array, decodeLen);
    delete [] base64Array;
#ifdef _DEBUG
    HCP_Logger_noid(DEBUG, MODULE_NAME) <<"After decode length:"<<out.length()<< HCPENDLOG;
#endif
}

bool CodeConvert::EncodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out)
{
    if (bufferSize <= 0 || bufferSize > MAX_ENCODE_BASE64_LENGTH) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "BufferSize abnormal: " << bufferSize << HCPENDLOG;
        return false;
    }
    unsigned char* base64Array = new(std::nothrow) unsigned char [bufferSize];
    if (base64Array == nullptr) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Memory malloc failed." << HCPENDLOG;
        return false;
    }

    memset_s(base64Array, bufferSize, 0, bufferSize);
    std::size_t len = in.length();
    // base6_array will be written some data
    EVP_EncodeBlock(base64Array, (unsigned char*)const_cast<char*>(in.c_str()), (int)len);
    out = std::string((char*)base64Array);

    delete [] base64Array;
    base64Array = nullptr;

    return true;
}

bool CodeConvert::DecodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out)
{
    if (bufferSize <= 0 || bufferSize > MAX_ENCODE_BASE64_LENGTH) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "BufferSize abnormal: " << bufferSize << HCPENDLOG;
        return false;
    }
    unsigned char* base64Array = new(std::nothrow) unsigned char [bufferSize];
    if (base64Array == nullptr) {
        HCP_Logger_noid(ERR, MODULE_NAME) << "Memory malloc failed." << HCPENDLOG;
        return false;
    }
    
    memset_s(base64Array, bufferSize, 0, bufferSize);

    std::size_t len = in.length();
    int decodeLen = EVP_DecodeBlock(base64Array, (unsigned char*)const_cast<char*>(in.c_str()), (int)len);
    if (decodeLen < 0) {
        delete [] base64Array;
        base64Array = nullptr;
        return false;
    }
    
    while (in[--len] == '=') {
        decodeLen--;
    }
    out = std::string((char*)base64Array, decodeLen);

    delete [] base64Array;
    base64Array = nullptr;

    return true;
}
}

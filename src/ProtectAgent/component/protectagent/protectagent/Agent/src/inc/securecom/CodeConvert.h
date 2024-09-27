/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2022. All rights reserved.
 *
 * @file CodeConvert.h
 * @brief The implemention about SSL
 * @version 1.0.0.0
 * @date 2014-6-12
 * @author lili 00254913
 */
#ifndef _CODECONVERT_H_
#define _CODECONVERT_H_

#include <string>

class CodeConvertInterface {
public:
    virtual ~CodeConvertInterface() = 0;
};
 
class CodeConvert : public CodeConvertInterface {
public:
    static bool EncodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out);
    static bool DecodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out);
};

#endif

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
#ifndef _CODECONVERT_H_
#define _CODECONVERT_H_

#include <string>

namespace Module {
class CodeConvertInterface {
public:
    virtual ~CodeConvertInterface() = 0;
};
 
class CodeConvert : public CodeConvertInterface {
public:
    static void ConvertBinary2ASCII(const std::string& in, std::string& out);
    static void ConvertASCII2Binary(const std::string& in, std::string& out);
    static bool EncodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out);
    static bool DecodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out);
};
}
#endif
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
#ifndef OBSCTX_CLOUDE_SERVICE_UTILS
#define OBSCTX_CLOUDE_SERVICE_UTILS

#include <string>
#include <ctime>
#include "securec.h"
#include "interface/CloudServiceRequest.h"

namespace Module {
    static inline char* String2CharPtr(const std::string& str) {
        if (str.empty()) {
            return nullptr;
        } else {
            return const_cast<char*>(str.c_str());
        }
    }
    static inline bool NewAndCopyCStr(char* dst, int len, const char* src) {
        dst = new char[len];
        if (strcpy_s(dst, len, src) != 0) { // strcpy_s内部会校验三个入参是否合法
            delete[] dst;
            return false;
        }
        return true;
    }
    static inline void DelCStr(const char** ppStr) {
        if (*ppStr != nullptr) {
            delete[] *ppStr;
            *ppStr = nullptr;
        }
    }
    static inline std::string ConvertCStr2Str(const char* ptr) {
        if (ptr == nullptr) {
            return "";
        } else {
            return ptr;
        }
    }

    static inline bool AscendingSort(const UploadInfo& a, const UploadInfo& b)
    {
        return (a.partNumber < b.partNumber);
    }

    // 对象名中的特殊字符会被转换为以%前缀的ascii码，比如'/'会被转换为"%2F"，所以读取的时候需要还原
    static inline bool ConvertSpecailChar2Normal(const std::string& input, std::string& output)
    {
        static const std::string specailCharPrefix = "%";
        static const int specailCharLen = 2;
        static const int hexBase = 16;
        static const int one = 1;

        size_t findStartPos = 0;
        size_t findNextPos = input.size();
        while (true) {
            findNextPos = input.find(specailCharPrefix, findStartPos);
            if (findNextPos == std::string::npos) {
                output += input.substr(findStartPos, input.size() - findStartPos);
                break;
            }
            if (findNextPos + specailCharLen >= input.size()) {
                return false;
            }
            const std::string hexNum = input.substr(findNextPos + one, specailCharLen);
            char specailChar = stoi(hexNum, nullptr, hexBase);
            output += input.substr(findStartPos, findNextPos - findStartPos) + specailChar;

            findStartPos = findNextPos + specailCharLen + one;
        }

        return true;
    }

    static inline time_t GmtToTimestamp(const std::string gmtTime)
    {
        struct tm timeinfo = {0};
        strptime(gmtTime.c_str(), "%a, %d %b %Y %H:%M:%S", &timeinfo);
        time_t convertedTime = mktime(&timeinfo);

        return convertedTime;
    }

    static inline time_t UtcToTimestamp(const std::string utcTime)
    {
        struct tm timeinfo  = {0};
        strptime(utcTime.c_str(), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
        time_t convertedTime = mktime(&timeinfo);

        return convertedTime;
    }

    static inline bool IsEndsWith(const std::string& str, const std::string& suffix)
    {
        return (str.length() >= suffix.length()) && (str.substr(str.length() - suffix.length()) == suffix);
    }
}

#endif  // OBSCTX_CLOUDE_SERVICE_UTILS
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
#ifndef MOUDLE_DATAMOVER_UTILITY_H
#define MOUDLE_DATAMOVER_UTILITY_H

#include "define/Defines.h"
#include "define/Types.h"
#include <string>
#include <set>

namespace Module {
    namespace {
        const std::string LEFT_ANGLE_BRACKET = "&lt;";
        const std::string RIGHT_ANGLE_BRACKET = "&rt;";
        const std::string LEFT_CICLE_BRACKET = "&#40;";
        const std::string RIGHT_CICLE_BRACKET = "&#41;";
        const std::string AND_SYMBOL = "&amp;";
        constexpr int HEX = 16;
    }
    // url处理工具
    class DataMoverUtility {
    public:
        static std::string UrlEncode(const std::string& strToEncode)
        {
            std::string src = strToEncode;
            char hex[] = "0123456789ABCDEF";
            std::string strEncoded;
            for (size_t i = 0; i < src.size(); ++i) {
                unsigned char cc = src[i];
                if ((cc >= 'A' && cc <= 'Z') || (cc >='a' && cc <= 'z') || (cc >='0' && cc <= '9')
                    || cc == '.' || cc == '_' || cc == '-' || cc == '~' || cc == '!') {
                    strEncoded += cc;
                } else {
                    strEncoded += '%';
                    strEncoded += hex[cc / HEX];
                    strEncoded += hex[cc % HEX];
                }
            }
            return strEncoded;
        }
        static std::string OceanStorSeriesEscapeChar(const std::string& strToEscape)
        {
            std::string strEscaped = strToEscape;
            boost::replace_all(strEscaped, "&", AND_SYMBOL);
            boost::replace_all(strEscaped, "<", LEFT_ANGLE_BRACKET);
            boost::replace_all(strEscaped, ">", RIGHT_ANGLE_BRACKET);
            boost::replace_all(strEscaped, "(", LEFT_CICLE_BRACKET);
            boost::replace_all(strEscaped, ")", RIGHT_CICLE_BRACKET);
            return strEscaped;
        }
    };

} // namespace Module

#endif // MOUDLE_DATAMOVER_UTILITY_H
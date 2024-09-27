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
#ifndef MODULE_STRING_H
#define MODULE_STRING_H

#include <stdarg.h>
#include <stdlib.h>
#ifndef WIN32
#include <strings.h>
#include <termios.h>
#endif
#include <list>
#include <vector>
#include "define/Types.h"
#include "define/Defines.h"

namespace Module {

class AGENT_API CMpString {
public:
    static int StringtoPositiveInt32(const std::string & str, int & num);
    static char* Trim(char str[]);
    static char* TrimLeft(char str[]);
    static char* TrimRight(char str[]);
    static std::string Trim(std::string str);
    static std::string TrimLeft(std::string str);
    static std::string TrimRight(std::string str);
    static char* ToUpper(char str[]);
    static char* ToLower(char str[]);
    static std::string ToUpper(std::string& str);
    static std::string ToLower(std::string& str);
    static char* TotallyTrimRight(char str[]);
    static std::string TotallyTrimRight(std::string str);
    static std::string TotallyTrim(const std::string& str);
    static void FormatLUNID(std::string& rstrINLUNID, std::string& rstrOUTLUNID);
    static bool HasSpace(const char str[]);
    static void StringToUInteger(std::string strVal, uint64_t& integerVal);
    static void UIntegerToString(uint64_t IntegerVal, std::string& strVal);
    static void StrToken(std::string strToken, std::string strSeparator, std::list<std::string>& plstStr);
    // 根据指定分割符分割字符串，并将结果存放vector中
    static void StrSplit(std::vector<std::string>& vecTokens, const std::string& strText, char cSep);
    // 根据指定分割符分割字符串，并将结果存放vector中(支持连续分隔符)
    static void StrSplitEx(std::vector<std::string>& tokens, const std::string& str, const std::string& delimiters);
    // 字符串中包含空格，前后添加引号，解决路径中包含空格问题
    static std::string StrReplace(const std::string& str, const std::string& to_replaced, const std::string& newchars);
    static std::string BlankComma(const std::string& strPath);
    static std::string StrJoin(const std::vector<std::string>& vecStr, std::string cSep);
    static std::string to_string(const uint32_t& uintVal);
    static std::string to_string(const long& longVal);
    static std::string to_string(const unsigned long& ulongVal);
    static std::string to_string(const int& intVal);
    static std::string to_string(const unsigned long long& ulonglongVal);
    static bool FormattingPath(std::string& strPath);

#ifdef WIN32
    static std::string UnicodeToANSI(const std::wstring& wStrSrc);
    static std::wstring ANSIToUnicode(const std::string& strSrc);
    static std::string WString2String(const std::wstring src);
    static std::wstring String2WString(const std::string src);
    static std::string ANSIToUTF8(const std::string& strSrc);
    static std::string UTF8ToANSI(const std::string& strSrc);
#else
    static int GetCh();
#endif
};

} // namespace Module

#endif  // MP_STRING_H

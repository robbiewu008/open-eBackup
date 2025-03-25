#ifndef AGENT_STRING_H
#define AGENT_STRING_H

#include <stdarg.h>
#include <stdlib.h>
#ifndef WIN32
#include <strings.h>
#include <termios.h>
#endif
#include <list>
#include <vector>
#include "common/Types.h"
#include "common/Defines.h"

class AGENT_API CMpString {
public:
    static mp_char* Trim(mp_char str[]);
    static mp_char* TrimLeft(mp_char str[]);
    static mp_char* TrimRight(mp_char str[]);
    static mp_string Trim(const mp_string& str);
    static mp_string TrimLeft(const mp_string& str);
    static mp_string TrimRight(const mp_string& str);
    static mp_char* ToUpper(mp_char str[]);
    static mp_char* ToLower(mp_char str[]);
    static mp_string ToUpper(mp_string& str);
    static mp_string ToLower(mp_string& str);
    static mp_char* TotallyTrimRight(mp_char str[]);
    static mp_string TotallyTrimRight(const mp_string& str);
    static mp_string TotallyTrim(const mp_string& str);
    static mp_void FormatLUNID(mp_string& rstrINLUNID, mp_string& rstrOUTLUNID);
    static mp_bool HasSpace(const mp_char str[]);
    static mp_void StringToUInteger(const mp_string& strVal, mp_uint64& integerVal);
    static mp_void UIntegerToString(mp_uint64 IntegerVal, mp_string& strVal);
    static mp_void StrToken(const mp_string& strToken, const mp_string& strSeparator, std::list<mp_string>& plstStr);
    // 根据指定分割符分割字符串，并将结果存放vector中
    static mp_void StrSplit(std::vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep);
    // 按照第一个分隔符来分割字符串，将分隔符左侧解析为key, 右侧解析为value
    static mp_void StrSplitToKeyVal(const mp_string& strText, mp_string& key, mp_string& value,
        const mp_string& delimiters);
    // 根据指定分割符分割字符串，并将结果存放vector中(支持连续分隔符)
    static void StrSplitEx(std::vector<mp_string>& tokens, const mp_string& str, const mp_string& delimiters);
    // 字符串中包含空格，前后添加引号，解决路径中包含空格问题
    static mp_string StrReplace(const mp_string& str, const mp_string& to_replaced, const mp_string& newchars);
    static mp_string BlankComma(const mp_string& strPath);
    static mp_string StrJoin(const std::vector<mp_string>& vecStr, const mp_string& cSep);
    static mp_string to_string(const mp_uint32& uintVal);
    static mp_string to_string(const mp_long& longVal);
    static mp_string to_string(const mp_ulong& ulongVal);
    static mp_string to_string(const mp_int32& intVal);
    static mp_string to_string(const mp_uint64& intVal);
    static mp_bool FormattingPath(mp_string& strPath);
    static mp_bool EndsWith(const mp_string& str, const mp_string& subStr);
    static mp_string RealPath(const mp_string& strPath);
    static mp_string Base64Decode(const mp_string &base64Str);
    static mp_bool IsDigits(const std::string &str);
    static int64_t SafeStoll(const std::string& str, int64_t defaultValue = 0);
    static int32_t SafeStoi(const std::string& str, int32_t defaultValue = 0);
    static double SafeStod(const std::string& str, double defaultValue = 0);
#ifdef WIN32
    static mp_string UnicodeToANSI(const mp_wstring& wStrSrc);
    static mp_wstring ANSIToUnicode(const mp_string& strSrc);
    static mp_string WString2String(const mp_wstring src);
    static mp_wstring String2WString(const mp_string src);
    static mp_string ANSIToUTF8(const mp_string& strSrc);
    static mp_string UTF8ToANSI(const mp_string& strSrc);
#else
    static mp_int32 GetCh();
#endif
};

#endif  // __AGENT_STRING_H__

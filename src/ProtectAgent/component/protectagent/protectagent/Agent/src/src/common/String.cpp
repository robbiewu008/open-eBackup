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
#include "common/MpString.h"

#include <sstream>
#ifdef WIN32
#include <Shlwapi.h>
#endif
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Types.h"
#include "securec.h"
#ifdef LINUX
#include <unistd.h>
#endif

using namespace std;

/*------------------------------------------------------------
Description  : 将无符号整型转换为string类型
Input        : IntegerVal -- 待转换整型变量 strVal--- 转换后的字符串
Output       : 无
Return       : 无
Create By    : z00455045
Modification : 2019/11/01
-------------------------------------------------------------*/
mp_void CMpString::UIntegerToString(mp_uint64 IntegerVal, mp_string& strVal)
{
    std::stringstream ss;
    ss << IntegerVal;
    ss >> strVal;
}

/*------------------------------------------------------------
Description  : 将string类型整数转换为无符号整型变量
Input        : strVal -- 待转换后的字符串          IntegerVal -- 转换后的整型
Output       : 无
Return       : 无
Create By    : z00455045
Modification : 2019/11/01
-------------------------------------------------------------*/
mp_void CMpString::StringToUInteger(const mp_string& strVal, mp_uint64& integerVal)
{
    std::stringstream ss;
    ss << strVal;
    ss >> integerVal;
}

/*------------------------------------------------------------
Description  : 提取字符串
Input        :  strToken--- 字符串，strSeparator---分割符
Output       : 无
Return       : 无
Create By    : 无
Modification : 无
-------------------------------------------------------------*/
mp_void CMpString::StrToken(const mp_string& strToken, const mp_string& strSeparator, list<mp_string>& plstStr)
{
    mp_char* pszTmpToken;
    mp_char* pNextToken;

    pNextToken = NULL;
    pszTmpToken = strtok_s(const_cast<mp_char*>(strToken.c_str()), const_cast<mp_char*>(strSeparator.c_str()),
        &pNextToken);
    while (pszTmpToken != NULL) {
        plstStr.push_back(mp_string(pszTmpToken));
        pszTmpToken = strtok_s(NULL, const_cast<mp_char*>(strSeparator.c_str()), &pNextToken);
    }
}

/*------------------------------------------------------------
Description  :判断空格
Input        :  str--- 字符串
Output       :  无
Return       :  MP_TRUE---有空格
                MP_FALSE---没有空格
Create By    :  无
Modification :  无
-------------------------------------------------------------*/
mp_bool CMpString::HasSpace(const mp_char str[])
{
    if (str == NULL) {
        return MP_FALSE;
    }

    for (; *str != '\0'; ++str) {
        if (*str == ' ') {
            return MP_TRUE;
        }
    }

    return MP_FALSE;
}

/* ------------------------------------------------------------
Description  :整理字符串
Input        :  str--- 字符串
Return       :  返回字符串
                   NULL--- 字符串为空
------------------------------------------------------------- */
mp_char* CMpString::Trim(mp_char str[])
{
    return TrimRight(TrimLeft(str));
}

/* ------------------------------------------------------------
Description  :消除从左侧起所遇到的所有空格字符
Input        :  str--- 字符串
Return       :  返回整理后字符串
                   NULL--- 字符串为空
------------------------------------------------------------- */
mp_char* CMpString::TrimLeft(mp_char str[])
{
    if (str == NULL) {
        return NULL;
    }

    mp_char* ptr = str;
    while (*ptr == ' ') {
        ++ptr;
    }

    if (ptr == str) {
        return ptr;
    }

    mp_char* pret = str;
    while (*ptr) {
        *str = *ptr;
        ++str;
        ++ptr;
    }

    *str = 0;
    return pret;
}
/* ------------------------------------------------------------
Description  :消除从右侧起所遇到的所有空格字符
Input        :  str--- 字符串
Return       :  返回整理后字符串
                   NULL--- 字符串为空
------------------------------------------------------------- */
mp_char* CMpString::TrimRight(mp_char str[])
{
    if (str == NULL) {
        return NULL;
    }

    mp_char* p1 = str;
    mp_char* p2 = str;

    do {
        while (*p1 != ' ' && *p1 != '\0') {
            p1++;
        }
        p2 = p1;

        while (*p1 == ' ') {
            p1++;
        }
    } while (*p1 != '\0');
    *p2 = '\0';

    return str;
}

mp_string CMpString::Trim(const mp_string& str)
{
    return TrimRight(TrimLeft(str));
}

/* ------------------------------------------------------------
Description  :消除从左侧起所遇到的所有空格字符
Input        :  str--- 字符串
Return       :  返回整理后字符串
                   NULL--- 字符串为空
------------------------------------------------------------- */
mp_string CMpString::TrimLeft(const mp_string& str)
{
    if (str.empty()) {
        return "";
    }

    std::size_t idx = 0;
    while (idx < str.length() && str[idx] == ' ') {
        ++idx;
    }

    return str.substr(idx);
}
/* ------------------------------------------------------------
Description  :消除从右侧起所遇到的所有空格字符
Input        :  str--- 字符串
Return       :  返回整理后字符串
                   NULL--- 字符串为空
------------------------------------------------------------- */
mp_string CMpString::TrimRight(const mp_string& str)
{
    if (str.empty()) {
        return "";
    }

    std::size_t idx = str.length() - 1;
    while (idx >= 0 && str[idx] == ' ') {
        --idx;
    }

    return str.substr(0, idx + 1);
}

/* ------------------------------------------------------------
Description  :消除 所遇到的所有空格\t \n \r
Input        :  str--- 字符串
Return       :  返回整理后字符串
                   NULL--- 字符串为空
------------------------------------------------------------- */
mp_char* CMpString::TotallyTrimRight(mp_char str[])
{
    if (str == NULL) {
        return NULL;
    }

    mp_int32 iWhile = (mp_int32)strlen(str);
    if (iWhile == 0) {
        return NULL;
    }
    while ((iWhile > 0) && ((str[iWhile - 1] == ' ') || (str[iWhile - 1] == '\t') || (str[iWhile - 1] == '\n') ||
           (str[iWhile - 1]) == '\r')) {
        iWhile--;
    }

    str[iWhile] = 0;
    return str;
}

mp_string CMpString::TotallyTrimRight(const mp_string& str)
{
    if (str.empty()) {
        return "";
    }

    std::size_t idx = str.length() - 1;
    while (idx >= 0 && ((str[idx] == ' ') || (str[idx] == '\t') || (str[idx] == '\n') || (str[idx] == '\r'))) {
        --idx;
    }

    return str.substr(0, idx + 1);
}

/* ------------------------------------------------------------
Description  :消除 所遇到的所有空格\t \n \r
Input        :  str--- 字符串
Return       :  返回整理后字符串
------------------------------------------------------------- */
mp_string CMpString::TotallyTrim(const mp_string& str)
{
    mp_string strRet;
    mp_size nSize = str.size();
    for (mp_size i = 0; i < nSize; ++i) {
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r') {
            strRet += str[i];
        }
    }
    return strRet;
}

// 将查询出的LUN ID前面多余的0去掉
mp_void CMpString::FormatLUNID(mp_string& rstrINLUNID, mp_string& rstrOUTLUNID)
{
    mp_string strNumerics("123456789");
    mp_string::size_type strIndex;

    strIndex = rstrINLUNID.find_first_of(strNumerics);
    if (mp_string::npos != strIndex) {
        rstrOUTLUNID = rstrINLUNID.substr(strIndex);
    } else {
        rstrOUTLUNID = "0";
    }
}

/* ------------------------------------------------------------
Description  :转换为大写
Input        :  str--- 字符串
Return       :   返回转换后的字符串
------------------------------------------------------------- */
mp_char* CMpString::ToUpper(mp_char str[])
{
    mp_char* ptr = str;
    while (*ptr) {
        *ptr = (mp_char)toupper(*ptr);
        ++ptr;
    }

    return str;
}
/* ------------------------------------------------------------
Description  :转换为小写
Input        :  str--- 字符串
Return       :   返回转换后的字符串
------------------------------------------------------------- */
mp_char* CMpString::ToLower(mp_char str[])
{
    mp_char* ptr = str;
    while (*ptr) {
        *ptr = (mp_char)tolower(*ptr);
        ++ptr;
    }

    return str;
}

/* ------------------------------------------------------------
Description  :转换为大写
Input        :  str--- 字符串
Return       :   返回转换后的字符串
------------------------------------------------------------- */
mp_string CMpString::ToUpper(mp_string& str)
{
    std::size_t idx = 0;
    while (idx < str.length()) {
        str[idx] = (mp_char)toupper(str[idx]);
        ++idx;
    }

    return str;
}
/* ------------------------------------------------------------
Description  :转换为小写
Input        :  str--- 字符串
Return       :   返回转换后的字符串
------------------------------------------------------------- */
mp_string CMpString::ToLower(mp_string& str)
{
    std::size_t idx = 0;
    while (idx < str.length()) {
        str[idx] = (mp_char)tolower(str[idx]);
        ++idx;
    }

    return str;
}

/* ------------------------------------------------------------
Description  : 分割字符串
------------------------------------------------------------- */
mp_void CMpString::StrSplit(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep)
{
    mp_string::size_type start = 0;
    mp_string::size_type end;
    while ((end = strText.find(cSep, start)) != mp_string::npos) {
        vecTokens.push_back(strText.substr(start, end - start));
        start = end + 1;
    }

    vecTokens.push_back(strText.substr(start));
}

/* ------------------------------------------------------------
Description  : 分割字符串为key value形式
------------------------------------------------------------- */
mp_void CMpString::StrSplitToKeyVal(const mp_string& strText, mp_string& key, mp_string& value,
    const mp_string& delimiters)
{
    mp_string::size_type mid = strText.find(delimiters, 0);
    if (mid == mp_string::npos || mid == 0) {
        return;
    }

    mp_string::size_type startKey = 0;
    key = strText.substr(startKey, mid - startKey);

    mp_string::size_type startValue = mid + 1;
    if (startValue < strText.size()) {
        value = strText.substr(startValue);
    }
}

/* ------------------------------------------------------------
Description  : 分割字符串(支持连续分割符)
------------------------------------------------------------- */
mp_void CMpString::StrSplitEx(vector<mp_string>& tokens, const mp_string& str, const mp_string& delimiters)
{
    mp_string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    mp_string::size_type pos = str.find_first_of(delimiters, lastPos);
    while (mp_string::npos != pos || mp_string::npos != lastPos) {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}

/* ------------------------------------------------------------
Description  : 连接字符串
------------------------------------------------------------- */
mp_string CMpString::StrJoin(const std::vector<mp_string>& vecStr, const mp_string& cSep)
{
    mp_string ret;
    for (mp_size i = 0; i < vecStr.size(); i++) {
        ret += vecStr.at(i);
        if (i != vecStr.size() - 1) {
            ret += cSep;
        }
    }
    return ret;
}

/* ------------------------------------------------------------
Description  : replace all
------------------------------------------------------------- */
mp_string CMpString::StrReplace(const mp_string& str, const mp_string& to_replaced, const mp_string& newchars)
{
    mp_string strRet = str;
    for (mp_string::size_type pos = 0; pos != mp_string::npos; pos += newchars.length()) {
        pos = strRet.find(to_replaced, pos);
        if (pos != mp_string::npos) {
            strRet.replace(pos, to_replaced.length(), newchars);
        } else {
            break;
        }
    }
    return strRet;
}

/* ------------------------------------------------------------
Function Name: BlankComma
Description  : 对于有空格的字符串，前后加引号，用于路径处理，解决路径包含空格问题
Others       :-------------------------------------------------------- */
mp_string CMpString::BlankComma(const mp_string& strPath)
{
    if (strPath.find(" ") != mp_string::npos && strPath[0] != '"') {
        return "\"" + strPath + "\"";
    } else {
        return strPath;
    }
}

#ifdef WIN32
/*------------------------------------------------------------
Description  : unicode转换为ansi
Return       :    返回ansi值
-------------------------------------------------------------*/
mp_string CMpString::UnicodeToANSI(const mp_wstring& wStrSrc)
{
    mp_string strText("");
    mp_char* pElementText = NULL;
    mp_int32 iRet;
    mp_int32 iTextLen;

    // wide char to multi char
    iTextLen = WideCharToMultiByte(CP_ACP, 0, wStrSrc.c_str(), -1, NULL, 0, NULL, NULL);

    NEW_ARRAY_CATCH(pElementText, mp_char, iTextLen + 1);
    if (pElementText == NULL) {
        return strText;
    }

    iRet = memset_s(reinterpret_cast<mp_void*>(pElementText), sizeof(mp_char) * (iTextLen + 1),
                    0, sizeof(mp_char) * (iTextLen + 1));
    if (iRet != EOK) {
        delete[] pElementText;
        return strText;
    }

    ::WideCharToMultiByte(CP_ACP, 0, wStrSrc.c_str(), -1, pElementText, iTextLen, NULL, NULL);

    strText = pElementText;
    delete[] pElementText;
    return strText;
}
/*------------------------------------------------------------
Description  : ansi转换为unicode
Return       :    返回unicode值
-------------------------------------------------------------*/
mp_wstring CMpString::ANSIToUnicode(const mp_string& strSrc)
{
    mp_wstring wStrTmp;
    mp_int32 dwNum;
    mp_wchar* pElementText = NULL;
    mp_int32 iRet;

    dwNum = MultiByteToWideChar(CP_ACP, 0, strSrc.c_str(), -1, NULL, 0);
    if (dwNum <= 0) {
        return wStrTmp;
    }

    NEW_ARRAY_CATCH(pElementText, mp_wchar, dwNum + 1)
    if (pElementText == NULL) {
        return wStrTmp;
    }

    iRet = memset_s(reinterpret_cast<mp_void*>(pElementText), sizeof(mp_wchar) * (dwNum + 1),
                    0, sizeof(mp_wchar) * (dwNum + 1));
    if (iRet != EOK) {
        delete[] pElementText;
        return wStrTmp;
    }

    dwNum = MultiByteToWideChar(CP_ACP, 0, strSrc.c_str(), -1, pElementText, dwNum);
    if (dwNum == 0) {
        delete[] pElementText;
        return wStrTmp;
    }

    wStrTmp = pElementText;
    delete[] pElementText;
    return wStrTmp;
}
/*------------------------------------------------------------
Description  : WideChar to String
-------------------------------------------------------------*/
mp_string CMpString::WString2String(const mp_wstring src)
{
    vector<CHAR> chBuffer;

    mp_int32 iChars = WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, NULL, 0, NULL, NULL);
    if (iChars <= 0) {
        return "";
    }

    chBuffer.resize(iChars);
    iChars = WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, &chBuffer.front(), (mp_int32)chBuffer.size(), NULL, NULL);
    if (iChars <= 0) {
        return "";
    }

    return std::string(&chBuffer.front());
}

/*------------------------------------------------------------
Description  : String to WideChar
-------------------------------------------------------------*/
mp_wstring CMpString::String2WString(const mp_string src)
{
    wstring wStrTmp;
    mp_int32 dwNum;
    wchar_t* pElementText = NULL;
    mp_int32 iRet;
    dwNum = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, NULL, 0);
    if (dwNum <= 0) {
        return wStrTmp;
    }

    NEW_ARRAY_CATCH(pElementText, wchar_t, dwNum + 1);
    if (pElementText == NULL) {
        return wStrTmp;
    }
    iRet = memset_s(reinterpret_cast<mp_void*>(pElementText), sizeof(wchar_t) * (dwNum + 1),
                    0, sizeof(wchar_t) * (dwNum + 1));
    if (iRet != EOK) {
        delete[] pElementText;
        return wStrTmp;
    }

    dwNum = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, pElementText, dwNum);
    if (dwNum == 0) {
        delete[] pElementText;
        return wStrTmp;
    }

    wStrTmp = pElementText;
    delete[] pElementText;

    return wStrTmp;
}

/*------------------------------------------------------------
Description  : ansi to utf-8
-------------------------------------------------------------*/
mp_string CMpString::ANSIToUTF8(const mp_string& strSrc)
{
    mp_string strUTF8;
    mp_wstring wstrSrc = CMpString::String2WString(strSrc);
    mp_int32 u8Len = WideCharToMultiByte(CP_UTF8, NULL, wstrSrc.c_str(), -1, NULL, 0, NULL, NULL);
    if (u8Len <= 0) {
        COMMLOG(OS_LOG_ERROR, "u8Len is litter than 0, u8Len is %d.", u8Len);
        return strUTF8;
    }

    mp_char* cUTF8 = NULL;
    NEW_ARRAY_CATCH(cUTF8, mp_char, u8Len + 1);
    if (cUTF8 == NULL) {
        return strUTF8;
    }
    mp_int32 iRet = memset_s(cUTF8, u8Len + 1, 0, u8Len + 1);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] cUTF8;
        return strUTF8;
    }

    u8Len = WideCharToMultiByte(CP_UTF8, NULL, wstrSrc.c_str(), -1, cUTF8, u8Len, NULL, NULL);
    if (u8Len <= 0) {
        COMMLOG(OS_LOG_ERROR, "u8Len is litter than 0, u8Len is %d.", u8Len);
        delete[] cUTF8;
        return strUTF8;
    }

    cUTF8[u8Len] = '\0';
    strUTF8 = cUTF8;
    delete[] cUTF8;
    return strUTF8;
}

/*------------------------------------------------------------
Description  : utf-8 to ansi
-------------------------------------------------------------*/
mp_string CMpString::UTF8ToANSI(const mp_string& strSrc)
{
    mp_string strAnsi;
    mp_int32 ansiLen = MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, NULL, 0);
    if (ansiLen <= 0) {
        COMMLOG(OS_LOG_ERROR, "ansiLen is litter than 0, ansiLen is %d.", ansiLen);
        return strAnsi;
    }

    mp_wchar* wAnsi = NULL;
    NEW_ARRAY_CATCH(wAnsi, mp_wchar, ansiLen + 1);
    mp_int32 iRet = memset_s(wAnsi, sizeof(wchar_t) * (ansiLen + 1), 0, sizeof(wchar_t) * (ansiLen + 1));
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] wAnsi;
        return strAnsi;
    }

    ansiLen = MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, wAnsi, ansiLen);
    if (ansiLen <= 0) {
        COMMLOG(OS_LOG_ERROR, "ansiLen is litter than 0, ansiLen is %d.", ansiLen);
        delete[] wAnsi;
        return strAnsi;
    }
    mp_string resultStr = CMpString::WString2String(wAnsi);
    delete[] wAnsi;
    return resultStr;
}

#else
/* ------------------------------------------------------------
Description  : 获取字符
Return       :    返回字符
------------------------------------------------------------- */
mp_int32 CMpString::GetCh()
{
    struct termios oldt;
    struct termios newt;
    tcgetattr(STDIN_FILENO, &oldt);  // 得到原来的终端属性
    newt = oldt;
    newt.c_lflag &= ~ICANON;                  // 设置非正规模式，如果程序每次要从终端读取一个字符的话，这是必须的
    newt.c_lflag &= ~ECHO;                    // 关闭回显
    newt.c_cc[VMIN] = 1;                      // 设置非正规模式下的最小字符数
    newt.c_cc[VTIME] = 0;                     // 设置非正规模式下的读延时
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // 设置新的终端属性
    mp_int32 iChar = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // 恢复旧属性
    return iChar;
}

#endif

mp_string CMpString::to_string(const mp_int32& intVal)
{
    mp_string outStr;
    std::stringstream ss;
    ss << intVal;
    ss >> outStr;
    return outStr;
}
mp_string CMpString::to_string(const mp_uint32& uintVal)
{
        mp_string outStr;
    std::stringstream ss;
    ss << uintVal;
    ss >> outStr;
    return outStr;
}
mp_string CMpString::to_string(const mp_long& longVal)
{
    mp_string outStr;
    std::stringstream ss;
    ss << longVal;
    ss >> outStr;
    return outStr;
}
mp_string CMpString::to_string(const mp_ulong& ulongVal)
{
    mp_string outStr;
    std::stringstream ss;
    ss << ulongVal;
    ss >> outStr;
    return outStr;
}
mp_string CMpString::to_string(const mp_uint64& ulongVal)
{
    mp_string outStr;
    std::stringstream ss;
    ss << ulongVal;
    ss >> outStr;
    return outStr;
}

mp_bool CMpString::FormattingPath(mp_string& strPath)
{
#ifdef WIN32
    char path[MAX_FULL_PATH_LEN + 1] = { 0x00 };
    if (strPath.length() > MAX_FULL_PATH_LEN || PathCanonicalize(path, strPath.c_str()) == MP_FALSE) {
        COMMLOG(OS_LOG_ERROR, "FormattingPath %s failed.", strPath.c_str());
        return MP_FAILED;
    }
#else
    mp_size nPos = strPath.find(DOUBLE_SLASH);
    while (nPos != mp_string::npos) {
        strPath = strPath.replace(nPos, DOUBLE_SLASH.size(), PATH_SEPARATOR);
        nPos = strPath.find(DOUBLE_SLASH);
    }

    // rerase ..
    mp_size pos1 = strPath.find(PARENT_PATH_SEPARATOR);
    while (pos1 != mp_string::npos) {
        mp_string pre = strPath.substr(0, pos1);
        mp_size pos2 = pre.find_last_of(PATH_SEPARATOR);
        if (pos2 == mp_string::npos) {
            break;
        }
        mp_string mid = pre.substr(pos2);
        strPath.erase(pos1, PARENT_PATH_SEPARATOR.size());
        strPath.erase(pos2, mid.size());

        pos1 = strPath.find(PARENT_PATH_SEPARATOR);
    }

    char path[PATH_MAX + 1] = { 0x00 };
    if (strPath.length() > PATH_MAX || NULL == realpath(strPath.c_str(), path)) {
        COMMLOG(OS_LOG_ERROR, "FormattingPath %s failed, errno[%d]:%s.", strPath.c_str(), errno, strerror(errno));
        return MP_FAILED;
    }
#endif
    strPath = path;
    return MP_TRUE;
}

mp_string CMpString::RealPath(const mp_string& strPath)
{
    mp_string realPath = strPath;
    mp_size nPos = realPath.find("/./");
    int delimiterlength = 2;
    while (nPos != mp_string::npos) {
        realPath.erase(nPos, delimiterlength);
        nPos = realPath.find("/./");
    }
    return realPath;
}

mp_bool CMpString::EndsWith(const mp_string& str, const mp_string& subStr)
{
    if (!str.empty() && subStr.empty()) {
        return MP_FALSE;
    }

    if (str.length() < subStr.length()) {
        COMMLOG(OS_LOG_ERROR, "str length is longger than  subStr length.");
        return MP_FALSE;
    }

    std::size_t idxEnd = str.rfind(subStr);
    if (idxEnd == std::string::npos) {
        COMMLOG(OS_LOG_ERROR, "String(%s) is not end with %s.", str.c_str(), subStr.c_str());
        return MP_FALSE;
    }

    return (idxEnd == (str.length() - subStr.length()));
}

mp_string CMpString::Base64Decode(const std::string &base64Str)
{
    std::string out;
    std::vector<int> T(BASE64_NUM256, -1);
 
    for (int i = 0; i < BASE64_NUM64; ++i) {
        T[BASE64_STR[i]] = i;
    }
    int val = 0;
    int valb = -8;
    for (unsigned char c : base64Str) {
        if (T[c] == -1) {
            break;
        }
        val = (val << BASE64_NUM6) + T[c];
        valb += BASE64_NUM6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= BASE64_NUM8;
        }
    }
    return out;
}

mp_bool CMpString::IsDigits(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit);
}

int64_t CMpString::SafeStoll(const std::string& str, int64_t defaultValue)
{
    try {
        int64_t res = std::stoll(str);
        return res;
    } catch (const std::exception& erro) {
        ERRLOG("Invalid number , erro: %s.", erro.what());
        return defaultValue;
    }
}

int32_t CMpString::SafeStoi(const std::string& str, int32_t defaultValue)
{
    try {
        int32_t res = std::stoi(str);
        return res;
    } catch (const std::exception& erro) {
        ERRLOG("Invalid number , erro: %s.", erro.what());
        return defaultValue;
    }
}

double CMpString::SafeStod(const std::string& str, double defaultValue)
{
    try {
        double res = std::stod(str);
        return res;
    } catch (const std::exception& erro) {
        ERRLOG("Invalid number , erro: %s.", erro.what());
        return defaultValue;
    }
}
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
#include "MpString.h"

#include <sstream>
#include <cstring>
#include "securec.h"

#include "log/Log.h"
#include "common/Utils.h"
#include "define/Types.h"
#ifdef LINUX
#include <unistd.h>
#endif

#ifdef WIN32
#include <shlwapi.h>
#endif

using namespace std;

namespace Module {

int CMpString::StringtoPositiveInt32(const string & str, int & num)
{     
    int tmp = atoi(str.c_str());
    if (tmp < 0) {  
        return FAILED;
    }

    const int MAX_INT_LEN = 10;
    char buf[MAX_INT_LEN + 2] = {0};
    (void)snprintf_s(buf, MAX_INT_LEN + 2,MAX_INT_LEN + 1, "%d", tmp);
    if (strcmp(str.c_str(), buf) != 0) {
        return FAILED;
    }

    num = tmp;
    return SUCCESS;
}

void CMpString::UIntegerToString(uint64_t IntegerVal, string& strVal)
{
    stringstream ss;
    ss << IntegerVal;
    ss >> strVal;
}

void CMpString::StringToUInteger(string strVal, uint64_t& integerVal)
{
    stringstream ss;
    ss << strVal;
    ss >> integerVal;
}

void CMpString::StrToken(string strToken, string strSeparator, list<string>& plstStr)
{
    char* pszTmpToken;
    char* pNextToken;

    pNextToken = NULL;
    pszTmpToken = strtok_s(const_cast<char*>(strToken.c_str()), const_cast<char*>(strSeparator.c_str()),
        &pNextToken);
    while (pszTmpToken != NULL) {
        plstStr.push_back(string(pszTmpToken));
        pszTmpToken = strtok_s(NULL, const_cast<char*>(strSeparator.c_str()), &pNextToken);
    }
}

bool CMpString::HasSpace(const char str[])
{
    if (str == NULL) {
        return false;
    }

    for (; *str != '\0'; ++str) {
        if (*str == ' ') {
            return true;
        }
    }

    return false;
}

char* CMpString::Trim(char str[])
{
    if (str == NULL) {
        return NULL;
    }

    return TrimRight(TrimLeft(str));
}

char* CMpString::TrimLeft(char str[])
{
    if (str == NULL) {
        return NULL;
    }

    char* ptr = str;
    char* pret = str;
    while (*ptr == ' ') {
        ++ptr;
    }

    if (ptr == str) {
        return ptr;
    }

    while (*ptr) {
        *str = *ptr;
        ++str;
        ++ptr;
    }

    *str = 0;
    return pret;
}

char* CMpString::TrimRight(char str[])
{
    if (str == NULL) {
        return NULL;
    }

    char* p1 = str;
    char* p2 = str;

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

string CMpString::Trim(string str)
{
    return TrimRight(TrimLeft(str));
}

string CMpString::TrimLeft(string str)
{
    if (str.empty()) {
        return "";
    }

    size_t idx = 0;
    while (idx < str.length() && str[idx] == ' ') {
        ++idx;
    }

    return str.substr(idx);
}

string CMpString::TrimRight(string str)
{
    if (str.empty()) {
        return "";
    }

    size_t idx = str.length() - 1;
    /*lint -e685 -e568*/
    while (idx >= 0 && str[idx] == ' ') {
        --idx;
    }

    return str.substr(0, idx + 1);
}

char* CMpString::TotallyTrimRight(char str[])
{
    if (str == NULL) {
        return NULL;
    }

    int iWhile = (int)strlen(str);
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

string CMpString::TotallyTrimRight(string str)
{
    if (str.empty()) {
        return "";
    }

    size_t idx = str.length() - 1;
    while (idx >= 0 && ((str[idx] == ' ') || (str[idx] == '\t') || (str[idx] == '\n') || (str[idx] == '\r'))) {
        --idx;
    }

    return str.substr(0, idx + 1);
}

string CMpString::TotallyTrim(const string& str)
{
    string strRet;
    size_t nSize = str.size();
    for (size_t i = 0; i < nSize; ++i) {
        if (str[i] != ' ' && str[i] != '\t' && str[i] != '\n' && str[i] != '\r') {
            strRet += str[i];
        }
    }
    return strRet;
}

void CMpString::FormatLUNID(string& rstrINLUNID, string& rstrOUTLUNID)
{
    string strNumerics("123456789");
    string::size_type strIndex;

    strIndex = rstrINLUNID.find_first_of(strNumerics);
    if (string::npos != strIndex) {
        rstrOUTLUNID = rstrINLUNID.substr(strIndex);
    } else {
        rstrOUTLUNID = "0";
    }
}

char* CMpString::ToUpper(char str[])
{
    char* ptr = str;
    while (*ptr) {
        *ptr = (char)toupper(*ptr);
        ++ptr;
    }

    return str;
}

char* CMpString::ToLower(char str[])
{
    char* ptr = str;
    while (*ptr) {
        *ptr = (char)tolower(*ptr);
        ++ptr;
    }

    return str;
}

string CMpString::ToUpper(string& str)
{
    size_t idx = 0;
    while (idx < str.length()) {
        /*lint -e864*/
        str[idx] = (char)toupper(str[idx]);
        ++idx;
    }

    return str;
}

string CMpString::ToLower(string& str)
{
    size_t idx = 0;
    while (idx < str.length()) {
        /*lint -e864*/
        str[idx] = (char)tolower(str[idx]);
        ++idx;
    }

    return str;
}

void CMpString::StrSplit(vector<string>& vecTokens, const string& strText, char cSep)
{
    string::size_type start = 0;
    string::size_type end;
    while ((end = strText.find(cSep, start)) != string::npos) {
        vecTokens.push_back(strText.substr(start, end - start));
        start = end + 1;
    }

    vecTokens.push_back(strText.substr(start));
}

void CMpString::StrSplitEx(vector<string>& tokens, const string& str, const string& delimiters)
{
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    string::size_type pos = str.find_first_of(delimiters, lastPos);
    while (string::npos != pos || string::npos != lastPos) {
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        lastPos = str.find_first_not_of(delimiters, pos);
        pos = str.find_first_of(delimiters, lastPos);
    }
}

string CMpString::StrJoin(const vector<string>& vecStr, string cSep)
{
    string ret;
    for (size_t i = 0; i < vecStr.size(); i++) {
        ret += vecStr.at(i);
        if (i != vecStr.size() - 1) {
            ret += cSep;
        }
    }
    return ret;
}

string CMpString::StrReplace(const string& str, const string& to_replaced, const string& newchars)
{
    string strRet = str;
    for (string::size_type pos = 0; pos != string::npos; pos += newchars.length()) {
        pos = strRet.find(to_replaced, pos);
        if (pos != string::npos) {
            strRet.replace(pos, to_replaced.length(), newchars);
        } else {
            break;
        }
    }
    return strRet;
}

string CMpString::BlankComma(const string& strPath)
{
    if (strPath.find(" ") != string::npos && strPath[0] != '"') {
        return "\"" + strPath + "\"";
    } else {
        return strPath;
    }
}

#ifdef WIN32
string CMpString::UnicodeToANSI(const wstring& wStrSrc)
{
    string strText("");
    char* pElementText = NULL;
    int iRet;
    int iTextLen;

    // wide char to multi char
    iTextLen = WideCharToMultiByte(CP_ACP, 0, wStrSrc.c_str(), -1, NULL, 0, NULL, NULL);

    NEW_ARRAY_CATCH(pElementText, char, iTextLen + 1);
    if (pElementText == NULL) {
        return strText;
    }

    iRet = memset_s(reinterpret_cast<void*>(pElementText), sizeof(char) * (iTextLen + 1),
                    0, sizeof(char) * (iTextLen + 1));
    if (iRet != EOK) {
        delete[] pElementText;
        return strText;
    }

    ::WideCharToMultiByte(CP_ACP, 0, wStrSrc.c_str(), -1, pElementText, iTextLen, NULL, NULL);

    strText = pElementText;
    delete[] pElementText;
    return strText;
}

wstring CMpString::ANSIToUnicode(const string& strSrc)
{
    wstring wStrTmp;
    int dwNum;
    wchar_t* pElementText = NULL;
    int iRet;

    dwNum = MultiByteToWideChar(CP_ACP, 0, strSrc.c_str(), -1, NULL, 0);
    if (dwNum <= 0) {
        return wStrTmp;
    }

    NEW_ARRAY_CATCH(pElementText, wchar_t, dwNum + 1)
    if (pElementText == NULL) {
        return wStrTmp;
    }

    iRet = memset_s(reinterpret_cast<void*>(pElementText), sizeof(wchar_t) * (dwNum + 1),
                    0, sizeof(wchar_t) * (dwNum + 1));
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

string CMpString::WString2String(const wstring src)
{
    vector<CHAR> chBuffer;

    int iChars = WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, NULL, 0, NULL, NULL);
    if (iChars <= 0) {
        return "";
    }

    chBuffer.resize(iChars);
    iChars = WideCharToMultiByte(CP_ACP, 0, src.c_str(), -1, &chBuffer.front(), (int)chBuffer.size(), NULL, NULL);
    if (iChars <= 0) {
        return "";
    }

    return string(&chBuffer.front());
}

wstring CMpString::String2WString(const string src)
{
    wstring wStrTmp;
    int dwNum;
    wchar_t* pElementText = NULL;
    int iRet;
    dwNum = MultiByteToWideChar(CP_ACP, 0, src.c_str(), -1, NULL, 0);
    if (dwNum <= 0) {
        return wStrTmp;
    }

    NEW_ARRAY_CATCH(pElementText, wchar_t, dwNum + 1);
    if (pElementText == NULL) {
        return wStrTmp;
    }
    iRet = memset_s(reinterpret_cast<void*>(pElementText), sizeof(wchar_t) * (dwNum + 1),
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

string CMpString::ANSIToUTF8(const string& strSrc)
{
    string strUTF8;
    wstring wstrSrc = CMpString::String2WString(strSrc);
    int u8Len = WideCharToMultiByte(CP_UTF8, NULL, wstrSrc.c_str(), -1, NULL, 0, NULL, NULL);
    if (u8Len <= 0) {
        COMMLOG(OS_LOG_ERROR, "u8Len is litter than 0, u8Len is %d.", u8Len);
        return strUTF8;
    }

    char* cUTF8 = NULL;
    NEW_ARRAY_CATCH(cUTF8, char, u8Len + 1);
    if (cUTF8 == NULL) {
        return strUTF8;
    }
    int iRet = memset_s(cUTF8, u8Len + 1, 0, u8Len + 1);
    if (iRet != SUCCESS) {
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

string CMpString::UTF8ToANSI(const string& strSrc)
{
    string strAnsi;
    int ansiLen = MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, NULL, 0);
    if (ansiLen <= 0) {
        COMMLOG(OS_LOG_ERROR, "ansiLen is litter than 0, ansiLen is %d.", ansiLen);
        return strAnsi;
    }

    wchar_t* wAnsi = NULL;
    NEW_ARRAY_CATCH(wAnsi, wchar_t, ansiLen + 1);
    int iRet = memset_s(wAnsi, sizeof(wchar_t) * (ansiLen + 1), 0, sizeof(wchar_t) * (ansiLen + 1));
    if (iRet != SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret %d.", iRet);
        delete[] wAnsi;
        return strAnsi;
    }

    ansiLen = MultiByteToWideChar(CP_UTF8, 0, strSrc.c_str(), -1, wAnsi, ansiLen);
    if (ansiLen <= 0) {
        COMMLOG(OS_LOG_ERROR, "ansiLen is litter than 0, ansiLen is %d.", ansiLen);
        return strAnsi;
    }

    return CMpString::WString2String(wAnsi);
}
#else
int CMpString::GetCh()
{
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);  // 得到原来的终端属性
    newt = oldt;
    newt.c_lflag &= ~ICANON;                  // 设置非正规模式，如果程序每次要从终端读取一个字符的话，这是必须的
    newt.c_lflag &= ~ECHO;                    // 关闭回显
    newt.c_cc[VMIN] = 1;                      // 设置非正规模式下的最小字符数
    newt.c_cc[VTIME] = 0;                     // 设置非正规模式下的读延时
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // 设置新的终端属性
    int iChar = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // 恢复旧属性
    return iChar;
}
#endif

string CMpString::to_string(const uint32_t& uintVal)
{
    string outStr;
    stringstream ss;
    ss << uintVal;
    ss >> outStr;
    return outStr;
}

string CMpString::to_string(const long& longVal)
{
    string outStr;
    stringstream ss;
    ss << longVal;
    ss >> outStr;
    return outStr;
}

string CMpString::to_string(const unsigned long& ulongVal)
{
    string outStr;
    stringstream ss;
    ss << ulongVal;
    ss >> outStr;
    return outStr;
}

string CMpString::to_string(const int& intVal)
{
    string outStr;
    stringstream ss;
    ss << intVal;
    ss >> outStr;
    return outStr;
}

string CMpString::to_string(const unsigned long long& ulonglongVal)
{
    string outStr;
    stringstream ss;
    ss << ulonglongVal;
    ss >> outStr;
    return outStr;
}

bool CMpString::FormattingPath(string& strPath)
{
#ifdef WIN32
    char path[MAX_FULL_PATH_LEN + 1] = { 0x00 };
    if (strPath.length() > MAX_FULL_PATH_LEN || PathCanonicalize(path, strPath.c_str()) == false) {
        COMMLOG(OS_LOG_ERROR, "FormattingPath failed.");
        return false;
    }
#else
    char path[PATH_MAX + 1] = { 0x00 };
    if (strPath.length() > PATH_MAX || NULL == realpath(strPath.c_str(), path)) {
        COMMLOG(OS_LOG_ERROR, "FormattingPath failed,errno[%d]:%s.", errno, strerror(errno));
        return false;
    }
#endif
    strPath = path;
    return true;
}

} // namespace Module

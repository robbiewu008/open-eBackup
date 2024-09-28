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
#include "ParserUtils.h"
#include <random>
#include <thread>
#include <sys/stat.h>
#include "Log.h"
#include "common/Thread.h"

#ifdef WIN32
/* Hint: to provide dir exist check */
#include <filesystem>
#endif

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "SCANNER_UTILS";
}

bool ParserUtils::CheckParentDirIsReachable(string parentDirPath)
{
    DBGLOG("check if parent dir %s exist", parentDirPath.c_str());
    int retryCnt = 0;
    int ret = 0;
    do {
        retryCnt++;
        Module::SleepFor(chrono::milliseconds(CTRL_FILE_SERVER_RETRY_INTERVAL));
#ifdef WIN32
        ret = std::filesystem::is_directory(parentDirPath) ? 0 : -1; /* msvc support C++17 */
#else
        struct stat64 st {};
        ret = lstat64(parentDirPath.c_str(), &st);
#endif
        if (ret == -1) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, MODULE) << "Stat failed for directory: " << parentDirPath << " retry count: "
                << retryCnt << " ERR: " << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        }
    } while ((ret == -1) && (retryCnt < CTRL_FILE_SERVER_RETRY_CNT));
    if (ret == -1) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "Parent Directory is not reachable, DirPath: " << parentDirPath
            << " ERR: " << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return false;
    }
    HCP_Log(ERR, MODULE) << "Parent Dir is reachable, retryCnt: " << retryCnt << " DirPath: "
        << parentDirPath << HCPENDLOG;
    return true;
}

string ParserUtils::GetParentDirOfFile(string filePath)
{
    string parentDirPath = filePath.substr(0, filePath.find_last_of(Module::PATH_SEPARATOR));
    return parentDirPath;
}

uint32_t ParserUtils::GetRandomNumber(uint32_t minNum, uint32_t maxNum)
{
    random_device rd;  // Will be used to obtain a seed for the random number engine
    mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    uniform_int_distribution<> distrib(minNum, maxNum);
    uint32_t randomNum = (uint32_t)distrib(gen);
    return randomNum;
}

string ParserUtils::ConstructStringName(uint32_t &offset, uint32_t &totCommaCnt, vector<string> &lineContents)
{
    uint32_t tempCommaCnt = 0;
    string strName {};

    do {
        strName.append(lineContents[offset]);
        if (tempCommaCnt != totCommaCnt) {
            strName.append(",");
        }
        ++offset;
        ++tempCommaCnt;
    } while (tempCommaCnt <= totCommaCnt);

    return strName;
}

uint32_t ParserUtils::GetCommaCountOfString(const char *str)
{
    uint32_t count = 0;
    for (int i = 0; str[i] != '\0'; ++i) {
        if (',' == str[i])
            ++count;
    }
    return count;
}

uint16_t ParserUtils::Atou16(const char *s)
{
    int myInt(stoi(s));
    uint16_t myInt16(0);
    if (myInt <= static_cast<int>(UINT16_MAX) && myInt >= 0) {
        myInt16 = static_cast<uint16_t>(myInt);
    } else {
        HCP_Log(ERR, MODULE) << "Coverting stringto uint16 failed value: " << s << HCPENDLOG;
    }
    return myInt16;
}

time_t ParserUtils::GetCurrentTimeInSeconds()
{
    chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    time_t currTime = chrono::duration_cast<chrono::seconds>(duration).count();
    return currTime;
}

vector<pair<string, string>> ParserUtils::ParseXattr(const std::vector<XMetaField> &xmetalist)
{
    vector<pair<string, string>> xattrlist;
    for (const XMetaField &xmeta : xmetalist) {
        if (xmeta.m_xMetaType != XMETA_TYPE::XMETA_TYPE_EXTEND_ATTRIBUTES) {
            continue;
        }
        const string& refStr = xmeta.m_value;
        size_t pos = refStr.find('&');
        if (pos == string::npos) {
            return {};
        }
        int kl = atoi(refStr.substr(0, pos).c_str());
        string key = refStr.substr(pos + 1, kl);
        pos = pos + kl + 1;
        size_t pos2 = refStr.find('&', pos);
        if (pos2 == string::npos) {
            return {};
        }
        string value = refStr.substr(pos2 + 1);
        xattrlist.emplace_back(make_pair(key, value));
    }
    return xattrlist;
}

string ParserUtils::ParseDefaultAcl(const std::vector<XMetaField> &xmetalist)
{
    for (const XMetaField &xmeta : xmetalist) {
        if (xmeta.m_xMetaType == XMETA_TYPE::XMETA_TYPE_ACL) {
            if (xmeta.m_value.find("default:") == 0) {
                return xmeta.m_value.substr(8);
            }
        }
    }
    return "";
}

string ParserUtils::ParseAccessAcl(const std::vector<XMetaField> &xmetalist)
{
    for (const XMetaField &xmeta : xmetalist) {
        if (xmeta.m_xMetaType == XMETA_TYPE::XMETA_TYPE_ACL) {
            if (xmeta.m_value.find("access:") == 0) {
                return xmeta.m_value.substr(7);
            }
        }
    }
    return "";
}

string ParserUtils::ParseObjectBucketName(const std::string& path)
{
    size_t len = PATH_SEPARATOR.size();
    if (path.size() <= len) {
        return "";
    }

    string bukName;
    size_t pos = path.substr(len).find(PATH_SEPARATOR);
    if (pos == std::string::npos) {
        bukName = path.substr(len);
    } else {
        bukName = path.substr(len, pos);
    }
    return bukName;
}

string ParserUtils::ParseObjectKey(const std::vector<XMetaField> &xmetalist)
{
    for (const XMetaField &xmeta : xmetalist) {
        if (xmeta.m_xMetaType == XMETA_TYPE::XMETA_TYPE_KEY) {
            return xmeta.m_value;
        }
    }
    return "";
}

string ParserUtils::ParseObjectPath(const std::string& path, const std::vector<XMetaField> &xmetalist)
{
    string obsPath = "";
    string obsName = ParserUtils::ParseObjectKey(xmetalist);
    string bukName = ParserUtils::ParseObjectBucketName(path);
    if (obsName == bukName) {
        obsPath = PATH_SEPARATOR + bukName;
    } else {
        obsPath = PATH_SEPARATOR + bukName + PATH_SEPARATOR + obsName;
    }
    if (obsPath.back() == '/') {
        obsPath.pop_back();
    }
    return obsPath;
}

string ParserUtils::ParseSecurityDescriptor(const std::vector<XMetaField> &xmetalist)
{
    for (const XMetaField &xmeta : xmetalist) {
        if (xmeta.m_xMetaType == XMETA_TYPE::XMETA_TYPE_SECURITYDESCRIPTOR) {
            return xmeta.m_value;
        }
    }
    return "";
}

string ParserUtils::ParseSymbolicLinkTargetPath(const std::vector<XMetaField> &xmetalist)
{
    for (const XMetaField &xmeta : xmetalist) {
        if (xmeta.m_xMetaType == XMETA_TYPE::XMETA_TYPE_SYMBOLIC_TARGET) {
            return xmeta.m_value;
        }
    }
    return "";
}

string ParserUtils::ParseJunctionPointTargetPath(const std::vector<XMetaField> &xmetalist)
{
    for (const XMetaField &xmeta : xmetalist) {
        if (xmeta.m_xMetaType == XMETA_TYPE::XMETA_TYPE_JUNCTION_TARGET) {
            return xmeta.m_value;
        }
    }
    return "";
}

vector<pair<uint64_t, uint64_t>> ParserUtils::ParseSparseInfo(const std::vector<XMetaField> &xmetalist)
{
    vector<pair<uint64_t, uint64_t>> sparselist;
    for (const XMetaField &xmeta : xmetalist) {
        if (xmeta.m_xMetaType != XMETA_TYPE::XMETA_TYPE_SPARSE_INFO) {
            continue;
        }
        const string& refStr = xmeta.m_value;
        size_t comma = 0, semicol = -1;
        do {
            comma = refStr.find(',', semicol + 1);
            if (comma == string::npos) {
                break;
            }
            uint64_t beg = atol(refStr.substr(semicol + 1, comma).c_str());
            semicol = refStr.find(';', comma + 1);
            if (semicol == string::npos) {
                break;
            }
            uint64_t end = atol(refStr.substr(comma + 1, semicol).c_str());
            sparselist.emplace_back(make_pair(beg, end));
        } while (semicol < refStr.size() - 1);
    }
    return sparselist;
}

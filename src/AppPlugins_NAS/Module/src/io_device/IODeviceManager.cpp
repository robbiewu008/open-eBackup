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
#include "IODeviceManager.h"

#include <string>
#include <boost/bind/bind.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <boost/xpressive/xpressive_dynamic.hpp>

#include "log/Log.h"
#include "FileSystemIO.hpp"
#include "S3System.hpp"
#include "config_reader/ConfigIniReader.h"

using namespace std;
using namespace Module;

namespace {
    const string SQLITE_S3_TEMP_TILE = "127.0.0.1:/SQLiteTmpFile.txt";
}

IODeviceManager::IODeviceManager() : isInitS3SDK(false)
{
    HCP_Log(DEBUG, IODeviceModule) << "IODeviceManager init " << HCPENDLOG;
    uint32_t needInitS3 = ConfigReader::getInt("BackupNode", "NeedInitS3");
    HCP_Log(DEBUG, IODeviceModule) << "IODeviceManager init " << needInitS3 << HCPENDLOG;
    if (needInitS3 == 1) {
        m_needInitS3 = true;
    } else {
        m_needInitS3 = false;
    }
}

IODeviceManager::~IODeviceManager()
{
    HCP_Log(DEBUG, IODeviceModule) << "IODeviceManager released" << HCPENDLOG;
}

IODeviceManager &IODeviceManager::GetInstance()
{
    static IODeviceManager manager;
    return manager;
}

bool IODeviceManager::CheckIPV4(const string& ip)
{
    boost::xpressive::sregex regIpv4 = boost::xpressive::sregex::compile(
        "^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})$");
    if (regex_match(ip, regIpv4)) {
        return true;
    }
    return false;
}

bool IODeviceManager::CheckIPV6(const string& ip, bool& withBracket)
{
    withBracket = false;
    string ipv6RegStr =
        "((([0-9A-Fa-f]{1,4}:){7}[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){6}:[0-9A-Fa-f]{1,4})|(([0-9A-Fa-f]{1,4}:){5}(:[0-9A-Fa-f]{1,4}){1,2})|(([0-9A-Fa-f]{1,4}:){4}(:[0-9A-Fa-f]{1,4}){1,3})|(([0-9A-Fa-f]{1,4}:){3}(:[0-9A-Fa-f]{1,4}){1,4})|(([0-9A-Fa-f]{1,4}:){2}(:[0-9A-Fa-f]{1,4}){1,5})|(([0-9A-Fa-f]{1,4})?:(:[0-9A-Fa-f]{1,4}){1,6})|(([0-9A-Fa-f]{1,4}:){1,6}:)|(::)|(([0-9A-Fa-f]{1,4}:){6}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})|(([0-9A-Fa-f]{1,4}:){5}:(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})|(([0-9A-Fa-f]{1,4}:){4}:([0-9A-Fa-f]{1,4}:){0,1}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})|(([0-9A-Fa-f]{1,4}:){3}:([0-9A-Fa-f]{1,4}:){0,2}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})|(([0-9A-Fa-f]{1,4}:){2}:([0-9A-Fa-f]{1,4}:){0,3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3})|(([0-9A-Fa-f]{1,4})?::([0-9A-Fa-f]{1,4}:){0,4}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)(\\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]?|0)){3}))";
    boost::xpressive::sregex regIpv6 = boost::xpressive::sregex::compile(
        "^" + ipv6RegStr + "$");
    if (regex_match(ip, regIpv6)) {
        return true;
    }

    regIpv6 = boost::xpressive::sregex::compile("^\\[" + ipv6RegStr + "\\]$");
    if (regex_match(ip, regIpv6)) {
        withBracket = true;
        return true;
    }
    return false;
}

bool IODeviceManager::CheckIP(const string &path)
{
    auto pos = path.find(":/");
    if (pos == string::npos) {
        HCP_Log(ERR, IODeviceModule) << path << " is invalid" << HCPENDLOG;
        return false;
    }

    string ip = path.substr(0, pos);
    HCP_Log(DEBUG, IODeviceModule) << "s3 ip: " << ip << HCPENDLOG;

    // 匹配不带[]格式的IPV4
    if (CheckIPV4(ip)) {
        HCP_Log(DEBUG, IODeviceModule) << ip << " is ipv4" << HCPENDLOG;
        return true;
    }

    bool withBracket = false;
    if (CheckIPV6(ip, withBracket)) {
        HCP_Log(DEBUG, IODeviceModule) << ip << " is ipv6" << HCPENDLOG;
        return true;
    }

    HCP_Log(DEBUG, IODeviceModule) << ip << " is domain name" << HCPENDLOG;
    return false;
}

obs_uri_style IODeviceManager::GetS3URLStyle(const string &path)
{
    int urlStyle = ConfigReader::getInt("BackupNode", "S3URLStyle");
    HCP_Log(DEBUG, IODeviceModule) << "s3 url style:" << urlStyle << HCPENDLOG;
    if (urlStyle == 1) {
        if (CheckIP(path)) {
            return OBS_URI_STYLE_PATH;
        } else {
            return OBS_URI_STYLE_VIRTUALHOST;
        }
    } else {
        return OBS_URI_STYLE_PATH;
    }
}

bool IODeviceManager::RegisterIODevice(const IODeviceInfo &info,
                                       function<shared_ptr<IODevice>(const IODeviceInfo &, OBJECT_TYPE)> creator)
{
    if ((creator == nullptr) || info.path_prefix.empty()) {
        HCP_Log(ERR, IODeviceModule) << "param error !" << HCPENDLOG;
        return false;
    }
    lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_IODeviceRegInfos.find(info.path_prefix);

    HCP_Log(DEBUG, IODeviceModule) << "IODevice refcount increase. device prefix is " << info.path_prefix << HCPENDLOG;

    {
        lock_guard<std::mutex> lock(m_InitS3Mutex);
        if (!isInitS3SDK && m_needInitS3) {
            // 关闭s3 SDK的openssl初始化;
            set_openssl_callback(OBS_OPENSSL_CLOSE);
            obs_status status = obs_initialize(OBS_INIT_ALL);
            if (OBS_STATUS_OK != status) {
                HCP_Log(ERR, "libs3IO") << "obs_initialize(OBS_INIT_ALL) failed." << HCPENDLOG;
            }
            HCP_Log(INFO, "libs3IO") << "obs_initialize success." << HCPENDLOG;
            isInitS3SDK = true;
        }
    }

    if (it == m_IODeviceRegInfos.end()) {
        IODeviceRegInfo regInfo;
        regInfo.ref_count = 0;
        it = m_IODeviceRegInfos.insert(make_pair(info.path_prefix, regInfo)).first;
    }

    it->second.device_info = info;
    it->second.creator = creator;
    it->second.ref_count++;
    return true;
}

void IODeviceManager::UnregisterIODevice(const string &pathPrefix)
{
    lock_guard<std::mutex> lock(m_Mutex);
    auto it = m_IODeviceRegInfos.find(pathPrefix);
    if (it == m_IODeviceRegInfos.end()) {
        HCP_Log(WARN, IODeviceModule) << "iodevice don't find. device prefix is " << pathPrefix << HCPENDLOG;
        return;
    }

    HCP_Log(DEBUG, IODeviceModule) << "IODevice refcount decrease. device prefix is " << pathPrefix << HCPENDLOG;
    it->second.ref_count--;

    if (it->second.ref_count == 0) {
        m_IODeviceRegInfos.erase(it);
    }
}

shared_ptr<IODevice> IODeviceManager::CreateIODeviceByPath(const string &path, OBJECT_TYPE fileType)
{
    lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_IODeviceRegInfos.begin();
    auto matchedIt = m_IODeviceRegInfos.end();

    string::size_type notMatchCharNum = 0xFFFFFFFFFFFFFFFF;
    for (; it != m_IODeviceRegInfos.end(); ++it) {
        if (path.size() >= it->first.size()
            && path.substr(0, it->first.size()) == it->first) {
            if (notMatchCharNum > path.size() - it->first.size()) {
                notMatchCharNum = path.size() - it->first.size();
                matchedIt = it;
            }
        }
    }

    if (matchedIt != m_IODeviceRegInfos.end()) {
        IODeviceInfo deviceInfo = matchedIt->second.device_info;
        string::size_type pos = string::npos;
        if (path.size() > deviceInfo.path_prefix.size()) {
            pos = path.find("/", deviceInfo.path_prefix.size());
            deviceInfo.path_prefix = path;
            if (string::npos != pos) {
                deviceInfo.path_prefix = path.substr(0, pos);
            }
        }
        return matchedIt->second.creator(deviceInfo, fileType);
    }
    // in order to solve the problem that sqlite will open a file which is a null pointer
    if (path == SQLITE_S3_TEMP_TILE) {
        IODeviceInfo s3IODeviceInfoSqlite;
        s3IODeviceInfoSqlite.path_prefix = SQLITE_S3_TEMP_TILE;
        s3IODeviceInfoSqlite.using_https = true;
        return S3SystemIO::CreateInstance(s3IODeviceInfoSqlite, OBJECT_CACHE_DATA);
    }
    return FileSystemIO::CreateInstance(IODeviceInfo());
}

IODeviceInfo IODeviceManager::GetDeviceInfo(const string &path)
{
    struct IODeviceInfo ioDeviceInfo;
    lock_guard<std::mutex> lock(m_Mutex);

    auto it = m_IODeviceRegInfos.begin();
    auto matchedIt = m_IODeviceRegInfos.end();

    string::size_type notMatchCharNum = 0xFFFFFFFFFFFFFFFF;
    for (; it != m_IODeviceRegInfos.end(); ++it) {
        if (path.size() >= it->first.size()
            && path.substr(0, it->first.size()) == it->first) {
            if (notMatchCharNum > path.size() - it->first.size()) {
                notMatchCharNum = path.size() - it->first.size();
                matchedIt = it;
            }
        }
    }

    if (matchedIt != m_IODeviceRegInfos.end()) {
        IODeviceInfo deviceInfo = matchedIt->second.device_info;
        ioDeviceInfo = deviceInfo;
    }

    return ioDeviceInfo;
}

bool IODeviceManager::IsInitS3SDK()
{
    lock_guard<std::mutex> lck(m_InitS3Mutex);
    return isInitS3SDK;
}

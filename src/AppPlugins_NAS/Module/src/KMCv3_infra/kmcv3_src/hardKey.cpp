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
#include <fstream>
#include "logProxy.h"
#include "hardKey.h"

#define MODULE_NAME "KMC"

namespace {
    const unsigned int ROOT_KEY_LENGTH = 32;
    const unsigned int MAX_KEY_LENGTH = 256;
}

KMCHWKey &KMCHWKey::GetInstance()
{
    static KMCHWKey instance;
    return instance;
}

bool KMCHWKey::SetKeyPath(const std::string &path, const std::string &bakPath)
{
    m_keyPath = path;
    m_bakKeyPath = bakPath;
    return true;
}

bool KMCHWKey::LoadKey()
{
    HCP_LOGGER_NOID(INFO, MODULE_NAME) << "Load HW key";

    if (m_keyPath.empty()) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "Load key failed, no path specified";
        return false;
    }

    std::ifstream ifs;
    ifs.open(m_keyPath.c_str(), std::ifstream::in | std::ifstream::binary);

    if (!ifs.is_open()) {
        HCP_LOGGER_NOID(ERR, COM_MODULE) << "Open key file failed, file path is:" << m_keyPath;
        return false;
    }
    char buff[ROOT_KEY_LENGTH] = {0};
    ifs.read(buff, ROOT_KEY_LENGTH);

    m_key = std::string(buff, ROOT_KEY_LENGTH);

    HCP_LOGGER_NOID(INFO, COM_MODULE) << "Load key size:" << m_key.length();
    return true;
}

bool KMCHWKey::NewKey()
{
    HCP_LOGGER_NOID(INFO, MODULE_NAME) << "Generate new HW key";

    if (m_keyPath == "") {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "New key failed, no path specified";
        return false;
    }
    char buff[MAX_KEY_LENGTH] = {0};
    unsigned int rootKeySize = 8;
    unsigned int len = ROOT_KEY_LENGTH * rootKeySize;
    FILE *fet = nullptr;
    fet = fopen("/dev/random", "rb");
    if (fet == nullptr) {
        HCP_LOGGER_NOID(ERR, COM_MODULE) << "Open /dev/random failed";
        return false;
    }
    if (len != fread(buff, 1, len, fet)) {
        HCP_LOGGER_NOID(ERR, COM_MODULE) << "read /dev/random failed, return is:" << len;
        (void)fclose(fet);
        return false;
    }
    (void)fclose(fet);
    fet = nullptr;

    std::ofstream fout(m_keyPath.c_str(), std::ios::binary | std::ios::trunc);
    if (!fout) {
        HCP_LOGGER_NOID(ERR, COM_MODULE) << "NewKey failed, file path is:" << m_keyPath;
        return false;
    }
    fout.write(buff, len);

    std::ofstream fbakOut(m_bakKeyPath.c_str(), std::ios::binary | std::ios::trunc);
    if (!fbakOut) {
        HCP_LOGGER_NOID(ERR, COM_MODULE) << "New bakKey failed, file path is:" << m_bakKeyPath;
        return false;
    }
    fbakOut.write(buff, len);

    m_key = std::string(buff, ROOT_KEY_LENGTH);
    return true;
}

std::string KMCHWKey::GetKey()
{
    return m_key;
}

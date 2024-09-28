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
#include "ConfigIniValue.h"
#ifndef WIN32
#include <regex.h>
#endif
#include "log/Log.h"
#include "common/IniFile.h"

using namespace Module;
using namespace std;

namespace Module {

void IntConfigValue::refreshValue(const string & sectionName,
        const string & keyName,
        vector<CIniFile*> & iniFiles)
{
    //m_returnValue = m_defaultValue;
    vector<CIniFile*>::iterator it = iniFiles.begin();
    bool result = false;
    for(; it != iniFiles.end(); ++it) {
        int iRet = (*it)->GetIntValue(sectionName.c_str(),
                keyName.c_str(),
                m_returnValue);
        if (iRet == SUCCESS) {
            result = true;
            break;
        }
    }

    if (!result) {
        m_returnValue = m_defaultValue;
        return;
    }

	//lint +e1024
    if (m_returnValue < m_minValue || m_returnValue > m_maxValue) {
        HCP_Logger_noid(DEBUG, MODULE_NAME) << "The value for " << sectionName << "." << keyName
                << " in file is invalid,use the default value:" << m_defaultValue << "." << HCPENDLOG;
        m_returnValue = m_defaultValue;
    } else {
        HCP_Logger_noid(DEBUG, MODULE_NAME) << "The new value for "
                << sectionName << "." << keyName << " is:" << m_returnValue<<HCPENDLOG;
    }
}

void StringConfigValue::refreshValue(const string & sectionName,
        const string & keyName,
        vector<CIniFile*> & iniFiles)
{
    //m_returnValue = m_defaultValue;
    vector<CIniFile*>::iterator it = iniFiles.begin();
    bool result = false;
    for(; it != iniFiles.end(); ++it) {
        int iRet = (*it)->GetStrValue(sectionName.c_str(),
                keyName.c_str(),
                m_returnValue);
        if (iRet == SUCCESS) {
            result = true;
            break;
        }
    }

    if (!result) {
        m_returnValue = m_defaultValue;
        return;
    }
}

void IPStringConfigValue::refreshValue(const string & sectionName,
        const string & keyName,
        vector<CIniFile*> & iniFiles)
{
    //m_returnValue = m_defaultValue;
    vector<CIniFile*>::iterator it = iniFiles.begin();
    bool result = false;
    for(; it != iniFiles.end(); ++it) {
        int iRet = (*it)->GetStrValue(sectionName.c_str(),
                keyName.c_str(),
                m_returnValue);
        if (iRet == SUCCESS) {
            result = true;
            break;
        }
    }

    if (!result) {
        m_returnValue = m_defaultValue;
        HCP_Logger_noid(WARN, MODULE_NAME) << "Config item " << sectionName << "." << keyName
             << " is not exist in config file, use the default value: " << m_returnValue << HCPENDLOG;
        return;
    }

    const char* pattern = "(22[0-3]|2[0-1][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\."
            "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\."
            "(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\\."
            "(25[0-4]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])";


#ifdef WIN32
    std::regex reg(pattern);
    std::smatch match;
    int ret = std::regex_match(m_returnValue.c_str(), reg);
#else
    regex_t reg;
    regmatch_t match;
    int ret = regcomp(&reg, pattern,REG_EXTENDED);
    if (0 != ret) {
        HCP_Logger_noid(WARN, MODULE_NAME) << "The value for " << sectionName
            << "." << keyName << " in file is invalid,use the default value:"
            << m_defaultValue << "." << HCPENDLOG;
        m_returnValue = m_defaultValue;
        return;
    }

    ret = regexec(&reg, m_returnValue.c_str(), 1, &match, 0);

    if (ret != 0) {
        HCP_Logger_noid(WARN, MODULE_NAME) << "The value for " << sectionName
                << "." << keyName << " in file is invalid,use the default value:"
                << m_defaultValue << "." << HCPENDLOG;
        m_returnValue = m_defaultValue;
    } else {
        HCP_Logger_noid(INFO, MODULE_NAME) << "The new value for " << sectionName
            << "." << keyName << " is:" <<m_returnValue<<HCPENDLOG;
    }

    regfree(&reg);
#endif
}

} // namespace Module

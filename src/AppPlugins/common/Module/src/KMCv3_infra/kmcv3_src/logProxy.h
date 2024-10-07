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
#ifndef RC_SRC_KMCV3_INFRA_KMCV3_SRC_LOGPROXY_H
#define RC_SRC_KMCV3_INFRA_KMCV3_SRC_LOGPROXY_H

#include <sstream>
#ifdef __cplusplus
extern "C" {
#endif

class LogProxyKmc : public std::ostringstream {
public:
    LogProxyKmc(
        const int level,
        const std::string &functionName,
        const unsigned int line) : m_logLevel(level),
                                   m_funcName(functionName),
                                   m_line(line)
    {
    };
    ~LogProxyKmc() override;

private:
    int m_logLevel;
    std::string m_funcName;
    unsigned int m_line;
};

enum SeverityLevel {
    TRACE = 0,
    DEBUG = 1,
    INFO = 2,
    WARN = 3,
    ERR = 4,
    CRIT = 5
};

#define HCP_LOGGER_NOID(level, module) LogProxyKmc(level, __FUNCTION__, __LINE__) << 0

/*
 * Function: bool InitKmcLog(std::string moduleName)
 * Description: 日志模块初始化接口
 * Called By: InitKMCV1c   InitKMCV3c
 * Input:  std::string moduleName  不能带特殊字符
 * Output: nullptr
 * Return: ture/false
 */
bool InitKmcLog(const std::string &moduleName);

#ifdef __cplusplus
    }
#endif

#endif

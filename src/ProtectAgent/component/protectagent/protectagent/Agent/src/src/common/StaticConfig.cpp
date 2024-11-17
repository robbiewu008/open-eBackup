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

#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/Utils.h"
#include "common/StaticConfig.h"

namespace StaticConfig {

bool IsInnerAgent()
{
    static bool initFlag = false;
    static mp_int32 installType = AGENT_INSTALL_TYPE_EXTERNAL;
    if (initFlag) {
        return (installType == AGENT_INSTALL_TYPE_INTERNAL);
    }

    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_BACKUP_SCENE, installType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get back up scene failed.");
        return false;
    }
    initFlag = true;
    if (installType == AGENT_INSTALL_TYPE_INTERNAL) {
        return true;
    }
    return false;
}

bool NeedClusterEsn()
{
    if (!IsInnerAgent()) {
        return false;
    }
    mp_string deployType;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DEPLOY_TYPE, deployType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get deploy type failed.");
        return false;
    }

    if (deployType == HOST_ENV_DEPLOYTYPE_X8000 ||
        deployType == HOST_ENV_DEPLOYTYPE_X6000 ||
        deployType == HOST_ENV_DEPLOYTYPE_X3000 ||
        deployType == HOST_ENV_DEPLOYTYPE_X9000) {
        return true;
    }
    return false;
}

bool GetAgentIp(mp_string& agentIp)
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_AGENT_IP, agentIp);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get agent ip failed.");
        return false;
    }
    return true;
}

bool IsInnerAgentMainDeploy()
{
    if (!IsInnerAgent()) {
        return false;
    }

    mp_string deployType;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DEPLOY_TYPE, deployType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get deploy type failed.");
        return false;
    }

    if (deployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT ||
        deployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT_NO_BRAND ||
        deployType == HOST_ENV_DEPLOYTYPE_HYPERDETECT_CYBER_ENGINE ||
        deployType == HOST_ENV_DEPLOYTYPE_E6000 ||
        deployType == HOST_ENV_DEPLOYTYPE_DATABACKUP) {
        return false;
    }
    return true;
}
};
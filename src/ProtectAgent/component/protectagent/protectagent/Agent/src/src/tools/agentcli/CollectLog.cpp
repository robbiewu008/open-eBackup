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
#include "tools/agentcli/CollectLog.h"
#include "common/Defines.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/Uuid.h"
#include "common/Utils.h"
#include "common/CSystemExec.h"
#include "securecom/Password.h"
#include "securecom/SecureUtils.h"

/* ------------------------------------------------------------
Description  : 各进程的运行状态
Return       : MP_SUCCESS -- 成功
------------------------------------------------------------- */
mp_int32 CollectLog::Handle()
{
    printf("%s\n", COLLECTLOG_HINT);
    mp_string strTmpChoice;
    mp_uint32 uiCount = 0;
    mp_uint32 uiRetryCount = 3;

    while (uiCount < uiRetryCount) {
        printf("%s", CONTINUE);
        CPassword::GetInput(CONTINUE, strTmpChoice);
        if (strTmpChoice == "n") {
            return MP_FAILED;
        } else if (strTmpChoice == "y") {
            break;
        } else {
            uiCount += 1;
        }
    }

    if (uiCount >= uiRetryCount) {
        printf("Input invalid value over 3 times.\n");
        return MP_FAILED;
    }

    mp_time nowTime;
    CMpTime::Now(nowTime);
    mp_string strNowTime = CMpTime::GetTimeString(nowTime);
    // 去除字符串中非数字
    mp_string strTime;
    for (mp_uint32 i = 0; i < strNowTime.length(); i++) {
        if (strNowTime[i] >= '0' && strNowTime[i] <= '9') {
            strTime.push_back(strNowTime[i]);
        }
    }
    /* LOG FILE FORMAT:
     * WIN: AGENTLOG_{UUID}_{TIMESTAMP}.zip
     * LINUX: AGENTLOG_{UUID}_{TIMESTAMP}.tar.gz
     */
    mp_string uuid;
    if (CUuidNum::GetUuidStandardStr(uuid) != MP_SUCCESS) {
        printf("Generate uuid failed.");
        COMMLOG(OS_LOG_ERROR, "Generate uuid failed.");
        return MP_FAILED;
    }
    mp_string logId = uuid + SIGN_UNDERLINE + strTime;
    mp_string strLogName = mp_string(AGENT_LOG_ZIP_NAME) + SIGN_UNDERLINE + logId + ZIP_SUFFIX;
    printf("Log File: %s\n", strLogName.c_str());
    COMMLOG(OS_LOG_INFO, "Log File: %s", strLogName.c_str());

    mp_int32 iRet = SecureCom::PackageLog(strLogName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Package log failed, iRet = %d.", iRet);
        printf("Collect log failed.\n");
        return iRet;
    }

    return MP_SUCCESS;
}

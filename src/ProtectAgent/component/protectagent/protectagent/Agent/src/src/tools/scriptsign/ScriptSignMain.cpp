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
#include "common/ConfigXmlParse.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "securecom/CryptAlg.h"
#include "securecom/SecureUtils.h"
#include "securec.h"

/* ------------------------------------------------------------
Description  : main函数，无输入
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    (mp_void)argc;

    // 初始化路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (iRet != MP_SUCCESS) {
        printf("Init path %s failed.\n", argv[0]);
        return iRet;
    }

    // 初始化xml配置
    mp_string strXMLConfPath = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    iRet = CConfigXmlParser::GetInstance().Init(strXMLConfPath);
    if (iRet != MP_SUCCESS) {
        printf("Init xml conf file %s failed.\n", AGENT_XML_CONF.c_str());
        return iRet;
    }

    // 初始化日志文件
    mp_string strLogPath = CPath::GetInstance().GetLogPath();
    CLogger::GetInstance().Init(mp_string(SCRIPT_SIGN_LOG_NAME).c_str(), strLogPath);

    // 初始化KMC
    iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init crypt failed, ret = %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Begin generating script sign file.");

    iRet = SecureCom::GenSignFile();
    // 程序即将退出，此处不判断返回值
    (mp_void) FinalizeCrypt();
#ifndef WIN32
    (mp_void) ChangeGmonDir();  // change profile out put dir
#endif
    COMMLOG(OS_LOG_INFO, "End generating script sign file.");
    return iRet;
}

/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file CdpDataPath.cpp
 * @brief  Implementation of the Class OracleNativeBackupPlugin
 * @version 1.0.0
 * @date 2019-11-15
 * @author wangguitao 00510599
 */
#include <iostream>
#include "common/CMpThread.h"
#include "common/Types.h"
#include "common/TimeOut.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/JsonUtils.h"
#include "message/tcp/CSocket.h"
#include "dataprocess/datamessage/DataMessage.h"
#include "dataprocess/dataconfig/DataConfig.h"
#include "dataprocess/datareadwrite/DataStream.h"
#include "dataprocess/datapath/CdpDataPath.h"

// 定义DataPath命令处理函数
using cdpMsgHandlerFunPtr = mp_int32 (*)(Json::Value);
static cdpMsgHandlerFunPtr g_cdpCmdHandlerFuns[CMD_COUNT];

CdpDataPath::CdpDataPath(const mp_string& dpParam) : DataPath(SERVICE_CDP, dpParam)
{}

CdpDataPath::~CdpDataPath()
{}

mp_int32 CdpDataPath::HandleProtect(Json::Value bodyMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_string vname;

    iRet = CJsonUtils::GetJsonString(bodyMsg[MANAGECMD_KEY_BODY], EXT_CMD_PROTECT_VOL_NAMAE, vname);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    mp_string vid;
    iRet = CJsonUtils::GetJsonString(bodyMsg[MANAGECMD_KEY_BODY], EXT_CMD_PROTECT_VOL_ID, vid);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    COMMLOG(OS_LOG_INFO,
        "Received Message\nCMD: EXT_CMD_PROTECT_VOL\n Vol Name: %s\n Vol Id: %s\n",
        vname.c_str(),
        vid.c_str());

    return iRet;
}

mp_int32 CdpDataPath::HandleAddVol(Json::Value bodyMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, "Received Message: EXT_CMD_ADD_VOL\n");
    return iRet;
}

mp_int32 CdpDataPath::HandleDelVol(Json::Value bodyMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, "Received Message: EXT_CMD_DEL_VOL\n");
    return iRet;
}
mp_int32 CdpDataPath::HandleModVol(Json::Value bodyMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, "Received Message: EXT_CMD_MOD_VOL\n");
    return iRet;
}
mp_int32 CdpDataPath::HandleVolReady(Json::Value bodyMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, "Received Message: EXT_CMD_VOL_READY\n");
    return iRet;
}

mp_int32 CdpDataPath::HandlePause(Json::Value bodyMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, "Received Message: EXT_CMD_PAUSE\n");
    return iRet;
}

mp_int32 CdpDataPath::HandleResume(Json::Value bodyMsg)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_INFO, "Received Message: EXT_CMD_RESUME\n");
    return iRet;
}

mp_void CdpDataPath::HandleClose()
{
    INFOLOG("Received Message: EXT_CMD_CLOSE\n");
    sendExitFlag = MP_TRUE;
}

mp_int32 CdpDataPath::ParseCdpCommand(mp_int32 cmd, const Json::Value& bodyMsg)
{
    // 初始化函数表
    for (int i = 0; i < CMD_COUNT; i++) {
        g_cdpCmdHandlerFuns[i] = NULL;
    }
    g_cdpCmdHandlerFuns[CMD_PROTECT_VOL] = &HandleProtect;
    g_cdpCmdHandlerFuns[CMD_ADD_VOL] = &HandleAddVol;
    g_cdpCmdHandlerFuns[CMD_DEL_VOL] = &HandleDelVol;
    g_cdpCmdHandlerFuns[CMD_MOD_VOL] = &HandleModVol;
    g_cdpCmdHandlerFuns[CMD_VOL_READY] = &HandleVolReady;
    g_cdpCmdHandlerFuns[CMD_PAUSE] = &HandlePause;
    g_cdpCmdHandlerFuns[CMD_RESUME] = &HandleResume;

    cmd = cmd - 0x0500;

    if (cmd > CMD_COUNT) {
        ERRLOG("Unknown command, id: '%d'.", cmd);
        return MP_FAILED;
    }

    cdpMsgHandlerFunPtr pFun = g_cdpCmdHandlerFuns[cmd];
    if (!pFun) {
        ERRLOG("Unable to get function, id: '%d'.", cmd);
        return MP_FAILED;
    }

    return pFun(bodyMsg);
}
mp_int32 CdpDataPath::ExtCmdProcess(CDppMessage &message)
{
    mp_int32 iRet = DataPath::ExtCmdProcess(message);
    if (iRet == MP_SUCCESS) {
        return (iRet);
    }

    Json::Value bodyMsg;
    mp_uint32 mCmd = message.GetManageCmd();
    iRet = message.GetManageBody(bodyMsg);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get cmd=0x%x, seq=%llu manageBody failed.", mCmd, message.GetOrgSeqNo());
        return iRet;
    }

    iRet = ParseCdpCommand(mCmd, bodyMsg);
    return iRet;
}

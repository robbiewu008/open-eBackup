/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file ChgSnmp.cpp
 * @brief  The implemention about Host ChgSnmp
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "common/ConfigXmlParse.h"
#include "common/Defines.h"
#include "common/Utils.h"
#include "securecom/Password.h"
#include "tools/agentcli/ChgSnmp.h"

/* ------------------------------------------------------------
Description  :处理函数，为降低圈复杂度其原handle函数进行拆分
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 ChgSnmp::HandleInner()
{
    SNMP_CHOOSE_TYPE eChooseType = GetChoice();
    mp_int32 iRet = MP_FAILED;
    switch (eChooseType) {
        case SNMP_CHOOSE_SET_AUTH_PASSWD:
            printf("%s\n", SET_AUTH_PASSWD_HINT);
            if (CheckSNMPPwd(PASSWORD_SNMP_AUTH)) {
                iRet = CPassword::ChgPwd(PASSWORD_SNMP_AUTH);
            }
            COMMLOG(OS_LOG_INFO, "change SNMP auth.");
            break;

        case SNMP_CHOOSE_SET_PRI_PASSWD:
            printf("%s\n", SET_PRI_PASSWD_HINT);
            if (CheckSNMPPwd(PASSWORD_SNMP_PRIVATE)) {
                iRet = CPassword::ChgPwd(PASSWORD_SNMP_PRIVATE);
            }
            COMMLOG(OS_LOG_INFO, "change SNMP private.");
            break;

        case SNMP_CHOOSE_SET_AUTH_PROTOCOL:
            printf("%s\n", SET_AUTH_PROTOCOL_HINT);
            iRet = ChgAuthProtocol();
            COMMLOG(OS_LOG_INFO, "change SNMP auth protocol.");
            break;

        case SNMP_CHOOSE_SET_PRI_PROTOCOL:
            printf("%s\n", SET_PRI_PROTOCOL_HINT);
            iRet = ChgPrivateProtocol();
            COMMLOG(OS_LOG_INFO, "change SNMP private protocol.");
            break;

        case SNMP_CHOOSE_SET_SECURITY_NAME:
            iRet = ChgSecurityName();
            COMMLOG(OS_LOG_INFO, "change SNMP security name.");
            break;

        case SNMP_CHOOSE_SET_SECURITY_LEVEL:
        case SNMP_CHOOSE_SET_SECURITY_MODEL:
        case SNMP_CHOOSE_SET_CONTEXT_ENGID:
        case SNMP_CHOOSE_SET_CONTEXT_NAME:
        case SNMP_CHOOSE_SET_BUTT:
        default:
            iRet = HandleInner2(eChooseType);
            break;
    }
    return iRet;
}

/* ------------------------------------------------------------
Description  :处理函数，为降低圈复杂度其原HandleInner函数进行拆分
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 ChgSnmp::HandleInner2(SNMP_CHOOSE_TYPE eChooseType)
{
    mp_int32 iRet = MP_FAILED;
    switch (eChooseType) {
        case SNMP_CHOOSE_SET_SECURITY_LEVEL:
            printf("%s\n", SET_SECURITY_LEVEL_HINT);
            iRet = ChgSecurityLevel();
            COMMLOG(OS_LOG_INFO, "change SNMP security level.");
            break;

        case SNMP_CHOOSE_SET_SECURITY_MODEL:
            printf("%s\n", SET_SECURITY_MODEL_HINT);
            iRet = ChgSecurityModel();
            COMMLOG(OS_LOG_INFO, "change SNMP security mode.");
            break;

        case SNMP_CHOOSE_SET_CONTEXT_ENGID:
            iRet = ChgContextEngineID();
            COMMLOG(OS_LOG_INFO, "change SNMP context engid.");
            break;

        case SNMP_CHOOSE_SET_CONTEXT_NAME:
            iRet = ChgContextName();
            COMMLOG(OS_LOG_INFO, "change SNMP context name.");
            break;

        case SNMP_CHOOSE_SET_AUTH_PASSWD:
        case SNMP_CHOOSE_SET_PRI_PASSWD:
        case SNMP_CHOOSE_SET_AUTH_PROTOCOL:
        case SNMP_CHOOSE_SET_PRI_PROTOCOL:
        case SNMP_CHOOSE_SET_SECURITY_NAME:
        case SNMP_CHOOSE_SET_BUTT:
        default:
            iRet = MP_SUCCESS;
            break;
    }
    return iRet;
}

/* ------------------------------------------------------------
Description  :处理函数
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 ChgSnmp::Handle()
{
    // 校验当前管理员密码
    mp_string strUsrName;
    mp_int32 iRet = CPassword::VerifyAgentUser(strUsrName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Verify user(%s) failed.", strUsrName.c_str());
        return iRet;
    }

    iRet = HandleInner();
    if (iRet != MP_SUCCESS) {
        printf("%s\n", OPERATION_PROCESS_FAIL_HINT);
    } else {
        printf("%s\n", OPERATION_PROCESS_SUCCESS_HINT);
        COMMLOG(OS_LOG_INFO, "SNMP configuration is modified successfully.");
    }

    return iRet;
}
/* ------------------------------------------------------------
Description  : 提示操作选项并读入用户输入
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
SNMP_CHOOSE_TYPE ChgSnmp::GetChoice()
{
    printf("%s:\n", CHOOSE_OPERATION_HINT);
    printf("%d: %s\n", (mp_int32)SNMP_CHOOSE_SET_AUTH_PASSWD, SET_AUTH_PASSWD_HINT);
    printf("%d: %s\n", (mp_int32)SNMP_CHOOSE_SET_PRI_PASSWD, SET_PRI_PASSWD_HINT);
    printf("%d: %s\n", (mp_int32)SNMP_CHOOSE_SET_AUTH_PROTOCOL, SET_AUTH_PROTOCOL_HINT);
    printf("%d: %s\n", (mp_int32)SNMP_CHOOSE_SET_PRI_PROTOCOL, SET_PRI_PROTOCOL_HINT);
    printf("%d: %s\n", (mp_int32)SNMP_CHOOSE_SET_SECURITY_NAME, SET_SECURITY_NAME_HINT);
    printf("%d: %s\n", (mp_int32)SNMP_CHOOSE_SET_SECURITY_LEVEL, SET_SECURITY_LEVEL_HINT);
    printf("%d: %s\n", (mp_int32)SNMP_CHOOSE_SET_SECURITY_MODEL, SET_SECURITY_MODEL_HINT);
    printf("%d: %s\n", (mp_int32)SNMP_CHOOSE_SET_CONTEXT_ENGID, SET_CONTEXT_ENGID_HINT);
    printf("%d: %s\n", (mp_int32)SNMP_CHOOSE_SET_CONTEXT_NAME, SET_CONTEXT_NAME_HINT);
    printf("%s\n", QUIT_HINT);
    printf("%s", CHOOSE_HINT);

    mp_string strChoice;
    CPassword::GetInput(CHOOSE_HINT, strChoice);

    if (strChoice.length() == 1 && (strChoice[0] >= '0' + SNMP_CHOOSE_SET_AUTH_PASSWD) &&
        (strChoice[0] < '0' + SNMP_CHOOSE_SET_BUTT)) {
        return SNMP_CHOOSE_TYPE(strChoice[0] - '0');
    } else {
        return SNMP_CHOOSE_SET_BUTT;
    }
}
/* ------------------------------------------------------------
Description  : 更改SNMP认证加密算法
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 ChgSnmp::ChgAuthProtocol()
{
    printf("%d: %s\n", (mp_int32)AUTH_PROTOCOL_NONE, NONE.c_str());
    printf("%d: %s\n", (mp_int32)AUTH_PROTOCOL_MD5, MD5.c_str());
    printf("%d: %s\n", (mp_int32)AUTH_PROTOCOL_SHA1, SHA_1.c_str());
    printf("%d: %s\n", (mp_int32)AUTH_PROTOCOL_SHA2, SHA_2.c_str());
    printf("%s", CHOOSE_HINT);

    mp_string strChoice;
    mp_int32 iRet;
    CPassword::GetInput(CHOOSE_HINT, strChoice);

    mp_bool bAuthProCheck = strChoice.length() != 1 ||
                            ((strChoice[0] != '0' + AUTH_PROTOCOL_NONE) && (strChoice[0] != '0' + AUTH_PROTOCOL_MD5) &&
                                (strChoice[0] != '0' + AUTH_PROTOCOL_SHA1) &&
                                (strChoice[0] != '0' + AUTH_PROTOCOL_SHA2));
    if (bAuthProCheck) {
        printf("%s\n", AUTH_PROTOCOL_NOT_SUPPORTED);
        return MP_FAILED;
    }

    if (strChoice[0] != '0' + AUTH_PROTOCOL_SHA2) {
        printf("%s\n", AUTH_PROTOCOL_NOT_SAFE);
        printf("%s", CONTINUE);
        mp_string strTmpChoice;
        CPassword::GetInput(CONTINUE, strTmpChoice);
        if (strTmpChoice != "y" && strTmpChoice != "Y") {
            return MP_FAILED;
        }
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_AUTH_PROTOCOL, strChoice);
    if (iRet != MP_SUCCESS) {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;
}
/* ------------------------------------------------------------
Description  : 更改SNMP私有加密算法
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 ChgSnmp::ChgPrivateProtocol()
{
    printf("%d: %s\n", (mp_int32)PRIVATE_PROTOCOL_NONE, NONE.c_str());
    printf("%d: %s\n", (mp_int32)PRIVATE_PROTOCOL_DES, DES.c_str());
    printf("%d: %s\n", (mp_int32)PRIVATE_PROTOCOL_AES128, AES128.c_str());
    printf("%s", CHOOSE_HINT);
    mp_string strChoice;
    mp_int32 iRet;
    CPassword::GetInput(CHOOSE_HINT, strChoice);

    if (strChoice.length() != 1 || (strChoice[0] != '0' + PRIVATE_PROTOCOL_NONE) &&
        (strChoice[0] != '0' + PRIVATE_PROTOCOL_AES128) && (strChoice[0] != '0' + PRIVATE_PROTOCOL_DES)) {
        printf("%s\n", PRIVATE_PROTOCOL_NOT_SUPPORTED);
        return MP_FAILED;
    }

    if (strChoice[0] != '0' + PRIVATE_PROTOCOL_AES128) {
        printf("%s\n", PRIVATE_PROTOCOL_NOT_SAFE);
        printf("%s", CONTINUE);
        mp_string strTmpChoice;
        CPassword::GetInput(CONTINUE, strTmpChoice);
        if (strTmpChoice != "y" && strTmpChoice != "Y") {
            return MP_FAILED;
        }
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_PRIVATE_PROTOCOL, strChoice);
    if (iRet != MP_SUCCESS) {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;
}
/* ------------------------------------------------------------
Description  : 更改SNMP安全名称
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 ChgSnmp::ChgSecurityName()
{
    printf("%s", SECURITY_NAME_HINT);
    mp_string strName;
    CPassword::GetInput(SECURITY_NAME_HINT, strName);

    if (strName.length() > MAX_SNMP_PARAM_LEN) {
        printf("%s\n", INPUT_TOO_LONG_HINT);
        return MP_FAILED;
    }

    mp_int32 iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_SECURITY_NAME, strName);
    if (iRet != MP_SUCCESS) {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;
}
/* ------------------------------------------------------------
Description  : 更改SNMP安全级别
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 ChgSnmp::ChgSecurityLevel()
{
    printf("%d: %s\n", (mp_int32)SECURITY_LEVEL_NOAUTH_NOPRIV, NOAUTH_NOPRIV);
    printf("%d: %s\n", (mp_int32)SECURITY_LEVEL_NOPRI, AUTH_NOPRIV);
    printf("%d: %s\n", (mp_int32)SECURITY_LEVEL_AUTH_PRIV, AUTH_PRIV);
    printf("%s", CHOOSE_HINT);
    mp_string strChoice;
    mp_int32 iRet;
    CPassword::GetInput(CHOOSE_HINT, strChoice);

    if (strChoice.length() != 1 || (strChoice[0] < '0' + SECURITY_LEVEL_NOAUTH_NOPRIV) ||
        (strChoice[0] > '0' + SECURITY_LEVEL_AUTH_PRIV)) {
        printf("%s\n", SECURITY_LEVEL_NOT_SUPPORTED);
        return MP_FAILED;
    }

    if (strChoice[0] != '0' + SECURITY_LEVEL_AUTH_PRIV) {
        printf("%s\n", SECURITY_LEVEL_NOT_SAFE);
        printf("%s", CONTINUE);
        mp_string strTmpChoice;
        CPassword::GetInput(CONTINUE, strTmpChoice);
        if (strTmpChoice != "y" && strTmpChoice != "Y") {
            return MP_FAILED;
        }
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_SECURITY_LEVEL, strChoice);
    if (iRet != MP_SUCCESS) {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;
}
/* ------------------------------------------------------------
Description  : 更改SNMP安全模式
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 ChgSnmp::ChgSecurityModel()
{
    printf("%d: %s\n", (mp_int32)SECURITY_MODEL_ANY, ANY.c_str());
    printf("%d: %s\n", (mp_int32)SECURITY_MODEL_V1, V1.c_str());
    printf("%d: %s\n", (mp_int32)SECURITY_MODEL_V2, V2.c_str());
    printf("%d: %s\n", (mp_int32)SECURITY_MODEL_USM, USM.c_str());
    printf("%s", CHOOSE_HINT);

    mp_string strChoice;
    mp_int32 iRet;
    CPassword::GetInput(CHOOSE_HINT, strChoice);

    if (strChoice.length() != 1 || (strChoice[0] < '0' + SECURITY_MODEL_ANY) ||
        (strChoice[0] > '0' + SECURITY_MODEL_USM)) {
        printf("%s\n", SECURITY_MODEL_NOT_SUPPORTED);
        return MP_FAILED;
    }

    if (strChoice[0] != '0' + SECURITY_MODEL_USM) {
        printf("%s\n", SECURITY_MODEL_NOT_SAFE);
        printf("%s", CONTINUE);
        mp_string strTmpChoice;
        CPassword::GetInput(CONTINUE, strTmpChoice);
        if (strTmpChoice != "y" && strTmpChoice != "Y") {
            return MP_FAILED;
        }
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_SECURITY_MODEL, strChoice);
    if (iRet != MP_SUCCESS) {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Set value into xml configuration file failed.");
    }

    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : 更改SNMP上下文引擎ID
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 ChgSnmp::ChgContextEngineID()
{
    mp_string strID;
    printf("%s", CONTEXT_EN_ID_HINT);
    mp_int32 iRet;
    CPassword::GetInput(CONTEXT_EN_ID_HINT, strID);

    if (strID.length() > MAX_SNMP_PARAM_LEN) {
        printf("%s\n", INPUT_TOO_LONG_HINT);
        return MP_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_ENGINE_ID, strID);
    if (iRet != MP_SUCCESS) {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;
}
/* ------------------------------------------------------------
Description  : 更改SNMP上下文
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 ChgSnmp::ChgContextName()
{
    mp_string strName;
    printf("%s", CONTEXT_NAME_HINT);
    mp_int32 iRet;
    CPassword::GetInput(CONTEXT_NAME_HINT, strName);

    if (strName.length() > MAX_SNMP_PARAM_LEN) {
        printf("%s\n", INPUT_TOO_LONG_HINT);
        return MP_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, CFG_CONTEXT_NAME, strName);
    if (iRet != MP_SUCCESS) {
        printf("Set value into xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Set value into xml configuration file failed.");
    }

    return iRet;
}

mp_bool ChgSnmp::CheckSNMPPwd(PASSWOD_TYPE eType)
{
    mp_int32 iInputFailedTimes = 0;
    mp_string strSNMPPwd;
    while (iInputFailedTimes <= MAX_FAILED_COUNT) {
        CPassword::InputUserPwd("", strSNMPPwd, INPUT_SNMP_OLD_PWD);
        if (CPassword::CheckOtherOldPwd(eType, strSNMPPwd)) {
            break;
        } else {
            iInputFailedTimes++;
            continue;
        }
    }

    ClearString(strSNMPPwd);
    if (iInputFailedTimes > static_cast<mp_int32>(MAX_FAILED_COUNT)) {
        return MP_FALSE;
    } else {
        return MP_TRUE;
    }
}

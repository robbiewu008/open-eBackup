/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file RegExtMk.cpp
 * @brief  Contains function declarations for RegExtMk
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "tools/agentcli/RegExtMk.h"

#include <sstream>
#include "common/Defines.h"
#include "common/Log.h"
#include "securecom/CryptAlg.h"
#include "securecom/Password.h"
#include "securecom/SDPFunc.h"
#include "common/CSystemExec.h"
#include "securecom/RootCaller.h"
#include "common/ConfigXmlParse.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "securecom/SecureUtils.h"
using namespace std;
// static vars initialization
mp_string RegExtMk::nginxKeyPasswd;
mp_string RegExtMk::snmpPrivatePasswd;
mp_string RegExtMk::snmpAuthPasswd;
mp_string RegExtMk::nginxCiphertext;
mp_string RegExtMk::snmpPrivateCiphertext;
mp_string RegExtMk::snmpAuthCiphertext;

/* ------------------------------------------------------------
Description  : handle the request of register external MK
Return       : MP_SUCCESS -- register successfully
               MP_FAILED -- register failed
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_int32 RegExtMk::Handle()
{
    COMMLOG(OS_LOG_INFO, "Begin handle register external MK request.");

    // 1. Verfiy agent passwd
    mp_int32 iRet = VerifyUserPasswd();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "verify agent password failed.");
        return iRet;
    }

    // 2. Get and checkout plainText which length is in [1, 127]
    mp_string plainText;
    iRet = CheckPlainText(plainText);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Check plaintext failed.");
        return iRet;
    }

    // 3. Get and checkout MK life days, value is in [1,365]
    mp_uint32 keyLifeDays = 0;
    iRet = CheckMkLifeDays(keyLifeDays);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Check life days of MK failed.");
        goto Handle_error;
    }

    // 4. DeCrypt stored password
    iRet = DecryptStoredPasswd();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Decrypt stored password failed.");
        goto Handle_error;
    }

    // 5. Register new KMC domain if domainid = 2 not exists , and cretae new MK
    iRet = RegisterExternalMK(plainText, keyLifeDays);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Register new KMC domain or create MK failed.");
        goto Handle_error;
    }

    // 6. re-sign all scripts with new added MK
    iRet = SignAllScriptsWithNewMK();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Re-sign sripts failed.");
        goto Handle_error;
    }

    // 7. Re-encrypt all passwd and write them into file
    iRet = EncryptPasswdWithExtMk();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Re-encrypt or write pawssword to file failed.");
        goto Handle_error;
    }

    HINT_REGISTER_MK_SUCC();
    COMMLOG(OS_LOG_INFO, "Register external MK succ.");
    return iRet;

Handle_error:
    ClearString(plainText);
    SafeClearPasswd();
    HINT_REGISTER_MK_FAILED();
    return MP_FAILED;
}

/* ------------------------------------------------------------
Description  : verify the agent password
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_int32 RegExtMk::VerifyUserPasswd()
{
    mp_string strUserName;
    return CPassword::VerifyAgentUser(strUserName);
}

/* ------------------------------------------------------------
Description  : get the input plaintext firstly, then check its length
Output       : plainText -- return the checked plaintext
Return       : MP_SUCCESS -- get a valid plaintext
               MP_FAILED -- invalid plaintext
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_int32 RegExtMk::CheckPlainText(mp_string& plainText)
{
    mp_uint32 inputTimes = 0;

    // there is 3 chance to input plaintext
    while (inputTimes <= MAX_FAILED_COUNT) {
        printf("%s", INPUT_PLAIN_TEXT);
        CPassword::GetInputEchoWithStar(INPUT_PLAIN_TEXT, plainText, PLAIN_TEXT_MAX_LEN);
        if (plainText.size() < PLAIN_TEXT_MIN_LEN || plainText.size() > PLAIN_TEXT_MAX_LEN) {
            printf("%s\n", "The input plaintext length is invalid. Try again.");
            COMMLOG(OS_LOG_ERROR, "Input plaintext is invalid.");
            inputTimes++;
            continue;
        } else {
            break;
        }
    }

    if (inputTimes > MAX_FAILED_COUNT) {
        printf("%s\n", "You have input the invalid plaintext for 3 times. The program exits.");
        COMMLOG(OS_LOG_ERROR, "Input invalid plaintext more than 3 times.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : Get life days of MK, then check if it is valid
Output       : keyLifeDays -- return the checked life days of MK
Return       : MP_SUCCESS -- get a valid life days of MK
               MP_FAILED -- invalid input
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_int32 RegExtMk::CheckMkLifeDays(mp_uint32& keyLifeDays)
{
    mp_uint32 inputTimes = 0;
    mp_string strKeyLifeDays;

    // there is 3 chance to input key life days
    while (inputTimes <= MAX_FAILED_COUNT) {
        printf("%s", INPUT_MK_LIFE_DAYS);
        CPassword::GetInput(INPUT_MK_LIFE_DAYS, strKeyLifeDays, REGEXTMK_NUM_4);

        keyLifeDays = StringToUint32(strKeyLifeDays);
        if (keyLifeDays < MIN_MK_LIFE_DAYS || keyLifeDays > MAX_MK_LIFE_DAYS) {
            printf("%s\n", "The key validity period is invalid. Try again.");
            COMMLOG(OS_LOG_ERROR, "Input life days of MK is invalid.");
            inputTimes++;
            continue;
        } else {
            break;
        }
    }

    ClearString(strKeyLifeDays);
    if (inputTimes > MAX_FAILED_COUNT) {
        printf("%s\n", "You have input the key validity period for 3 times. The program exits.");
        COMMLOG(OS_LOG_ERROR, "Input invalid life days of MK more than 3 times.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : convert string to mp_uint32
Output       : originStr -- the originstr to convert
Return       : the result of converting string to mp_uint32
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_uint32 RegExtMk::StringToUint32(const mp_string& originStr)
{
    mp_uint32 ul;
    std::stringstream ss(originStr, ios_base::in);
    ss >> ul;
    return ul;
}

/* ------------------------------------------------------------
Description  : Decrypt stored password, and restore it in memory
Return       : MP_SUCCESS -- decrypt all passwd successfully
               MP_FAILED -- decrypt one or more passwd unsuccessfully
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_int32 RegExtMk::DecryptStoredPasswd()
{
    mp_int32 iRet;

    // 1.read encrypted passwd from configure file
    iRet = CConfigXmlParser::GetInstance().GetValueString(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, nginxCiphertext);
    if (iRet != MP_SUCCESS) {
        printf("%s\n", "Getting the nginx ssl key password failed.");
        COMMLOG(OS_LOG_ERROR, "Getting the nginx ssl key password failed.");
        return MP_FAILED;
    }

    // 2. decrypt all passwd
    DecryptStrKMC(nginxCiphertext, nginxKeyPasswd);
    if (nginxKeyPasswd.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Decrypt nginx passwd failed.");
        goto Decrypt_error;
    }

    DecryptStrKMC(snmpPrivateCiphertext, snmpPrivatePasswd);
    if (snmpPrivatePasswd.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Decrypt snmp private passwd failed.");
        goto Decrypt_error;
    }

    DecryptStrKMC(snmpAuthCiphertext, snmpAuthPasswd);
    if (snmpAuthPasswd.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Decrypt snmp auth passwd failed.");
        goto Decrypt_error;
    }

    COMMLOG(OS_LOG_INFO, "Decrypt all password successfully.");
    return MP_SUCCESS;

Decrypt_error:

    SafeClearPasswd();
    return MP_FAILED;
}

/* ------------------------------------------------------------
Description  : re-encrypt all passwd with new MK
Return       : MP_SUCCESS --
               MP_FAILED --
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_int32 RegExtMk::EncryptPasswdWithExtMk()
{
    mp_string nginxKey = "";
    mp_string snmpPrivateKey = "";
    mp_string snmpAuthKey = "";
    mp_int32 iRet;

    // 1. encrypt all passwd
    EncryptStr(nginxKeyPasswd, nginxKey);
    if (nginxKey.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Encrypt nginx passwd failed.");
        goto Encrypt_error;
    }

    EncryptStr(snmpPrivatePasswd, snmpPrivateKey);
    if (snmpPrivateKey.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Encrypt snmp private passwd failed.");
        goto Encrypt_error;
    }

    EncryptStr(snmpAuthPasswd, snmpAuthKey);
    if (snmpAuthKey.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Encrypt snmp auth passwd failed.");
        goto Encrypt_error;
    }

    // 2. write passwd to configure file
    iRet = CConfigXmlParser::GetInstance().SetValue(
        CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, nginxKey);
    if (iRet != MP_SUCCESS) {
        printf("%s\n", "Setting the nginx ssl key password failed.");
        COMMLOG(OS_LOG_ERROR, "Setting the nginx ssl key password failed.");
        goto Encrypt_error;
    }

    COMMLOG(OS_LOG_INFO, "Encrypt password and wirte them into file successfully.");
    return MP_SUCCESS;

Encrypt_error:
    (void)RollbackCiphertext();
    SafeClearPasswd();

    return MP_FAILED;
}

/* ------------------------------------------------------------
Description  : roll back origin passwd (Ciphertext)
Return       : MP_SUCCESS -- roll back success
               MP_FAILED -- roll back failed
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_void RegExtMk::RollbackCiphertext()
{
    mp_int32 iRet = MP_FAILED;
    mp_int32 retryTimes = 0;

    while (retryTimes <= MAX_FAILED_COUNT) {
        iRet = CConfigXmlParser::GetInstance().SetValue(
            CFG_MONITOR_SECTION, CFG_NGINX_SECTION, CFG_SSL_KEY_PASSWORD, nginxCiphertext);
        if (iRet != MP_SUCCESS) {
            printf("%s\n", "Rolling back the nginx ssl key password failed.");
            COMMLOG(OS_LOG_ERROR, "Rolling back the nginx ssl key password failed, retryTimes=%d.", retryTimes);
            retryTimes++;
        } else {
            printf("%s\n", "Rolling back the nginx ssl key password successful.");
            COMMLOG(OS_LOG_INFO, "Rolling back the nginx ssl key password succ.");
            break;
        }
    }
}

mp_void RegExtMk::SafeClearPasswdInner(mp_string& str)
{
    if (!str.empty()) {
        memset_s(&str[0], str.length(), 0, str.length());
    }
}

/* ------------------------------------------------------------
Description  : safe clear passwd info, in case leakage
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_void RegExtMk::SafeClearPasswd()
{
    SafeClearPasswdInner(nginxKeyPasswd);
    SafeClearPasswdInner(snmpPrivatePasswd);
    SafeClearPasswdInner(snmpAuthPasswd);
    SafeClearPasswdInner(nginxCiphertext);
    SafeClearPasswdInner(snmpPrivateCiphertext);
    SafeClearPasswdInner(snmpAuthCiphertext);
}

/* ------------------------------------------------------------
Description  : re-sign all scripts
Return       : MP_SUCCESS -- use new MK to sign all scripts (HMAC)
               MP_FAILED -- sign scripts failed
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_int32 RegExtMk::SignAllScriptsWithNewMK()
{
    COMMLOG(OS_LOG_INFO, "Begin to re-sign all scripts with new MK.");

#ifndef WIN32
    CRootCaller rootCaller;
    if (MP_SUCCESS == rootCaller.Exec((mp_int32)ROOT_COMMAND_SIGN_SCRIPT, "", NULL)) {
        COMMLOG(OS_LOG_INFO, "Re-sign all script success.");
        return MP_SUCCESS;
    }
#else
    if (MP_SUCCESS == SecureCom::GenSignFile()) {
        COMMLOG(OS_LOG_INFO, "Re-sign all script success.");
        return MP_SUCCESS;
    }
#endif

    return MP_FAILED;
}

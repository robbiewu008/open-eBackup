#include "securecom/Password.h"
#include <sstream>
#include <algorithm>
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/Path.h"
#include "securecom/CryptAlg.h"

using namespace std;
namespace {
const mp_uchar PASSSWD_NUM_2 = 2;
}
/* ------------------------------------------------------------
Description  :修改密码
Input        :      eType---密码类型
Return       : MP_SUCCESS---修改成功
                  MP_FAILED---修改失败
------------------------------------------------------------- */
mp_int32 CPassword::ChgPwd(PASSWOD_TYPE eType)
{
    mp_uint32 uiFailedTimes;
    // 从配置文件读取用户名
    mp_string strUsrName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (iRet != MP_SUCCESS) {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }

    // 输入新密码
    uiFailedTimes = 0;
    mp_string strNewPwd;
    while (uiFailedTimes <= MAX_FAILED_COUNT) {
        InputUserPwd(strUsrName, strNewPwd, INPUT_DEFAULT);
        if (CheckNewPwd(eType, strNewPwd)) {
            break;
        }
        
        uiFailedTimes++;
    }
    if (uiFailedTimes > MAX_FAILED_COUNT) {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }

    // 重复输入新密码
    uiFailedTimes = 0;
    mp_string confirmedPwd;
    while (uiFailedTimes <= MAX_FAILED_COUNT) {
        InputUserPwd(strUsrName, confirmedPwd, INPUT_CONFIRM_NEW_PWD);
        if (confirmedPwd == strNewPwd) {
            break;
        }

        uiFailedTimes++;
        if (uiFailedTimes <= MAX_FAILED_COUNT) {
            printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
        }
    }
    if (uiFailedTimes > MAX_FAILED_COUNT) {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }

    // 保存密码
    if (!SaveOtherPwd(eType, strNewPwd)) {
        printf("Save password failed.\n");
        COMMLOG(OS_LOG_ERROR, "Save password failed.");
        return MP_FAILED;
    }

    ClearString(strNewPwd);
    ClearString(confirmedPwd);
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  :修改密码
Input        :      eType---密码类型
Output       :     strPwd---密码
Return       : MP_SUCCESS---修改成功
                  MP_FAILED---修改失败
------------------------------------------------------------- */
mp_int32 CPassword::ChgPwd(PASSWOD_TYPE eType, mp_string& strPwd)
{
    mp_uint32 uiInputFailedTimes;
    // 从配置文件读取用户名
    mp_string strUsrName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (iRet != MP_SUCCESS) {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }

    // 输入新密码
    uiInputFailedTimes = 0;
    mp_string strNewPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT) {
        InputUserPwd(strUsrName, strNewPwd, INPUT_DEFAULT);
        if (CheckNewPwd(eType, strNewPwd)) {
            break;
        }
        
        uiInputFailedTimes++;
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT) {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }

    // 重复输入新密码
    uiInputFailedTimes = 0;
    mp_string strConfirmedPwd;
    while (uiInputFailedTimes <= MAX_FAILED_COUNT) {
        InputUserPwd(strUsrName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD);
        if (strConfirmedPwd == strNewPwd) {
            break;
        }

        uiInputFailedTimes++;
        if (uiInputFailedTimes <= MAX_FAILED_COUNT) {
            printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
        }
    }
    if (uiInputFailedTimes > MAX_FAILED_COUNT) {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }

    strPwd = std::move(strNewPwd);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :修改密码不做长度和密码复杂度验证
Output       :     strPwd---密码
Return       : MP_SUCCESS---修改成功
                  MP_FAILED---修改失败
------------------------------------------------------------- */
mp_int32 CPassword::ChgPwdNoCheck(mp_string& strPwd)
{
    mp_uint32 uiFailedTimes;
    mp_string strUsrName;
    mp_string strNewPwd;

    // 输入新密码
    InputUserPwd(strUsrName, strNewPwd, INPUT_DEFAULT, -1);

    // 重复输入新密码
    uiFailedTimes = 0;
    mp_string strConfirmedPwd;
    while (uiFailedTimes <= MAX_FAILED_COUNT) {
        InputUserPwd(strUsrName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD, -1);
        if (strConfirmedPwd == strNewPwd) {
            break;
        } else {
            uiFailedTimes++;
            if (uiFailedTimes <= MAX_FAILED_COUNT) {
                printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            }
            continue;
        }
    }
    if (uiFailedTimes > MAX_FAILED_COUNT) {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }

    strPwd = std::move(strNewPwd);
    ClearString(strNewPwd);
    ClearString(strConfirmedPwd);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :修改admin密码
Return       : MP_SUCCESS---修改成功
                  MP_FAILED---修改失败
------------------------------------------------------------- */
mp_int32 CPassword::ChgAdminPwd()
{
    // 从配置文件读取用户名
    mp_string strUsrName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (iRet != MP_SUCCESS) {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }

    // 校验当前管理员旧密码
    mp_string strNewPwd;
    iRet = VerifyOldUserPwd(strUsrName);
    if (iRet != MP_SUCCESS) {
        printf("%s\n", OPERATION_LOCKED_HINT);
        CPassword::LockAdmin();
        return MP_FAILED;
    }

    // 输入新密码
    iRet = InputNewUserPwd(strUsrName, strNewPwd);
    if (iRet != MP_SUCCESS) {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }

    // 校验新密码
    iRet = ConfirmNewUserPwd(strUsrName, strNewPwd);
    if (iRet != MP_SUCCESS) {
        printf("Input invalid password over 3 times.\n");
        return MP_FAILED;
    }

    // 保存密码
    if (!SaveAdminPwd(strNewPwd)) {
        printf("Save password failed.\n");
        COMMLOG(OS_LOG_ERROR, "Save password failed.");
        return MP_FAILED;
    }

    ClearString(strNewPwd);
    printf("Password is modified successfully.\n");
    COMMLOG(OS_LOG_INFO, "Password is modified successfully.");
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  :判定旧密码
Input        :       strUserName---用户名
Return       : MP_SUCCESS---密码匹配成功
                  MP_FAILED---密码不匹配
------------------------------------------------------------- */
mp_int32 CPassword::VerifyOldUserPwd(mp_string& strUserName)
{
    mp_string strOldPwd;
    mp_uint32 uiInputFailedTimes = 0;

    while (uiInputFailedTimes <= MAX_FAILED_COUNT) {
        InputUserPwd(strUserName, strOldPwd, INPUT_GET_ADMIN_OLD_PWD);
        if (CheckAdminOldPwd(strOldPwd)) {
            break;
        } else {
            uiInputFailedTimes++;
            continue;
        }
    }

    if (uiInputFailedTimes > MAX_FAILED_COUNT) {
        return MP_FAILED;
    }
    
    ClearString(strOldPwd);
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  :输入 用户新密码
Input        :       strUserName---用户名，strNewPwd---新密码
Return       : MP_SUCCESS---输入成功
                  MP_FAILED---输入失败
------------------------------------------------------------- */
mp_int32 CPassword::InputNewUserPwd(mp_string& strUserName, mp_string& strNewPwd)
{
    mp_uint32 uiInputFailedTimes = 0;

    while (uiInputFailedTimes <= MAX_FAILED_COUNT) {
        InputUserPwd(strUserName, strNewPwd, INPUT_DEFAULT);
        if (CheckNewPwd(PASSWORD_ADMIN, strNewPwd)) {
            break;
        } else {
            uiInputFailedTimes++;
            continue;
        }
    }

    if (uiInputFailedTimes > MAX_FAILED_COUNT) {
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  :确认 用户新密码
Input        : strUserName---用户名，strNewPwd---新密码
Return       : MP_SUCCESS---输入成功
                  MP_FAILED---输入失败
------------------------------------------------------------- */
mp_int32 CPassword::ConfirmNewUserPwd(mp_string& strUserName, mp_string& strNewPwd)
{
    mp_uint32 uiInputFailedTimes = 0;
    mp_string strConfirmedPwd;

    while (uiInputFailedTimes <= MAX_FAILED_COUNT) {
        InputUserPwd(strUserName, strConfirmedPwd, INPUT_CONFIRM_NEW_PWD);
        if (strConfirmedPwd == strNewPwd) {
            break;
        } else {
            uiInputFailedTimes++;
            if (uiInputFailedTimes <= MAX_FAILED_COUNT) {
                printf("%s\n", CHANGE_PASSWORD_NOT_MATCH);
            }
            continue;
        }
    }

    if (uiInputFailedTimes > MAX_FAILED_COUNT) {
        return MP_FAILED;
    }

    ClearString(strConfirmedPwd);
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  :输入用户 密码
Input        : strUserName---用户名，strNewPwd--- 密码，eType---密码类型
------------------------------------------------------------- */
mp_void CPassword::InputUserPwd(const mp_string& strUserName, mp_string& strUserPwd, INPUT_TYPE eType, mp_int32 iPwdLen)
{
    mp_uint32 uiIndex;
    mp_char chTmpPwd[PWD_LENGTH] = {0};
    GetIndexByType(strUserName, eType, uiIndex);

    mp_uchar chPwd = (mp_uchar)GETCHAR;
    mp_uint32 uiLen = uiIndex;
    mp_bool bIsBackSpace;
    mp_bool bOutofLen;
    while ((chPwd & 0xff) != ENTER_SPACE && (chPwd & 0xff) != NEWLINE_SPACE) {
        bIsBackSpace = (((chPwd & 0xff) == BACK_SPACE) && (uiIndex == uiLen));
        if (bIsBackSpace) {
            chPwd = (mp_uchar)GETCHAR;
            continue;
        }

        if ((chPwd & 0xff) == BACK_SPACE) {
            printf("\b \b");
            uiIndex--;
        } else {
            printf(" ");
            // CodeDex误报CSEC_LOOP_ARRAY_CHECKING，数组下标不会越界
            chTmpPwd[uiIndex - uiLen] = static_cast<char>(chPwd);
            uiIndex++;

            bOutofLen = ((iPwdLen > 0) && ((uiIndex - uiLen) > (mp_uint32)(iPwdLen)));
            if (bOutofLen) {
                break;
            }
        }
        chPwd = (mp_uchar)GETCHAR;
    }
    chTmpPwd[uiIndex - uiLen] = '\0';
    printf("\n");

    strUserPwd = chTmpPwd;
    (mp_void) memset_s(chTmpPwd, PWD_LENGTH, 0, PWD_LENGTH);
}

mp_void CPassword::GetIndexByType(const mp_string& strUserName, INPUT_TYPE eType, mp_uint32& uiIndex)
{
    switch (eType) {
        case INPUT_GET_ADMIN_OLD_PWD:
            printf("%s of %s:", INPUT_OLD_PASSWORD, strUserName.c_str());
            uiIndex = mp_string(INPUT_OLD_PASSWORD + strUserName).length() + mp_string(" of ").length();
            break;
        case INPUT_CONFIRM_NEW_PWD:
            printf(CONFIRM_PASSWORD);
            uiIndex = mp_string(CONFIRM_PASSWORD).length();
            break;
        case INPUT_SNMP_OLD_PWD:
            printf(INPUT_SNMP_OLD_PASSWORD);
            uiIndex = mp_string(INPUT_SNMP_OLD_PASSWORD).length();
            break;
        case INPUT_PWD:
            printf(INPUT_PASSWORD);
            uiIndex = mp_string(INPUT_PASSWORD).length();
            break;
        case INPUT_DEFAULT:
        default:
            printf(INPUT_NEW_PASSWORD);
            uiIndex = mp_string(INPUT_NEW_PASSWORD).length();
            break;
    }
}

/* ------------------------------------------------------------
Description  :检查admin旧密码
Input        : strOldPwd---旧密码
Return       :  MP_TRUE---旧密码匹配
                MP_FALSE ---旧密码不匹配
------------------------------------------------------------- */
mp_bool CPassword::CheckAdminOldPwd(const mp_string& strOldPwd)
{
    // 获取配置文件中保存的盐值
    mp_string strSalt;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSalt);
    if (iRet != MP_SUCCESS) {
        printf("Get salt value from xml configuration file failed.\n");
        COMMLOG(OS_LOG_INFO, "Get salt value from xml configuration file failed.");
        return MP_FALSE;
    }

    // 使用sha256获取hash值，保持与老版本兼容
    mp_string outHashHex;
    mp_string strInput = strOldPwd + strSalt;
    iRet = GetSha256Hash(strInput, strInput.length(), outHashHex, SHA256_BLOCK_SIZE + 1);
    ClearString(strInput);
    if (iRet != MP_SUCCESS) {
        printf("Get sha256 hash value failed.\n");
        COMMLOG(OS_LOG_ERROR, "Get sha256 hash value failed.");
        return MP_FAILED;
    }

    // 新版本均采用PBKDF2进行散列
    mp_string strOut;
    iRet = PBKDF2Hash(strOldPwd, strSalt, strOut);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "PBKDF2Hash failed, iRet = %d.", iRet);
        return iRet;
    }

    // 从配置文件中获取老密码的hash值
    mp_string strOldHash;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_HASH_VALUE, strOldHash);
    if (iRet != MP_SUCCESS) {
        printf("Parse xml config failed, key is hash.\n");
        COMMLOG(OS_LOG_ERROR, "Parse xml config failed, key is hash.");
        return MP_FAILED;
    }

    return (strOldHash == outHashHex || strOldHash == strOut) ? MP_TRUE : MP_FALSE;
}

/* ------------------------------------------------------------
Description  : 获取nginx的key
Input        : vecResult -- nginx配置文件内容
Output       : strKey -- key
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_void CPassword::GetNginxKey(mp_string& strKey, const vector<mp_string>& vecResult)
{
    mp_string strTmp;
    mp_string::size_type iPosSSLPwd = mp_string::npos;
    for (mp_uint32 i = 0; i < vecResult.size(); i++) {
        strTmp = vecResult[i];
        iPosSSLPwd = strTmp.find(NGINX_SSL_PWD, 0);
        if (iPosSSLPwd != mp_string::npos) {
            iPosSSLPwd += strlen(NGINX_SSL_PWD.c_str());
            mp_string::size_type iPosSemicolon = strTmp.find(CHAR_SEMICOLON, iPosSSLPwd);
            if (iPosSemicolon != mp_string::npos) {
                strKey = strTmp.substr(iPosSSLPwd, iPosSemicolon - iPosSSLPwd);
                strKey = CMpString::Trim(strKey);
            }
            break;
        }
    }
}

/* ------------------------------------------------------------
Description  : 检查新输入的nginx的key是否和老的一样
Input        : strNewPwd -- 新输入的nginx的key
Return       : MP_TRUE -- 一样
               MP_FALSE -- 不一样
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_bool CPassword::CheckNginxOldPwd(const mp_string& strNewPwd)
{
    mp_string strNginxConfFile = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConfFile)) {
        printf("Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE.c_str());
        COMMLOG(OS_LOG_ERROR, "Nginx config file does not exist, path is \"%s\"", AGENT_NGINX_CONF_FILE.c_str());
        return MP_FALSE;
    }

    vector<mp_string> vecResult;
    mp_int32 iRet = CMpFile::ReadFile(strNginxConfFile, vecResult);
    if (iRet != MP_SUCCESS || vecResult.size() == 0) {
        COMMLOG(OS_LOG_ERROR,
            "Read nginx config file failed, iRet = %d, size of vecResult is %d.",
            iRet,
            vecResult.size());
        return MP_FALSE;
    }

    mp_string strOldPwd = "";
    GetNginxKey(strOldPwd, vecResult);
    mp_string strDecryptPwd = "";
    DecryptStr(strOldPwd, strDecryptPwd);
    iRet = ((strDecryptPwd == strNewPwd) ? MP_TRUE : MP_FALSE);

    ClearString(strDecryptPwd);
    return iRet;
}
/* ------------------------------------------------------------
Description  :检查admin旧密码
Input        : strOldPwd---旧密码
Return       :  MP_TRUE---旧密码匹配
                MP_FALSE ---旧密码不匹配
------------------------------------------------------------- */
mp_bool CPassword::CheckOtherOldPwd(PASSWOD_TYPE eType, const mp_string& strOldPwd)
{
    // 如果是第一次初始化密码，不校验旧密码，返回成功
    if (eType == PASSWORD_INPUT) {
        return MP_SUCCESS;
    }

    if (eType == PASSWORD_NGINX_SSL) {
        return CheckNginxOldPwd(strOldPwd);
    }

    mp_string strCfgValue;
    mp_string strKeyName;
    mp_int32 iRet;
    if (eType == PASSWORD_SNMP_AUTH) {
        strKeyName = CFG_AUTH_PASSWORD;
    } else if (eType == PASSWORD_SNMP_PRIVATE) {
        strKeyName = CFG_PRIVATE_PASSWOD;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SNMP_SECTION, strKeyName, strCfgValue);
    if (iRet != MP_SUCCESS) {
        printf("Get password value from xml config failed.\n");
        COMMLOG(OS_LOG_ERROR, "Get password value from xml config failed.");
        return MP_FALSE;
    }

    mp_string strDecryptPwd = "";
    DecryptStr(strCfgValue, strDecryptPwd);
    iRet = ((strDecryptPwd == strOldPwd) ? MP_TRUE : MP_FALSE);
    ClearString(strDecryptPwd);
    return iRet;
}
/* ------------------------------------------------------------
Description  :检查新密码
Input        :  strNewPwd--- 密码，eType---密码类型
Return       :  bRet---新密码和旧密码相同
                   MP_FALSE---新密码和旧密码不同
------------------------------------------------------------- */
EXTER_ATTACK mp_bool CPassword::CheckNewPwd(PASSWOD_TYPE eType, const mp_string& strNewPwd)
{
    // 检查是否是用户名反转
    mp_string strUserName;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUserName);
    if (iRet != MP_SUCCESS) {
        printf("Get name value from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Get name value from xml configuration file failed.");
        return MP_FALSE;
    }
    mp_string strReverseUserName = strUserName;
    std::reverse(strReverseUserName.begin(), strReverseUserName.end());
    if (strUserName == strNewPwd || strReverseUserName == strNewPwd) {
        printf("Can't use username or reversed username as password.\n");
        return MP_FALSE;
    }

    // 检查是否和老密码相同
    mp_bool bRet = MP_FALSE;
    if (eType == PASSWORD_ADMIN) {
        bRet = CheckAdminOldPwd(strNewPwd);
    } else {
        bRet = CheckOtherOldPwd(eType, strNewPwd);
    }
    if (bRet) {
        printf("New password is equal to old password.\n");
        return MP_FALSE;
    }

    bRet = CheckCommon(strNewPwd);
    mp_bool bFlag = bRet &&
        ((eType == PASSWORD_SNMP_AUTH) || (eType == PASSWORD_SNMP_PRIVATE));
    // 检查是否循环校验，snmp漏洞
    if (bFlag) {
        if (CheckPasswordOverlap(strNewPwd)) {
            return MP_FALSE;
        }
    }

    return bRet;
}

/* ------------------------------------------------------------
Description  :保存admin密码，需要通过salt做hash后保存
Input        :  strPwd--- 密码
Return       :  MP_TRUE---保存成功
                MP_FALSE---保存失败
------------------------------------------------------------- */
mp_bool CPassword::SaveAdminPwd(const mp_string& strPwd)
{
    // 获取盐值
    mp_string strSalt;
    mp_int32 iRet;
    mp_uint64 randNum;

    iRet = GenRandomSalt(strSalt);
    if (iRet != MP_SUCCESS) {
        printf("Get Random salt string failed.\n");
        COMMLOG(OS_LOG_ERROR, "Get Random salt string failed.");
        return MP_FALSE;
    }
   
    mp_string strOut;
    iRet = PBKDF2Hash(strPwd, strSalt, strOut);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "PBKDF2Hash failed, iRet=%d.", iRet);
        return iRet;
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSalt);
    if (iRet != MP_SUCCESS) {
        printf("Set salt value failed.\n");
        COMMLOG(OS_LOG_ERROR, "Set salt value failed.");
        return MP_FALSE;
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_HASH_VALUE, strOut);
    if (iRet != MP_SUCCESS) {
        printf("Save PBKDF2 hash value failed.\n");
        COMMLOG(OS_LOG_ERROR, "Save SHA256 hash value failed.");
        return MP_FALSE;
    }

    return MP_TRUE;
}
/* ------------------------------------------------------------
Description  :保存其他密码
Input        :  strPwd--- 密码
Return       :  MP_TRUE---保存成功
                   MP_FALSE---保存失败
------------------------------------------------------------- */
mp_bool CPassword::SaveOtherPwd(PASSWOD_TYPE eType, const mp_string& strPwd)
{
    mp_string strEncrpytPwd;
    EncryptStr(strPwd, strEncrpytPwd);

    mp_string strKeyName;
    switch (PASSWOD_TYPE(eType)) {
        case PASSWORD_NGINX_SSL: {
            return SaveNginxPwd(strEncrpytPwd);
            break;
        }
        case PASSWORD_SNMP_AUTH: {
            strKeyName = CFG_AUTH_PASSWORD;
            break;
        }
        case PASSWORD_SNMP_PRIVATE: {
            strKeyName = CFG_PRIVATE_PASSWOD;
            break;
        }
        default: return MP_FALSE;
            break;
    }

    mp_int32 iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SNMP_SECTION, strKeyName, strEncrpytPwd);
    if (iRet != MP_SUCCESS) {
        printf("Set value into xml config failed.\n");
        COMMLOG(OS_LOG_ERROR, "Set value into xml config failed.");
        return MP_FALSE;
    }

    return MP_TRUE;
}

/* ------------------------------------------------------------
Description  : 保存新输入的nginx的key
Input        : strPwd -- 新输入的nginx的key
Return       : MP_TRUE -- 保存成功
               MP_FALSE -- 保存失败
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_bool CPassword::SaveNginxPwd(const mp_string& strPwd)
{
    mp_string strNginxConfFile = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConfFile)) {
        printf("Nginx config file does not exist, path is \"%s\".\n", AGENT_NGINX_CONF_FILE.c_str());
        COMMLOG(OS_LOG_ERROR,
            "Nginx config file does not exist, path is \"%s\".\n",
            AGENT_NGINX_CONF_FILE.c_str());
        return MP_FALSE;
    }

    vector<mp_string> vecResult;
    mp_int32 iRet = CMpFile::ReadFile(strNginxConfFile, vecResult);
    if (iRet != MP_SUCCESS || vecResult.size() == 0) {
        COMMLOG(OS_LOG_ERROR,
            "Read nginx config file failed, iRet = %d, size of vecResult is %d.",
            iRet,
            vecResult.size());
        return MP_FALSE;
    }

    mp_string strTmp;
    mp_string::size_type iPosSSLPwd;
    for (mp_uint32 i = 0; i < vecResult.size(); i++) {
        strTmp = vecResult[i];
        iPosSSLPwd = strTmp.find(NGINX_SSL_PWD, 0);
        if (iPosSSLPwd != mp_string::npos) {
            iPosSSLPwd += strlen(NGINX_SSL_PWD.c_str());
            mp_string strInsert = " " + strPwd + STR_SEMICOLON;
            vecResult[i].replace(iPosSSLPwd, strTmp.length() - iPosSSLPwd, strInsert);
            break;
        }
    }

    iRet = CIPCFile::WriteFile(strNginxConfFile, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Write nginx config file failed, iRet = %d, size of vecResult is %d.",
            iRet,
            vecResult.size());
        return MP_FALSE;
    }

    return MP_TRUE;
}
/* ------------------------------------------------------------
Description  :计算密码复杂度
Input        :  strPwd--- 密码
Output       : iNum---数字，iUppercase---大写字母，iLowcase---输入的小写字母，iSpecial---特殊字符
Return       :  MP_TRUE---保存成功
                   MP_FALSE---保存失败
------------------------------------------------------------- */
mp_int32 CPassword::CalComplexity(
    const mp_string& strPwd, mp_int32& iNum, mp_int32& iUppercase, mp_int32& iLowcase, mp_int32& iSpecial)
{
    static const mp_string PWD_REX  = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz`"
                                    "~!@#$%^&*()-_=+\\|[{}];:'\",<.>/?";
    static const mp_string SPECIAL_REX = "`~!@#$%^&*()-_=+\\|[{}];:'\",<.>/?";
    for (mp_uint32 uindex = 0; uindex < strPwd.length(); uindex++) {
        if (string::npos == PWD_REX.find(strPwd[uindex])) {
            printf("%s", WRONGPWD_HINT);
            return MP_FAILED;
        }

        mp_bool bFlag = strPwd[uindex] >= '0' && strPwd[uindex] <= '9';
        if (bFlag) {
            iNum++;
        }

        bFlag = strPwd[uindex] >= 'A' && strPwd[uindex] <= 'Z';
        if (bFlag) {
            iUppercase++;
        }

        bFlag = strPwd[uindex] >= 'a' && strPwd[uindex] <= 'z';
        if (bFlag) {
            iLowcase++;
        }
        if (string::npos != SPECIAL_REX.find(strPwd[uindex])) {
            iSpecial++;
        }
    }

    if (iSpecial == 0) {
        printf("%s", WRONGPWD_HINT);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  :CheckCommon功能
Input        :  strPwd--- 密码
Return       :  MP_TRUE---密码符合要求
                   MP_FALSE---密码简单
------------------------------------------------------------- */
mp_bool CPassword::CheckCommon(const mp_string& strPwd)
{
    // 长度检查
    if (strPwd.length() < PWD_MIN_LEN || strPwd.length() > PWD_MAX_LEN) {
        printf("%s", WRONGPWD_HINT);
        return MP_FALSE;
    }

    mp_int32 iNum = 0;
    mp_int32 iUppercase = 0;
    mp_int32 iLowcase = 0;
    mp_int32 iSpecial = 0;
    if (CalComplexity(strPwd, iNum, iUppercase, iLowcase, iSpecial) != MP_SUCCESS) {
        return MP_FALSE;
    }

    mp_int32 iComplex = CalculateComplexity(iNum, iUppercase, iLowcase);
    if (iComplex < PASSSWD_NUM_2) {
        printf("%s", WRONGPWD_HINT);
        return MP_FALSE;
    }

    return MP_TRUE;
}
/* ------------------------------------------------------------
Description  :计算复杂度
Input        :  strPwd--- 密码
Output       :  iNum---数字，iUppercase---大写字母，iLowcase---输入的小写字母
Return       :   iComplex---复杂度值
------------------------------------------------------------- */
mp_int32 CPassword::CalculateComplexity(mp_int32 iNumber, mp_int32 iUppercase, mp_int32 iLowcase)
{
    mp_int32 iComplex = 0;
    if (iNumber > 0) {
        iComplex++;
    }
    if (iUppercase > 0) {
        iComplex++;
    }
    if (iLowcase > 0) {
        iComplex++;
    }
    return iComplex;
}
/* ------------------------------------------------------------
Description  :检查密码循环重叠
Input        :  strPasswd--- 密码
Return       :  MP_TRUE--- 不设置密码
                   MP_FALSE--- 继续操作
------------------------------------------------------------- */
mp_bool CPassword::CheckPasswordOverlap(const mp_string& strPasswd)
{
    mp_uint32 uiIndex = 1;
    mp_uint32 uiMaxLen = strPasswd.length() / PASSSWD_NUM_2;
    if (uiMaxLen == 0) {
        return MP_FALSE;
    }
    for (; uiIndex <= uiMaxLen; uiIndex++) {
        // 如果此时长度不能被总长度整除，说明不会有循环模式，直接返回false
        mp_uint32 iRemainNum = strPasswd.length() % uiIndex;
        if (iRemainNum != 0) {
            continue;
        }

        mp_string strMeta = strPasswd.substr(0, uiIndex);
        mp_uint32 subIndex = uiIndex;
        mp_bool bFlag = MP_TRUE;  // 全部循环标识符号
        for (; subIndex <= (strPasswd.length() - uiIndex); subIndex += uiIndex) {
            mp_string strTmp = strPasswd.substr(subIndex, uiIndex);
            if (strTmp != strMeta) {
                bFlag = MP_FALSE;
                break;
            }
        }

        if (bFlag) {
            // 如果全部匹配，则说明是循环字符串
            // 如果是循环的密码，则不符合要求(SNMP漏洞)
            printf("%s", PASSWORD_NOT_SAFE);
            printf("%s", CONTINUE);
            mp_int32 iChoice = getchar();
            if (iChoice != 'y' && iChoice != 'Y') {
                // 如果不选择是，则用户认为这个密码是不想设置的，故需要返回是循环的
                return MP_TRUE;
            }
            // 如果用户选择继续操作，则认为用户接受了该循环，故代码后续判断该密码符合要求
            return MP_FALSE;
        }
    }

    return MP_FALSE;
}

/* ------------------------------------------------------------
Description  : 读入用户操作
Return       : MP_SUCCESS -- 成功
------------------------------------------------------------- */
mp_void CPassword::GetInput(const mp_string& strHint, mp_string& strInput, mp_int32 iInputLen)
{
    mp_char chTmpInput[PWD_LENGTH] = {0};
    mp_uchar ch = (mp_uchar)GETCHAR;
    mp_uint32 iIndex = strHint.length();
    mp_uint32 iLen = iIndex;
    while ((ch & 0xff) != ENTER_SPACE && (ch & 0xff) != NEWLINE_SPACE) {
        if (((ch & 0xff) == BACK_SPACE) && (iIndex == iLen)) {
            ch = (mp_uchar)GETCHAR;
            continue;
        }

        if ((ch & 0xff) == BACK_SPACE) {
            printf("\b \b");
            iIndex--;
        } else {
            // 支持不限制长度输入的情况
            if ((iInputLen > 0) && ((iIndex - iLen) == iInputLen)) {
                ch = (mp_uchar)GETCHAR;
                continue;
            }
            // CodeDex误报，CSEC_LOOP_ARRAY_CHECKING，数组下标不会越界
            chTmpInput[iIndex - iLen] = static_cast<char>(ch);
            printf("%c", ch);
            iIndex++;
        }
        ch = (mp_uchar)GETCHAR;
    }
    chTmpInput[iIndex - iLen] = '\0';
    strInput = chTmpInput;
    printf("\n");
}

/* ------------------------------------------------------------
Description  : get user input, and echo each char with a star '*'
------------------------------------------------------------- */
mp_void CPassword::GetInputEchoWithStar(const mp_string& strHint, mp_string& strInput, mp_int32 iInputLen)
{
    mp_char chTmpInput[PWD_LENGTH] = {0};
    mp_uchar ch = (mp_uchar)GETCHAR;
    mp_uint32 idx = strHint.length();
    mp_uint32 iLen = idx;
    while ((ch & 0xff) != ENTER_SPACE && (ch & 0xff) != NEWLINE_SPACE) {
        if (((ch & 0xff) == BACK_SPACE) && (idx == iLen)) {
            ch = (mp_uchar)GETCHAR;
            continue;
        }

        if ((ch & 0xff) == BACK_SPACE) {
            printf("\b \b");
            idx--;
        } else {
            // 支持不限制长度输入的情况
            if ((iInputLen > 0) && ((idx - iLen) == iInputLen)) {
                ch = (mp_uchar)GETCHAR;
                continue;
            }
            // CodeDex误报，CSEC_LOOP_ARRAY_CHECKING，数组下标不会越界
            chTmpInput[idx - iLen] = static_cast<char>(ch);
            printf("%c", '*');
            idx++;
        }
        ch = (mp_uchar)GETCHAR;
    }
    chTmpInput[idx - iLen] = '\0';
    strInput = chTmpInput;
    printf("\n");
}

/* ------------------------------------------------------------
Description  :  锁定用户
------------------------------------------------------------- */
mp_void CPassword::LockAdmin()
{
    mp_uint64 secTime = CMpTime::GetTimeSec();
    ostringstream oss;
    oss << secTime;
    mp_string strTime = oss.str();
    vector<mp_string> vecInput;
    vecInput.emplace_back(strTime);
    mp_string strPath = CPath::GetInstance().GetTmpFilePath(LOCK_ADMIN_FILE);
    mp_int32 iRet = CIPCFile::WriteFile(strPath, vecInput);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "WriteFile failed, iRet = %d", iRet);
    } else {
        COMMLOG(OS_LOG_INFO, "agentcli is locked.");
    }
    return;
}

/* ------------------------------------------------------------
Description  :  获取操作锁定时间
Input        :  无
------------------------------------------------------------- */
mp_uint64 CPassword::GetLockTime()
{
    // CodeDex误报，Type Mismatch:Signed to Unsigned
    vector<mp_string> vecOutput;
    mp_string strPath = CPath::GetInstance().GetTmpFilePath(LOCK_ADMIN_FILE);
    mp_int32 iRet = CMpFile::ReadFile(strPath, vecOutput);
    if (iRet != MP_SUCCESS || vecOutput.size() == 0) {
        COMMLOG(OS_LOG_INFO, "iRet = %d, vecOutput.size = %d", iRet, vecOutput.size());
        return 0;
    }

    return (mp_uint64)(atol(vecOutput.front().c_str()));
}

/* ------------------------------------------------------------
Description  :  清除锁定信息
------------------------------------------------------------- */
mp_void CPassword::ClearLock()
{
    mp_string strPath = CPath::GetInstance().GetTmpFilePath(LOCK_ADMIN_FILE);
    if (CMpFile::FileExist(strPath)) {
        COMMLOG(OS_LOG_INFO, "agentcli is unlocked.");
    }
    mp_int32 iRet = CMpFile::DelFile(strPath);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "DelFile failed, iRet = %d", iRet);
    }
    return;
}

/* ------------------------------------------------------------
Description  :  加密密码
------------------------------------------------------------- */
mp_int32 CPassword::EncPwd(mp_string& ciphertext, const mp_string& strvalue)
{
    COMMLOG(OS_LOG_INFO, "Begin encryption pwd.");
    mp_string pwd;
    if (strvalue == "") {
    InputUserPwd("", pwd, INPUT_PWD);
    } else {
        pwd = strvalue;
    }
    ciphertext = "";
    EncryptStr(pwd, ciphertext);
    ClearString(pwd);
    if (ciphertext == "") {
        COMMLOG(OS_LOG_ERROR, "Encryption failed.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :  验证agent用户名密码
Input        :  None
Output       :  strUsrName: agent用户名
Return       :  0: 验证成功, 其他: 验证失败
------------------------------------------------------------- */
mp_int32 CPassword::VerifyAgentUser(mp_string& strUsrName)
{
    strUsrName = "";

    // 从配置文件读取用户名
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_USER_NAME, strUsrName);
    if (iRet != MP_SUCCESS) {
        printf("Get user name from xml configuration file failed.\n");
        COMMLOG(OS_LOG_ERROR, "Get user name from xml configuration file failed.");
        return MP_FAILED;
    }
    mp_string strOldPwd;
    mp_uint32 iInputFailedTimes = 0;
    while (iInputFailedTimes <= MAX_FAILED_COUNT) {
        CPassword::InputUserPwd(strUsrName, strOldPwd, INPUT_GET_ADMIN_OLD_PWD);
        if (CPassword::CheckAdminOldPwd(strOldPwd)) {
            break;
        } else {
            iInputFailedTimes++;
            ClearString(strOldPwd);
            continue;
        }
    }

    ClearString(strOldPwd);
    if (iInputFailedTimes > MAX_FAILED_COUNT) {
        printf("%s.\n", OPERATION_LOCKED_HINT);
        COMMLOG(OS_LOG_ERROR, "The auth failed.");
        CPassword::LockAdmin();
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "The auth success.");
    return MP_SUCCESS;
}

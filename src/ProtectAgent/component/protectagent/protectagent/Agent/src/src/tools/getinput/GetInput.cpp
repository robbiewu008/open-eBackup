/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file GetInput.cpp
 * @brief  Contains function declarations Get input Operations
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include <sstream>
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common/Path.h"
#include "securecom/Password.h"
#include "securecom/CryptAlg.h"
#include "common/Utils.h"
#include "securecom/UniqueId.h"

using std::vector;
using std::ostringstream;
namespace {
const mp_string USER_PASSWORD_KEY = "userpwd";
const mp_uchar GETINPUT_NUM_2 = 2;

/* ------------------------------------------------------------
Function Name: CheckPasswdFromFile
Description  : 推送安装，从文件中读取密码,并校验
Return       : MP_SUCCESS 成功
                   MP_FAILED 失败
------------------------------------------------------------ */
mp_int32 CheckPasswdFromFile(mp_string& strPwd, const mp_string& strPwdConf)
{
    vector<mp_string> vecOutput;
    vector<mp_string>::iterator iter;
    mp_string::size_type spos = 0;

    mp_int32 iRet = CMpFile::ReadFile(strPwdConf, vecOutput);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Read info from the conf file failed, Ret is %d.", MP_FAILED);

        return MP_FAILED;
    }
    // CodeDex误报,KLOCWORK.NPD.FUNC.MUST
    for (iter = vecOutput.begin(); iter != vecOutput.end(); ++iter) {
        spos = (*iter).find('=');
        if (mp_string::npos != spos) {
            mp_string strKey = CMpString::Trim(iter->substr(0, spos));
            if (USER_PASSWORD_KEY == strKey) {
                strPwd = CMpString::Trim(iter->substr(spos + 1));
                break;
            }
        }
    }

    mp_bool bRet = CPassword::CheckCommon(strPwd);
    if (!bRet) {
        COMMLOG(OS_LOG_ERROR, "Check passwd rules failed, Ret is %d.", MP_FAILED);

        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 EncryptInput(mp_string& strPwd)
{
    mp_string strSalt;
    mp_int32 iRet = GenRandomSalt(strSalt);
    if (iRet != MP_SUCCESS) {
        printf("Get Random salt failed.");
        return MP_FAILED;
    }

    mp_string strOut;
    iRet = PBKDF2Hash(strPwd, strSalt, strOut);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "PBKDF2Hash exec failed, iRet=%d.", iRet);
        return iRet;
    }

    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_SALT_VALUE, strSalt);
    if (iRet != MP_SUCCESS) {
        printf("Save encrypt salt value failed.\n");
        return MP_FAILED;
    }

    vector<mp_string> vecInput;
    vecInput.push_back(strOut);
    mp_string strEnFilePath = CPath::GetInstance().GetTmpFilePath(EN_TMP_FILE);
    // 写入临时文件
    iRet = CIPCFile::WriteFile(strEnFilePath, vecInput);
    if (iRet != MP_SUCCESS) {
        printf("Write file failed.\n");
    }
#ifndef WIN32
    (mp_void) ChangeGmonDir();  // change profile out put dir
#endif
    return iRet;
}
}

/* ------------------------------------------------------------
Function Name: main
Description  : getinput进程主函数
------------------------------------------------------------- */
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_string strPwd, strPwdConf;

    // 初始化agentcli路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (iRet != MP_SUCCESS) {
        printf("Init getinput path failed.\n");
        return iRet;
    }

    // 初始化配置文件模块
    iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF));
    if (iRet != MP_SUCCESS) {
        printf("Init conf file %s failed.\n", AGENT_XML_CONF.c_str());
        return iRet;
    }

    // 初始化日志模块
    CLogger::GetInstance().Init(GET_INPUT_LOG_NAME.c_str(), CPath::GetInstance().GetLogPath());

    // 校验密码
    if (argc == GETINPUT_NUM_2) {
        strPwdConf = argv[1];

        iRet = CheckPasswdFromFile(strPwd, strPwdConf);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Check passwd from conf failed, Ret is %d.", MP_FAILED);
            return MP_FAILED;
        }
    } else if (argc == 1) {
        iRet = CPassword::ChgPwd(PASSWORD_INPUT, strPwd);
        if (iRet != MP_SUCCESS) {
            printf("Set password failed.\n");
            return iRet;
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "Input param is error, Ret is %d.", MP_FAILED);
        return MP_FAILED;
    }

    // 加密
    iRet = EncryptInput(strPwd);
    ClearString(strPwd);
    ClearString(strPwdConf);
    return iRet;
}

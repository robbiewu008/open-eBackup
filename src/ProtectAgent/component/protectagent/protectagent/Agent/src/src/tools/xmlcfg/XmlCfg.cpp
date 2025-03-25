/**
/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file XmlCfg.cpp
 * @brief  The implemention about xml config operations
 * @version 1.0.0.0
 * @date 2015-02-06
 * @author wangguitao 00510599
 */
#include "common/Path.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/ConfigXmlParse.h"
#include "common/Utils.h"
namespace {
const mp_string WRITE_CMD = "write";
const mp_string READ_CMD  = "read";
const mp_uchar XMLCFG_NUM_2 = 2;
const mp_uchar XMLCFG_NUM_3 = 3;
const mp_uchar XMLCFG_NUM_4 = 4;
const mp_uchar XMLCFG_NUM_5 = 5;
const mp_uchar XMLCFG_NUM_6 = 6;

mp_int32 HandleCmd(mp_int32 argc, mp_char** argv)
{
    mp_string strCmd = argv[1];
    mp_string strSectionName = argv[XMLCFG_NUM_2];
    mp_int32 iRet = MP_FAILED;
    if (strcmp(strCmd.c_str(), WRITE_CMD.c_str()) == 0) {
        if (argc == XMLCFG_NUM_5) {
            mp_string strKey = argv[XMLCFG_NUM_3];
            mp_string strValue = argv[XMLCFG_NUM_4];
            iRet = CConfigXmlParser::GetInstance().SetValue(strSectionName, strKey, strValue);
        } else { // 参数个数6
            mp_string strChildSectionName = argv[XMLCFG_NUM_3];
            mp_string strKey = argv[XMLCFG_NUM_4];
            mp_string strValue = argv[XMLCFG_NUM_5];
            iRet = CConfigXmlParser::GetInstance().SetValue(strSectionName, strChildSectionName, strKey, strValue);
        }
    } else { // 读命令
        mp_string strValue;
        if (argc == XMLCFG_NUM_4) {
            mp_string strKey = argv[XMLCFG_NUM_3];
            iRet = CConfigXmlParser::GetInstance().GetValueString(strSectionName, strKey, strValue);
        } else { // 参数个数5
            mp_string strChildSectionName = argv[XMLCFG_NUM_3];
            mp_string strKey = argv[XMLCFG_NUM_4];
            iRet =
                CConfigXmlParser::GetInstance().GetValueString(strSectionName, strChildSectionName, strKey, strValue);
        }

        if (iRet == MP_SUCCESS) {
            printf("%s\n", strValue.c_str());
        }
    }

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "XmlCfg failed, iRet = %d.", iRet);
    }
    return iRet;
}
}

/* ------------------------------------------------------------
Description  : xmlcfg主函数
------------------------------------------------------------- */
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_bool bFlag = (argc != XMLCFG_NUM_4 && argc != XMLCFG_NUM_5 && argc != XMLCFG_NUM_6) ||
                    (strcmp(argv[1], READ_CMD.c_str()) != 0 && strcmp(argv[1], WRITE_CMD.c_str()) != 0);
    if (bFlag) {
        printf("Usage: [path]xmlcfg write|read section [child_section] key [value]\n");
        return MP_FAILED;
    }

    bFlag = (0 == strcmp(argv[1], READ_CMD.c_str()) && argc == XMLCFG_NUM_6) ||
            (0 == strcmp(argv[1], WRITE_CMD.c_str()) && argc == XMLCFG_NUM_4);
    if (bFlag) {
        printf("Usage: [path]xmlcfg write|read section [child_section] key [value]\n");
        return MP_FAILED;
    }

    // 初始化XmlCfg路径
    mp_int32 iRet = CPath::GetInstance().Init(argv[0]);
    if (iRet != MP_SUCCESS) {
        printf("Init xmlcfg path failed.\n");
        return iRet;
    }

    // 初始化配置文件模块
    iRet = CConfigXmlParser::GetInstance().Init(CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF));
    if (iRet != MP_SUCCESS) {
        printf("Init conf file %s failed.\n", AGENT_XML_CONF.c_str());
        return iRet;
    }

    // 初始化日志模块
    CLogger::GetInstance().Init(XML_CFG_LOG_NAME.c_str(), CPath::GetInstance().GetLogPath());

    iRet = HandleCmd(argc, argv);
#ifndef WIN32
    (mp_void) ChangeGmonDir();  // change profile out put dir
#endif
    return iRet;
}

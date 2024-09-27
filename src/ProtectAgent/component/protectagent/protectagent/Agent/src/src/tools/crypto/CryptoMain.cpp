#include <iostream>
#include <sstream>
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "common/Types.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "securecom/UniqueId.h"
#include "securec.h"
#include "securecom/CryptAlg.h"
using namespace std;
#ifdef WIN32
const mp_uchar CRYPTOMAIN_NUM_2 = 2;
#endif
namespace {
/* ------------------------------------------------------------
Function Name: IsRunManually
Description  : 判断是否是自启动
-------------------------------------------------------- */
mp_bool IsRunManually()
{
#ifdef WIN32
    return MP_FALSE;
#else
    pid_t myPid;
    pid_t myGid;

    myPid = getpid();
    myGid = getpgrp();
    if (myPid == myGid) {
        return MP_TRUE;
    }

    return MP_FALSE;
#endif  // WIN32
}

/* ------------------------------------------------------------
Function Name: GenSeconds
Description  : 获取从今天0时开始的描述
-------------------------------------------------------- */
mp_int32 GenSeconds()
{
    time_t now;
    (mp_void)time(&now);
    struct tm* pTime = localtime(&now);
    if (pTime == nullptr) {
        COMMLOG(OS_LOG_ERROR, "Get localtime failed.");
        return MP_FAILED;
    }

    struct tm beginTime = *pTime;
    beginTime.tm_hour = 0;
    beginTime.tm_min = 0;
    beginTime.tm_sec = 0;
    beginTime.tm_mon = 0;
    beginTime.tm_mday = 1;

    double seconds = difftime(now, mktime(&beginTime));
    printf("%.f\n", seconds);

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GenSALT
Description  : 计算随机数盐值
-------------------------------------------------------- */
mp_int32 GenSALT()
{
    mp_int32 iRet;
    mp_string strSalt;
    // 生成随机数的SALT值
    iRet = GenRandomSalt(strSalt);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "get Random salt string failed.");
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 ProcessCryptoAfter(mp_int32 iAlg, const mp_string& strSalt, mp_string& strInput)
{
    mp_int32 iRet;
    mp_string strOutPut;
    switch (CRYPT_ALG(iAlg)) {
        case CRYPT_ENCYP_AES: {
            iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
            if (iRet != MP_SUCCESS) {
                printf("Init crypt failed.\n");
                return iRet;
            }
            EncryptStr(strInput, strOutPut);
            printf("%s\n", strOutPut.c_str());
            (mp_void)FinalizeCrypt();
            break;
        }
        case CRYPT_DECYP_AES: {
            iRet = InitCrypt(KMC_ROLE_TYPE_MASTER);
            if (iRet != MP_SUCCESS) {
                printf("Init crypt failed.\n");
                return iRet;
            }
            DecryptStr(strInput, strOutPut);
            printf("%s\n", strOutPut.c_str());
            (mp_void)FinalizeCrypt();
            break;
        }
        case CRYPT_PBKDF2: {
            mp_string strOut;
            iRet = PBKDF2Hash(strInput, strSalt, strOut);
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "PBKDF2Hash failed, iRet = %d.", iRet);
                return iRet;
            }
            printf("%s\n", strOut.c_str());
            break;
        }
        default: {
            COMMLOG(OS_LOG_ERROR, "algorithm [%d] not found.", iAlg);
            break;
        }
    }
    ClearString(strOutPut);

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: ProcessCrypto
Description  : 根据输入参数进行加解密处理
-------------------------------------------------------- */
mp_int32 ProcessCrypto(mp_int32 iAlg, mp_string& strFilePath, const mp_string& strSalt)
{
    if (iAlg == CRYPT_SECOND) {
        return GenSeconds();
    }

    if (iAlg == CRYPT_SALT) {
        return GenSALT();
    }

    vector<mp_string> vecResult;
    mp_int32 iRet = CIPCFile::ReadFile(strFilePath, vecResult);

    mp_bool bReadFile = (iRet != MP_SUCCESS || vecResult.size() == 0);
    if (bReadFile) {
        COMMLOG(OS_LOG_ERROR,
            "Read file failed, iRet = %d, size of vecResult is %d.",
            iRet,
            vecResult.size());
        return iRet;
    }
    // CodeDex误报,KLOCWORK.ITER.END.DEREF.MIGHT
    vector<mp_string>::iterator it = vecResult.begin();
    mp_string strInput = *it;
    ++it;
    for (; it != vecResult.end(); ++it) {
        // 补充结尾的换行符号
        strInput = strInput + "\n" + *it;
    }

    return ProcessCryptoAfter(iAlg, strSalt, strInput);
}

mp_int32 GetArgs(mp_int32 argc, mp_char** argv, mp_int32& iAlg, mp_string& strFilePath, mp_string& strSalt)
{
#ifdef WIN32
    if (argc > 1) {
        for (mp_uint32 i = 1; i < argc; i = i + CRYPTOMAIN_NUM_2) {
            if (i + 1 == argc) {
                iAlg = MP_FAILED;
                break;
            }
            mp_char* pszOp = argv[i];
            mp_char* pszOpValue = argv[i + 1];
            if (_stricmp(pszOp, "-a") == 0) {
                iAlg = atoi(pszOpValue);
            } else if (_stricmp(pszOp, "-i") == 0) {
                strFilePath = pszOpValue;
            } else if (_stricmp(pszOp, "-s") == 0) {
                strSalt = pszOpValue;
            } else {
                COMMLOG(OS_LOG_ERROR, "Invalid input param %s.", pszOp);
            }
        }
    }
#else
    mp_string pszOptString = "r:a:i:s:";
    mp_int32 iOpt = getopt(argc, argv, pszOptString.c_str());
    while (-1 != iOpt) {
        switch (iOpt) {
            case 'a':
                iAlg = atoi(optarg);
                break;
            case 'i':
                strFilePath = optarg;
                break;
            case 's':
                strSalt = optarg;
                break;
            default:
                return MP_FAILED;
        }

        iOpt = getopt(argc, argv, pszOptString.c_str());
    }
#endif  // WIN32

    return MP_SUCCESS;
}
}

/* ------------------------------------------------------------
Function Name: main函数
               Usage: crypto [-a <0|1|2>] [-i <inputFile>]);
               -a <0|1|2|3>    加密算法，枚举值，参考CRYPT_ALG的定义，必选参数
               -i <inputFile>  输入带加解密文件的路径，当加密算法是aes和sha时必选;
-------------------------------------------------------- */
mp_int32 main(mp_int32 argc, mp_char** argv)
{
    mp_int32 iAlg = MP_FAILED;
    mp_string strFilePath;
    mp_string strSalt;

    // CodeDex误报，Portability Flaw
    mp_int32 iRet = GetArgs(argc, argv, iAlg, strFilePath, strSalt);
    if (iRet != MP_SUCCESS) {
        std::cout <<"Get args failed.\n";
        return iRet;
    }

    if (iAlg == MP_FAILED) {
        printf("crypto [-a <Alg_Type>] [-i <inputFile>] [-s <Salt_Value>]\n");
        return MP_FAILED;
    }

    // 初始化路径
    iRet = CPath::GetInstance().Init(argv[0]);
    if (iRet != MP_SUCCESS) {
        printf("Init path %s failed.\n", argv[0]);
        return iRet;
    }

    // 初始化xml配置
    mp_string strXMLConfFilePath = CPath::GetInstance().GetConfFilePath(AGENT_XML_CONF);
    iRet = CConfigXmlParser::GetInstance().Init(strXMLConfFilePath);
    if (iRet != MP_SUCCESS) {
        printf("Init xml conf file %s failed.\n", AGENT_XML_CONF.c_str());
        return iRet;
    }

    // 初始化日志文件
    mp_string strLogFilePath = CPath::GetInstance().GetLogPath();
    CLogger::GetInstance().Init(CRYPTO_LOG_NAME.c_str(), strLogFilePath);

    COMMLOG(OS_LOG_INFO, "Alg = %d, input file path = %s", iAlg, strFilePath.c_str());

    iRet = ProcessCrypto(iAlg, strFilePath, strSalt);
#ifndef WIN32
    (mp_void) ChangeGmonDir();  // change profile out put dir
#endif
    return iRet;
}

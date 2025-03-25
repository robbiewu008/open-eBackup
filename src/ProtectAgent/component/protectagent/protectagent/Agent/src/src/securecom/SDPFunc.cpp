/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file SDPFunc.cpp
 * @brief  The implemention about SDP FUN
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "securecom/SDPFunc.h"
#include <string>
#include <sstream>
#include <iomanip>
#include "common/Log.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/ConfigXmlParse.h"
#include "include/wsecv2_type.h"
#include "wsecv2_itf.h"
#include "sdp/sdpv2_itf.h"
#include "sdp/sdpv1_itf.h"
#include "include/kmcv2_itf.h"
#include "securec.h"
#include "include/wsecv2_errorcode.h"
#include "securecom/KmcCallback.h"
#include "cac/cacv2_pri.h"
#include "common/wsecv2_order.h"

using namespace std;
namespace {
bool g_bKMCInitialized = false;
mp_uint32 g_domainId = 0;
const mp_uchar SDP_FUNC_NUM_2 = 2;
const mp_uchar SDP_FUNC_NUM_100 = 100;
//
const int SIGNED_NUM_NEGATIVE6 = -6;
const int NUM4 = 4;
const int NUM6 = 6;
const int NUM8 = 8;
const int NUM64 = 64;
const int NUM256 = 256;
const mp_int32 AGENT_INSTALL_TYPE_INTERNAL = 1;
const mp_string base64_arr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const std::string STORE_FILE_PATH = "/opt/logpath/protectmanager/kmc/master.ks";
const std::string STORE_BAK_FILE_PATH = "/kmc_conf/..data/backup.ks";
mp_int32 g_installType = 0;
}  // namespace

#define CHECK_IF_TRUE_LOG_AND_BREAK(express, logContent)                                                               \
    {                                                                                                                  \
        if (express) {                                                                                                 \
            COMMLOG(OS_LOG_ERROR, logContent);                                                                         \
            break;                                                                                                     \
        }                                                                                                              \
    }
#define CHECK_IF_TRUE_LOG_AND_RETURN(express, logContent)                                                              \
    {                                                                                                                  \
        if (express) {                                                                                                 \
            COMMLOG(OS_LOG_ERROR, logContent);                                                                         \
            return MP_FAILED;                                                                                          \
        }                                                                                                              \
    }
/* ------------------------------------------------------------
Description  : 将字符串转换为16进制字符串
Input        : strIn -- 输入字符串内存指针
               inLen -- 输入字符串内存长度
Output       : hexOut -- 16进制字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 StrToHex(const mp_uchar strIn[], mp_string& hexOut, mp_uint32 inLen)
{
    hexOut = "";
    if (inLen == 0) {
        COMMLOG(OS_LOG_ERROR, "Convert str to Hex str failed, strIn is Null or inlen equal 0.");
        return MP_FAILED;
    }
    ostringstream oss;
    oss << hex;
    for (mp_uint32 i = 0; i < inLen; i++) {
        oss << setw(SDP_FUNC_NUM_2) << setfill('0') << (mp_int32)strIn[i];
    }
    hexOut = oss.str();
    if (hexOut.size() != (inLen * SDP_FUNC_NUM_2)) {
        hexOut = "";
        COMMLOG(OS_LOG_ERROR, "length of out string is incorrect.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 将字符串转换为16进制字符串
Input        : hexIn -- 16进制字符串
               strOut -- 输出字符串内存指针
               outLen -- 输出字符串内存长度
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 HexToStr(const mp_string& hexIn, mp_uchar* strOut, mp_uint32 outLen)
{
    mp_uint32 inLen = hexIn.length();
    if (inLen == 0 || inLen % SDP_FUNC_NUM_2 == 1) {
        return MP_FAILED;
    }
    if (strOut == NULL) {
        return MP_FAILED;
    }
    inLen /= SDP_FUNC_NUM_2;
    if (outLen < inLen) {
        return MP_FAILED;
    }
    // CodeDex误报，Buffer Overflow
    mp_int32 ret = memset_s(strOut, outLen, 0, inLen);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret = %d.", ret);
        return MP_FAILED;
    }
    stringstream ss;
    mp_int32 tmp;
    ss << hex;
    for (mp_uint32 i = 0; i < inLen; i++) {
        ss << hexIn.substr(i * SDP_FUNC_NUM_2, SDP_FUNC_NUM_2);
        tmp = -1;
        ss >> tmp;
        if (tmp == -1) {
            return MP_FAILED;
        }
        strOut[i] = (mp_uchar)tmp;
        ss.str("");
        ss.clear();
    }

    return MP_SUCCESS;
}

mp_void AppRecvNotifyKmcNew(WsecUint32 eNtfCode)
{
    switch (eNtfCode) {
        case WSEC_KMC_NTF_KEY_STORE_CORRUPT:
            COMMLOG(OS_LOG_ERROR, "The Keystore is damaged.");
            break;
        case WSEC_KMC_NTF_CFG_FILE_CORRUPT:
            COMMLOG(OS_LOG_ERROR, "The KMC configuration file is damaged.");
            break;
        case WSEC_KMC_NTF_MK_NUM_OVERFLOW:
            COMMLOG(OS_LOG_ERROR, "The number of MKs is about to exceed the upper limit.");
            break;
        case WSEC_KMC_NTF_ONE_KSF_CORRUPT:
            COMMLOG(OS_LOG_ERROR, "A keystore file of the KMC is damaged and fails to be restored.");
            break;
        case WSEC_KMC_NTF_KSF_INITOPENFAIL:
            COMMLOG(OS_LOG_ERROR, "Two Files Cannot Be Opened When the Keystore Is Loaded.");
            break;
        case WSEC_KMC_NTF_THIRD_KSF_UPDATE:
            COMMLOG(OS_LOG_INFO, "The third backup KSF update  success notify.");
            break;
        default:
            COMMLOG(OS_LOG_INFO, "Notify Code is %u.", eNtfCode);
            break;
    }
}

/* ------------------------------------------------------------
Description  : 传给KMC的回调函数
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_void AppWriteLogKMC(mp_int32 nLevel, const mp_char* pszModuleName, const mp_char* pszOccurFileName,
    mp_int32 nOccurLine, const mp_char* pszLog)
{
    static const mp_int32 levelWarn = 1;
    static const mp_int32 levelErr = 2;
    switch (nLevel) {
        case 0:
            COMMLOG(OS_LOG_DEBUG, "[%s,%d]%s", pszOccurFileName, nOccurLine, pszLog);
            break;
        case levelWarn:
            COMMLOG(OS_LOG_WARN, "[%s,%d]%s", pszOccurFileName, nOccurLine, pszLog);
            break;
        case levelErr:
            COMMLOG(OS_LOG_ERROR, "[%s,%d]%s", pszOccurFileName, nOccurLine, pszLog);
            break;
        default:
            break;
    }
}

/* ------------------------------------------------------------
Description  : 传给KMC的回调函数
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_void AppRecvNotifyKmc(WsecUint32 eNtfCode, const WsecVoid* pData, size_t nDataSize)
{
    (mp_void) pData;
    (mp_void) nDataSize;
    switch (eNtfCode) {
        case WSEC_KMC_NTF_WRI_KEY_STORE_FAIL:
            COMMLOG(OS_LOG_ERROR, "Cannot write kmc store files.");
            break;
        case WSEC_KMC_NTF_WRI_CFG_FILE_FAIL:
            COMMLOG(OS_LOG_ERROR, "Cannot write kmc config files.");
            break;
        case WSEC_KMC_NTF_KEY_STORE_UPDATE:
            COMMLOG(OS_LOG_INFO, "Kmc store files updated.");
            break;
        case WSEC_KMC_NTF_CFG_FILE_UPDATE:
            COMMLOG(OS_LOG_INFO, "Kmc config files updated.");
            break;
        case WSEC_KMC_NTF_RK_EXPIRE:
            COMMLOG(OS_LOG_INFO, "Kmc root key (would be) expired.");
            break;
        case WSEC_KMC_NTF_MK_EXPIRE:
            COMMLOG(OS_LOG_INFO, "Kmc master key (would be) expired.");
            break;
        case WSEC_KMC_NTF_MK_CHANGED:
            COMMLOG(OS_LOG_INFO, "Kmc master key changed.");
            break;
        case WSEC_KMC_NTF_USING_EXPIRED_MK:
            COMMLOG(OS_LOG_DEBUG, "Kmc using expired master key.");
            break;
        default:
            AppRecvNotifyKmcNew(eNtfCode);
            break;
    }
}

/* ------------------------------------------------------------
Description  : 传给KMC的回调函数
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_void AppDoEventsKmc()
{
    return;
}

/* ------------------------------------------------------------
Description  : 传给KMC的回调函数
Create By    : lishuai 00349472
------------------------------------------------------------- */
unsigned long RegFunc()
{
    unsigned long returnValue;
    WsecCallbacks stMandatoryFun = {{0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}};

    stMandatoryFun.basicRelyCallbacks.writeLog = AppWriteLogKMC;
    stMandatoryFun.basicRelyCallbacks.notify = AppRecvNotifyKmc;
    stMandatoryFun.basicRelyCallbacks.doEvents = AppDoEventsKmc;

    // Regis self thread func
    stMandatoryFun.lockCallbacks.lock = ThreadLock;
    stMandatoryFun.lockCallbacks.unlock = ThreadUnlock;
    stMandatoryFun.lockCallbacks.createLock = CreateThreadLock;
    stMandatoryFun.lockCallbacks.destroyLock = DestroyThreadLock;

    stMandatoryFun.procLockCallbacks.createProcLock = CreateProcLock;
    stMandatoryFun.procLockCallbacks.destroyProcLock = DestroyProcLock;
    stMandatoryFun.procLockCallbacks.procLock = ProcLock;
    stMandatoryFun.procLockCallbacks.procUnlock = ProcUnlock;

    stMandatoryFun.fileCallbacks.fileOpen = (CallbackFopen)Fopen;
    stMandatoryFun.fileCallbacks.fileClose = Fclose;
    stMandatoryFun.fileCallbacks.fileRead = Fread;
    stMandatoryFun.fileCallbacks.fileWrite = Fwrite;
    stMandatoryFun.fileCallbacks.fileRemove = Fremove;
    stMandatoryFun.fileCallbacks.fileSeek = (CallbackFseek)Fseek;
    stMandatoryFun.fileCallbacks.fileTell = Ftell;
    stMandatoryFun.fileCallbacks.fileFlush = Fflush;
    stMandatoryFun.fileCallbacks.fileEof = Feof;
    stMandatoryFun.fileCallbacks.fileErrno = Ferrno;
    stMandatoryFun.fileCallbacks.fileExist = Fexist;

    stMandatoryFun.rngCallbacks.getRandomNum = NULL;
    stMandatoryFun.rngCallbacks.getEntropy = GetEntropy;
    stMandatoryFun.rngCallbacks.cleanupEntropy = CleanupEntropy;
    stMandatoryFun.timeCallbacks.gmTimeSafe = UtcTime;
    returnValue = WsecRegFuncEx(&stMandatoryFun);
    if (WSEC_SUCCESS != returnValue) {
        COMMLOG(OS_LOG_ERROR, "WsecRegFuncEx.returnValue=%lu %016X\n", returnValue, returnValue);
        return -1;
    }

    return returnValue;
}

/* ------------------------------------------------------------
Description  : 初始化KMC
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 InitKmc(const mp_string& kmcStoreFile, const mp_string& kmcStoreBakFile, const WsecUint32 roleType,
    const mp_string& kmcConfBakFile)
{
    COMMLOG(OS_LOG_DEBUG, "KMC init store file:%s.", kmcStoreFile.c_str());

    if (RegFunc() != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Register kmc functions failed.");
        return MP_FAILED;
    }

    KmcKsfName filename = {0};
    filename.keyStoreFile[0] = (mp_char*)(kmcStoreFile.c_str());
    filename.keyStoreFile[1] = (mp_char*)(kmcStoreBakFile.c_str());

    unsigned long ret = WsecInitializeEx(roleType, &filename, WSEC_FALSE, NULL);
    if (WSEC_SUCCESS != ret) {
        COMMLOG(OS_LOG_ERROR, "KMC initialize failed, ret = %lu.", ret);
        return MP_FAILED;
    }

    if (RegisterPrivateDomain() != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Register private domain [%u] failed.", PRIVATE_MK_DOMAIN_ID);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

namespace {
mp_int32 InitKMCBase64(const mp_string& kmcStoreFile, const mp_string& kmcStoreBakFile)
{
    if (RegFunc() != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Register kmc functions failed.");
        return MP_FAILED;
    }

    KmcKsfName filename = { { const_cast<char *>(kmcStoreFile.c_str()), const_cast<char *>(kmcStoreBakFile.c_str()) } };
    unsigned long retValue = WsecInitializeEx(KMC_ROLE_MASTER, &filename, WSEC_FALSE, nullptr);
    if (retValue != WSEC_SUCCESS) {
        ERRLOG("Call WsecInitializeEx failed. Return Value=%ld", retValue);
        return MP_FAILED;
    }
    INFOLOG("Init KMC success");
    return MP_SUCCESS;
}

std::string Base64Encode(const std::string &in)
{
    std::string out;
    int val = 0;
    int valb = -6;
    for (unsigned char c : in) {
        val = (val << NUM8) + c;
        valb += NUM8;
        while (valb >= 0) {
            out.push_back(base64_arr[(val >> valb) & 0x3F]);
            valb -= NUM6;
        }
    }
    if (valb > SIGNED_NUM_NEGATIVE6) {
        out.push_back(base64_arr[((val << NUM8) >> (valb + NUM8)) & 0x3F]);
    }
    while ((out.size() % NUM4) > 0) {
        out.push_back('=');
    }
    return out;
}

std::string Base64Decode(const std::string &in)
{
    std::string out;
    std::vector<int> T(NUM256, -1);

    for (int i = 0; i < NUM64; i++) {
        T[base64_arr[i]] = i;
    }
    int val = 0;
    int valb = -8;
    for (unsigned char c : in) {
        if (T[c] == -1) {
            break;
        }
        val = (val << NUM6) + T[c];
        valb += NUM6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= NUM8;
        }
    }
    return out;
}

mp_int32 DecryptStrKmcBase64(const mp_string& encText, mp_string& plainStr)
{
    std::string encStr = Base64Decode(encText);
    WsecUint32 tmpPlainLen = encStr.length();
    mp_uchar* plainBuffer = new mp_uchar[tmpPlainLen];
    if (!plainBuffer) {
        ERRLOG("Alloc plainBuffer failed");
        return MP_FAILED;
    }
    WsecUint32 plainLen = encText.length();
    unsigned long ret = SdpDecryptEx(g_domainId, (const mp_uchar *)encStr.c_str(), encStr.length(),
        plainBuffer, &plainLen);
    if (ret != WSEC_SUCCESS) {
        ERRLOG("SdpDecryptEx failed[%ld]", ret);
        delete[] plainBuffer;
        plainBuffer = NULL;
        return MP_FAILED;
    }
    mp_string tmpPlainStr((char *)plainBuffer, plainLen);
    plainStr = std::move(tmpPlainStr);
    (mp_void) memset_s(plainBuffer, tmpPlainLen, 0, tmpPlainLen);
    delete[] plainBuffer;
    plainBuffer = NULL;
    return MP_SUCCESS;
}


mp_int32 EncryptStrKmcBase64(const mp_string& plainStr, mp_string& encText)
{
    WsecUint32 cipherLen = 0;
    unsigned long ret = SdpGetCipherDataLenEx(plainStr.length(), &cipherLen);
    if (WSEC_SUCCESS != ret) {
        ERRLOG("SdpGetCipherDataLenEx fail[%ld]", ret);
        return MP_FAILED;
    }
    mp_uchar* cipherBuffer = new mp_uchar[cipherLen];
    if (!cipherBuffer) {
        ERRLOG("Alloc cipherBuffer failed");
        return MP_FAILED;
    }
    ret = SdpEncryptEx(g_domainId, WSEC_ALGID_AES256_GCM, (const mp_uchar *)plainStr.c_str(),
        plainStr.length(), cipherBuffer, &cipherLen);
    if (ret != WSEC_SUCCESS) {
        ERRLOG("SdpEncryptEx failed[%ld]", ret);
        delete[] cipherBuffer;
        cipherBuffer = NULL;
        return MP_FAILED;
    }
    mp_string cipherTxt((char *)cipherBuffer, cipherLen);
    delete[] cipherBuffer;
    cipherBuffer = NULL;
    encText = Base64Encode(cipherTxt);
    return MP_SUCCESS;
}
}; // namespace

/* ------------------------------------------------------------
Description  : 初始化KMC
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : zhangyoupeng wx206957
------------------------------------------------------------- */
mp_int32 InitKmcByFile(const mp_string& kmcStoreFile, const mp_string& kmcStoreBakFile, const WsecUint32 roleType,
    const mp_string& kmcConfBakFile)
{
    if (g_bKMCInitialized) {
        COMMLOG(OS_LOG_ERROR, "The KMC has initialized, please release first.");
        return MP_FAILED;
    }

    mp_int32 ret = InitKMCBase64(kmcStoreFile, kmcStoreBakFile);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "KMC initialize failed.");
        return MP_FAILED;
    }

    g_bKMCInitialized = true;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 初始化KMC
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
Modification : 2018-08-23:将KMC初始化代码提成函数Init_KMC
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 InitializeKmc(const mp_uint32 roleType)
{
    if (!g_bKMCInitialized) {
        mp_string strKmcStoreFile;
        mp_string strKmcStoreFileBak;

        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION,
            CFG_BACKUP_SCENE, g_installType);
        if (iRet == MP_SUCCESS && g_installType == AGENT_INSTALL_TYPE_INTERNAL) {
            strKmcStoreFile = STORE_FILE_PATH;
            strKmcStoreFileBak = STORE_BAK_FILE_PATH;
        } else {
            g_installType = 0;
            strKmcStoreFile = CPath::GetInstance().GetConfFilePath(KMC_STORE_FILE);
            strKmcStoreFileBak = CPath::GetInstance().GetNginxConfFilePath(KMC_STORE_FILE_BAK);
        }

        int disableSafeRandomNumber = 0;
        iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SECURITY_SECTION,
            CFG_DISABLE_SAFE_RANDOM_NUMBER, disableSafeRandomNumber);
        if ((iRet == MP_SUCCESS) && (disableSafeRandomNumber != 0)) {
            g_kmcNotUsePseudoRandomNumbers = false;
            COMMLOG(OS_LOG_INFO, "Use pseudo random number.");
        }

        mp_int32 ret = InitKMCBase64(strKmcStoreFile.c_str(), strKmcStoreFileBak.c_str());
        if (ret != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "KMC initialize failed.");
            return MP_FAILED;
        }

        if (MP_SUCCESS != GetExternalDomainInfo()) {
            COMMLOG(OS_LOG_ERROR, "Get KMC external domain info failed.");
            return MP_FAILED;
        }

        g_bKMCInitialized = true;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 销毁KMC分配的资源
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 FinalizeKmc()
{
    if (g_bKMCInitialized) {
        unsigned long ret = WsecFinalizeEx();
        if (ret != WSEC_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "KMC finalize failed.");
            return MP_FAILED;
        }

        g_bKMCInitialized = false;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 重置KMC
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 ResetKmc()
{
    unsigned long ret = WsecResetEx();
    if (ret != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "KMC reset failed.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : KMC定时驱动函数，定时调用以驱动KMC进行生命周期检测
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_void TimerKmc()
{}

mp_int32 CheckEnValid(const mp_string& inStr, mp_string& outStr, mp_uint32& outLen)
{
    if (MP_SUCCESS != GetExternalDomainInfo()) {
        COMMLOG(OS_LOG_ERROR, "Get KMC external domain info failed.");
        return MP_FAILED;
    }

    outStr = "";
    if (inStr.length() == 0) {
        COMMLOG(OS_LOG_ERROR, "inStr is empty.");
        return MP_FAILED;
    }

    unsigned long ret = SdpGetCipherDataLen(inStr.length(), &outLen);
    if (ret != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get cipher len failed.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 使用KMC加密字符串
Input        : inStr -- 输入字符串
Output       : outStr -- 输出字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 EncryptStrKmc(const mp_string& inStr, mp_string& outStr)
{
    if (!g_bKMCInitialized || g_installType == AGENT_INSTALL_TYPE_INTERNAL) {
        return EncryptStrKmcBase64(inStr, outStr);
    }

    mp_uint32 outLen = 0;
    mp_int32 iRet = CheckEnValid(inStr, outStr, outLen);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // CodeDex误报，ZERO_LENGTH_ALLOCATIONS
    mp_uint32 inLen = inStr.length();
    std::vector<mp_uchar> inVev(inLen + 1, 0);
    mp_uchar* inBuf = reinterpret_cast<mp_uchar*>(inVev.data());

    mp_int32 ret1 = memcpy_s(inBuf, inLen, inStr.c_str(), inLen);
    if (ret1 != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memcpy_s failed, ret = %d.", ret1);
        return MP_FAILED;
    }

    mp_uint32 outBufLen = outLen;
    std::vector<mp_uchar> outVev(outBufLen + 1, 0);
    mp_uchar* outBuf = reinterpret_cast<mp_uchar*>(outVev.data());
    std::fill(outVev.begin(), outVev.end(), 0);
    unsigned long ret = SdpEncrypt(
        g_domainId, WSEC_ALGID_AES256_CBC, WSEC_ALGID_HMAC_SHA256, inBuf, inLen, outBuf, &outLen);
    std::fill(inVev.begin(), inVev.end(), 0);

    if (ret != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "KMC encryption failed, ret = %d.", ret);
        return MP_FAILED;
    }

    ret1 = StrToHex(outBuf, outStr, outLen);
    if (ret1 != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert str to Hex str failed.");
        std::fill(outVev.begin(), outVev.end(), 0);
    }

    return ret1;
}

/* ------------------------------------------------------------
Description  : 使用KMC加密字符串，进行处理前首先重置KMC
Input        : inStr -- 输入字符串
Output       : outStr -- 输出字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 EncryptStrKmcWithReset(const mp_string& inStr, mp_string& outStr)
{
    mp_int32 ret = ResetKmc();
    if (ret != MP_SUCCESS) {
        return MP_FAILED;
    }
    ret = EncryptStrKmc(inStr, outStr);
    return ret;
}

mp_int32 PreCheck(const mp_string& inStr, mp_uint32& iLen)
{
    iLen = inStr.length();
    if (iLen == 0 || iLen % SDP_FUNC_NUM_2 == 1) {
        return MP_FAILED;
    }
    iLen /= SDP_FUNC_NUM_2;

    GetExternalDomainInfo();
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 使用KMC解密字符串
Input        : inStr -- 输入字符串
Output       : outStr -- 输出字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 DecryptStrKmc(const mp_string& inStr, mp_string& outStr)
{
    if (!g_bKMCInitialized || g_installType == AGENT_INSTALL_TYPE_INTERNAL) {
        return DecryptStrKmcBase64(inStr, outStr);
    }

    mp_uint32 inLen = 0;
    if (PreCheck(inStr, inLen) != MP_SUCCESS) {
        return MP_FAILED;
    }

    std::vector<mp_uchar> inVec(inLen, 0);
    mp_uchar* inBuf = reinterpret_cast<mp_uchar*>(inVec.data());

    unsigned long ret1 = 0;
    mp_uint32 outLen = inLen;
    mp_uint32 outBufLen = inLen + 1;
    mp_uchar* outBuf = NULL;

    mp_int32 ret = HexToStr(inStr, inBuf, inLen);
    if (ret != MP_SUCCESS) {
    }
    std::vector<mp_uchar> outVec(outBufLen, 0);
    outBuf = reinterpret_cast<mp_uchar*>(outVec.data());

    ret = memset_s(outBuf, outBufLen, 0, outBufLen);
    std::fill(outVec.begin(), outVec.end(), 0);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, ret = %d.", ret);
    }
    ret1 = SdpDecrypt(g_domainId, inBuf, inLen, outBuf, &outLen);
    if (ret1 != WSEC_SUCCESS) {
        ret = MP_FAILED;
    }

    outStr = reinterpret_cast<mp_char*>(outVec.data());
    std::fill(outVec.begin(), outVec.end(), 0);

    return ret;
}

/* ------------------------------------------------------------
Description  : 使用KMC解密字符串，进行处理前首先重置KMC
Input        : inStr -- 输入字符串
Output       : outStr -- 输出字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 DecryptStrKmcWithReset(const mp_string& inStr, mp_string& outStr)
{
    mp_int32 ret = ResetKmc();
    if (ret != MP_SUCCESS) {
        return MP_FAILED;
    }
    ret = DecryptStrKmc(inStr, outStr);
    return ret;
}

mp_uchar* CheckGenValid(const mp_string& filePath, mp_string& fileHMAC, SdpHmacAlgAttributes& stHmacAlgAttr)
{
    fileHMAC = "";
    if (filePath.length() == 0) {
        ERRLOG("filePath is null.");
        return nullptr;
    }

    if (MP_SUCCESS != GetExternalDomainInfo()) {
        ERRLOG("Get KMC external domain info failed.");
        return nullptr;
    }

    unsigned long ret = SdpGetHmacAlgAttr(g_domainId, WSEC_ALGID_HMAC_SHA256, &stHmacAlgAttr);
    if (ret != WSEC_SUCCESS) {
        ERRLOG("Get HMAC algorithm attribute failed, ret = %d.", ret);
        return nullptr;
    }
    WsecUint32 ulHDLen = SDP_FUNC_NUM_100;
    mp_uchar* pvHmacData = new (std::nothrow) mp_uchar[ulHDLen];
    return pvHmacData;
}

/* ------------------------------------------------------------
Description  : 使用KMC计算文件HMAC
Input        : filePath -- 文件路径
Output       : fileHMAC -- 文件HMAC值
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 GenFileHmacKmc(const mp_string& filePath, mp_string& fileHMAC)
{
    SdpHmacAlgAttributes stHmacAlgAttr = {0};
    mp_uchar* pvHmacData = CheckGenValid(filePath, fileHMAC, stHmacAlgAttr);
    if (pvHmacData == NULL) {
        COMMLOG(OS_LOG_ERROR, "New buf failed.");
        return MP_FAILED;
    }

    WsecUint32 ulHDLen = SDP_FUNC_NUM_100;
    unsigned long ret = SdpFileHmac(g_domainId, filePath.c_str(), &stHmacAlgAttr, (mp_void*)pvHmacData, &ulHDLen);
    if (ret != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Generate file HMAC failed, ret = %d.", ret);
        delete[] pvHmacData;
        return MP_FAILED;
    }

    mp_uint32 outLen = sizeof(SdpHmacAlgAttributes) + ulHDLen;
    mp_uchar* outBuf = new mp_uchar[outLen];
    if (outBuf == NULL) {
        COMMLOG(OS_LOG_ERROR, "New buf failed.");
        delete[] pvHmacData;
        delete[] outBuf;
        return MP_FAILED;
    }

    // CodeDex误报，HW_NSCC_CPP_SECFUNPARAMCHECKER
    mp_int32 ret1 = memcpy_s(outBuf, outLen, (void*)stHmacAlgAttr.buff, sizeof(SdpHmacAlgAttributes));
    if (ret1 != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memcpy_s failed, ret = %d.", ret1);
        delete[] pvHmacData;
        delete[] outBuf;
        return MP_FAILED;
    }
    ret1 = memcpy_s(outBuf + sizeof(SdpHmacAlgAttributes), ulHDLen, (void*)pvHmacData, ulHDLen);
    if (ret1 != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memcpy_s failed, ret = %d.", ret1);
        delete[] pvHmacData;
        delete[] outBuf;
        return MP_FAILED;
    }

    ret1 = StrToHex(outBuf, fileHMAC, outLen);
    if (ret1 != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert str to Hex str failed.");
        fileHMAC = "";
    }
    delete[] pvHmacData;
    delete[] outBuf;
    return ret1;
}

/* ------------------------------------------------------------
Description  : 使用KMC计算文件HMAC，进行处理前首先重置KMC
Input        : filePath -- 文件路径
Output       : fileHMAC -- 文件HMAC值
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 GenFileHmacKmcWithReset(const mp_string& filePath, mp_string& fileHMAC)
{
    mp_int32 ret = ResetKmc();
    if (ret != MP_SUCCESS) {
        return MP_FAILED;
    }
    ret = GenFileHmacKmc(filePath, fileHMAC);
    return ret;
}

mp_int32 CheckVerifyValid(mp_uint32& hmacLen, const mp_string& filePath, const mp_string& fileHMAC)
{
    if (MP_SUCCESS != GetExternalDomainInfo()) {
        ERRLOG("Get KMC external domain info failed.");
        return MP_FAILED;
    }
    if (filePath.length() == 0 || fileHMAC.length() == 0) {
        ERRLOG("filePath or fileHMAC is null.");
        return MP_FAILED;
    }
    if (hmacLen % SDP_FUNC_NUM_2 == 1) {
        ERRLOG("hmacLen % 2 failed.");
        return MP_FAILED;
    }
    hmacLen /= SDP_FUNC_NUM_2;
    if (hmacLen < sizeof(SdpHmacAlgAttributes)) {
        ERRLOG("hmacLen < %u.", sizeof(SdpHmacAlgAttributes));
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 使用KMC校验文件HMAC
Input        : filePath -- 文件路径
               fileHMAC -- 文件HMAC值
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 VerifyFileHmacKmc(const mp_string& filePath, const mp_string& fileHMAC)
{
    mp_uint32 hmacLen = fileHMAC.length();
    if (CheckVerifyValid(hmacLen, filePath, fileHMAC) != MP_SUCCESS) {
        return MP_FAILED;
    }

    // CodeDex误报，ZERO_LENGTH_ALLOCATIONS
    mp_uchar* hmacBuf = new mp_uchar[hmacLen];
    if (hmacBuf == NULL) {
        COMMLOG(OS_LOG_ERROR, "New buf failed.");
        return MP_FAILED;
    }

    mp_int32 ret = HexToStr(fileHMAC, hmacBuf, hmacLen);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert Hex str to str failed.");
        return MP_FAILED;
    }

    WsecUint32 ulHDLen = hmacLen - sizeof(SdpHmacAlgAttributes);
    mp_uchar* pvHmacData = new (std::nothrow) mp_uchar[ulHDLen];
    if (pvHmacData == NULL) {
        COMMLOG(OS_LOG_ERROR, "New buf failed.");
        delete[] hmacBuf;
        return MP_FAILED;
    }
    // CodeDex误报，HW_NSCC_CPP_SECFUNPARAMCHECKER
    // CodeDex误报，Integer Overflow
    SdpHmacAlgAttributes stHmacAlgAttr = { 0 };
    ret = memcpy_s((void*)(stHmacAlgAttr.buff), sizeof(SdpHmacAlgAttributes), hmacBuf, sizeof(SdpHmacAlgAttributes));
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memcpy_s failed, ret = %d.", ret);
        delete[] hmacBuf;
        delete[] pvHmacData;
        return MP_FAILED;
    }
    ret = memcpy_s((void*)pvHmacData, ulHDLen, hmacBuf + sizeof(SdpHmacAlgAttributes), ulHDLen);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call memcpy_s failed, ret = %d.", ret);
        delete[] hmacBuf;
        delete[] pvHmacData;
        return MP_FAILED;
    }

    unsigned long ret1 = SdpVerifyFileHmac(g_domainId, filePath.c_str(), &stHmacAlgAttr, pvHmacData, ulHDLen);
    delete[] hmacBuf;
    delete[] pvHmacData;
    if (ret1 != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Verify file HMAC failed, ret = %d.", ret1);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 使用KMC校验文件HMAC，进行处理前首先重置KMC
Input        : filePath -- 文件路径
               fileHMAC -- 文件HMAC值
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 VerifyFileHmacKmcWithReset(const mp_string& filePath, const mp_string& fileHMAC)
{
    mp_int32 ret = ResetKmc();
    if (ret != MP_SUCCESS) {
        return MP_FAILED;
    }
    ret = VerifyFileHmacKmc(filePath, fileHMAC);
    return ret;
}

/* ------------------------------------------------------------
Description  : Visit KMC config file to get domain info (id=PRIVATE_MK_DOMAIN_ID)
Return       : MP_SUCCESS -- PRIVATE_MK_DOMAIN_ID exists or Not
               MP_FAILED -- PRIVATE_MK_DOMAIN_ID exists but get KMC max MK id failed
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_int32 GetExternalDomainInfo()
{
    if (g_domainId == PRIVATE_MK_DOMAIN_ID) {
        return MP_SUCCESS;
    }

    if (CheckIfDomainExists(PRIVATE_MK_DOMAIN_ID) == MP_FALSE) {
        return MP_SUCCESS;
    }

    g_domainId = PRIVATE_MK_DOMAIN_ID;  // external MK exists
    COMMLOG(OS_LOG_INFO, "Now begin to use external MK.");
    return MP_SUCCESS;
}

mp_bool CheckIfDomainExists(const mp_uint32& domainId)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to check if domain %d exist.", domainId);

    WsecUint32 maxKeyId = 0;
    unsigned long ret = KmcGetMaxMkId(PRIVATE_MK_DOMAIN_ID, &maxKeyId);
    if (ret == WSEC_ERR_KMC_MK_MISS) {
        COMMLOG(OS_LOG_DEBUG, "domain[%u] has not been added MK.", PRIVATE_MK_DOMAIN_ID);
        return MP_FALSE;
    } else if (ret != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get max key id of domain[%u] failed, ret = %lu.", PRIVATE_MK_DOMAIN_ID, ret);
        return MP_FALSE;
    } else {
        COMMLOG(OS_LOG_DEBUG, "The target domain is not exists in KMC.");
        return MP_TRUE;
    }
}

mp_int32 RegisterMk(const mp_string& plainTextKey)
{
    WsecUint32 maxKeyId = 0;
    unsigned long ret = KmcGetMaxMkId(PRIVATE_MK_DOMAIN_ID, &maxKeyId);
    maxKeyId += 1;
    if (ret == WSEC_ERR_KMC_MK_MISS) {
        maxKeyId = 0;
        COMMLOG(OS_LOG_INFO, "This is the first time to get MK id before register.");
    } else if (ret != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get max key id of domain[%u] failed, ret = %lu.", PRIVATE_MK_DOMAIN_ID, ret);
        return MP_FAILED;
    }

    // regeister MK
    mp_uchar* pPlainTextKey = reinterpret_cast<mp_uchar*>(const_cast<char*>(plainTextKey.c_str()));
    ret = KmcRegisterMkEx(PRIVATE_MK_DOMAIN_ID, maxKeyId, pPlainTextKey, plainTextKey.size());
    if (ret != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "KMC register mk failed, ret = %d.", ret);
        return MP_FAILED;
    }

    ret = KmcActivateMk(PRIVATE_MK_DOMAIN_ID, maxKeyId);
    if (ret != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Set domain [%u] key id [%u] to active status failed, ret = %d.",
            PRIVATE_MK_DOMAIN_ID,
            maxKeyId + 1,
            ret);
        return ret;
    }

    g_domainId = PRIVATE_MK_DOMAIN_ID;

    COMMLOG(OS_LOG_INFO, "KMC register external mk success, domain id is %u.", g_domainId);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : Register new KMC domain, and create MK according to plainText
Input        : plainTextKey -- the key of plain text
                keyLifeDays --  the life days of MK
Return       : MP_SUCCESS -- register success
                MP_FAILED -- error
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_int32 RegisterExternalMk(const mp_string& plainTextKey, const mp_uint32& keyLifeDays)
{
    mp_bool bFlag = (plainTextKey.size() < PLAIN_TEXT_MIN_LEN || plainTextKey.size() > PLAIN_TEXT_MAX_LEN);
    if (bFlag) {
        COMMLOG(OS_LOG_ERROR, "MK len is too long");
        return MP_FAILED;
    }

    bFlag = keyLifeDays < MIN_MK_LIFE_DAYS || keyLifeDays > MAX_MK_LIFE_DAYS;
    if (bFlag) {
        COMMLOG(OS_LOG_ERROR, "Key life days should greater than 0!");
        return MP_FAILED;
    }

    unsigned long ret = KmcRmvDomainKeyTypeEx(PRIVATE_MK_DOMAIN_ID, KMC_KEY_TYPE_ENCRPT_INTEGRITY);
    if (ret != WSEC_SUCCESS && ret != WSEC_ERR_KMC_DOMAIN_KEYTYPE_MISS) {
        COMMLOG(OS_LOG_ERROR, "KMC remove domain key type failed, ret = %d.", ret);
        return MP_FAILED;
    }

    KmcCfgKeyType domainKeyType = {KMC_KEY_TYPE_ENCRPT_INTEGRITY, WSEC_MK_PLAIN_LEN_MAX - 1, 0, {0}};
    domainKeyType.keyLifeDays = keyLifeDays;
    ret = KmcAddDomainKeyTypeEx(PRIVATE_MK_DOMAIN_ID, &domainKeyType);
    if (ret != WSEC_SUCCESS && ret != WSEC_ERR_KMC_ADD_REPEAT_KEY_TYPE) {
        COMMLOG(OS_LOG_ERROR, "KMC add domain key type failed, ret = %d.", ret);
        return MP_FAILED;
    }

    return RegisterMk(plainTextKey);
}

/* ------------------------------------------------------------
Description  : Register new KMC domain and key type, V3 need invoke this functions after init kmc
Input        : no
Return       : MP_SUCCESS -- register success
               MP_FAILED -- error
Create By    : zhuyuanjie 00455045  2020/08/04
------------------------------------------------------------- */
mp_int32 RegisterPrivateDomain()
{
    mp_char mkDesc[] = KMC_DOMAIN_DESC;
    unsigned long ret;

    // 1. add new domain, domain id = PRIVATE_MK_DOMAIN_ID (2)
    KmcCfgDomainInfo privateMK;
    privateMK.domainId = PRIVATE_MK_DOMAIN_ID;
    privateMK.domainKeyFrom = KMC_MK_GEN_BY_IMPORT;  // import mk from users
    privateMK.domainType = KMC_DOMAIN_TYPE_SHARE;

    CHECK_NOT_OK(memcpy_s(privateMK.desc, sizeof(privateMK.desc), mkDesc, sizeof(mkDesc)));
    CHECK_NOT_OK(memset_s(privateMK.reserve, sizeof(privateMK.reserve), 0, sizeof(privateMK.reserve)));
    ret = KmcAddDomainEx(&privateMK);
    if (ret != WSEC_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "KMC add domain failed, ret = %d.", ret);
        return MP_FAILED;
    }

    // 2. add domain key type
    const WsecUint32 defaultKeyLifeDays = 180;
    KmcCfgKeyType domainKeyType = { KMC_KEY_TYPE_ENCRPT_INTEGRITY, WSEC_MK_PLAIN_LEN_MAX - 1, 0, {0} };
    domainKeyType.keyLifeDays = defaultKeyLifeDays;
    ret = KmcAddDomainKeyTypeEx(PRIVATE_MK_DOMAIN_ID, &domainKeyType);
    if (ret != WSEC_SUCCESS && ret != WSEC_ERR_KMC_ADD_REPEAT_KEY_TYPE) {
        COMMLOG(OS_LOG_ERROR, "KMC add domain key type failed, ret = %d.", ret);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : Create a new KMC key.
Input        : no
Return       : MP_SUCCESS -- register success
               MP_FAILED -- error
Create By    : kWX884906  2021/04/20
------------------------------------------------------------- */
mp_int32 CreateInternalMK()
{
    mp_int32 domainNum = KmcGetDomainCount();
    if (domainNum < 0) {
        COMMLOG(OS_LOG_ERROR, "KMC get domain count failed.");
        return MP_FAILED;
    }
    KmcCfgDomainInfo stDomainInfo;
    unsigned long ret = WSEC_SUCCESS;
    for (int i = 0; i < domainNum; i++) {
        ret = KmcGetDomain(i, &stDomainInfo);
        if (ret != WSEC_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "KMC get domain info failed, ret = %d.", ret);
            return MP_FAILED;
        }
        COMMLOG(OS_LOG_ERROR,
            "domainId=%u, domainKeyFrom=%u, desc=%s, domainType=%u",
            stDomainInfo.domainId,
            stDomainInfo.domainKeyFrom,
            stDomainInfo.desc,
            stDomainInfo.domainType);
        if (stDomainInfo.domainKeyFrom == KMC_MK_GEN_BY_INNER) {
            WsecUint32 keyId = 0;
            ret = KmcCreateMkEx(stDomainInfo.domainId, &keyId);
            if (ret != WSEC_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "KMC create MK failed, ret = %d.", ret);
                return MP_FAILED;
            }
            ret = KmcActivateMk(stDomainInfo.domainId, keyId);
            if (ret != WSEC_SUCCESS) {
                COMMLOG(OS_LOG_ERROR,
                    "Set domain [%d] key id [%u] to active status failed, ret = %d.",
                    stDomainInfo.domainId,
                    keyId,
                    ret);
                return MP_FAILED;
            }
        }
    }
    return MP_SUCCESS;
}
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
#include <iostream>
#include <memory>
#include <vector>
#include <functional>
#include <cstdlib>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <fcntl.h>

#include "wsecv2_itf.h"
#include "kmcv2_itf.h"
#include "wsecv2_type.h"
#include "sdpv2_itf.h"
#include "sdpv1_itf.h"
#include "cacv2_pri.h"
#include "wsecv2_order.h"
#include "securec.h"
#include "wsecv2_errorcode.h"
#include "logProxy.h"
#include "hardKey.h"
#include "kmcv3.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace {
    const string MODULE_NAME = "KMC";
    const unsigned int HARDWARE_KEY_LENGTH = 32;
    const unsigned int RANDOM_MAX_LEN = 100;
    const unsigned int AUTH_ENC_LEN = 32;
    const unsigned int MK_SIZE = 16;
    const unsigned int MAX_BUFFER_SIZE = 10485760;
    const int SIGNED_NUM_NEGATIVE6 = -6;
    const int NUM4 = 4;
    const int NUM6 = 6;
    const int NUM8 = 8;
    const int NUM64 = 64;
    const int NUM256 = 256;
    const std::string base64_arr = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
}

#pragma pack(1)
typedef struct TagKmcEnc {
    unsigned char iv[16];
    unsigned char tag[16];
    unsigned int enclen;
    unsigned char enc[0];
} KmcEnc;
#pragma pack()

// CallBack functions
void CB_WriLog(int nLevel, const char *module, const char *filename, int numline, const char *pszLog)
{
    numline;
    HCP_LOGGER_NOID(INFO, MODULE_NAME) << "CB_WriLog [" << module << "]"
                                       << "[" << filename << "]" << pszLog;
}
void CB_RecvNotify(unsigned int eNtfCode, const void *data, size_t nDataSize)
{
    data;
    nDataSize;
    HCP_LOGGER_NOID(DEBUG, MODULE_NAME) << "CB_RecvNotify eNtfCode:" << eNtfCode;
}

void CB_DoEvents(void) {}

int CB_CreateThreadLock(WsecHandle *phMutex)
{
    int retVal = WSEC_FALSE;
    if (phMutex == nullptr) {
        return retVal;
    }
    if (*phMutex != nullptr) {
        return retVal;
    }

    pthread_mutex_t *css = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    if (css == nullptr) {
        return retVal;
    }

    memset_s(css, sizeof(pthread_mutex_t), 0, sizeof(pthread_mutex_t));
    if (pthread_mutex_init(css, nullptr) == 0) {
        retVal = WSEC_TRUE;
        *phMutex = css;
    }
    return retVal;
}

void CB_DestroyThreadLock(WsecHandle hMutex)
{
    pthread_mutex_t *css = (pthread_mutex_t *)hMutex;
    if (css == nullptr) {
        return;
    }
    pthread_mutex_destroy(css);
    free(css);
    css = nullptr;
}

void CB_ThreadLock(WsecHandle hMutex)
{
    pthread_mutex_t *css = (pthread_mutex_t *)hMutex;
    if (css == nullptr) {
        return;
    }
    pthread_mutex_lock(css);
}

void CB_ThreadUnlock(WsecHandle hMutex)
{
    pthread_mutex_t *css = (pthread_mutex_t *)hMutex;
    if (css == nullptr) {
        return;
    }
    pthread_mutex_unlock(css);
}

int CB_CreateProcLock(WsecHandle *cProcLock)
{
    cProcLock;
    return WSEC_TRUE;
}

void CB_DestroyProcLock(WsecHandle dProcLock)
{
    dProcLock;
    return;
}

void CB_ProcLock(WsecHandle procLock)
{
    procLock;
    return;
}

void CB_ProcUnlock(WsecHandle procUnlock)
{
    procUnlock;
    return;
}

unsigned long GetRandomNumbers(unsigned char *buff, unsigned int len)
{
    unsigned long ret = 0xF;
    FILE *fet = fopen("/dev/random", "rb");
    if (fet != nullptr) {
        if (len == fread(buff, 1, len, fet)) {
            ret = 0;
        }
        fclose(fet);
        fet = nullptr;
    }
    return ret;
}

int WSECGetEntropy(unsigned char **ppEnt, size_t buffLen)
{
    int bre = WSEC_FALSE;

    if (buffLen > MAX_BUFFER_SIZE) {
        return bre;
    }
    *ppEnt = (unsigned char *)malloc(buffLen);
    if (*ppEnt == nullptr) {
        return bre;
    }
    unsigned long ret = GetRandomNumbers(*ppEnt, buffLen);
    if (ret == 0) {
        bre = WSEC_TRUE;
    }
    if (WSEC_TRUE != bre) {
        memset_s(*ppEnt, buffLen, 0, buffLen);
        free(*ppEnt);
        *ppEnt = nullptr;
    }
    return bre;
}

void WSECCleanupEntropy(unsigned char *pEnt, size_t buffLen)
{
    memset_s(pEnt, buffLen, 0, buffLen);
    free(pEnt);
}

void *Fopen(char *filePathName, unsigned int mode)
{
    int flag = 0;
    int *ret = nullptr;
    if (mode == KMC_FILE_READ_BINARY) {
        flag = O_RDONLY;
    } else if (mode == KMC_FILE_WRITE_BINARY) {
        flag = O_CREAT | O_WRONLY;
    } else if (mode == KMC_FILE_READWRITE_BINARY) {
        flag = O_CREAT | O_RDWR;
    } else {
        return nullptr;
    }
    if (filePathName == nullptr) {
        return nullptr;
    }
    int retFd = open(filePathName, flag, S_IRUSR | S_IWUSR);
    if (retFd != -1) {
        ret = (int *)malloc(sizeof(int));
        if (ret == nullptr) {
            close(retFd);
            return nullptr;
        }
        *ret = retFd;
    }
    return ret;
}

int Fclose(WsecHandle stream)
{
    int fd = *(int *)stream;
    int ret = close(fd);
    free(stream);
    return ret;
}

int Fread(WsecHandle buffer, size_t count, WsecHandle stream)
{
    int fd = *(int *)stream;
    if (buffer == nullptr) {
        return WSEC_FALSE;
    }
    // 产品需考虑读稳定性，如判断EINTR、多次尝试读
    long int ret = (long int)read(fd, buffer, count);
    return (count != (size_t)ret || ret == -1) ? WSEC_FALSE : WSEC_TRUE;
}

int Fwrite(const void *buffer, size_t count, void *stream)
{
    int fd = *(int *)stream;
    if (buffer == nullptr) {
        return WSEC_FALSE;
    }
    long int ret = static_cast<long int>(write(fd, buffer, count));
    if (fsync(fd) != 0) {
        return WSEC_FALSE;
    }
    return (count != (size_t)ret || ret == -1) ? WSEC_FALSE : WSEC_TRUE;
}

// 成功返0， 失败返-1
int Fflush(WsecHandle stream)
{
    int fd = *(int *)stream;
    int ret = fsync(fd);
    return ret;
}

// 成功返0，失败返-1
int Fremove(const char *path)
{
    return remove(path);
}
long Ftell(WsecHandle stream)
{
    int fd = *(int *)stream;
    long ret = lseek(fd, 0, SEEK_CUR);
    return ret;
}

// 成功返回偏移位置【相对文件起始】，失败返-1
long Fseek(WsecHandle stream, long offset, unsigned int origin)
{
    int realOri = 0;
    int fd = *(int *)stream;

    if (origin == KMC_FILE_SEEK_CUR) {
        realOri = SEEK_CUR;
    } else if (origin == KMC_FILE_SEEK_SET) {
        realOri = SEEK_SET;
    } else if (origin == KMC_FILE_SEEK_END) {
        realOri = SEEK_END;
    } else {
        return -1;
    }
    return lseek(fd, offset, realOri);
}
int Feof(WsecHandle stream, int *endOfFile)
{
    int fd = *(int *)stream;
    if (endOfFile == nullptr) {
        return -1;
    }
    int curPos = lseek(fd, 0, SEEK_CUR);
    if (curPos == -1) {
        return -1;
    }
    long len = lseek(fd, 0, SEEK_END);
    if (len == -1) {
        return -1;
    }
    if (lseek(fd, curPos, SEEK_SET) != curPos) {
        return -1;
    }
    if (len != curPos) {
        *endOfFile = WSEC_FALSE;
    } else {
        *endOfFile = WSEC_TRUE;
    }
    return 0;
}
int Ferrno(WsecHandle stream)
{
    stream;
    return errno;
}

int Fexist(const char *filePathName)
{
    if (filePathName == nullptr) {
        return WSEC_FALSE;
    }
    return (access(filePathName, F_OK) == 0) ? WSEC_TRUE : WSEC_FALSE;
}

int UtcTime(const time_t *curTime, struct tm *curTm)
{
    int ret = WSEC_FALSE;
    if ((curTime != nullptr) && (curTm != nullptr)) {
        ret = gmtime_r(curTime, curTm) == nullptr ? WSEC_FALSE : WSEC_TRUE;
    }
    return ret;
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

unsigned long RegFuc(void)
{
    static bool alreadyReg = false;
    if (alreadyReg) {
        return WSEC_SUCCESS;
    } else {
        alreadyReg = true;
    }

    WsecCallbacks stMandatoryFun = { { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 }, { 0 } };
    WsecRngCallbacks stRandomFB = { nullptr, nullptr, nullptr };
    WsecTimeCallbacks stTimeFB = { nullptr };

    stMandatoryFun.basicRelyCallbacks.writeLog = CB_WriLog;
    stMandatoryFun.basicRelyCallbacks.notify = CB_RecvNotify;
    stMandatoryFun.basicRelyCallbacks.doEvents = CB_DoEvents;
    // Regis self thread func
    stMandatoryFun.lockCallbacks.lock = CB_ThreadLock;
    stMandatoryFun.lockCallbacks.unlock = CB_ThreadUnlock;
    stMandatoryFun.lockCallbacks.createLock = CB_CreateThreadLock;
    stMandatoryFun.lockCallbacks.destroyLock = CB_DestroyThreadLock;
    stMandatoryFun.procLockCallbacks.createProcLock = CB_CreateProcLock;
    stMandatoryFun.procLockCallbacks.destroyProcLock = CB_DestroyProcLock;
    stMandatoryFun.procLockCallbacks.procLock = CB_ProcLock;
    stMandatoryFun.procLockCallbacks.procUnlock = CB_ProcUnlock;
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
    stMandatoryFun.rngCallbacks.getRandomNum = nullptr;
    stMandatoryFun.rngCallbacks.getEntropy = WSECGetEntropy;
    stMandatoryFun.rngCallbacks.cleanupEntropy = WSECCleanupEntropy;
    stMandatoryFun.timeCallbacks.gmTimeSafe = UtcTime;

    unsigned long returnValue = WsecRegFuncEx(&stMandatoryFun);
    if (WSEC_SUCCESS != returnValue) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "WsecRegFuncEx failed. Return Value=" << returnValue;
        return WSEC_FAILURE;
    }
    return returnValue;
}

/*
 * Function: InitKMCV3c
 * Description: KMC初始化，v3版本，C接口
 * Input:  const char * storeFilePath,         存储路径
           const char * storeBakFilePath,      备份文件路径
           const char * moduleName             模块名，建议字母，数字，下划线
 * Output: nullptr
 * Return: ture/false
 * Others: 实际调用V1接口
 */
KMC_RET InitKMCV3c(const char *storeFilePath, const char *storeBakFilePath, const char *moduleName)
{
    std::string module = moduleName;

    if ((storeFilePath == nullptr) || (storeFilePath == nullptr) || (moduleName == nullptr)) {
        HCP_LOGGER_NOID(INFO, MODULE_NAME) << "Input nullptr argus";
        return KMC_FAIL;
    }

    (void)InitKmcLog(module);
    HCP_LOGGER_NOID(INFO, MODULE_NAME) << "storeFilePath:" << storeFilePath << " storeBakFilePath:" <<
        storeBakFilePath << " moduleName:" << moduleName;

    KmcKsfName f = { { const_cast<char *>(storeFilePath), const_cast<char *>(storeBakFilePath) } };
    unsigned long retValue = RegFuc();
    if (WSEC_SUCCESS != retValue) {
        HCP_LOGGER_NOID(INFO, MODULE_NAME) << "RegFuc Error, retValue=" << retValue;
        return KMC_FAIL;
    }
    retValue = WsecInitializeEx(KMC_ROLE_MASTER, &f, WSEC_FALSE, nullptr);
    if (retValue != WSEC_SUCCESS) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "Call WsecInitializeEx failed. Return Value=" << retValue;
        return KMC_FAIL;
    }
    HCP_LOGGER_NOID(INFO, MODULE_NAME) << "Init KMC success";
    return KMC_SUCESS;
}

/*
 * Function: EncryptV3c
 * Description: 加密函数，v3版本，C接口
 * Calls:  EncryptV3
 * Input:  KMC_DOMAIN domain,
           const char *plainTxt,  明文
 * Output: char **encText         密文
 * Return: ture/false
 * Others: 输出的密文内存，需要调用Kmc_Free释放
 */
KMC_RET EncryptV3c(KMC_DOMAIN domain, const char *plainTxt, char **encText)
{
    errno_t rc;
    std::string plainStr = plainTxt;
    WsecUint32 cipherLen = 0;

    if ((plainTxt == nullptr) || (encText == nullptr)) {
        HCP_LOGGER_NOID(INFO, MODULE_NAME) << "Input nullptr argus";
        return KMC_FAIL;
    }
    unsigned long ret = SdpGetCipherDataLenEx(plainStr.length(), &cipherLen);
    if (WSEC_SUCCESS != ret) {
        HCP_LOGGER_NOID(INFO, MODULE_NAME) << "SdpGetCipherDataLenEx Err, ret=" << ret;
        return KMC_FAIL;
    }

    std::unique_ptr<unsigned char[]> cipherBuffer = std::make_unique<unsigned char[]>(cipherLen);
    if (!cipherBuffer) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "Alloc cipherBuffer failed";
        return KMC_FAIL;
    }
    ret = SdpEncryptEx(domain, WSEC_ALGID_AES256_GCM, (const unsigned char *)plainStr.c_str(), plainStr.length(),
        cipherBuffer.get(), &cipherLen);
    std::string cipherTxt((char *)cipherBuffer.get(), cipherLen);
    std::string encStr = Base64Encode(cipherTxt);

    int length = strlen(encStr.c_str()) + 1;
    *encText = (char *)malloc(length);
    if (*encText == nullptr) {
        HCP_LOGGER_NOID(INFO, MODULE_NAME) << "malloc return nullptr, length = " << length;
        return KMC_FAIL;
    }
    rc = memset_s(*encText, length, 0, length);
    if (EOK != rc) {
        free(*encText);
        *encText = nullptr;
        return KMC_FAIL;
    }
    rc = strcpy_s(*encText, length, encStr.c_str());
    if (EOK != rc) {
        free(*encText);
        *encText = nullptr;
        return KMC_FAIL;
    }
    return KMC_SUCESS;
}

/*
 * Function: DecryptV3c
 * Description: 解密函数，v3版本，C接口
 * Calls:    DecryptV3c
 * Input:  KMC_DOMAIN domain,
           const char *encTextB64, 密文
 * Output: char **plainTxt         明文
 * Return: KMC_RET
 * Others: 输出的明文内存，需要调用Kmc_Free释放
 */
KMC_RET DecryptV3c(KMC_DOMAIN domain, char **plainTxt, const char *encTextB64)
{
    if ((plainTxt == nullptr) || (encTextB64 == nullptr)) {
        HCP_LOGGER_NOID(INFO, MODULE_NAME) << "Input nullptr argus";
        return KMC_FAIL;
    }
    std::string encStr = Base64Decode(encTextB64);
    std::unique_ptr<unsigned char[]> plainBuffer = std::make_unique<unsigned char[]>(encStr.length());
    if (!plainBuffer) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "Alloc plainBuffer failed";
        return KMC_FAIL;
    }
    WsecUint32 plainLen = strlen(encTextB64);
    unsigned long ret = SdpDecryptEx(domain,
        (const unsigned char *)encStr.c_str(),
        encStr.length(),
        plainBuffer.get(),
        &plainLen);
    // 密文无效
    if ((WSEC_ERR_INVALID_ARG == ret) || (WSEC_ERR_INPUT_BUFF_NOT_ENOUGH == ret) ||
        (WSEC_ERR_SDP_INVALID_CIPHER_TEXT == ret)) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "encTextB64 is invalid!,ret=" << ret;
        return KMC_ENCTXT_INVAILD;
    } else if (WSEC_SUCCESS != ret) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "SdpDecryptEx error, ret =" << ret;
        return KMC_FAIL;
    }

    int length = plainLen + 1;
    *plainTxt = (char *)malloc(length);
    if (*plainTxt == nullptr) {
        HCP_LOGGER_NOID(INFO, MODULE_NAME) << "malloc return nullptr, length = " << length;
        return KMC_FAIL;
    }
    errno_t rc = memset_s(*plainTxt, length, 0, length);
    if (EOK != rc) {
        free(*plainTxt);
        *plainTxt = nullptr;
        HCP_LOGGER_NOID(INFO, MODULE_NAME) <<"Memset_s failed.";
        return KMC_FAIL;
    }
    rc = strcpy_s(*plainTxt, length, (const char *)plainBuffer.get());
    if (EOK != rc) {
        free(*plainTxt);
        *plainTxt = nullptr;
        HCP_LOGGER_NOID(INFO, MODULE_NAME) <<"Strcpy_s failed";
        return KMC_FAIL;
    }
    return KMC_SUCESS;
}


/* ------------------------------------------------------------
Description  : 将字符串转换为16进制字符串
Input        : hexIn -- 16进制字符串
               strOut -- 输出字符串内存指针
               outLen -- 输出字符串内存长度
Return       : KMC_SUCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
bool HexToStr(const std::string& hexIn, unsigned char* strOut, unsigned int outLen)
{
    unsigned char SDP_FUNC_NUM_2 = 2;
    unsigned int inLen = hexIn.length();
    if (inLen == 0 || inLen % SDP_FUNC_NUM_2 == 1) {
        return false;
    }
    if (strOut == nullptr) {
        return false;
    }
    inLen /= SDP_FUNC_NUM_2;
    if (outLen < inLen) {
        return false;
    }
    int ret = memset_s(strOut, outLen, 0, inLen);
    if (ret != KMC_SUCESS) {
        HCP_LOGGER_NOID(INFO, MODULE_NAME) << "Call memset_s failed, ret = " << ret;
        return false;
    }
    stringstream ss;
    int tmp;
    ss << hex;
    for (unsigned int i = 0; i < inLen; i++) {
        ss << hexIn.substr(i * SDP_FUNC_NUM_2, SDP_FUNC_NUM_2);
        tmp = -1;
        ss >> tmp;
        if (tmp == -1) {
            return false;
        }
        strOut[i] = static_cast<unsigned char>(tmp);
        ss.str("");
        ss.clear();
    }
    return true;
}

bool PreCheck(const std::string& inStr, unsigned int& iLen)
{
    unsigned char SDP_FUNC_NUM_2 = 2;
    iLen = inStr.length();
    if (iLen == 0 || iLen % SDP_FUNC_NUM_2 == 1) {
        HCP_LOGGER_NOID(INFO, MODULE_NAME) << "Pre check failed.";
        return false;
    }
    iLen /= SDP_FUNC_NUM_2;
    return true;
}

KMC_RET DecryptPwdV1(KMC_DOMAIN domain, std::string& outStr, const std::string& inStr)
{
    HCP_LOGGER_NOID(INFO, MODULE_NAME) << "Enter DecryptV1...";
    unsigned int inLen = 0;
    if (!PreCheck(inStr, inLen)) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "Precheck failed";
        return KMC_FAIL;
    }

    std::vector<unsigned char> inVec(inLen, 0);
    unsigned char* inBuf = reinterpret_cast<unsigned char*>(inVec.data());

    unsigned long ret1 = 0;
    unsigned int outLen = inLen;
    unsigned int outBufLen = inLen + 1;
    unsigned char* outBuf = nullptr;

    if (!HexToStr(inStr, inBuf, inLen)) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "Hex to str failed.";
        return KMC_FAIL;
    }
    std::vector<unsigned char> outVec(outBufLen, 0);
    outBuf = reinterpret_cast<unsigned char*>(outVec.data());

    int ret = memset_s(outBuf, outBufLen, 0, outBufLen);
    std::fill(outVec.begin(), outVec.end(), 0);
    if (ret != KMC_SUCESS) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "Call memset_s failed, ret = " << ret;
        return KMC_FAIL;
    }
    ret = SdpDecrypt(domain, inBuf, inLen, outBuf, &outLen);
    if (ret != WSEC_SUCCESS) {
        HCP_LOGGER_NOID(ERR, MODULE_NAME) << "SdpDecrypt failed, ret = " << ret;
        return KMC_FAIL;
    }

    outStr = reinterpret_cast<char*>(outVec.data());
    std::fill(outVec.begin(), outVec.end(), 0);
    return KMC_SUCESS;
}

/*
 * Function: DeInitKmc
 * Description:去初始化KMCV3/KMCV1
 * Input:  nullptr
 * Output: nullptr
 * Return: 0: 成功，else: 失败
 */
unsigned long DeInitKmc(void)
{
    return WsecFinalizeEx();
}

/*
 * Function: Kmc_Free
 * Description: 释放KMCV3/KMCV1申请的内存
 * Input:  char *mem  内存指针
 * Output: nullptr
 * Return: nullptr
 */
void KmcFree(char *mem)
{
    if (mem != nullptr) {
        free(mem);
    }
}

#ifdef __cplusplus
}
#endif

/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file CryptAlg.cpp
 * @brief  Contains function declarations kmc Encrypt
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#pragma warning(disable : 4996)
#include "securecom/CryptAlg.h"
#include <sstream>
#include <iostream>
#include "securec.h"
#include "openssl/aes.h"
#include "openssl/sha.h"
#include "openssl/evp.h"
#include "openssl/rand.h"
#include "common/Defines.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/CSystemExec.h"
#include "securecom/SDPFunc.h"

using namespace std;
namespace {
const mp_uchar CRYPT_ALG_NUM_2 = 2;
const mp_uchar CRYPT_ALG_NUM_8 = 8;
const mp_uchar CRYPT_ALG_NUM_16 = 16;
const mp_uchar CRYPT_ALG_NUM_48 = 48;
/* 设置最小盐值长度为40 */
const mp_uint64 MIN_SALT_BITS_NUM = 40;
/* 敏感信息覆写三次 */
const mp_int32 SENSITIVE_DATA_WIPE_TIMES = 3;
const mp_int32 TRUE_RANDOM_NUM_SEED_LENGTH = 128;
}  // namespace

/* ------------------------------------------------------------
Function Name:getMinLen
Description  : 字符串有最小长度限制，当前密钥和IV要求最小48
------------------------------------------------------------- */
size_t GetMinLen(size_t len)
{
    return ((len < CRYPT_ALG_NUM_48) ? CRYPT_ALG_NUM_48 : ((len / CRYPT_ALG_NUM_16 + 1) * CRYPT_ALG_NUM_16));
}

/* ------------------------------------------------------------
Function Name:checkDecryptLen
Description  : 检查解密密钥长度
------------------------------------------------------------- */
mp_bool CheckDecryptLen(size_t len)
{
    return ((len >= CRYPT_ALG_NUM_48) && (len % CRYPT_ALG_NUM_16 == static_cast<mp_uchar>(0)));
}

/* ------------------------------------------------------------
Function Name:HexStr2ASCII
Description  : hex字符串转换成ascii
------------------------------------------------------------- */
mp_int32 HexStr2ASCII(const mp_string& hexStr, mp_string& ASCIIStr, size_t hexLen)
{
    // CodeDex误报，UNINIT
    if (hexLen % CRYPT_ALG_NUM_2 == 1) {
        COMMLOG(OS_LOG_ERROR, "Length of hexStr is error, [%s]", hexStr.c_str());
        return MP_FAILED;
    }

    stringstream ssResult;
    stringstream ss;
    mp_int32 tmp;
    ss << hex;
    for (mp_uint32 i = 0; i < hexLen / CRYPT_ALG_NUM_2; ++i) {
        ss << hexStr.substr(i * CRYPT_ALG_NUM_2, CRYPT_ALG_NUM_2);
        tmp = -1;
        ss >> tmp;
        if (tmp == -1) {
            return MP_FAILED;
        }
        ssResult << (mp_uchar)tmp;
        ss.str("");
        ss.clear();
    }
    ASCIIStr = ssResult.str();

    return MP_SUCCESS;
}
mp_void DestoryRes(mp_uchar rKey[], mp_int32 keyLen, mp_uchar iv[], mp_int32 ivLen)
{
    if (rKey != nullptr && iv != nullptr) {
        for (mp_int32 loop = 0; loop < SENSITIVE_DATA_WIPE_TIMES; loop++) {
            (mp_void)memset_s(rKey, AES_KEY_LEN, 0, AES_KEY_LEN);
            (mp_void)memset_s(iv, AES_IV_LEN, 0, AES_IV_LEN);
        }
    }
}
/* ------------------------------------------------------------
Function Name: GetSha256Hash
Description  : 生成sha256校验码
-------------------------------------------------------- */
mp_int32 GetSha256Hash(const mp_string& buff, const std::size_t length, mp_string& hashHex, std::size_t hexLen)
{
    mp_uchar acHash[SHA256_DIGEST_LENGTH] = {0};
    mp_char outHashHex[SHA256_BLOCK_SIZE + 1] = {0};
    SHA256_CTX c;
    // CodeDex误报，UNINIT
    SHA256_Init(&c);
    SHA256_Update(&c, buff.c_str(), length);
    SHA256_Final(acHash, &c);
    for (mp_uint32 i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        mp_int32 iRet = sprintf_s(
            &outHashHex[0] + CRYPT_ALG_NUM_2 * i, hexLen - CRYPT_ALG_NUM_2 * i, "%02x", acHash[i]);
        if (iRet == MP_FAILED) {
            return MP_FAILED;
        }
    }

    hashHex = outHashHex;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:GetRandom
Description  : 获取随机数
------------------------------------------------------------- */
#ifdef WIN32
mp_int32 GetRandom(mp_uint64& num, bool notPreudo)
{
    HCRYPTPROV hCryptProv;
    mp_uint64 lastCode;
    if (!::CryptAcquireContextW(&hCryptProv, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        lastCode = GetLastError();
        COMMLOG(OS_LOG_ERROR, "CryptAcquireContextW exec failed, errorcode [%lu].", lastCode);
        return MP_FAILED;
    }

    if (!CryptGenRandom(hCryptProv, sizeof(mp_uint64), reinterpret_cast<BYTE*>(&num))) {
        lastCode = GetLastError();
        COMMLOG(OS_LOG_ERROR, "CryptGenRandom exec failed, errorcode [%lu].", lastCode);
        ::CryptReleaseContext(hCryptProv, 0);
        return MP_FAILED;
    }

    if (!::CryptReleaseContext(hCryptProv, 0)) {
        lastCode = GetLastError();
        COMMLOG(OS_LOG_ERROR, "CryptReleaseContext exec failed, errorcode [%lu].", lastCode);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
#else
mp_int32 GetRandom(mp_uint64& num, bool notPreudo)
{
    static bool initFlag = false;
    static char seedBuf[TRUE_RANDOM_NUM_SEED_LENGTH];

    if (notPreudo) {
        COMMLOG(OS_LOG_INFO, "Begin get random bytes which is not preudo.");
    } else {
        COMMLOG(OS_LOG_DEBUG, "Begin get random bytes which is preudo.");
    }

    if (notPreudo && !initFlag) {
        // 用 /dev/random 产生真随机数，长度1024bit，即128字节
        COMMLOG(OS_LOG_INFO, "Begin get random bytes to init rand seed.");
        mp_int32 fd = open("/dev/random", O_RDONLY);
        if (fd == -1) {
            COMMLOG(OS_LOG_ERROR, "open /dev/random failed[%d]:%s.", errno, strerror(errno));
            return MP_FAILED;
        }

        if (read(fd, seedBuf, TRUE_RANDOM_NUM_SEED_LENGTH) == -1) {
            COMMLOG(OS_LOG_ERROR, "read file failed, errno[%d]:%s.", errno, strerror(errno));
            close(fd);
            return MP_FAILED;
        }
        close(fd);

        // 用真随机数作为种子，种子长度1024bit，即128字节（高于标准中建议的888bit）
        RAND_seed(seedBuf, TRUE_RANDOM_NUM_SEED_LENGTH);
        COMMLOG(OS_LOG_INFO, "Generate RAND seed finish.");
        initFlag = true;
    }

    // check rand status
    if (RAND_status() != 1) {
        COMMLOG(OS_LOG_ERROR, "RAND status not good.");
        return MP_FAILED;
    }

    // 使用Openssl自带的RAND_priv_bytes产生伪随机(pseudo random)数
    if (RAND_priv_bytes(reinterpret_cast<unsigned char*>(&num), sizeof(mp_uint64)) != 1) {
        COMMLOG(OS_LOG_ERROR, "Generate random priv bytes fail.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Get random num(%lld) success", num);
    return MP_SUCCESS;
}
#endif
/* ------------------------------------------------------------
Function Name:GenRandomSalt
Description  : 获取随机数盐值并转换成字符串
------------------------------------------------------------- */
mp_int32 GenRandomSalt(mp_string& saltstring)
{
    mp_uint32 iRet;
    mp_uint64 randNum;
    stringstream ss;

    while (ss.str().length() < MIN_SALT_BITS_NUM) {
        iRet = GetRandom(randNum, true);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "get Random number failed.");
            return iRet;
        }
        ss << randNum;
    }
    saltstring = ss.str();

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name:PBKDF2Hash
Description  : 获取PBKDF2散列值
------------------------------------------------------------- */
mp_int32 PBKDF2Hash(const mp_string& strPlainText, const mp_string& strSalt, mp_string& strCipherText)
{
    mp_uchar pUnChSalt[PBKDF_SALT_MAX_LEN + 1] = {0x0};
    mp_int32 saltLen = (mp_int32)strSalt.length();
    if (saltLen > PBKDF_SALT_MAX_LEN) {
        COMMLOG(OS_LOG_ERROR, "strSalt is too long, len = %d", saltLen);
        return MP_FAILED;
    }
    for (mp_uint32 i = 0; i < saltLen; ++i) {
        pUnChSalt[i] = static_cast<mp_uchar>(strSalt[i]);
    }

    // CodeDex误报，Weak Cryptographic Hash:Insecure PBE Iteration Count，迭代次数5000,符合安全要求
    // Call PKCS5_PBKDF2_HMAC
    mp_uchar out[PBKDF_KEY_LEN] = {0x0};
    mp_int32 iRet = PKCS5_PBKDF2_HMAC(strPlainText.c_str(),
        static_cast<int>(strPlainText.length()),
        pUnChSalt,
        saltLen,
        PBKDF_ITER_TIMES,
        EVP_sha256(),
        PBKDF_KEY_LEN,
        out);
    if (iRet != 1) {
        COMMLOG(OS_LOG_ERROR, "call PKCS5_PBKDF2_HMAC failed, ret = %d", iRet);
        return MP_FAILED;
    }

    mp_uchar base64Array[CIPHER_BUFF_LEN] = {0x0};
    EVP_EncodeBlock(base64Array, out, PBKDF_KEY_LEN);
    strCipherText = std::move(reinterpret_cast<char*>(base64Array));
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 初始化加密组件
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 InitCrypt(const mp_uint32 roleType)
{
    return InitializeKmc(roleType);
}

/* ------------------------------------------------------------
Description  : 初始化加密组件
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : zhangyoupeng
------------------------------------------------------------- */
mp_int32 InitCryptByFile(const mp_string& kmcStoreFile, const mp_string& kmcStoreBakFile, const mp_uint32 roleType,
    const mp_string& kmcConfBakFile)
{
    return InitKmcByFile(kmcStoreFile, kmcStoreBakFile, roleType, kmcConfBakFile);
}

/* ------------------------------------------------------------
Description  : 释放加密组件资源
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 FinalizeCrypt()
{
    return FinalizeKmc();
}

/* ------------------------------------------------------------
Description  : 调用加密组件的定时驱动函数
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_void CallCryptTimer()
{
    TimerKmc();
}

/* ------------------------------------------------------------
Description  : 加密字符串
Input        : inStr -- 输入字符串
Output       : outStr -- 输出字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_void EncryptStr(const mp_string& inStr, mp_string& outStr)
{
    mp_int32 ret = EncryptStrKmc(inStr, outStr);
    if (ret != MP_SUCCESS) {
        outStr = "";
        COMMLOG(OS_LOG_ERROR, "EncryptStr failed.");
        return;
    }
}

/* ------------------------------------------------------------
Description  : 解密字符串
Input        : inStr -- 输入字符串
Output       : outStr -- 输出字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_void DecryptStr(const mp_string& inStr, mp_string& outStr)
{
    mp_int32 ret = DecryptStrKmc(inStr, outStr);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "DecryptStr failed.");
        return;
    }
}

/* ------------------------------------------------------------
Description  : 解密字符串用KMC
Input        : inStr -- 输入字符串
Output       : outStr -- 输出字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : zhangyoupeng
------------------------------------------------------------- */
mp_void DecryptStrKMC(const mp_string& inStr, mp_string& outStr)
{
    mp_int32 ret = DecryptStrKmc(inStr, outStr);
    if (ret != MP_SUCCESS) {
        outStr = "";
        COMMLOG(OS_LOG_ERROR, "DecryptStr failed.");
        return;
    }
}

/* ------------------------------------------------------------
Description  : 计算文件HMAC
Input        : filePath -- 文件路径
Output       : fileHMAC -- 文件HMAC值
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 ComputeHMAC(const mp_string& filePath, mp_string& fileHMAC)
{
    mp_int32 ret = GenFileHmacKmc(filePath, fileHMAC);
    if (ret == MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "ComputeHMAC succ.");
    }
    return ret;
}

/* ------------------------------------------------------------
Description  : 校验文件HMAC
Input        : filePath -- 文件路径
               fileHMAC -- 文件HMAC值
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : lishuai 00349472
------------------------------------------------------------- */
mp_int32 VerifyHMAC(const mp_string& filePath, const mp_string& fileHMAC)
{
    mp_int32 ret = VerifyFileHmacKmc(filePath, fileHMAC);
    if (ret == MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "VerifyHMAC succ.");
    }
    return ret;
}

/* ------------------------------------------------------------
Description  : Register external MK
Input        : plainText -- use plainText to create MK
               keyLifeDays -- the life days of MK
Return       : MP_SUCCESS -- register MK success
               MP_FAILED -- unsuccessful
Create By    : zhuyuanjie 00455045
------------------------------------------------------------- */
mp_int32 RegisterExternalMK(const mp_string& plainText, const mp_uint32 keyLifeDays)
{
    mp_int32 ret = RegisterExternalMk(plainText, keyLifeDays);
    if (ret == MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Register external mk succ.");
        return MP_SUCCESS;
    }

    COMMLOG(OS_LOG_ERROR, "Register external mk failed.");
    return ret;
}

mp_int32 RestartAgent()
{
#ifdef WIN32
    mp_string strCmd = mp_string("sc stop ") + AGENT_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    strCmd = mp_string("sc start ") + AGENT_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    static const mp_int32 sleepTime = 1000; // 1000ms
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));

    CMpTime::DoSleep(sleepTime);
    strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif
    return MP_SUCCESS;
}

mp_int32 ManualUpdateDmcKey()
{
    mp_int32 ret = CreateInternalMK();
    if (ret == MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "CreateInternalMK succ.");
        std::cout << "The kmc key is updated successfully and takes effect after the restart." << endl;
        std::cout << "Are you sure you want to restart immediately ? (y|n):";
        mp_string userIn;
        std::cin >> userIn;
        if (userIn == "Y" || userIn == "y") {
            ret = RestartAgent();
            if (ret == MP_SUCCESS) {
                std::cout << "DataBackup ProtectAgent was restarted succ." << endl;
            }
        }
        return ret;
    } else {
        COMMLOG(OS_LOG_ERROR, "CreateInternalMK failed.");
        return ret;
    }
}
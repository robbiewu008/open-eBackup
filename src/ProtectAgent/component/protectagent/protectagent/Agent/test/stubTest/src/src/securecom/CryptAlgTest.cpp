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
#include "securecom/CryptAlgTest.h"
#include "openssl/evp.h"
#include "openssl/rand.h"
#include "stub.h"
#include "common/Log.h"
#include "common/Types.h"
#include "common/CSystemExec.h"
#include "securecom/SDPFunc.h"

typedef mp_void (*StubFuncType)(void);
typedef mp_void (CLogger::*LogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);

static mp_void StubCLoggerLog(void){
    return;
}


mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_bool StubFalse()
{
    return MP_FALSE;
}

mp_bool StubTrue()
{
    return MP_TRUE;
}

mp_string StubCin()
{
    return "Y";
}

TEST_F(CryptAlgTest, EncryptAndDescryptTest){
    stub.set(&CLogger::Log, StubCLoggerLog);

    // ??????
    mp_string encryptIn;
    mp_string encryptOut;
    mp_string descryptOut;

    EncryptStr(encryptIn, encryptOut);
    DecryptStr(encryptOut, descryptOut);
    EXPECT_EQ(encryptIn, descryptOut);
}
extern mp_bool CheckDecryptLen(size_t len);
mp_bool StubCheckDecryptLen(size_t len)
{
    return MP_TRUE;
}
TEST_F(CryptAlgTest, EncryptTest){
    stub.set(&CLogger::Log, StubCLoggerLog);
    // ?????
    mp_string descryptIn = "0000000000000000";
    mp_string descryptOut = "0000000000000000";

    stub.set(CheckDecryptLen, StubCheckDecryptLen);
    DecryptStr(descryptIn, descryptOut);
}

TEST_F(CryptAlgTest, GetMinLenTest){
    std::size_t len;
    int ret;

    size_t GetMinLen(size_t len);

    len = 1;
    ret = GetMinLen(len);
    EXPECT_EQ(48, ret);

    len = 49;
    ret = GetMinLen(len);
    EXPECT_EQ(64, ret);
}

TEST_F(CryptAlgTest, CheckDecryptLenTest){
    std::size_t len;
    mp_bool ret;

    mp_bool CheckDecryptLen(size_t len);
    
    len = 32;
    ret = CheckDecryptLen(len);
    EXPECT_EQ(ret, false);

    len = 64;
    ret = CheckDecryptLen(len);
    EXPECT_EQ(ret, true);

    len = 65;
    ret = CheckDecryptLen(len);
    EXPECT_EQ(ret, false);
}

int stubOpenError(void){
    return -1;
}

ssize_t stubReadSuccess(void) {
    return 0;
}

ssize_t stubReadError(void){
    return -1;
}

int RAND_statusGood(void){
    return 1;
}

int RAND_statusNotGood(void){
    return -1;
}

TEST_F(CryptAlgTest, GetRandomTest){
    mp_uint64 number;
    mp_string randomSalt;
    mp_int32 ret;

    stub.set(&CLogger::Log, StubCLoggerLog);

    typedef ssize_t (*orignalType)(int fd, void *buf, size_t count);
    typedef ssize_t (*stubType)(void);
    stub.set(open, StubSuccess);
    stub.set(read, stubReadSuccess);
    stub.set(RAND_status, RAND_statusGood);

    ret = GetRandom(number, false);
    EXPECT_EQ(ret, MP_SUCCESS);

    ret = GetRandom(number, true);
    EXPECT_EQ(ret, MP_SUCCESS);

    ret = GenRandomSalt(randomSalt);
    EXPECT_EQ(ret, MP_SUCCESS);
    do {
      typedef int (*orignalType)(const char *, int, ...);
      typedef int (*stubType)(void);
      stub.set(open, stubOpenError);
      stub.set(RAND_status, RAND_statusNotGood);
      ret = GetRandom(number, false);
      EXPECT_EQ(ret, MP_FAILED);
    } while(0);

    do {
      typedef ssize_t (*orignalType)(int fd, void *buf, size_t count);
      typedef ssize_t (*stubType)(void);
      stub.set(open, StubSuccess);
      stub.set(read, stubReadError);
      stub.set(RAND_status, RAND_statusGood);
      ret = GetRandom(number, false);
      EXPECT_EQ(ret, MP_SUCCESS);
    } while(0);

    stub.reset(open);
    stub.reset(read);
}

/*
* 用例名称：获取PBKDF2散列值
* 前置条件：无
* check点：检查返回值
*/
TEST_F(CryptAlgTest, PBKDF2HashTest){
    mp_string strPlainText = "12345678901234567890123456789012345678901234567890";
    mp_string strSalt = "12345678901234567890123456789012345678901234567890";
    mp_string strCipherText;
    stub.set(&CLogger::Log, StubCLoggerLog);

    EXPECT_EQ(MP_FAILED, PBKDF2Hash(strPlainText, strSalt, strCipherText));

    strSalt = "12345678901234567890123456789012345678";
    stub.set(PKCS5_PBKDF2_HMAC, StubFalse);
    EXPECT_EQ(MP_FAILED, PBKDF2Hash(strPlainText, strSalt, strCipherText));

    stub.set(PKCS5_PBKDF2_HMAC, StubTrue);
    stub.set(EVP_EncodeBlock, StubTrue);
    EXPECT_EQ(MP_SUCCESS, PBKDF2Hash(strPlainText, strSalt, strCipherText));

    stub.reset(PKCS5_PBKDF2_HMAC);
    stub.reset(EVP_EncodeBlock);
}

TEST_F(CryptAlgTest, GetSha256HashTest)
{
    mp_string outHashHex;
    mp_string strInput = "qgswhoqwhsowhcqhwqjsdwiohsshw";
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 ret = GetSha256Hash(strInput.c_str(), strInput.length(), outHashHex, SHA256_BLOCK_SIZE + 1);
    EXPECT_EQ(MP_SUCCESS, ret);
    
    stub.set(sprintf_s, StubFailed);
    ret = GetSha256Hash(strInput.c_str(), strInput.length(), outHashHex, SHA256_BLOCK_SIZE + 1);
    EXPECT_EQ(MP_FAILED, ret);
}

// TEST_F(CryptAlgTest, HexStr2ASCIITest)
// {
//     mp_string hexStr = "000011110000111100001111";
//     mp_string ASCIIStr;
//     size_t hexLen = 24;

//     stub.set(&CLogger::Log, StubCLoggerLog);
//     mp_int32 ret = HexStr2ASCII(hexStr.c_str(), ASCIIStr, hexLen);
//     EXPECT_EQ(MP_FAILED, ret);
// }

TEST_F(CryptAlgTest, InitCryptTest)
{
    mp_uint32 roleType = 0;

    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(InitializeKmc, StubSuccess);
    mp_int32 ret = InitCrypt(roleType);
    EXPECT_EQ(MP_SUCCESS, ret);
}

TEST_F(CryptAlgTest, InitCryptByFileTest)
{
    mp_string kmcStoreFile;
    mp_string kmcStoreBakFile;
    mp_uint32 roleType;
    mp_string kmcConfBakFile;

    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(InitKmcByFile, StubSuccess);
    mp_int32 ret = InitCryptByFile(kmcStoreFile, kmcStoreBakFile, roleType, kmcConfBakFile);
    EXPECT_EQ(MP_SUCCESS, ret);
}

TEST_F(CryptAlgTest, FinalizeCryptTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(FinalizeKmc, StubSuccess);
    mp_int32 ret = FinalizeCrypt();
    EXPECT_EQ(MP_SUCCESS, ret);
}

TEST_F(CryptAlgTest, ComputeHMACTest)
{
    mp_string filePath = "test path";
    mp_string fileHMAC = "test file";
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(GenFileHmacKmc, StubFailed);
    mp_int32 ret = ComputeHMAC(filePath, fileHMAC);
    EXPECT_EQ(MP_FAILED, ret);

    stub.set(GenFileHmacKmc, StubSuccess);
    ret = ComputeHMAC(filePath, fileHMAC);
    EXPECT_EQ(MP_SUCCESS, ret);
}

TEST_F(CryptAlgTest, VerifyHMACTest)
{
    mp_string filePath = "test path";
    mp_string fileHMAC = "test file";
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(VerifyFileHmacKmc, StubSuccess);
    mp_int32 ret = VerifyHMAC(filePath, fileHMAC);
    EXPECT_EQ(MP_SUCCESS, ret);
}

TEST_F(CryptAlgTest, RegisterExternalMKTest)
{
    mp_string plainText = "plainText";
    mp_uint32 keyLifeDays = 1;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(AGENT_API::RegisterExternalMk, StubSuccess);
    mp_int32 ret = RegisterExternalMK(plainText, keyLifeDays);
    EXPECT_EQ(MP_SUCCESS, ret);
}


/*
* 用例名称：重启Agent
* 前置条件：无
* check点：检查返回值
*/
TEST_F(CryptAlgTest, RestartAgent)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubFailed);
    mp_int32 ret = RestartAgent();
    EXPECT_EQ(MP_FAILED, ret);

    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubSuccess);
    ret = RestartAgent();
    EXPECT_EQ(MP_SUCCESS, ret);
}

/*
* 用例名称：重启Agent
* 前置条件：无
* check点：检查返回值
*/
TEST_F(CryptAlgTest, ManualUpdateDmcKey)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(CreateInternalMK, StubFailed);
    mp_int32 ret = ManualUpdateDmcKey();
    EXPECT_EQ(MP_FAILED, ret);
}

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
#include "securecom/SDPFuncTest.h"
#include <string>
#include <sstream>
#include <iomanip>
#include "common/Log.h"
#include "common/Path.h"
#include "include/wsecv2_type.h"
#include "wsecv2_itf.h"
#include "src/sdp/sdpv1_itf.h"
#include "include/kmcv2_itf.h"
#include "securec.h"
#include "include/wsecv2_errorcode.h"
#include "securecom/KmcCallback.h"
#include "securecom/SDPFunc.h"
#include "common/ConfigXmlParse.h"

namespace {
static mp_int32 flag = 0;
static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubWSECSuccess(mp_void* pthis)
{
    return WSEC_SUCCESS;
}

mp_int32 StubWSECFailed(mp_void* pthis)
{
    return WSEC_FAILURE;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailedOnTwo(mp_void* pthis)
{
    if (flag ++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_bool StubTrue(mp_void* pthis)
{
    return MP_TRUE;
}

unsigned long StubKmcGetMaxMkIdErrMiss(WsecUint32 domainId, WsecUint32 *maxKeyId)
{
    return WSEC_ERR_KMC_MK_MISS;
}

unsigned long StubKmcGetMaxMkIdErrMk(WsecUint32 domainId, WsecUint32 *maxKeyId)
{
    return WSEC_ERR_KMC_RECREATE_MK;
}

unsigned long StubKmcGetMaxMkId(WsecUint32 domainId, WsecUint32 *maxKeyId)
{
    return WSEC_SUCCESS;
}

unsigned long StubKmcGetDomain(int idx, KmcCfgDomainInfo *domainInfo)
{
    domainInfo->domainKeyFrom = KMC_MK_GEN_BY_INNER;
    return WSEC_SUCCESS;
}

int StubKmcGetDomainCount(void)
{
    return 1;
}
}

/*
 * 用例名称：初始化KMC
 * 前置条件：无
 * check点：InitKMCBase64失败
 */
TEST_F(SDPFuncTest, InitKmcByFile){
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet;
    mp_string kmcStoreFile;
    mp_string kmcStoreBakFile;
    WsecUint32 roleType;
    mp_string kmcConfBakFile;

    {
        // g_bKMCInitialized = false
        iRet = InitKmcByFile(kmcStoreFile, kmcStoreBakFile, roleType, kmcConfBakFile);
        EXPECT_EQ(MP_FAILED, iRet);
    }
}

TEST_F(SDPFuncTest, InitializeKmc){
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string inStr;
    mp_string outStr;
    mp_string filePath;
    mp_string fileHMAC;
    mp_uint32 roleType = 0;
    
    InitializeKmc(roleType);
    ResetKmc();
    TimerKmc();

    EncryptStrKmcWithReset(inStr,outStr);
    DecryptStrKmcWithReset(inStr,outStr);
    GenFileHmacKmcWithReset(filePath,fileHMAC);
    VerifyFileHmacKmcWithReset(filePath,fileHMAC);
}

TEST_F(SDPFuncTest, FinalizeKmc){
    mp_int32 ret;
    
    {
        ret = FinalizeKmc();
        EXPECT_EQ(ret,MP_SUCCESS);
    }
    // {
    //     Initialize_KMC();
    //     ret = FinalizeKmc();
    //     EXPECT_EQ(ret,MP_FAILED);
    // }
}

unsigned long SdpGetCipherDataLenStub(WsecUint32 plainLen, WsecUint32 *cipherLen){
    return 0;
}

unsigned long SdpEncryptStub(WsecUint32 domain, WsecUint32 cipherAlgId, WsecUint32 hmacAlgId,
    const unsigned char *plainText, WsecUint32 plainLen, unsigned char *cipherText, WsecUint32 *cipherLen){
        *cipherLen = 5; // GDB调出来的，不然参数校验不通过，会失败。
        printf("SdpEncryptStub\n");
        return 0;
    }

mp_int32 StubCConfigXmlParserGetValueInt32Fail(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return -1;
}

mp_int32 StubCConfigXmlParserGetValueInt32SuccOutput0(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    iValue = 0;
    return 0;
}

mp_int32 StubCConfigXmlParserGetValueInt32SuccOutput1(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    iValue = 1;
    return 0;
}

mp_int32 StubSDPFuncEncryptStrKmcBase64Fail(const mp_string& inStr, mp_string& outStr)
{
    return -1;
}

mp_int32 StubSDPFuncEncryptStrKmcBase64Succ(const mp_string& inStr, mp_string& outStr)
{
    return 0;
}


TEST_F(SDPFuncTest, EncryptStrKmc){
    mp_int32 ret;
    mp_string inStr;
    mp_string outStr;
    mp_string filePath;
    mp_string fileHMAC;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32Fail);
    {
        ret = EncryptStrKmc(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32SuccOutput0);
    {
        inStr = "test";
        ret = EncryptStrKmc(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    { 
        inStr = "test";
        stub.set(&SdpGetCipherDataLen, SdpGetCipherDataLenStub);
        ret = EncryptStrKmc(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }   
        
    {        
        inStr = "test";
        stub.set(&SdpGetCipherDataLen, SdpGetCipherDataLenStub);
        ret = EncryptStrKmc(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }

    { 
        inStr = "test";
        stub.set(&SdpEncrypt, SdpEncryptStub);
        stub.set(&SdpGetCipherDataLen, SdpGetCipherDataLenStub);
        ret = EncryptStrKmc(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }
}

TEST_F(SDPFuncTest, DecryptStrKmc){
    mp_int32 ret;
    mp_string inStr;
    mp_string outStr;
    mp_string filePath;
    mp_string fileHMAC;
    stub.set(&CLogger::Log, StubCLoggerLog);
    {
        ret = DecryptStrKmc(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        inStr = "test1";
        ret = DecryptStrKmc(inStr,outStr);
        EXPECT_EQ(ret,MP_FAILED);
    }
}

unsigned long SdpGetHmacAlgAttrStub(WsecUint32 domain, WsecUint32 algId, SdpHmacAlgAttributes *hmacAlgAttributes){
    return 0;
}

unsigned long SdpFileHmacStub(WsecUint32 domain,
    const char *file,
    const SdpHmacAlgAttributes *hmacAlgAttributes,
    WsecVoid *hmacData, WsecUint32 *hmacLen){
    return 0;
}

TEST_F(SDPFuncTest, GenFileHmacKmc){
    mp_int32 ret;
    mp_string inStr;
    mp_string outStr;
    mp_string filePath;
    mp_string fileHMAC;
    stub.set(&CLogger::Log, StubCLoggerLog);

    {
        ret = GenFileHmacKmc(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        filePath = "test1";
        ret = GenFileHmacKmc(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }

    {
        filePath = "test";
        stub.set(&SdpGetHmacAlgAttr, SdpGetHmacAlgAttrStub);
        ret = GenFileHmacKmc(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }

    {
        filePath = "test";
        stub.set(&SdpGetHmacAlgAttr, &SdpGetHmacAlgAttrStub);
        stub.set(&SdpFileHmac, &SdpFileHmacStub);
        ret = GenFileHmacKmc(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_SUCCESS);
    }
}

TEST_F(SDPFuncTest, VerifyFileHmacKmc){
    mp_int32 ret;
    mp_string inStr;
    mp_string outStr;
    mp_string filePath;
    mp_string fileHMAC;
    stub.set(&CLogger::Log, StubCLoggerLog);

    {
        ret = VerifyFileHmacKmc(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }
    
    {
        filePath = "test1";
        fileHMAC = "DDDdkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkktestfdfdfdfdfdf12311555151ffffffffff";
        ret = VerifyFileHmacKmc(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }

    {
        filePath = "test1";
        fileHMAC = "DDDdkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkktestfdfdfdfdfdf12311555151ffffffffff";
        stub.set(memcpy_s, StubFailed);
        ret = VerifyFileHmacKmc(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }

    {
        filePath = "test1";
        fileHMAC = "DDDdkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkktestfdfdfdfdfdf12311555151ffffffffff";
        stub.set(memcpy_s, StubFailedOnTwo);
        ret = VerifyFileHmacKmc(filePath,fileHMAC);
        EXPECT_EQ(ret,MP_FAILED);
    }
}

TEST_F(SDPFuncTest, GetExternalDomainInfo){
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(CheckIfDomainExists, StubTrue);
    mp_int32 ret =GetExternalDomainInfo();
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(SDPFuncTest, CheckIfDomainExists){
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_uint32 domainId;

    stub.set(KmcGetMaxMkId, StubKmcGetMaxMkIdErrMiss);
    mp_int32 ret =CheckIfDomainExists(domainId);
    EXPECT_EQ(ret, MP_FALSE);

    stub.set(KmcGetMaxMkId, StubKmcGetMaxMkIdErrMk);
    ret =CheckIfDomainExists(domainId);
    EXPECT_EQ(ret, MP_FALSE);

     stub.set(KmcGetMaxMkId, StubKmcGetMaxMkId);
    ret =CheckIfDomainExists(domainId);
    EXPECT_EQ(ret, MP_TRUE);
}

/*
 * 用例名称：Register new KMC domain and key type
 * 前置条件：no
 * check点：register success
 */
TEST_F(SDPFuncTest, RegisterPrivateDomain){
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(memcpy_s, StubFailed);
    mp_int32 ret = RegisterPrivateDomain();
    EXPECT_EQ(ret, MP_FAILED);

    flag = 0;
    stub.set(memcpy_s, StubFailedOnTwo);
    ret = RegisterPrivateDomain();
    EXPECT_EQ(ret, MP_FAILED);

    stub.set(memcpy_s, StubSuccess);
    stub.set(KmcAddDomainEx, StubWSECFailed);
    ret = RegisterPrivateDomain();
    EXPECT_EQ(ret, MP_FAILED);

    stub.set(KmcAddDomainEx, StubWSECSuccess);
    stub.set(KmcAddDomainKeyTypeEx, StubWSECFailed);
    ret = RegisterPrivateDomain();
    EXPECT_EQ(ret, MP_FAILED);

    stub.set(KmcAddDomainKeyTypeEx, StubWSECSuccess);
    ret = RegisterPrivateDomain();
    EXPECT_EQ(ret, MP_SUCCESS);
}

/*
 * 用例名称：Create a new KMC key.
 * 前置条件：no
 * check点：register success
 */
TEST_F(SDPFuncTest, CreateInternalMK){
    stub.set(&CLogger::Log, StubCLoggerLog);

    {
        stub.set(KmcGetDomainCount, StubFailed);
        mp_int32 ret = CreateInternalMK();
        EXPECT_EQ(ret, MP_FAILED);
    }

    {
        stub.set(KmcGetDomainCount, StubKmcGetDomainCount);
        stub.set(KmcGetDomain, StubWSECFailed);
        mp_int32 ret = CreateInternalMK();
        EXPECT_EQ(ret, MP_FAILED);
    }

    {
        stub.set(KmcGetDomainCount, StubKmcGetDomainCount);
        stub.set(KmcGetDomain, StubKmcGetDomain);
        stub.set(KmcCreateMkEx, StubWSECFailed);
        mp_int32 ret = CreateInternalMK();
        EXPECT_EQ(ret, MP_FAILED);
    }

    {
        stub.set(KmcGetDomainCount, StubKmcGetDomainCount);
        stub.set(KmcGetDomain, StubKmcGetDomain);
        stub.set(KmcCreateMkEx, StubWSECSuccess);
        stub.set(KmcActivateMk, StubWSECFailed);
        mp_int32 ret = CreateInternalMK();
        EXPECT_EQ(ret, MP_FAILED);
    }

    {
        stub.set(KmcGetDomainCount, StubKmcGetDomainCount);
        stub.set(KmcGetDomain, StubKmcGetDomain);
        stub.set(KmcCreateMkEx, StubWSECSuccess);
        stub.set(KmcActivateMk, StubWSECSuccess);
        mp_int32 ret = CreateInternalMK();
        EXPECT_EQ(ret, MP_SUCCESS);
    }

}

TEST_F(SDPFuncTest, RegisterExternalMk)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    mp_string plainTextKey;
    mp_uint32 keyLifeDays;
    EXPECT_EQ(MP_FAILED, RegisterExternalMk("", 0));
    EXPECT_EQ(MP_FAILED, RegisterExternalMk("99999999999999999999999999999999", 0));

    stub.set(&KmcRmvDomainKeyTypeEx, StubWSECFailed);
    EXPECT_EQ(MP_FAILED, RegisterExternalMk("99999999999999999999999999999999", 30));

    stub.set(&KmcRmvDomainKeyTypeEx, StubWSECSuccess);
    stub.set(&KmcAddDomainKeyTypeEx, StubWSECFailed);
    EXPECT_EQ(MP_FAILED, RegisterExternalMk("99999999999999999999999999999999", 30));

    stub.set(&KmcRmvDomainKeyTypeEx, StubWSECSuccess);
    stub.set(&KmcAddDomainKeyTypeEx, StubWSECSuccess);
    stub.set(&KmcGetMaxMkId, StubWSECFailed);
    EXPECT_EQ(MP_FAILED, RegisterExternalMk("99999999999999999999999999999999", 30));

    stub.set(&KmcRmvDomainKeyTypeEx, StubWSECSuccess);
    stub.set(&KmcAddDomainKeyTypeEx, StubWSECSuccess);
    stub.set(&KmcGetMaxMkId, StubWSECSuccess);
    stub.set(&KmcRegisterMkEx, StubWSECFailed);
    EXPECT_EQ(MP_FAILED, RegisterExternalMk("99999999999999999999999999999999", 30));

    stub.set(&KmcRmvDomainKeyTypeEx, StubWSECSuccess);
    stub.set(&KmcAddDomainKeyTypeEx, StubWSECSuccess);
    stub.set(&KmcGetMaxMkId, StubWSECSuccess);
    stub.set(&KmcRegisterMkEx, StubWSECSuccess);
    stub.set(&KmcActivateMk, StubWSECFailed);
    EXPECT_EQ(WSEC_FAILURE, RegisterExternalMk("99999999999999999999999999999999", 30));

    stub.set(&KmcRmvDomainKeyTypeEx, StubWSECSuccess);
    stub.set(&KmcAddDomainKeyTypeEx, StubWSECSuccess);
    stub.set(&KmcGetMaxMkId, StubWSECSuccess);
    stub.set(&KmcRegisterMkEx, StubWSECSuccess);
    stub.set(&KmcActivateMk, StubWSECSuccess);
    EXPECT_EQ(MP_SUCCESS, RegisterExternalMk("99999999999999999999999999999999", 30));
}
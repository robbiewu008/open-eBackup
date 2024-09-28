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
#include "securecom/PasswordTest.h"

namespace {
mp_int32 flag = 0;
static mp_int32 StubCConfigXmlParserGetValueInt32ReturnSuccess(mp_void* pthis, mp_string strSection, mp_string strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubCMpStringGetCh()
{
    mp_int32 a = 0x32323232;
    return a;
}

mp_int32 StubCMpStringGetEnter()
{
    mp_int32 a = 13;
    return a;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_int32 StubTrue(mp_void* pthis)
{
    return MP_TRUE;
}

mp_int32 StubCPasswordGetValueStringFailedOnTwo(mp_string strSection, mp_string strKey, mp_string& strValue){
    if (flag ++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_void StubDecryptStr(const mp_string& inStr, mp_string& outStr)
{
    outStr = "test";
    return;
}

mp_int32 StubCalculateComplexity(mp_void* pthis)
{
    return 3;
}

mp_int32 StubCalculateComplexityFailed(mp_void* pthis)
{
    return 1;
}

mp_int32 StubReadFileSucc(mp_string& strFilePath, vector<mp_string>& vecOutput){
    vecOutput.push_back("ssl_certificate");
    vecOutput.push_back("ssl_certificate_key_password");
    return MP_SUCCESS;
}

mp_int32 StubGetchar()
{
    return 'n';
}

mp_void StubEncryptStr(const mp_string& inStr, mp_string& outStr)
{
    outStr = "test";
}
mp_void StubDecryptStrKMC(const mp_string& inStr, mp_string& outStr)
{
    outStr = "test";
}
}

TEST_F(CPasswordTest,ChgPwd){
    mp_string strPwd = "test";
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CMpString,GetCh), StubCMpStringGetCh);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubInputUserPwd);
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCPasswordGetValueString);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL,strPwd);
    }
    
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCPasswordGetValueString0);
        stub.set(&CPassword::CheckNewPwd, StubCheckNewPwdPwd);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL,strPwd);
    }
    
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCPasswordGetValueString0);
        stub.set(&CPassword::CheckNewPwd, StubCheckNewPwdPwd0);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL);
        CPassword::ChgPwd(PASSWORD_NGINX_SSL,strPwd);
    }
}


/*
 * 用例名称：检查密码循环重叠
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CPasswordTest,CheckPasswordOverlap){
	mp_string strPasswd = "Admin@123";
    mp_int32 iRet;

    iRet = CPassword::CheckPasswordOverlap(strPasswd);
    EXPECT_EQ(MP_FALSE, iRet);

    strPasswd = "";
    iRet = CPassword::CheckPasswordOverlap(strPasswd);
    EXPECT_EQ(MP_FALSE, iRet);
}

TEST_F(CPasswordTest,ChgAdminPwd){
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CMpString::GetCh, StubGetCh);
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString);
        iRet = CPassword::ChgAdminPwd();
        EXPECT_EQ(iRet, MP_FAILED);
    }
    
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        stub.set(&CPassword::VerifyOldUserPwd, &StubVerifyOldUserPwd);
        iRet = CPassword::ChgAdminPwd();
        EXPECT_EQ(iRet, MP_FAILED);
    }

    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        stub.set(&CPassword::VerifyOldUserPwd, &StubVerifyOldUserPwd0);
        stub.set(&CPassword::InputNewUserPwd, &StubInputNewUserPwd);
        iRet = CPassword::ChgAdminPwd();
        EXPECT_EQ(iRet, MP_FAILED);
    }
    
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        stub.set(&CPassword::VerifyOldUserPwd, &StubVerifyOldUserPwd0);
        stub.set(&CPassword::InputNewUserPwd, &StubInputNewUserPwd0);
        stub.set(&CPassword::ConfirmNewUserPwd, &StubConfirmNewUserPwd);
        iRet = CPassword::ChgAdminPwd();
        EXPECT_EQ(iRet, MP_FAILED);
    }

    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        stub.set(&CPassword::VerifyOldUserPwd, &StubVerifyOldUserPwd0);
        stub.set(&CPassword::InputNewUserPwd, &StubInputNewUserPwd0);
        stub.set(&CPassword::ConfirmNewUserPwd, &StubConfirmNewUserPwd0);
        stub.set(&CPassword::SaveAdminPwd, &StubSaveAdminPwd);
        iRet = CPassword::ChgAdminPwd();
        EXPECT_EQ(iRet, MP_SUCCESS);
    }

    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        stub.set(&CPassword::VerifyOldUserPwd, &StubVerifyOldUserPwd0);
        stub.set(&CPassword::InputNewUserPwd, &StubInputNewUserPwd0);
        stub.set(&CPassword::ConfirmNewUserPwd, &StubConfirmNewUserPwd0);
        stub.set(&CPassword::SaveAdminPwd, &StubSaveAdminPwdFailed);
        iRet = CPassword::ChgAdminPwd();
        EXPECT_EQ(iRet, MP_FAILED);
    }
}

TEST_F(CPasswordTest,VerifyOldUserPwd){
    mp_string strUserName = "test";
    mp_int32 iRet;
    
    stub.set(&CPassword::InputUserPwd, &StubInputUserPwd);
    {
        stub.set(&CPassword::CheckAdminOldPwd, &StubCheckAdminOldPwd);
        iRet = CPassword::VerifyOldUserPwd(strUserName);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }
    
    {
        stub.set(&CPassword::CheckAdminOldPwd, &StubCheckAdminOldPwd0);
        iRet = CPassword::VerifyOldUserPwd(strUserName);
        EXPECT_EQ(iRet, MP_FAILED);
    }
}

TEST_F(CPasswordTest,InputNewUserPwd){
    mp_string strUserName = "test";
    mp_string strNewPwd = "test";
    mp_int32 iRet;
    
    stub.set(&CPassword::InputUserPwd, &StubInputUserPwd);
    {
        stub.set(&CPassword::CheckNewPwd, &StubCheckNewPwd);
        iRet = CPassword::InputNewUserPwd(strUserName,strNewPwd);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }
    
    {
        stub.set(&CPassword::CheckNewPwd, &StubCheckNewPwd0);
        iRet = CPassword::InputNewUserPwd(strUserName,strNewPwd);\
        EXPECT_EQ(iRet, MP_FAILED);
    }
}

TEST_F(CPasswordTest,ConfirmNewUserPwd){
    mp_string strUserName = "test";
    mp_string strNewPwd = "test";
    mp_int32 iRet;
    
    stub.set(&CMpString::GetCh, &StubGetCh);
    iRet = CPassword::ConfirmNewUserPwd(strUserName,strNewPwd);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CMpString::GetCh, &StubGetCh);
    stub.set(&CPassword::InputUserPwd, StubInputUserPwd1);
    iRet = CPassword::ConfirmNewUserPwd(strUserName,strNewPwd);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CPasswordTest,InputUserPwd){
    mp_string strUserName = "test";
    mp_string strUserPwd = "test";
    INPUT_TYPE eType;
    
    stub.set(&CMpString::GetCh, &StubGetCh);
    {
        eType = INPUT_GET_ADMIN_OLD_PWD;
        CPassword::InputUserPwd(strUserName,strUserPwd,eType);
    }
    
    {
        eType = INPUT_SNMP_OLD_PWD;
        CPassword::InputUserPwd(strUserName,strUserPwd,eType);
    }
    
    {
        eType = INPUT_DEFAULT;
        CPassword::InputUserPwd(strUserName,strUserPwd,eType);
    }
}

TEST_F(CPasswordTest,CheckAdminOldPwd){
    mp_string strOldPwd = "test";
    mp_int32 iRet;

    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CMpString::GetCh, &StubGetCh);
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString);
        iRet = CPassword::CheckAdminOldPwd(strOldPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        stub.set(&PBKDF2Hash, &StubPBKDF2Hash);
        iRet =  CPassword::CheckAdminOldPwd(strOldPwd);
        EXPECT_EQ(iRet, MP_TRUE);
    }
    
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        stub.set(&PBKDF2Hash, &StubPBKDF2Hash);
        iRet = CPassword::CheckAdminOldPwd(strOldPwd);
        EXPECT_EQ(iRet, MP_TRUE);
    }
    
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        stub.set(&GetSha256Hash, &StubGetSha256Hash);
        iRet = CPassword::CheckAdminOldPwd(strOldPwd);
        EXPECT_EQ(iRet, MP_FAILED);
    }

    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString),  &StubCPasswordGetValueStringFailedOnTwo);
        stub.set(&PBKDF2Hash, &StubSuccess);
        stub.set(&GetSha256Hash, &StubGetSha256Hash);
        iRet = CPassword::CheckAdminOldPwd(strOldPwd);
        EXPECT_EQ(iRet, MP_FAILED);
    }
}

TEST_F(CPasswordTest,CheckOtherOldPwd){
    PASSWOD_TYPE eType;
    mp_string strPwd = "test";
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);

    {
        eType = PASSWORD_INPUT;
        iRet = CPassword::CheckOtherOldPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        eType = PASSWORD_INPUT;
        iRet = CPassword::CheckOtherOldPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        eType = PASSWORD_SNMP_PRIVATE;
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString);
        iRet = CPassword::CheckOtherOldPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        eType = PASSWORD_SNMP_AUTH;
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        iRet = CPassword::CheckOtherOldPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }

    {
        eType = PASSWORD_NGINX_SSL;
        stub.set(&CPassword::CheckNginxOldPwd, StubTrue);
        iRet = CPassword::CheckOtherOldPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_TRUE);
    }
}

TEST_F(CPasswordTest,CheckNginxOldPwd){
    mp_string strOldPwd = "test";
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    {
        stub.set(&CMpFile::FileExist, &CPasswordStubFileExist);
        iRet = CPassword::CheckNginxOldPwd(strOldPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        stub.set(&CMpFile::FileExist, &CPasswordStubFileExist0);
        stub.set(&CIPCFile::ReadFile, &StubReadFile);
        iRet = CPassword::CheckNginxOldPwd(strOldPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        stub.set(&CMpFile::FileExist, &CPasswordStubFileExist0);
        stub.set(&CIPCFile::ReadFile, &StubReadFile0);
        iRet = CPassword::CheckNginxOldPwd(strOldPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }

    {
        stub.set(&CMpFile::FileExist, CPasswordStubFileExist);
        stub.set(&CMpFile::ReadFile, StubReadFileSucc);
        stub.set(&CPassword::GetNginxKey, StubTrue);
        stub.set(DecryptStr, StubDecryptStr);
        iRet = CPassword::CheckNginxOldPwd(strOldPwd);
        EXPECT_EQ(iRet, MP_TRUE);
        stub.reset(&CPassword::GetNginxKey);
        stub.reset(DecryptStr);
    }
}

TEST_F(CPasswordTest,GetNginxKey){
    mp_string strKey = "test";
    vector<mp_string> vecResult;
    vecResult.push_back("ssl_certificate_key_password");
    
    CPassword::GetNginxKey(strKey,vecResult);

    vecResult.clear();
    vecResult.push_back("ssl_certificate_key_password;;");
    CPassword::GetNginxKey(strKey,vecResult);
}

TEST_F(CPasswordTest,CheckNewPwd){
    PASSWOD_TYPE eType = PASSWORD_ADMIN;
    mp_string strPwd = "test";
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&PBKDF2Hash, StubSuccess);
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString);
        iRet = CPassword::CheckNewPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }

    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueStringl);
        iRet = CPassword::CheckNewPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        iRet = CPassword::CheckNewPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        eType = PASSWORD_SNMP_PRIVATE;
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        iRet = CPassword::CheckNewPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    } 

    {
        eType = PASSWORD_SNMP_PRIVATE;
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubCPasswordGetValueString0);
        stub.set(&CPassword::CheckCommon, StubTrue);
        stub.set(&CPassword::CheckPasswordOverlap, StubTrue);
        iRet = CPassword::CheckNewPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
}

TEST_F(CPasswordTest,SaveAdminPwd){
    mp_string strPwd = "test";
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    {
        stub.set(&GetRandom, &StubGetRandom);
        iRet = CPassword::SaveAdminPwd(strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        stub.set(&GetRandom, &StubGetRandom0);
        stub.set(&PBKDF2Hash, &StubPBKDF2Hash);
        iRet = CPassword::SaveAdminPwd(strPwd);
        EXPECT_EQ(iRet, MP_TRUE);
    }
    
    {
        stub.set(&GetRandom, &StubGetRandom0);
        stub.set(&PBKDF2Hash, &StubPBKDF2Hash0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue), &StubSetValue);
        iRet = CPassword::SaveAdminPwd(strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        stub.set(&GetRandom, &StubGetRandom0);
        stub.set(&PBKDF2Hash, &StubPBKDF2Hash0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue), &StubSetValue0);
        iRet = CPassword::SaveAdminPwd(strPwd);
        EXPECT_EQ(iRet, MP_TRUE);
    }
}

TEST_F(CPasswordTest,SaveOtherPwd){
    PASSWOD_TYPE eType;
    mp_string strPwd = "test";
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    {
        eType = PASSWORD_NGINX_SSL;
        iRet = CPassword::SaveOtherPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        eType = PASSWORD_SNMP_AUTH;
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue), &StubSetValue);
        iRet = CPassword::SaveOtherPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        eType = PASSWORD_SNMP_PRIVATE;
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue), &StubSetValue0);
        iRet = CPassword::SaveOtherPwd(eType,strPwd);
        EXPECT_EQ(iRet, MP_TRUE);
    }

}

TEST_F(CPasswordTest,SaveNginxPwd){
    mp_string strPwd = "test";
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    {
        stub.set(&CMpFile::FileExist, &CPasswordStubFileExist0);
        iRet = CPassword::SaveNginxPwd(strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        stub.set(&CMpFile::FileExist, &CPasswordStubFileExist);
        stub.set(&CMpFile::ReadFile, &StubReadFile);
        iRet = CPassword::SaveNginxPwd(strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }
    
    {
        stub.set(&CMpFile::FileExist, &CPasswordStubFileExist);
        stub.set(&CMpFile::ReadFile, &StubReadFile0);
        iRet = CPassword::SaveNginxPwd(strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }

    {
        stub.set(&CMpFile::FileExist, &CPasswordStubFileExist);
        stub.set(&CMpFile::ReadFile, &StubReadFileSucc);
        stub.set(&CIPCFile::WriteFile, StubFailed);
        iRet = CPassword::SaveNginxPwd(strPwd);
        EXPECT_EQ(iRet, MP_FALSE);
    }

    {
        stub.set(&CMpFile::FileExist, &CPasswordStubFileExist);
        stub.set(&CMpFile::ReadFile, &StubReadFileSucc);
        stub.set(&CIPCFile::WriteFile, StubSuccess);
        iRet = CPassword::SaveNginxPwd(strPwd);
        EXPECT_EQ(iRet, MP_TRUE);
    }
}

TEST_F(CPasswordTest,CalComplexity){
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strPwd;
    mp_int32 iNum;
    mp_int32 iUppercase;
    mp_int32 iLowcase;
    mp_int32 iSpecial;
    mp_int32 iRet;
    
    strPwd ="1";
    iRet = CPassword::CalComplexity(strPwd,iNum,iUppercase,iLowcase,iSpecial);
    EXPECT_EQ(iRet, MP_FAILED);

    strPwd ="A";
    iRet = CPassword::CalComplexity(strPwd,iNum,iUppercase,iLowcase,iSpecial);
    EXPECT_EQ(iRet, MP_FAILED);

    strPwd ="a";
    iSpecial = 0;
    iRet = CPassword::CalComplexity(strPwd,iNum,iUppercase,iLowcase,iSpecial);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：CheckCommon功能
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(CPasswordTest,CheckCommon){
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strPwd = "abcde1234";
    mp_int32 iRet;

    stub.set(&CPassword::CalComplexity, StubSuccess);
    stub.set(&CPassword::CalculateComplexity, StubCalculateComplexity);
    iRet = CPassword::CheckCommon(strPwd);
    EXPECT_EQ(iRet, MP_TRUE);

    stub.set(&CPassword::CalculateComplexity, StubCalculateComplexityFailed);
    iRet = CPassword::CheckCommon(strPwd);
    EXPECT_EQ(iRet, MP_FALSE);
}

TEST_F(CPasswordTest,CalculateComplexity){
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iNumber = 1;
    mp_int32 iUppercase = 1;
    mp_int32 iLowcase = 1;
    mp_int32 iRet;
    
    iRet = CPassword::CalculateComplexity( iNumber,  iUppercase,  iLowcase);
    EXPECT_EQ(iRet, 3);
}


TEST_F(CPasswordTest,GetInput){
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string strHint = "test";
    mp_string strInput = "test";
    
    stub.set(&CMpString::GetCh, &StubGetCh);
    CPassword::GetInput( strHint, strInput);
}

TEST_F(CPasswordTest,GetLockTime){
    mp_uint64 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);

    iRet = CPassword::GetLockTime();

    stub.set(&CMpFile::ReadFile, StubFailed);
    iRet = CPassword::GetLockTime();
    EXPECT_EQ(iRet, 0);
}

TEST_F(CPasswordTest,ClearLock){
    stub.set(&CLogger::Log, StubCLoggerLog);
    CPassword::ClearLock();
}

TEST_F(CPasswordTest,ChgPwdNoCheck){
    mp_string strPwd = "test";
    mp_int32 iRet;
    stub.set(ADDR(CMpString,GetCh), StubCMpStringGetCh);
    stub.set(&CPassword::InputUserPwd, StubInputUserPwd);
    iRet = CPassword::ChgPwdNoCheck(strPwd);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CPasswordTest,GetInputEchoWithStar){
    mp_string strPwd = "test";
    mp_int32 iRet;
    mp_string inPut;
    stub.set(ADDR(CMpString,GetCh), StubCMpStringGetEnter);
    CPassword::GetInputEchoWithStar(strPwd, inPut, 5);
}

TEST_F(CPasswordTest,LockAdmin){
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CIPCFile, WriteFile), StubSuccess);
    CPassword::LockAdmin();

    stub.set(ADDR(CIPCFile, WriteFile), StubFailed);
    CPassword::LockAdmin();
}

TEST_F(CPasswordTest,EncPwd)
{
    mp_string strPwd = "test";
    mp_string strvalue = "";
    mp_int32 iRet;
    mp_string inPut;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CMpString,GetCh), StubCMpStringGetCh);

    iRet = CPassword::EncPwd(strPwd, strvalue);
    EXPECT_EQ(iRet, MP_FAILED);

    strvalue = "test";
    stub.set(EncryptStr, StubEncryptStr);
    iRet = CPassword::EncPwd(strPwd, strvalue);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CPasswordTest,DecPwdAndConfuse)
{
    mp_string strPwd = "test";
    mp_int32 iRet;
    mp_string text;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CMpString,GetCh), StubCMpStringGetCh);

    iRet = CPassword::DecPwdAndConfuse(strPwd,text);
    EXPECT_EQ(iRet, MP_FAILED);

     stub.set(DecryptStrKMC, StubDecryptStrKMC);
    iRet = CPassword::DecPwdAndConfuse(strPwd,text);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CPasswordTest,Confuse)
{
    mp_string inStr = "test";
    mp_int32 iRet;
    mp_string outStr;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CMpString,GetCh), StubCMpStringGetCh);

    CPassword::Confuse(inStr,outStr);
}

TEST_F(CPasswordTest,VerifyAgentUser)
{
    mp_string useName = "test";
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&PBKDF2Hash, StubSuccess);
    stub.set(ADDR(CMpString,GetCh), StubCMpStringGetCh);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString),StubReturnResultSetSuccess);

    EXPECT_EQ(MP_SUCCESS, CPassword::VerifyAgentUser(useName));
}

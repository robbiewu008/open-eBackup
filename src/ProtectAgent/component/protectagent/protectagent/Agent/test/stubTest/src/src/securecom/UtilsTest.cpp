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
#include "securecom/UtilsTest.h"
#include <vector>
#include "common/CSystemExec.h"
#include "securecom/SDPFunc.h"

using namespace std;

namespace {
mp_int32 flag = 0;
mp_bool StubTrue(mp_void* pthis)
{
    return MP_TRUE;
}

mp_bool StubFalse(mp_void* pthis)
{
    return MP_FALSE;
}

mp_bool StubTrueTwo(mp_void* pthis)
{
    if (flag ++ < 1) {
        return MP_FALSE;
    }
    return MP_TRUE;
}

mp_bool StubTrueThree(mp_void* pthis)
{
    if (flag ++ < 2) {
        return MP_FALSE;
    }
    return MP_TRUE;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailedTwo(mp_void* pthis)
{
    if (flag ++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_string StubBlankComma(const mp_string& strPath)
{
    return "test";
}

BIO* StubBioNotNull(mp_void *pobj)
{
    BIO* bio;
    return bio;
}

X509* StubX509NotNull(mp_void *pobj)
{
    X509* x509;
    return x509;
}

X509* StubX509Null(mp_void *pobj)
{
    return nullptr;
}
}

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

/*
 * 用例名称：获取OS类型，需要使用root权限执行
 * 前置条件：无
 * check点：判断操作系统类型
 */
TEST_F(UtilsTest,GetOSType){
    mp_int32 iOSType;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(&CMpFile::FileExist, StubTrue);
    SecureCom::GetOSType(iOSType);
    EXPECT_EQ(HOST_OS_SUSE, iOSType);

    flag = 0;
    stub.set(&CMpFile::FileExist, StubTrueTwo);
    SecureCom::GetOSType(iOSType);
    EXPECT_EQ(HOST_OS_ORACLE_LINUX, iOSType);

    flag = 0;
    stub.set(&CMpFile::FileExist, StubTrueThree);
    SecureCom::GetOSType(iOSType);
    EXPECT_EQ(HOST_OS_REDHAT, iOSType);

    stub.set(&CMpFile::FileExist, StubFalse);
    SecureCom::GetOSType(iOSType);
    EXPECT_EQ(HOST_OS_OTHER_LINUX, iOSType);
}

/*
 * 用例名称：获取OS版本信息
 * 前置条件：无
 * check点：判断操作系统类型获取版本信息
 */
TEST_F(UtilsTest,GetOSVersion){
    mp_int32 iRet;
    mp_int32 iOSType;
    mp_string strOSVersion;
    stub.set(&CLogger::Log, StubCLoggerLog);

    iOSType = HOST_OS_SUSE;
    stub.set(&CSystemExec::ExecSystemWithEchoNoWin, StubSuccess);
    iRet = SecureCom::GetOSVersion(iOSType, strOSVersion);
    EXPECT_EQ(MP_FAILED, iRet);

    iOSType = HOST_OS_AIX;
    iRet = SecureCom::GetOSVersion(iOSType, strOSVersion);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(UtilsTest,GetAdminUserInfo){
    mp_int32 iRet = 0;
    mp_string userName = "123";
    mp_string userPwd = "456";
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubGetValueString_fail);
    iRet = SecureCom::GetAdminUserInfo(userName, userPwd);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubGetValueString_succ);
    iRet = SecureCom::GetAdminUserInfo(userName, userPwd);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(DecryptStrKmc, StubSuccess);
    iRet = SecureCom::GetAdminUserInfo(userName, userPwd);
    EXPECT_EQ(MP_SUCCESS, iRet);

    flag = 0;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubFailedTwo);
    iRet = SecureCom::GetAdminUserInfo(userName, userPwd);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
 * 用例名称：生成所有脚本的签名
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(UtilsTest,GenSignFile){
    mp_int32 iRet = 0;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(&CMpFile::FileExist, StubTrue);
    stub.set(&CMpFile::DelFile, StubFailed);
    iRet = SecureCom::GenSignFile();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CMpFile::DelFile, StubSuccess);
    stub.set(SecureCom::GenScriptSign, StubSuccess);
    stub.set(&CIPCFile::WriteFile, StubSuccess);
    iRet = SecureCom::GenSignFile();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 用例名称：检查执行参数
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(UtilsTest,CheckExecParam){
    mp_int32 iRet = 0;
    mp_string strScriptFileName;
    mp_string strParam;
    vector<mp_string> pvecResult;
    mp_bool bNeedCheckSign;
    stub.set(&CLogger::Log, StubCLoggerLog);

    bNeedCheckSign = MP_TRUE;
    stub.set(&CMpFile::FileExist, StubTrue);
    SecureCom::SysExecScript(strScriptFileName, strParam, &pvecResult, bNeedCheckSign);

    bNeedCheckSign = MP_FALSE;
    stub.set(&CMpString::BlankComma, StubBlankComma);
    stub.set(&CIPCFile::WriteInput, StubFailed);
    SecureCom::SysExecScript(strScriptFileName, strParam, &pvecResult, bNeedCheckSign);
}

TEST_F(UtilsTest,PackageLog){
    mp_string strLogName;
    stub.set(&CLogger::Log, StubCLoggerLog);
    SecureCom::PackageLog(strLogName);
}

TEST_F(UtilsTest,CryptoThreadCleanup){
    stub.set(&CLogger::Log, StubCLoggerLog);
    SecureCom::CryptoThreadCleanup();
}

TEST_F(UtilsTest,CryptoThreadSetup){
    SecureCom::CryptoThreadSetup();
}

// TEST_F(UtilsTest,SignalRegister){
//     mp_int32 signo;
//     signal_proc func;
//     stub.set(&CLogger::Log, StubCLoggerLog);
//     SignalRegister(signo,func);
// }

// TEST_F(UtilsTest,DlibOpen){
//     mp_char pszLibName;
//     stub.set(&CLogger::Log, StubCLoggerLog);
//     DlibOpen(&pszLibName);
// }

// TEST_F(UtilsTest,DlibOpenEx){
//     mp_char pszLibName;
//     mp_bool bLocal;
//     stub.set(&CLogger::Log, StubCLoggerLog);

//     DlibOpenEx(&pszLibName,bLocal);
// }

// TEST_F(UtilsTest,DlibClose){
//     mp_handle_t hDlib;
//     stub.set(&CLogger::Log, StubCLoggerLog);

//     DlibClose(hDlib);
// }

// TEST_F(UtilsTest,DlibDlsym){
//     mp_char pszFname;
//     mp_handle_t hDlib;
    
// //    DlibDlsym(hDlib,&pszFname);
// }

// thread_lock_t GetValueStub()
// {
//     thread_lock_t plock;
//     return plock;
// }



// TEST_F(UtilsTest,InitCommonModules){
//     mp_string pszFullBinPath;
//     stub.set(&CLogger::Log, StubCLoggerLog);

//     InitCommonModules(pszFullBinPath);
    
//     stub.set(&CPath::Init, &StubCPathInit);
//     InitCommonModules(pszFullBinPath);
    
//     stub.set(&CConfigXmlParser::Init, &StubCConfigXmlParserInit);
//     InitCommonModules(pszFullBinPath);
// }

// TEST_F(UtilsTest,GetHostName){
//     mp_string strHostName;
//     stub.set(&CLogger::Log, StubCLoggerLog);

//     GetHostName(strHostName);
// }

// TEST_F(UtilsTest,GetOSError){
//     GetOSError();
// }
// /*
// TEST_F(UtilsTest,GetOSStrErr){
//     mp_int32 err;
// 	mp_char buf;
// 	mp_size buf_len;
	
//     GetOSStrErr(err,&buf,buf_len);
// }*/

// TEST_F(UtilsTest,GetCurrentUserName){
//      mp_string strUserName;
//      mp_ulong iErrCode;
//     stub.set(&CLogger::Log, StubCLoggerLog);

//      GetCurrentUserName(strUserName,iErrCode) ;
// }

// TEST_F(UtilsTest,ChownFile){
//      mp_string strFileName;
//      mp_int32 uid;
//      mp_int32 gid; 
//     stub.set(&CLogger::Log, StubCLoggerLog);

//      ChownFile(strFileName,uid,gid);
// }

TEST_F(UtilsTest,CheckParamString)
{
    mp_string paramValue;
    mp_int32 lenBeg;
    mp_int32 lenEnd;
    mp_string strEnd;
    mp_string strEnd1;
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckParamString(paramValue,lenBeg,lenEnd,strEnd,strEnd1);

    {
        paramValue = "12345";
        lenBeg = 1; 
        lenEnd = 100;
        strEnd = "/";

        CheckParamString(paramValue,lenBeg,lenEnd,strEnd,strEnd1);
    }
}

TEST_F(UtilsTest,CheckParamString1)
{
    mp_string paramValue;
    mp_int32 lenBeg;
    mp_int32 lenEnd;
    mp_string strEnd;
    stub.set(&CLogger::Log, StubCLoggerLog);

    CheckParamString(paramValue,lenBeg,lenEnd,strEnd);  	
    {
        paramValue = "12345";
        lenBeg = 1;
        lenEnd = 100;
        strEnd = "/";

        CheckParamString(paramValue,lenBeg,lenEnd,strEnd);
    }
}

TEST_F(UtilsTest,CheckParamStringEnd)
{
    mp_string paramValue;
    mp_int32 lenBeg;
    mp_int32 lenEnd;
    mp_string strEnd;
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckParamStringEnd(paramValue,lenBeg,lenEnd,strEnd);
	
	{
		paramValue = "12345";
		lenBeg = 1;
		lenEnd = 100;
		strEnd = "/";
		
		CheckParamStringEnd(paramValue,lenBeg,lenEnd,strEnd);
	}
}

TEST_F(UtilsTest,CheckParamInteger32)
{
    mp_int32 paramValue;
    mp_int32 begValue;
    mp_int32 endValue;
    vector<mp_int32> vecExclude;
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckParamInteger32(paramValue,begValue,endValue,vecExclude);
	
	{
	    paramValue = 5;
		begValue = 1;
		endValue = 2;
		
		CheckParamInteger32(paramValue,begValue,endValue,vecExclude);
	}
	
		{
	    paramValue = 5;
		begValue = 1;
		endValue = 10;
		
		CheckParamInteger32(paramValue,begValue,endValue,vecExclude);
	}
}

TEST_F(UtilsTest,CheckParamInteger64)
{
    mp_int64 paramValue;
    mp_int64 begValue;
    mp_int64 endValue;
    vector<mp_int64> vecExclude;
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckParamInteger64(paramValue,begValue,endValue,vecExclude);
	
	{
	    paramValue = 5;
		begValue = 1;
		endValue = 2;
		
		CheckParamInteger64(paramValue,begValue,endValue,vecExclude);
	}
	
		{
	    paramValue = 5;
		begValue = 1;
		endValue = 10;
		
		CheckParamInteger64(paramValue,begValue,endValue,vecExclude);
	}
}

TEST_F(UtilsTest,CheckParamStringIsIP)
{
    mp_string paramValue;
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckParamStringIsIP(paramValue);
}

TEST_F(UtilsTest,CheckPathString)
{
    mp_string pathValue;
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckPathString(pathValue);
}

TEST_F(UtilsTest,CheckPathString1)
{
    mp_string pathValue = "//1234";
	mp_string strPre = "/";
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckPathString(pathValue,strPre);
}

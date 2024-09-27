#include "common/UtilsTest.h"
#include "common/ErrorCode.h"
#include "common/AlarmInfoXmlParser.h"
#include "common/Utils.h"
#include "common/Log.h"
#include "common/Path.h"
#include "securecom/UniqueId.h"
#include "common/ConfigXmlParse.h"
#include <vector>

using namespace std;

namespace {
static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}
}

TEST_F(UtilsTest,DoSleep){
    mp_uint32 ms = 1;

    DoSleep(ms);
}

TEST_F(UtilsTest,SignalRegister){
    mp_int32 signo;
    signal_proc func;
    SignalRegister(signo,func);
}

TEST_F(UtilsTest,InitCommonModules){
    mp_string pszFullBinPath;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(CPath, Init), StubFailed);
    EXPECT_EQ(MP_FAILED, InitCommonModules(pszFullBinPath));

    stub.set(ADDR(CPath, Init), StubSuccess);
    stub.set(ADDR(CConfigXmlParser, Init), StubFailed);
    EXPECT_EQ(MP_FAILED, InitCommonModules(pszFullBinPath));

    stub.set(ADDR(CPath, Init), StubSuccess);
    stub.set(ADDR(CConfigXmlParser, Init), StubSuccess);
    stub.set(ADDR(AlarmInfoXmlParser, Init), StubFailed);
    EXPECT_EQ(MP_FAILED, InitCommonModules(pszFullBinPath));

    stub.set(ADDR(CPath, Init), StubSuccess);
    stub.set(ADDR(CConfigXmlParser, Init), StubSuccess);
    stub.set(ADDR(AlarmInfoXmlParser, Init), StubSuccess);
    stub.set(ADDR(CLogger, Init), StubSuccess);
    stub.set(ADDR(CUniqueID, Init), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, InitCommonModules(pszFullBinPath));
}

TEST_F(UtilsTest,GetHostName){
    mp_string strHostName;
    stub.set(&CLogger::Log, StubCLoggerLog);

    GetHostName(strHostName);
}

TEST_F(UtilsTest,GetOSErrorTest)
{
    GetOSError();
}

TEST_F(UtilsTest,GetOSStrErr){
    mp_int32 err;
    mp_char buf;
    mp_size buf_len;
    GetOSStrErr(err,&buf,buf_len);
}

TEST_F(UtilsTest, GetCurrentUserNameLLTTest)
{
    mp_string strUserName;
    mp_ulong iErrCode;
    stub.set(&CLogger::Log, StubCLoggerLog);
    GetCurrentUserName(strUserName,iErrCode) ;
}

TEST_F(UtilsTest,ChownFile)
{
    mp_string strFileName;
    mp_int32 uid;
    mp_int32 gid; 
    stub.set(&CLogger::Log, StubCLoggerLog);
    ChownFile(strFileName,uid,gid);
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
    stub.set(&CLogger::Log, StubCLoggerLog);
    vector<mp_int64> vecExclude;
    vecExclude.push_back(8);
    EXPECT_EQ(MP_FAILED, CheckParamInteger64(4, 5, 10, vecExclude));
    EXPECT_EQ(MP_FAILED, CheckParamInteger64(11, 5, 10, vecExclude));
    EXPECT_EQ(MP_FAILED, CheckParamInteger64(8, 5, 10, vecExclude));
    EXPECT_EQ(MP_SUCCESS, CheckParamInteger64(7, 5, 10, vecExclude));
}

TEST_F(UtilsTest,CheckParamStringIsIP)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string paramValue = "192.168.0.1";
    EXPECT_EQ(MP_SUCCESS, CheckParamStringIsIP(paramValue));
    paramValue = "255.168.0.1";
    EXPECT_EQ(MP_FAILED, CheckParamStringIsIP(paramValue));
    paramValue = "0.168.0.1";
    EXPECT_EQ(MP_FAILED, CheckParamStringIsIP(paramValue));
    paramValue = "192.168.01";
    EXPECT_EQ(MP_FAILED, CheckParamStringIsIP(paramValue));

    paramValue = "0000:0123:ff00:00:ff:0:00ff:1111";
    EXPECT_EQ(MP_SUCCESS, CheckParamStringIsIP(paramValue));
    paramValue = "0000:0123:ff00:00:ff:0:00ff";
    EXPECT_EQ(MP_FAILED, CheckParamStringIsIP(paramValue));
    paramValue = "0000:0123:ff00:00:ff:0:00ff:1111f";
    EXPECT_EQ(MP_FAILED, CheckParamStringIsIP(paramValue));
    paramValue = "0000:0123:ff00:00:ff:g:00ff:1111";
    EXPECT_EQ(MP_FAILED, CheckParamStringIsIP(paramValue));
    paramValue = "fe80::40c3:f356:90b5:4bd2";
    EXPECT_EQ(MP_SUCCESS, CheckParamStringIsIP(paramValue));
    paramValue = "fe80::40c3::90b5:4bd2";
    EXPECT_EQ(MP_FAILED, CheckParamStringIsIP(paramValue));
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

TEST_F(UtilsTest,CheckFileSysMountParam)
{
    mp_string strDeviceName = "/\\1234";
	mp_string strMountPoint = "/\\1234";
	mp_int32 volumeType;
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckFileSysMountParam(strDeviceName,volumeType,strMountPoint);
}

TEST_F(UtilsTest,CheckFileSysFreezeParam)
{
    mp_string strDiskNames = "//1234";
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckFileSysFreezeParam(strDiskNames);
}

TEST_F(UtilsTest,MakeLogUpperBound)
{
    mp_int32 iRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = MakeLogUpperBound(3);
    EXPECT_EQ(iRet, 2);
}

TEST_F(UtilsTest,GetUidByUserName)
{
    mp_int32 iRet;
    mp_string nameStr = "test";
    mp_int32 uid;
    mp_int32 gid;

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = GetUidByUserName(nameStr, uid, gid);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：获取用户rdadmin的uid和gid
* 前置条件：1.root用户必须存在
* check点: 执行成功
*/
TEST_F(UtilsTest,GetUidByUserName_SUC)
{
    mp_int32 iRet;
    mp_string nameStr = "root";
    mp_int32 uid;
    mp_int32 gid;

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = GetUidByUserName(nameStr, uid, gid);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(UtilsTest,SendIOControl)
{
    mp_int32 iRet;
    mp_string nameStr = "test";
    mp_int32 cmd;
    mp_char data[] = "test";

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = SendIOControl(nameStr, cmd, data, sizeof(data));
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(UtilsTest,IsHuweiStorage)
{
    mp_int32 iRet;
    mp_string arrayVendor = "test";

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = IsHuweiStorage(arrayVendor);
    EXPECT_EQ(iRet, MP_FALSE);

    arrayVendor = "HUAWEI";
    iRet = IsHuweiStorage(arrayVendor);
    EXPECT_EQ(iRet, MP_TRUE);
}

TEST_F(UtilsTest,ChangeGmonDir)
{
    mp_int32 iRet;
    mp_string arrayVendor = "test";

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = ChangeGmonDir();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(UtilsTest,WipeSensitiveForJsonData)
{
    mp_int32 iRet;
    mp_string rawBuffer = "{\"Password\":\"ABCD\"}";
    mp_string value;

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(UtilsTest, WipeSensitiveForJsonDataTestOriginRule)
{
/*
    passwordKeyArray.insert("Password");
    passwordKeyArray.insert("chapPwd");
    passwordKeyArray.insert("password");
    passwordKeyArray.insert("dbPwd");
    passwordKeyArray.insert("ASMPwd");
    passwordKeyArray.insert("EncAlgo");
    passwordKeyArray.insert("EncKey");
    passwordKeyArray.insert("dbPwd");
    passwordKeyArray.insert("ASMPwd");
    passwordKeyArray.insert("authKey");
    passwordKeyArray.insert("authUser");
    passwordKeyArray.insert("authPwd");
    passwordKeyArray.insert("Key");
    passwordKeyArray.insert("authType");
    passwordKeyArray.insert("Initiator");
*/    
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string value;
    mp_string rawBuffer = "{\"Password\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"Password\":\"******\"}\n");

    rawBuffer = "{\"UserPassword\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"UserPassword\":\"******\"}\n");

    rawBuffer = "{\"chapPwd\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"chapPwd\":\"******\"}\n");

    rawBuffer = "{\"password\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"password\":\"******\"}\n");

    rawBuffer = "{\"dbPwd\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"dbPwd\":\"******\"}\n");

    rawBuffer = "{\"ASMPwd\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"ASMPwd\":\"******\"}\n");

    rawBuffer = "{\"EncAlgo\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"EncAlgo\":\"******\"}\n");

    rawBuffer = "{\"EncKey\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"EncKey\":\"******\"}\n");

    rawBuffer = "{\"authKey\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"authKey\":\"******\"}\n");

    rawBuffer = "{\"authUser\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"authUser\":\"******\"}\n");

    rawBuffer = "{\"authPwd\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"authPwd\":\"******\"}\n");

    rawBuffer = "{\"Key\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"Key\":\"******\"}\n");

    rawBuffer = "{\"authType\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"authType\":\"******\"}\n");

    rawBuffer = "{\"Initiator\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"Initiator\":\"******\"}\n");
}

TEST_F(UtilsTest, WipeSensitiveForJsonDataTestNewRules)
{
    mp_string value;
    stub.set(&CLogger::Log, StubCLoggerLog);

    mp_string rawBuffer = "{\"paSs\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"paSs\":\"******\"}\n");

    rawBuffer = "{\"Pwd\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"Pwd\":\"******\"}\n");

    rawBuffer = "{\"Key\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"Key\":\"******\"}\n");

    rawBuffer = "{\"cRypto\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"cRypto\":\"******\"}\n");

    rawBuffer = "{\"1session1\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"1session1\":\"******\"}\n");

    rawBuffer = "{\"token\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"token\":\"******\"}\n");

    rawBuffer = "{\"fingerprint\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"fingerprint\":\"******\"}\n");

    rawBuffer = "{\"1auth1\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"1auth1\":\"******\"}\n");

    rawBuffer = "{\"enc\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"enc\":\"******\"}\n");

    rawBuffer = "{\"1dec1\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"1dec1\":\"******\"}\n");

    rawBuffer = "{\"tgTar\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"tgTar\":\"******\"}\n");

    rawBuffer = "{\"iqn1\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"iqn1\":\"******\"}\n");

    rawBuffer = "{\"initiator\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"initiator\":\"******\"}\n");

    rawBuffer = "{\"secreT\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"secreT\":\"******\"}\n");

    rawBuffer = "{\"cErt\":\"ABCD\"}";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(value, "{\"cErt\":\"******\"}\n");

    rawBuffer = "{\"cErt\":\"ABCD\"";
    mp_int32 iRet = WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(value, "");

    rawBuffer = "";
    WipeSensitiveForJsonData(rawBuffer, value);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(value, "");
}

TEST_F(UtilsTest,CheckEscapeChtTest)
{
    mp_string param = "123";
    mp_int32 iRet = CheckEscapeCht(param);
    EXPECT_EQ(iRet, MP_SUCCESS);

    param = "\a";
    iRet = CheckEscapeCht(param);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    param = "\b";
    iRet = CheckEscapeCht(param);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    param = "\f";
    iRet = CheckEscapeCht(param);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    param = "\n";
    iRet = CheckEscapeCht(param);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    param = "\r";
    iRet = CheckEscapeCht(param);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    param = "\t";
    iRet = CheckEscapeCht(param);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    param = "\v";
    iRet = CheckEscapeCht(param);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);

    param = "";
    iRet = CheckEscapeCht(param);
    EXPECT_EQ(iRet, ERROR_COMMON_INVALID_PARAM);
}

/*
 * 用例名称：检查IP合法性
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(UtilsTest, CheckIpAddressValid)
{
    mp_string ipAddress = "192.168.1.1";
    mp_int32 iRet = CheckIpAddressValid(ipAddress);
    EXPECT_EQ(iRet, MP_SUCCESS);

    ipAddress = "256.168.1.1";
    iRet = CheckIpAddressValid(ipAddress);
    EXPECT_EQ(iRet, MP_FAILED);

    ipAddress = "1050:0:0:0:5:600:300c:326b";
    iRet = CheckIpAddressValid(ipAddress);
    EXPECT_EQ(iRet, MP_SUCCESS);

    ipAddress = "10501:0:0:0:5:600:300c:326b";
    iRet = CheckIpAddressValid(ipAddress);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(UtilsTest, GrepE)
{
    vector<mp_string> vecInput;
    vecInput.push_back("abcdefg");
    vecInput.push_back("1234567");

    {
        vector<mp_string> vecOutput = GrepE(vecInput, "a");
        EXPECT_EQ(1, vecOutput.size());
        EXPECT_EQ("abcdefg", vecOutput[0]);
    }
    {
        vector<mp_string> vecCondition;
        vecCondition.push_back("a");
        vecCondition.push_back("1");
        vector<mp_string> vecOutput = GrepE(vecInput, vecCondition);
        EXPECT_EQ(2, vecOutput.size());
        EXPECT_EQ("abcdefg", vecOutput[0]);
        EXPECT_EQ("1234567", vecOutput[1]);
    }
}

TEST_F(UtilsTest, GrepV)
{
    vector<mp_string> vecInput;
    vecInput.push_back("abcdefg");
    vecInput.push_back("1234567");

    {
        vector<mp_string> vecOutput = GrepV(vecInput, "a");
        EXPECT_EQ(1, vecOutput.size());
        EXPECT_EQ("1234567", vecOutput[0]);
    }
    {
        vector<mp_string> vecCondition;
        vecCondition.push_back("a");
        vecCondition.push_back("1");
        vector<mp_string> vecOutput = GrepV(vecInput, vecCondition);
        EXPECT_EQ(0, vecOutput.size());
    }
}

TEST_F(UtilsTest, GrepW)
{
    vector<mp_string> vecInput;
    vecInput.push_back("abcdefg");
    vecInput.push_back("1234567");

    {
        vector<mp_string> vecOutput = GrepW(vecInput, "abcdefg");
        EXPECT_EQ(1, vecOutput.size());
        EXPECT_EQ("abcdefg", vecOutput[0]);
    }
    {
        vector<mp_string> vecOutput = GrepW(vecInput, "a");
        EXPECT_EQ(0, vecOutput.size());
    }
}

TEST_F(UtilsTest, Awk)
{
    vector<mp_string> vecInput;
    vecInput.push_back("rdadmin:x:22222:22222::/home/rdadmin:/sbin/nologin");

    {
        vector<mp_string> vecOutput = Awk(vecInput, -1, ':');
        EXPECT_EQ("/sbin/nologin", vecOutput[0]);
    }
    {
        vector<mp_string> vecOutput = Awk(vecInput, 0, ':');
        EXPECT_EQ("rdadmin:x:22222:22222::/home/rdadmin:/sbin/nologin", vecOutput[0]);
    }
    {
        vector<mp_string> vecOutput = Awk(vecInput, 200, ':');
        EXPECT_EQ("", vecOutput[0]);
    }
    {
        vector<mp_string> vecOutput = Awk(vecInput, 1, ':');
        EXPECT_EQ("rdadmin", vecOutput[0]);
    }
}

TEST_F(UtilsTest, Awk2)
{
    vector<mp_string> vecInput;
    vecInput.push_back("grid:x:54322:54321::/home/grid:/bin/bash");
    vecInput.push_back("rdadmin:x:22222:22222::/home/rdadmin:/sbin/nologin");

    {
        vector<mp_string> vecOutput = Awk(vecInput, 1, "rdadmin", -1, ':');
        EXPECT_EQ("/sbin/nologin", vecOutput[0]);
    }
    {
        vector<mp_string> vecOutput = Awk(vecInput, 1, "rdadmin", 0, ':');
        EXPECT_EQ("rdadmin:x:22222:22222::/home/rdadmin:/sbin/nologin", vecOutput[0]);
    }
    {
        vector<mp_string> vecOutput = Awk(vecInput, 1, "rdadmin", 200, ':');
        EXPECT_EQ("", vecOutput[0]);
    }
    {
        vector<mp_string> vecOutput = Awk(vecInput, 1, "rdadmin", 1, ':');
        EXPECT_EQ("rdadmin", vecOutput[0]);
    }
}

TEST_F(UtilsTest, CalibrationFormat)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    {
        mp_string str = "9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"
            "999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"
            "999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"
            "999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"
            "999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999"
            "9999999999999999999999999";
        EXPECT_EQ(MP_FAILED, CalibrationFormatString(str));
        str = "abcd";
        EXPECT_EQ(MP_SUCCESS, CalibrationFormatString(str));
    }
    {
        mp_string str = "9999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999";
        EXPECT_EQ(MP_FAILED, CalibrationFormatTaskId(str));
        str = "abcd";
        EXPECT_EQ(MP_SUCCESS, CalibrationFormatTaskId(str));
    }
    {
        EXPECT_EQ(MP_SUCCESS, CalibrationFormatStorProtocol(1));
        EXPECT_EQ(MP_SUCCESS, CalibrationFormatStorProtocol(2));
        EXPECT_EQ(MP_FAILED, CalibrationFormatStorProtocol(3));
    }
}

TEST_F(UtilsTest, ModifyLineDataTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string fileName = "/tmp/testcfg-026c8d59.tmp";
    EXPECT_EQ(MP_FAILED, ModifyLineData(fileName, "test-key", "test-value"));
}

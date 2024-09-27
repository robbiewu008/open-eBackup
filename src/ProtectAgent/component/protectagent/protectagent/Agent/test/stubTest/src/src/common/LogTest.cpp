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
#include "common/LogTest.h"
#include <sstream>
#include "common/ErrorCode.h"
#include "common/ConfigXmlParse.h"

using namespace std;

namespace {
mp_bool StubTrue(mp_void *pthis)
{
    return MP_TRUE;
}

mp_bool StubFalse(mp_void *pthis)
{
    return MP_FALSE;
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_void StubVoid(mp_void *pthis)
{
    return;
}
FILE* StubGetLogFile()
{
    FILE* file;
    return file;
}

FILE* StubGetLogFileNull()
{
    FILE* file = nullptr;
    return file;
}
}

static mp_void StubCLoggerLog(mp_void)
{
    return;
}

mp_int32 StubGetTestCfgInfo(void* obj, mp_string& strSceneType)
{
    strSceneType = "1";
    return MP_SUCCESS;
}

typedef void (*CallbackWriteLogPtr)(int32_t level, const std::string& filePathName, int32_t lineNum,
    const std::string& funcName, const std::string& logString);

static int32_t setCallNum = 0;
static int32_t noSetCallNum = 0;
mp_void WriteLogFunc(int32_t level, const std::string& filePathName, int32_t lineNum, const std::string& funcName,
    const std::string& logString)
{
    setCallNum++;
}

mp_int32 InitLogContentMock(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string pszFormat, va_list& pszArgp, std::ostringstream& strMsg)
{
    noSetCallNum++;
}

TEST_F(CLoggerTest, CLoggerSetLogLevelTest)
{
    mp_int32 level = -1;
    CLogger& log = CLogger::GetInstance();

    // LogLevel < 0
    mp_int32 iRet = log.SetLogLevel(level);
    EXPECT_EQ(iRet, MP_FAILED);
    // LogLevel > 4
    level = 5;
    iRet = log.SetLogLevel(level);
    EXPECT_EQ(iRet, MP_FAILED);
    // LogLevel = 2
    level = 2;
    iRet = log.SetLogLevel(level);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CLoggerTest, CLoggerSetLogCountTest)
{
    mp_int32 logCount = 0;
    CLogger& log = CLogger::GetInstance();

    // logCount <= 0
    mp_int32 iRet = log.SetLogCount(logCount);
    EXPECT_EQ(iRet, MP_FAILED);

    // logCount > 0
    logCount = 2;
    iRet = log.SetLogCount(logCount);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

typedef mp_int32 (CConfigXmlParser::*TypeGetValueInt32)(mp_string, mp_string, mp_int32&);
typedef mp_int32 (*TypeStubGetValueInt32)(void* This, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue);
static mp_int32 StubGetValueInt32_1(void* This, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    static int i = -1;
    i++;
    if (i == 0) {  //第一次调用返回失败
        return ERROR_COMMON_READ_CONFIG_FAILED;
    } else if (i == 1) {  //第二次返回成功
        iValue = 2;
        return MP_SUCCESS;
    } else if (i == 2) {  //第三次返回失败
        return ERROR_COMMON_READ_CONFIG_FAILED;
    } else {
        iValue = 2;
        return MP_SUCCESS;
    }
    return ERROR_COMMON_READ_CONFIG_FAILED;
}

TEST_F(CLoggerTest, CLoggerReadLevelAndCountTest)
{
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetValueInt32_1);
    CLogger& log = CLogger::GetInstance();
    log.ReadLevelAndCount();
    log.ReadLevelAndCount();
    log.ReadLevelAndCount();
}

TEST_F(CLoggerTest, MkHead)
{
    mp_int32 ret;
    mp_int32 iLevel = 0;
    mp_char pszHeadBuf[10] = {0};
    mp_int32 iBufLen = 10;
    ret = CLogger::GetInstance().MkHead(iLevel, pszHeadBuf, iBufLen);

    iLevel = 1;
    ret = CLogger::GetInstance().MkHead(iLevel, pszHeadBuf, iBufLen);

    iLevel = 2;
    ret = CLogger::GetInstance().MkHead(iLevel, pszHeadBuf, iBufLen);

    iLevel = 3;
    ret = CLogger::GetInstance().MkHead(iLevel, pszHeadBuf, iBufLen);

    iLevel = 4;
    ret = CLogger::GetInstance().MkHead(iLevel, pszHeadBuf, iBufLen);

    iLevel = 5;
    ret = CLogger::GetInstance().MkHead(iLevel, pszHeadBuf, iBufLen);
}

TEST_F(CLoggerTest, SwitchLogFile)
{
    mp_int32 ret;
    mp_string pszLogPath = "test";
    mp_string pszLogName = "test";
    mp_int32 iLogCount = 10;
    mp_time LogKeepTime = 100;
    stub.set(&CLogger::Log, StubCLoggerLog);
    ret = CLogger::GetInstance().SwitchLogFile(pszLogPath, pszLogName, iLogCount, LogKeepTime);
}

TEST_F(CLoggerTest, OpenLogCache)
{
    CLogger::GetInstance().OpenLogCache();
}

TEST_F(CLoggerTest, CloseLogCache)
{
    CLogger::GetInstance().CloseLogCache();
}

TEST_F(CLoggerTest, WriteLog2Cache)
{
    ostringstream strMsg;

    CLogger::GetInstance().WriteLog2Cache(strMsg);
}

TEST_F(CLoggerTest, GetLogFile)
{
    mp_int32 ret;

    CLogger::GetInstance().GetLogFile();
}

TEST_F(CLoggerTest, Log)
{
    mp_int32 ret;
    mp_int32 iLevel = 1;
    mp_int32 iFileLine = 1;
    mp_uint64 ulCode = 1;
    mp_char pszFileName = 't';
    mp_char pszFormat = 't';
    mp_char pszFuncction = 't';
}

/*
 * 用例名称：测试日志写函数
 * 前置条件：1、设置日志写函数
 * check点：1、日志写函数是否被调用
 */
TEST_F(CLoggerTest, SetWriteLogFunc)
{
    setCallNum = 0;
    CLogger::GetInstance().SetWriteLogFunc(&WriteLogFunc);
    INFOLOG("this is llt");
    EXPECT_EQ(setCallNum, 1);
}

/*
 * 用例名称：测试日志写函数
 * 前置条件：1、未设置日志写函数
 * check点：1、mock的日志写函数是否被调用
 */
TEST_F(CLoggerTest, NoSetWriteLogFunc)
{
    stub.set(&CLogger::ReadLevelAndCount, StubReadLevelAndCount);
    stub.set(&CLogger::ReadLogCacheThreshold, StubReadLevelAndCount);
    stub.set(&CLogger::ReadMaxSizeAndKeepTime, StubReadLevelAndCount);
    stub.set(&CLogger::InitLogContent, InitLogContentMock);

    CLogger::GetInstance().SetWriteLogFunc(NULL);
    INFOLOG("this is llt");
    EXPECT_EQ(noSetCallNum, 1);
}

/*
 * 用例名称：查看是否内置Agent
 * 前置条件：
 * check点：1、检查返回值
 */
TEST_F(CLoggerTest, IsInAgent)
{
    stub.set(&CLogger::GetTestCfgInfo, StubGetTestCfgInfo);

    mp_bool ret = CLogger::GetInstance().IsInAgent();
    EXPECT_EQ(ret, MP_TRUE);
}

/*
 * 用例名称：擦除变量名为敏感字符的内容
 * 前置条件：
 * check点：1、检查返回值
 */
TEST_F(CLoggerTest, WipeSensitiveTest)
{
    mp_string value = Sensitive::WipeSensitive("qin", "value");
    EXPECT_EQ(value, "value");

    //pass, pwd, key, crypto, session, token, fingerprint, auth, enc, dec, tgt, iqn, initiator, secret, cert
    value = Sensitive::WipeSensitive("pass", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("pwd", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("key", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("crypto", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("session", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("token", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("fingerprint", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("auth", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("enc", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("dec", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("tgt", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("iqn", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("1iQn1", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("initiator", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("secret", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("cert", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("iv", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("salt", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("mk", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("private", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("verfiycode", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("authenticationcode", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("email", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("phone", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("rand", "value");
    EXPECT_EQ(value, "*");

    value = Sensitive::WipeSensitive("test", "secret");
    EXPECT_EQ(value, "*");

    value = Sensitive::WipeSensitive("sk", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("SK", "value");
    EXPECT_EQ(value, "*");
    value = Sensitive::WipeSensitive("sk1", "value");
    EXPECT_EQ(value, "value");
}

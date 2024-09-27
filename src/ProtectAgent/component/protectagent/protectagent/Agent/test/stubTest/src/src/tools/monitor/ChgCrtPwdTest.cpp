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
#include "tools/agentcli/ChgCrtPwdTest.h"

using namespace std;

static mp_int32 StubGetValueString2(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ > 0)
    {
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

static mp_int32 StubGetValueString0(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ > 0)
    {
        strValue = "12345678";
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

static mp_void stubInputUserPwd2(const mp_string& strUserName, mp_string &strUserPwd, INPUT_TYPE eType, mp_int32 iPwdLen)
{
	static mp_int32 iCounter = 0;
    if (iCounter++ > 3)
    {
        strUserPwd = "12345678";
    }
    else
    {   
        strUserPwd = "87654321";
    }
}

static mp_void stubInputUserPwd0(const mp_string& strUserName, mp_string &strUserPwd, INPUT_TYPE eType, mp_int32 iPwdLen)
{
	static mp_int32 iCounter = 0;
    if (iCounter == 0)
    {
        strUserPwd = "12345678";
		iCounter++;
    }
    else if ( iCounter == 5)
    {   
        strUserPwd = "876543210";
		iCounter++;
    }
	else 
    {   
        strUserPwd = "87654321";
		iCounter++;
    }
}


static mp_void stubInputUserPwd1(mp_string strUserName, mp_string &strUserPwd, INPUT_TYPE eType, mp_int32 iPwdLen)
{
	static mp_int32 iCounter = 0;
    if (iCounter++ > 0)
    {
        strUserPwd = "12345678";
    }
    else
    {   
        strUserPwd = "87654321";
    }
}


static mp_int32 stubInputCrtNewPwd(mp_void)
{
    static int iCounter = 0;
    if (iCounter == 0)
    {
        iCounter++;
        return CHECK_PASSWORD_OVER_TIMES;
    }
    else if ( 0 < iCounter && iCounter < 3 )
    {
        iCounter++;
        return MP_FAILED;
    }
    else
    {
        iCounter++;
        return MP_SUCCESS;
    }	
}

static mp_int32 stubChangeCrtPwd(mp_void)
{
    static int iCounter = 0;
    if (++iCounter <= 1)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 StubChgSetValue(mp_void)
{
    static int iCounter = 0;
    if (++iCounter <= 1)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 StubGetCertFileName(mp_void)
{
	static int iCounter = 0;
	if (++iCounter <= 1)
	{
		return MP_FAILED;
	}
	else
	{
		return MP_SUCCESS;
	}
}

static mp_bool stubCheckCommon(mp_void)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 1)
	{
		return MP_TRUE;
	}
	else
	{
		return MP_FALSE;
	}

}

static mp_int32 stubInputUserPwd(mp_void)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 0)
	{
		return MP_SUCCESS;
	}
	else
	{
		return MP_FAILED;
	}
}

static mp_int32 stubDelFile(mp_void)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 0)
	{
		return MP_SUCCESS;
	}
	else
	{
		return MP_FAILED;
	}
}

static mp_int32 stubBuildCertFile(mp_void)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 0)
	{
		return MP_SUCCESS;
	}
	else
	{
		return MP_FAILED;
	}
}

static mp_int32 stubReadFile1(const mp_string& strFilePath, vector<mp_string>& vecOutput)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 0)
	{
	    vecOutput.push_back("ssl_certificate_key server.key;");
		vecOutput.push_back("ssl_certificate_key server.key;");
		vecOutput.push_back("ssl_certificate_key server.key;");
		vecOutput.push_back("ssl_certificate_key server.key;");
		return MP_SUCCESS;
	}
	else
	{
		return MP_FAILED;
	}
}

static mp_void stubStrSplit(vector<mp_string>& vecTokens, const mp_string& strText, mp_char cSep)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 1)
	{
		vecTokens.push_back("server.key");
	}
	else
	{
	    vecTokens.clear();
	}
}

static mp_int32 stubReadFile0(mp_void)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 0)
	{
		return MP_SUCCESS;
	}
	else
	{
		return MP_FAILED;
	}
}


static mp_int32 stubDelFile0(mp_void)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 0)
	{
		return MP_SUCCESS;
	}
	else
	{
		return MP_FAILED;
	}
}

static mp_int32 stubWriteFile(mp_void)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 0)
	{
		return MP_SUCCESS;
	}
	else
	{
		return MP_FAILED;
	}
}

static mp_bool stubCheckAdminOldPwd1(mp_void)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 2)
	{
		return MP_TRUE;
	}
	else
	{
		return MP_FALSE;
	}
}

static mp_int32 stubCSystemExecSystemWithoutEcho(mp_void)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 0)
	{
		return MP_SUCCESS;
	}
	else
	{
		return MP_FAILED;
	}
}

static mp_int32 stubInputCrtOldPwd(mp_void)
{
	static mp_int32 iCounter = 0;
	if (iCounter++ > 0)
	{
		return MP_SUCCESS;
	}
	else
	{
		return MP_FAILED;
	}
}

TEST_F(CChgCrtPwdTest, Handle)
{
    mp_int32 iRet = 0;
    ChgCrtPwd PwdObj;

    Stub mystub1;
    mystub1.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubGetValueString2);
    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    Stub mystub2;
    mystub2.set(&CPassword::CheckAdminOldPwd, stubCheckAdminOldPwd1);
    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&ChgCrtPwd::InputCrtOldPwd, stubInputCrtOldPwd);
    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&ChgCrtPwd::InputCrtNewPwd, stubInputCrtNewPwd);
    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&ChgCrtPwd::ChangeCrtPwd, stubChangeCrtPwd);
    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    // stub.set((mp_int32(CConfigXmlParser::*)(mp_string,mp_string,mp_string,mp_string))ADDR(CConfigXmlParser,SetValue), &StubChgSetValue);
    // iRet = PwdObj.Handle();
    // EXPECT_EQ(MP_FAILED, iRet);

    iRet = PwdObj.Handle();
    EXPECT_EQ(MP_FAILED, iRet);

    return;
}

TEST_F(CChgCrtPwdTest, InputCrtOldPwd)
{
    mp_int32 iRet = 0;
    ChgCrtPwd PwdObj;
    mp_string strCrtOldPwd;
	mp_string strCrtNewPwd = "test";

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubGetValueString0);
    iRet = PwdObj.InputCrtOldPwd(strCrtOldPwd, strCrtNewPwd);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(&CPassword::InputUserPwd, stubInputUserPwd2);
    iRet = PwdObj.InputCrtOldPwd(strCrtOldPwd, strCrtNewPwd);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = PwdObj.InputCrtOldPwd(strCrtOldPwd, strCrtNewPwd);
    EXPECT_EQ(MP_FAILED, iRet);

    return;
}

TEST_F(CChgCrtPwdTest, InputCrtNewPwd)
{
    mp_int32 iRet = 0;
    ChgCrtPwd PwdObj;
	mp_string strCrtOldPwd = "12345678";
	mp_string strCrtNewPwd;

    stub.set(&CPassword::InputUserPwd, stubInputUserPwd0);
    iRet = PwdObj.InputCrtNewPwd(strCrtNewPwd, strCrtOldPwd);
    EXPECT_EQ(CHECK_PASSWORD_OVER_TIMES, iRet);

    stub.set(&CPassword::CheckCommon, stubCheckCommon);
    iRet = PwdObj.InputCrtNewPwd(strCrtNewPwd, strCrtOldPwd);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = PwdObj.InputCrtNewPwd(strCrtNewPwd, strCrtOldPwd);
    EXPECT_EQ(MP_SUCCESS, iRet);

    return;
}


TEST_F(CChgCrtPwdTest, ChangeCrtPwd)
{
    mp_int32 iRet = 0;
    ChgCrtPwd PwdObj;
	mp_string strCrtNewPwd="123456789";
	mp_string strCrtOldPwd="987654321";
    
    stub.set(&ChgCrtPwd::GetCertFileName, StubGetCertFileName);
    iRet = PwdObj.ChangeCrtPwd(strCrtNewPwd, strCrtOldPwd);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, stubCSystemExecSystemWithoutEcho);
    stub.set(&CMpFile::DelFile, stubDelFile);

    iRet = PwdObj.ChangeCrtPwd(strCrtNewPwd, strCrtOldPwd);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = PwdObj.ChangeCrtPwd(strCrtNewPwd, strCrtOldPwd);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&ChgCrtPwd::BuildCertFile, stubBuildCertFile);
    iRet = PwdObj.ChangeCrtPwd(strCrtNewPwd, strCrtOldPwd);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = PwdObj.ChangeCrtPwd(strCrtNewPwd, strCrtOldPwd);
    EXPECT_EQ(MP_SUCCESS, iRet);

    return;
}


TEST_F(CChgCrtPwdTest, GetCertFileName)
{
    mp_int32 iRet = 0;
    ChgCrtPwd PwdObj;
	mp_string strCrtFileName;

    stub.set(&CMpFile::ReadFile, stubReadFile1);
    iRet = PwdObj.GetCertFileName(strCrtFileName);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CMpString::StrSplit, &stubStrSplit);
    iRet = PwdObj.GetCertFileName(strCrtFileName);
    EXPECT_EQ(MP_SUCCESS, iRet);

    return;
}

TEST_F(CChgCrtPwdTest, BuildCertFile)
{
    mp_int32 iRet = 0;
    ChgCrtPwd PwdObj;
	mp_string strNewCrtFileName="/etc/hosts";

    stub.set(&CIPCFile::ReadFile, stubReadFile0);
    iRet = PwdObj.BuildCertFile(strNewCrtFileName);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CMpFile::DelFile, stubDelFile0);
    iRet = PwdObj.BuildCertFile(strNewCrtFileName);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CIPCFile::WriteFile, stubWriteFile);
    iRet = PwdObj.BuildCertFile(strNewCrtFileName);
    EXPECT_EQ(MP_FAILED, iRet);


    iRet = PwdObj.BuildCertFile(strNewCrtFileName);
    EXPECT_EQ(MP_FAILED, iRet);

    return;
}






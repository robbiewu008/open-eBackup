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
#include "tools/agentcli/StartNginxTest.h"

using namespace std;

static mp_int32 StubGetCiphertext(mp_string &CipherStr)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ != 0)
    {
        CipherStr = "test";
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

static mp_int32 StubExecNginxStart(void)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ != 0)
    {
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

static mp_int32 StubWriteFile(mp_string& strPath, const vector<mp_string> &strPWD)
{
    static mp_int32 iCounter = 0;
    if ( iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 StubDelFile(const mp_char* strFilePath)
{
    static mp_int32 iCounter = 0;
    
    if ( iCounter == 1 )
    {
        return MP_FAILED;
    } 
    
    if ( iCounter++ == 0)
    {
        return MP_FAILED;
    }
    else
    {
        return MP_SUCCESS;
    }
}

static mp_int32 StubCheckScriptSign(const mp_string strFileName)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ != 0)
    {
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
    }
}

static mp_int32 StubGetValueString(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    static mp_int32 iCounter = 0;

    if (iCounter++ != 0)
    {
        return MP_SUCCESS;
    }
    else
    {
        return MP_FAILED;
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


TEST_F(CStartNginxTest, Handle)
{
    mp_int32 iRet = 0;
    StartNginx startObj;

    stub.set(&StartNginx::GetPassword, StubGetCiphertext);

    stub.set(&CIPCFile::WriteFile, StubWriteFile);
    stub.set(&StartNginx::ExecNginxStart, StubExecNginxStart);
    stub.set(&CMpFile::DelFile, StubDelFile);
    
    mp_string PARAM_START_NGINX = "startnginx";
    iRet = startObj.Handle(PARAM_START_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle(PARAM_START_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle(PARAM_START_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle(PARAM_START_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle(PARAM_START_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle(PARAM_START_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);

    mp_string PARAM_RELOAD_NGINX = "reloadnginx";
    iRet = startObj.Handle(PARAM_RELOAD_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle(PARAM_RELOAD_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle(PARAM_RELOAD_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle(PARAM_RELOAD_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle(PARAM_RELOAD_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.Handle(PARAM_START_NGINX);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(CStartNginxTest, ExecNginxStart)
{
    mp_int32 iRet = 0;
    StartNginx startObj;
    
    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, stubCSystemExecSystemWithoutEcho);
    iRet = startObj.ExecNginxStart();
    EXPECT_EQ(MP_FAILED, iRet);
    
    iRet = startObj.ExecNginxStart();
    EXPECT_EQ(MP_SUCCESS, iRet);
    
}

TEST_F(CStartNginxTest, GetPassword)
{
    mp_int32 iRet = 0;
    StartNginx startObj;
    mp_string pCipherStr;

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), &StubGetValueString);

    iRet = startObj.GetPassword(pCipherStr);
    EXPECT_EQ(MP_SUCCESS, iRet);
    
    iRet = startObj.GetPassword(pCipherStr);
    EXPECT_EQ(MP_SUCCESS, iRet);
    
}


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
#include "apps/oracle/OracleInfoTest.h"
#include "securecom/RootCaller.h"
namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)


mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_int32 ReadFileTest(const mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    vecOutput.push_back("aqqweq asd adsa");
    return MP_SUCCESS;
}
}


TEST_F(OracleInfoTest, GetDBInfoTest)
{
    DoGetJsonStringTest();
    OracleInfo info;
    mp_bool bIsInstalled;
    std::list<oracle_inst_info_t> lstOracleInstInfo;
    mp_string strVer;
    mp_string strHome;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    info.GetDBInfo(lstOracleInstInfo, strVer, strHome);
    stub.reset(ADDR(CRootCaller, Exec));
}

TEST_F(OracleInfoTest, AnalyseInstInfoScriptRstTest)
{
    DoGetJsonStringTest();
    OracleInfo info;
    std::vector<mp_string> vecResult;
    mp_string strVer;
    mp_string strHome;
    std::list<oracle_inst_info_t> lstOracleInstInfo;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    vecResult.push_back("111");
    info.AnalyseInstInfoScriptRst(vecResult, strVer, strHome, lstOracleInstInfo);
    vecResult.push_back("111;111;111");
    info.AnalyseInstInfoScriptRst(vecResult, strVer, strHome, lstOracleInstInfo);
    stub.reset(ADDR(CRootCaller, Exec));
}

TEST_F(OracleInfoTest, AnalyseDatabaseNameByConfFileTest)
{
    DoGetJsonStringTest();
    OracleInfo info;
    std::list<oracle_inst_info_t> lstOracleInstInfo;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    oracle_inst_info_t tt;
    lstOracleInstInfo.push_back(tt);
    info.AnalyseDatabaseNameByConfFile(lstOracleInstInfo);
    stub.set(ADDR(CMpFile, ReadFile), ReadFileTest);
    info.AnalyseDatabaseNameByConfFile(lstOracleInstInfo);
    stub.reset(ADDR(CMpFile, ReadFile));
    stub.reset(ADDR(CRootCaller, Exec));
}

TEST_F(OracleInfoTest, AnalysePDBInfoScriptRstTest)
{
    DoGetJsonStringTest();
    OracleInfo info;
    vector<mp_string> vecResult;
    vector<oracle_pdb_rsp_info_t> vecOraclePdbInfo;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    info.AnalysePDBInfoScriptRst(vecResult, vecOraclePdbInfo);
    stub.reset(ADDR(CRootCaller, Exec));
    vecResult.push_back("123456");
    info.AnalysePDBInfoScriptRst(vecResult, vecOraclePdbInfo);
    vecResult.clear();
    vecResult.push_back("32143;123;456");
    info.AnalysePDBInfoScriptRst(vecResult, vecOraclePdbInfo);
}

TEST_F(OracleInfoTest, TranslatePDBStatusTest)
{
    DoGetJsonStringTest();
    OracleInfo info;
    mp_string strStatus;
    mp_int32 iStatus;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    info.TranslatePDBStatus(strStatus, iStatus);
    stub.reset(ADDR(CRootCaller, Exec));
}

TEST_F(OracleInfoTest, AnalyseClsInfoScriptRstTest)
{
    DoGetJsonStringTest();
    OracleInfo info;
    std::vector<mp_string> vecResult;
    Json::Value clsInfo;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    vecResult.push_back("123456");
    info.AnalyseClsInfoScriptRst(vecResult, clsInfo);
    vecResult.clear();
    vecResult.push_back("ClusterType;1");
    info.AnalyseClsInfoScriptRst(vecResult, clsInfo);
    vecResult.clear();
    vecResult.push_back("ClusterName;1");
    info.AnalyseClsInfoScriptRst(vecResult, clsInfo);
    vecResult.clear();
    vecResult.push_back("ClusterIP;1");
    info.AnalyseClsInfoScriptRst(vecResult, clsInfo);
    vecResult.clear();
    vecResult.push_back("ClusterIPHost;1");
    info.AnalyseClsInfoScriptRst(vecResult, clsInfo);
    stub.reset(ADDR(CRootCaller, Exec));
}

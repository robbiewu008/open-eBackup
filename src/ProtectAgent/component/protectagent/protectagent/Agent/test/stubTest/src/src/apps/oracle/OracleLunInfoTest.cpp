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
#include "apps/oracle/OracleLunInfoTest.h"
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
    pvecResult->push_back("aaaa");
    return MP_SUCCESS;
}
mp_int32 GetLunUDEVInfoTest(const mp_string& strUDEVConfDir, const mp_string& strUDEVRoot, mp_string& strUDEVName, 
    mp_string& strUDEVRes, oracle_lun_info_t& oracle_lun_info)
{
    return MP_SUCCESS;
}

mp_int32 GetDevListSub(vector<mp_string>& vecResult, map<mp_string, luninfo_t>& mapLuninfo)
{
    vecResult.push_back("11111");
    return MP_SUCCESS;
}

mp_int32 AnalyzeLunListSub(vector<mp_string>& vecResult, map<mp_string, luninfo_t>& mapLuninfo)
{
    return MP_SUCCESS;
}
mp_int32 GetUDEVInfoSub(
    mp_string strUdevRulesFileDir, mp_string strUdevName, mp_string strUdevResult, mp_string& strUdevDeviceRecord)
{
    strUdevDeviceRecord = "aaa";
    return MP_SUCCESS;
}

mp_int32 GetArrayVendorAndProductSub(const mp_string& strDev, mp_string& strvendor, mp_string& strproduct)
{
    strvendor = "111";
    return MP_SUCCESS;
}

mp_int32 StubSUCCESS()
{
    return MP_SUCCESS;
}

mp_int32 StubGetLunInfoByStorageType(oracle_db_info_t stDBInfo, 
    std::vector<oracle_lun_info_t>& vecLUNInfos, const mp_string& strStorageType)
{
    return MP_SUCCESS;
}
}
TEST_F(OracleLunInfoTest, GetDBLUNInfoTest)
{
    DoGetJsonStringTest();
    stub.set(ADDR(OracleLunInfo, GetLunInfoByStorageType), StubGetLunInfoByStorageType);
    OracleLunInfo lunInfo;
    oracle_db_info_t stDBInfo;
    std::vector<oracle_lun_info_t> vecLUNInfos;
    stDBInfo.iGetArchiveLUN  = ORACLE_QUERY_ARCHIVE_LOGS;
    lunInfo.GetDBLUNInfo(stDBInfo, vecLUNInfos);
    stDBInfo.iGetArchiveLUN  = 0;
    lunInfo.GetDBLUNInfo(stDBInfo, vecLUNInfos);
}

TEST_F(OracleLunInfoTest, GetLunInfoByStorageTypeTest)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    OracleLunInfo lunInfo;
    oracle_db_info_t stDBInfo;
    std::vector<oracle_lun_info_t> vecLUNInfos;
    mp_string strStorageType;
    stDBInfo.iGetArchiveLUN  = ORACLE_QUERY_ARCHIVE_LOGS;
    lunInfo.GetLunInfoByStorageType(stDBInfo, vecLUNInfos, strStorageType);
    stDBInfo.iGetArchiveLUN  = !ORACLE_QUERY_ARCHIVE_LOGS;
    lunInfo.GetLunInfoByStorageType(stDBInfo, vecLUNInfos, strStorageType);
    stub.reset(ADDR(CRootCaller, Exec));
}

TEST_F(OracleLunInfoTest, AnalyseLunInfoScriptRSTTest)
{
    DoGetJsonStringTest();
    OracleLunInfo lunInfo;
    vector<mp_string> vecResult;
    vector<oracle_storage_script_info> vecDBStorageScriptInfo;
    lunInfo.AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
    vecResult.push_back("aaa");
    lunInfo.AnalyseLunInfoScriptRST(vecResult, vecDBStorageScriptInfo);
}

TEST_F(OracleLunInfoTest, AnalyseLunInfoByScriptRSTTest)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    OracleLunInfo lunInfo;
    vector<oracle_storage_script_info> vecDBStorageScriptInfo;
    vector<oracle_lun_info_t> vecLUNInfos;
    mp_string strStorageType;
    lunInfo.AnalyseLunInfoByScriptRST(vecDBStorageScriptInfo, vecLUNInfos, strStorageType);
}

TEST_F(OracleLunInfoTest, AnalyseLunInfoByScriptRSTNoWINTest)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    stub.set(ADDR(OracleLunInfo, GetUDEVConfig), StubSUCCESS);
    stub.set(ADDR(OracleLunInfo, GetDevList), StubSUCCESS);
    stub.set(ADDR(OracleLunInfo, AnalyzeLunList), StubSUCCESS);
    OracleLunInfo lunInfo;
    vector<oracle_storage_script_info> vecDBStorageScriptInfo;
    vector<oracle_lun_info_t> vecLUNInfos; 
    mp_string strStorageType;
    lunInfo.AnalyseLunInfoByScriptRSTNoWIN(vecDBStorageScriptInfo, vecLUNInfos, strStorageType);
}

TEST_F(OracleLunInfoTest, AnalyzeStorageGerInfoTest)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    OracleLunInfo lunInfo;
    vector<oracle_storage_script_info> vec;
    oracle_storage_script_info info;
    vec.push_back(info);
    vector<oracle_storage_script_info>::iterator iter = vec.begin();
    mp_string strUDEVConfDir;
    mp_string strUDEVRoot;
    map<mp_string, luninfo_t> mapLuninfo;
    vector<oracle_lun_info_t> vecLUNInfos;
    mp_string strStorageType;
    mp_string strDev;
    lunInfo.AnalyzeStorageGerInfo(iter, strUDEVConfDir, strUDEVRoot, mapLuninfo, vecLUNInfos, strStorageType, strDev);
    stub.set(ADDR(OracleLunInfo, GetLunUDEVInfo), GetLunUDEVInfoTest);
    lunInfo.AnalyzeStorageGerInfo(iter, strUDEVConfDir, strUDEVRoot, mapLuninfo, vecLUNInfos, strStorageType, strDev);
    stub.reset(ADDR(OracleLunInfo, GetLunUDEVInfo));

}

TEST_F(OracleLunInfoTest, AnalyzeStorageOtherInfoTest)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    OracleLunInfo lunInfo;
    vector<oracle_storage_script_info> vec;
    oracle_storage_script_info info;
    vec.push_back(info);
    vector<oracle_storage_script_info>::iterator iter = vec.begin();
    vector<oracle_lun_info_t> vecLUNInfos; 
    mp_string strStorageType;
    mp_string strDev;
    std::map<mp_string, luninfo_t> myMap;
    luninfo_t t;
    myMap["aaa"] =  t;
    map<mp_string, luninfo_t>::iterator mapIter = myMap.begin();
    oracle_lun_info_t oracle_lun_info;
    lunInfo.AnalyzeStorageOtherInfo(iter, vecLUNInfos, strStorageType, strDev, mapIter, oracle_lun_info);
}

TEST_F(OracleLunInfoTest, GetDevListTest)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    OracleLunInfo lunInfo;
    vector<oracle_storage_script_info> vecDBStorageScriptInfo;
    mp_string strDev;
    vector<mp_string> vecResult;
    oracle_storage_script_info info;
    vecDBStorageScriptInfo.push_back(info);
    lunInfo.GetDevList(vecDBStorageScriptInfo, strDev, vecResult);
    stub.set(ADDR(OracleLunInfo, GetLunUDEVInfo), GetLunUDEVInfoTest);
    lunInfo.GetDevList(vecDBStorageScriptInfo, strDev, vecResult);
    stub.reset(ADDR(OracleLunInfo, GetLunUDEVInfo));
}

TEST_F(OracleLunInfoTest, AnalyzeLunListTest)
{
    DoGetJsonStringTest();
    OracleLunInfo lunInfo;
    vector<mp_string> vecResult;
    map<mp_string, luninfo_t> mapLuninfo;
    lunInfo.AnalyzeLunList(vecResult, mapLuninfo);
    vecResult.push_back("a;a;a;a;a;a;a");
    luninfo_t t;
    mapLuninfo["aaa"] = t;
    lunInfo.AnalyzeLunList(vecResult, mapLuninfo);
}

TEST_F(OracleLunInfoTest, AnalyzeLunListTest1)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    OracleLunInfo lunInfo;
    mp_string strUDEVConfDir;
    mp_string strUDEVRoot;
    mp_string strUDEVName;
    mp_string strUDEVRes;
    oracle_lun_info_t oracle_lun_info;
    lunInfo.GetLunUDEVInfo(strUDEVConfDir, strUDEVRoot, strUDEVName, strUDEVRes, oracle_lun_info);
    strUDEVRes = "111";
    lunInfo.GetLunUDEVInfo(strUDEVConfDir, strUDEVRoot, strUDEVName, strUDEVRes, oracle_lun_info);
    stub.set(ADDR(OracleLunInfo, GetUDEVInfo), GetUDEVInfoSub);
    lunInfo.GetLunUDEVInfo(strUDEVConfDir, strUDEVRoot, strUDEVName, strUDEVRes, oracle_lun_info);
}


TEST_F(OracleLunInfoTest, CopyValueTest)
{
    DoGetJsonStringTest();
    OracleLunInfo lunInfo;
    oracle_lun_info_t oracle_lun_info;    
    vector<oracle_storage_script_info> vect;
    oracle_storage_script_info info;
    vect.push_back(info);
    vector<oracle_storage_script_info>::iterator iter = vect.begin();
    lunInfo.CopyValue(oracle_lun_info, iter);
}

TEST_F(OracleLunInfoTest, GetHPRawDiskNameTest)
{
    DoGetJsonStringTest();
    OracleLunInfo lunInfo;
    mp_string strDev;
    mp_string strSystemDevice;
    lunInfo.GetHPRawDiskName(strDev, strSystemDevice);
}

TEST_F(OracleLunInfoTest, CGetAndCheckArraySNTest)
{
    DoGetJsonStringTest();
    OracleLunInfo lunInfo;
    mp_string strDev;
    mp_string strArraySN;
    mp_string strStorageType;
    lunInfo.GetAndCheckArraySN(strDev, strArraySN, strStorageType);
    strStorageType = "must";
    lunInfo.GetAndCheckArraySN(strDev, strArraySN, strStorageType);
}

TEST_F(OracleLunInfoTest, GetVendorAndProductTest)
{
    DoGetJsonStringTest();
    OracleLunInfo lunInfo;
    mp_string strDev;
    mp_string strVendor; 
    mp_string strProduct;
    mp_string strStorageType;
    lunInfo.GetVendorAndProduct(strDev, strVendor, strProduct, strStorageType);
    stub.set(ADDR(Array, GetArrayVendorAndProduct), GetArrayVendorAndProductSub);
    lunInfo.GetVendorAndProduct(strDev, strVendor, strProduct, strStorageType);
    stub.reset(ADDR(Array, GetArrayVendorAndProduct));
}


TEST_F(OracleLunInfoTest, GetUDEVInfoaTest)
{
    DoGetJsonStringTest();
    OracleLunInfo lunInfo;
    mp_string strUdevRulesFileDir;
    mp_string strUdevName;
    mp_string strUdevResult;
    mp_string strUdevDeviceRecord;
    lunInfo.GetUDEVInfo(strUdevRulesFileDir, strUdevName, strUdevResult, strUdevDeviceRecord);
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    lunInfo.GetUDEVInfo(strUdevRulesFileDir, strUdevName, strUdevResult, strUdevDeviceRecord);
    stub.reset(ADDR(CRootCaller, Exec));

}


TEST_F(OracleLunInfoTest, CheckLUNInfoExistsTest)
{
    DoGetJsonStringTest();
    OracleLunInfo lunInfo;
    vector<oracle_lun_info_t> vecLUNInfos;
    oracle_lun_info_t t;
    vecLUNInfos.push_back(t);
    oracle_lun_info_t oracle_lun_info;
    lunInfo.CheckLUNInfoExists(vecLUNInfos, oracle_lun_info);

}

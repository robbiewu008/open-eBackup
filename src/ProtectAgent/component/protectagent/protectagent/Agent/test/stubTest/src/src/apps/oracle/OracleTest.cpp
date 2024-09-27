#include "apps/oracle/OracleTest.h"
#include "securecom/RootCaller.h"
#include "common/ConfigXmlParse.h"
namespace {

mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)

mp_int32 GetDBInfoTest(list<oracle_inst_info_t>& lstOracleInstInfo, mp_string& strVer, mp_string& strHome)
{
    return MP_SUCCESS;
}
mp_int32 BuildTruncateLogScriptParamTest(oracle_db_info_t& stDBInfo, mp_time truncTime, mp_string& strParam)
{
    return MP_SUCCESS;
}

mp_int32 StubGetDBLUNInfo(oracle_db_info_t& stDBInfo, std::vector<oracle_lun_info_t>& vecLUNInfos)
{
    return MP_SUCCESS;
}
}
mp_int32 IsInstalledTest(mp_bool& bIsInstalled)
{
    return bIsInstalled;
}


mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

TEST_F(COracleTest, GetDBInfoTest)
{
    mp_bool bIsInstalled;
    list<oracle_inst_info_t> lstOracleInstInfo;
    mp_string strVer;
    mp_string strHome;
    Oracle oracle;
    stub.set(ADDR(OracleInfo, GetDBInfo), GetDBInfoTest);
    oracle.GetDBInfo(lstOracleInstInfo, strVer, strHome);
    stub.reset(ADDR(OracleInfo, GetDBInfo));
}

TEST_F(COracleTest, StartPluginDBTest)
{
    DoGetJsonStringTest();
    oracle_pdb_req_info_t stPdbReqInfo;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    Oracle oracle;
    oracle.StartPluginDB(stPdbReqInfo);
    stub.reset(ADDR(CRootCaller, Exec));

}

TEST_F(COracleTest, QueryHostRoleInClusterTest)
{
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    Oracle oracle;

    oracle_db_info_t stDBinfo;
    std::vector<mp_string> storHosts;
    std::vector<mp_string> dbHosts;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    oracle.QueryHostRoleInCluster(stDBinfo, storHosts, dbHosts);
    stub.reset(ADDR(CRootCaller, Exec));
}

TEST_F(COracleTest, QueryTableSpaceTest)
{
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    Oracle oracle;
    oracle_db_info_t stDBInfo;
    std::vector<mp_string> tablespaces;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    oracle.QueryTableSpace(stDBInfo, tablespaces);
    tablespaces.push_back("aaa");
    oracle.QueryTableSpace(stDBInfo, tablespaces);
}

TEST_F(COracleTest, GetDBLUNInfoTest)
{
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    stub.set(ADDR(OracleLunInfo, GetDBLUNInfo), StubGetDBLUNInfo);
    Oracle oracle;
    oracle_db_info_t stDBInfo;
    std::vector<oracle_lun_info_t> vecLUNInfos;

    oracle_lun_info_t info;
    vecLUNInfos.push_back(info);
    oracle.GetDBLUNInfo(stDBInfo, vecLUNInfos);
}

TEST_F(COracleTest, BuildCheckThresholdScriptParamTest)
{
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    Oracle oracle;
    oracle_db_info_t stDBInfo;
    mp_string strParam;
    oracle.BuildCheckThresholdScriptParam(stDBInfo, strParam);
}


TEST_F(COracleTest, CheckArchiveTestTest)
{
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    Oracle oracle;
    oracle_db_info_t stDBInfo;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    oracle.Test(stDBInfo);
}

TEST_F(COracleTest, BuildTruncateLogScriptParamTest)
{
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    Oracle oracle;
    oracle_db_info_t stDBInfo;
    mp_time truncTime;
    mp_string strParam;
    oracle.BuildTruncateLogScriptParam(stDBInfo, truncTime, strParam);
}

TEST_F(COracleTest, BuildCheckCDBScriptParamTest)
{
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    Oracle oracle;

    oracle_db_info_t stDBInfo;
    mp_string strParam;
    oracle.BuildCheckCDBScriptParam(stDBInfo, strParam);
}

TEST_F(COracleTest, CheckCDBTest)
{
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    Oracle oracle;
    oracle_db_info_t stDBInfo;
    mp_int32 iCDBType;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    oracle.CheckCDB(stDBInfo, iCDBType);
    stub.reset(ADDR(CRootCaller, Exec));
}

TEST_F(COracleTest, GetCluterInfoTest)
{
    DoGetJsonStringTest();
    StubClogToVoidLogNullPointReference();
    Oracle oracle;
    Json::Value clsInfo;
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    oracle.GetCluterInfo(clsInfo);
    stub.reset(ADDR(CRootCaller, Exec));
}

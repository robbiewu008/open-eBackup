#include "alarm/AlarmTest.h"
#include "alarm/AlarmMgr.h"
#include "alarm/alarmdb.h"
#include "alarm/AppFreezeStatus.h"
#include "alarm/AlarmHandle.h"
#include "common/Log.h"
#include "common/DB.h"
#include "common/Types.h"
#include "common/ConfigXmlParse.h"
#include "common/Types.h"
#include "common/Path.h"
#include "common/Ip.h"
#include "common/Utils.h"
#include <cstdlib>
#include <vector>

using namespace std;

#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)

#define StubClogToVoidLogNullPointReference_cmp() do { \
    stub_cmp.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)
static mp_void StubCLoggerLog(mp_void){
    return;
}

static mp_int32 StubCConXmlPGetVaStr(mp_string strin, mp_string strin2, mp_string& strValue)
{
    strValue = "401C8F36B7DF2DA9336BAF749790A3F31386F8483E3163FE2D6661900A445B3E66F5F1F5213CB99437546D71987AEC97";
    return MP_FAILED;
}

mp_int32 StubGetValueString_fail(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}

mp_int32 StubGetValueString_succ(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32_fail(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_FAILED;
}

mp_int32 StubGetValueInt32_succ(const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

static mp_int32 StubExecSql()
{
   return MP_FAILED;
}

static mp_int32 StubExecSqlSU()
{
   return MP_SUCCESS;
    
}

static mp_int32 GetValIntT()
{
    return MP_FAILED;
}

static mp_int32 GetValStrT()
{
    return MP_FAILED;
}

static mp_int32 StubSqlite3_open()
{
    cout << "exec stub sqlite3_open." << endl;
    return SQLITE_OK;
}

static mp_string StubGetDbFilePath()
{
    return mp_string("zwgTest.db");
}

int enterCount = 0;
static mp_int32 StubAlarmResult()
{
    enterCount ++;
    return MP_SUCCESS;
}

static mp_int32 StubAlarmRetry()
{
    enterCount ++;
    if (enterCount%2 == 1) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

static mp_void StubDoSleep(mp_uint32 ms)
{
    return;
}

mp_int32 StubGetListenIPAndPort(mp_string& strIP, mp_string& strPort)
{
    strIP = "127.0.0.1";
    return MP_SUCCESS;
}

mp_int32 StubGetListenIPAndPortFailed(mp_string& strIP, mp_string& strPort)
{
    strIP = "127.0.0.1";
    return MP_FAILED;
}

mp_int32 StubSuccess(mp_void* pThis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pThis)
{
    return MP_FAILED;
}

mp_int32 StubGetCurrentAlarmInfoByAlarmID(const mp_string& strAlarmID, alarm_Info_t& stAlarmInfo)
{
    stAlarmInfo.iAlarmSN = -1;
    return MP_SUCCESS;
}

mp_int32 StubQueryTable(mp_void* pThis, const mp_string& strSql, DbParamStream& dpl, DBReader& readBuff, 
    mp_int32& iRowCount, mp_int32& iColCount)
{
    iRowCount = 1;
    mp_string strTmp = "1001";
    readBuff << strTmp;
    strTmp = "iAlarmID";
    readBuff << strTmp;
    strTmp = "1";
    readBuff << strTmp;
    strTmp = "1";
    readBuff << strTmp;
    strTmp = "1";
    readBuff << strTmp;
    strTmp = "1";
    readBuff << strTmp;
    strTmp = "1";
    readBuff << strTmp;
    strTmp = "strAlarmParam";
    readBuff << strTmp;
    strTmp = "192.168.1.1";
    readBuff << strTmp;
    return MP_SUCCESS;
}

static mp_int32 StubQueryTableZero(void* This, mp_string strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    iRowCount = 0;
    iColCount = 0;
    return MP_SUCCESS;
}

static mp_int32 CreateTable(mp_string strSql)
{
    LOGGUARD("");
    DbParamStream dps;
    mp_int32 iRet = CDB::GetInstance().ExecSql(strSql, dps);
    if (iRet != MP_SUCCESS)
    {
        COMMLOG(OS_LOG_ERROR, "Create table failed, iRet = %d.", iRet);
    }
    else
    {
        COMMLOG(OS_LOG_INFO, "Create table succeeded.");
    }
    return iRet;
}


static mp_int32 StubCheckTrapInfoTable()
{
    return MP_SUCCESS;
}

static bool StubUpdateAlarmObjFalse()
{
    enterCount ++;
    return false;
}

TEST_F(CAlarmDBTest, OperSqlittest)
{
        DBReader readBuff;
        mp_int32 iRowCount = 0;
        mp_int32 iColCount = 0;

        DbParamStream dps;
        StubClogToVoidLogNullPointReference();
        stub.set(&CLogger::Log, StubCLoggerLog);
        mp_int32 iRet = CDB::GetInstance().QueryTable("sss",dps, readBuff, iRowCount, iColCount);
        EXPECT_EQ(iRet, MP_FAILED); 
        iRet = CDB::GetInstance().ExecSql("ssss",dps);
        EXPECT_EQ(iRet, MP_FAILED);
        iRet = CDB::GetInstance().QueryTable("sss", dps, readBuff, iRowCount, iColCount);
        EXPECT_EQ(iRet, MP_FAILED);
        iRet = CDB::GetInstance().ExecSql("ssss", dps);
        EXPECT_EQ(iRet, MP_FAILED);
}

bool Ret_True (){
    return true;
}

mp_int32 Ret_Fail(){
    return MP_FAILED;
}

TEST_F(CAlarmDBTest, OperDBtest)
{
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
   try
    {
        //alarm_Info_t stAlarmInfo = {-1, -1, -1, -1, -1, "", "", ""};
        //alarm_Info_t stAlarmInfo = {-1, "-1", -1, -1, -1, "00:00:00", "00:00:10", ""};
        trap_server stTrapServer;
        stub.set(&CDB::ExecSql, &StubExecSql);
         /********Begin AlarmDB.cpp test********/
        
        alarm_Info_t stAlarmInfo;
        stAlarmInfo.iAlarmCategoryType = 1;
        stAlarmInfo.iAlarmID ="2";
        stAlarmInfo.severity = 3;
        stAlarmInfo.iAlarmSN = 4;
        stAlarmInfo.iAlarmType = 5;
        stAlarmInfo.strAlarmParam = "this is alarm param";
        stAlarmInfo.strEndTime = 0;
        stAlarmInfo.strStartTime = 1;

        mp_int32 iRet = AlarmDB::InsertAlarmInfo(stAlarmInfo);
        EXPECT_EQ(iRet, MP_FAILED);

        iRet = AlarmDB::DeleteAlarmInfo(1, stAlarmInfo.iAlarmID);
        EXPECT_EQ(iRet, MP_FAILED);

        iRet = AlarmDB::DeleteAlarmInfo(1, stAlarmInfo.iAlarmID);
        EXPECT_EQ(iRet, MP_FAILED);

        iRet = AlarmDB::UpdateAlarmInfo(stAlarmInfo);
        EXPECT_EQ(iRet, MP_FAILED);

        iRet = AlarmDB::DeleteTrapServer(stTrapServer);
        EXPECT_EQ(iRet, MP_FAILED);

        stTrapServer.iPort = 59526;
        stTrapServer.iVersion = 3;
        stTrapServer.strServerIP = "100.136.25.95";
        stub.set(&AlarmDB::CheckTrapInfoTable, StubCheckTrapInfoTable);
        iRet = AlarmDB::InsertTrapServer(stTrapServer);
        EXPECT_EQ(iRet, MP_FAILED);

        std::vector<trap_server> vecStServerInfo;
        iRet = AlarmDB::GetAllTrapInfo(vecStServerInfo);
        EXPECT_EQ(iRet, MP_FAILED);

        iRet = AlarmDB::UpdateAllTrapInfo(vecStServerInfo);
        EXPECT_EQ(iRet, MP_FAILED);

        iRet = AlarmDB::DeleteAllTrapServer();
        EXPECT_EQ(iRet, MP_FAILED);

        /******************************************/
        vector<alarm_Info_t> vecAlarmInfo;
        stub.set(&CDB::QueryTable, StubCheckTrapInfoTable);
        iRet = AlarmDB::GetAllAlarmInfo(vecAlarmInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);

        iRet = AlarmDB::GetAlarmInfoBySNAndID(1, stAlarmInfo.iAlarmID, stAlarmInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);

        iRet = AlarmDB::GetCurrentAlarmInfoByAlarmID("1", stAlarmInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);

        iRet = AlarmDB::GetAlarmInfoByParam(stAlarmInfo.iAlarmID, "this is param", stAlarmInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);

        iRet = AlarmDB::GetAllTrapInfo(vecStServerInfo);
        EXPECT_EQ(iRet, MP_SUCCESS);

        mp_int32 iAlarmSn = 0;
        stub.set(&CDB::QueryTable, StubCheckTrapInfoTable);
        stub.set(&CDB::ExecSql, &StubExecSqlSU);
        iRet = AlarmDB::GetSN(iAlarmSn);
        EXPECT_EQ(iRet, MP_SUCCESS);

        iRet = AlarmDB::SetSN(1);
        EXPECT_EQ(iRet, MP_SUCCESS);  

        /********End AlarmDB.cpp test********/

        /********Begin AppFreezeStatus.cpp test********/
        AppFreezeStatus oCAppFree;
        freeze_status stStatus;
        stStatus.strKey = "/home/tyj";
        stStatus.iStatus = 0;

        iRet = oCAppFree.Insert(stStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);

        iRet = oCAppFree.Delete(stStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);

        oCAppFree.Get(stStatus);
        EXPECT_EQ(stStatus.iStatus, DB_UNFREEZE);

        vector<freeze_status> vecStatus;
        iRet = oCAppFree.GetAll(vecStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);

        stub.set(&CDB::ExecSql, Ret_Fail);
        iRet = oCAppFree.Insert(stStatus);
        EXPECT_EQ(iRet, MP_FAILED);
        

        stub.set(&CDB::QueryTable, Ret_Fail);
        iRet = oCAppFree.Insert(stStatus);
        EXPECT_EQ(iRet, MP_FAILED);
        iRet = oCAppFree.GetAll(vecStatus);
        EXPECT_EQ(iRet, MP_FAILED);

        stub.set(&AppFreezeStatus::IsExist, Ret_True);
        iRet = oCAppFree.Insert(stStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);
        iRet = oCAppFree.Delete(stStatus);
        EXPECT_EQ(iRet, MP_FAILED);

        /********End AppFreezeStatus.cpp test********/

        /********Begin Trap.cpp test********/
        /********Begin AlarmHandle.cpp test********/
        alarm_param_t stAlarmParman;
        stAlarmParman.iAlarmID = 1;
        stAlarmParman.strAlarmParam = "this is param";
        AlarmHandle alarmHandle;
        //alarmHandle.Alarm(stAlarmParman);

        //alarmHandle.ClearAlarm(stAlarmParman);

        /********End AlarmHandle.cpp test********/

        iRet = oCAppFree.Insert(stStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }
    catch(...)
    {
        printf("Error on %s file %d line.\n", __FILE__, __LINE__);
        exit(0);
    }
}


TEST_F(CAlarmDBTest, DbParam_int64_uint32_uint64)
{
    mp_int64 itmp64;
    DbParam tmp1(itmp64);
    mp_int64 utmp32;
    DbParam tmp2(utmp32);
    mp_int64 utmp64;
    DbParam tmp3(utmp64);
}

TEST_F(CMpAlarmTest, GetSnmpV3Paramtest)
{
    StubClogToVoidLogNullPointReference_cmp();
    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    try
    {
        snmp_v3_param stLocalParam;

        stub_cmp.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString_fail);
        CAlarmConfig::GetSnmpV3Param(stLocalParam);

        mp_int32 iRet = CAlarmConfig::UpdateSnmpV3Param(stLocalParam);
        EXPECT_NE(iRet, MP_SUCCESS);

        stub_cmp.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString_succ);
        stub_cmp.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32_fail);
        CAlarmConfig::GetSnmpV3Param(stLocalParam);

        stub_cmp.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString_succ);
        stub_cmp.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32_succ);
        CAlarmConfig::GetSnmpV3Param(stLocalParam);
        
    }
    catch(...)
    {
        printf("Error on %s file %d line.\n", __FILE__, __LINE__);
        exit(0);
    }
}

TEST_F(CMpAlarmTest, PreCompileExecSqlTest)
{
    StubClogToVoidLogNullPointReference_cmp();
    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    typedef mp_void (*StubFuncType)(void);
    typedef mp_void (CLogger::*LogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);
    //Stub<LogType,StubFuncType, void> stubLog(&CLogger::Log, &StubCLoggerLog);
    typedef mp_string (CPath::*GetDbPathType)(mp_string);
    typedef mp_string (*StubTypeReturnString)(void);
    //Stub<GetDbPathType, StubTypeReturnString, void> stubGetPath(&CPath::GetDbFilePath, StubGetDbFilePath);

    //create table test
    stub_cmp.set(&CDB::ExecSql, &StubExecSqlSU);
    mp_string create_sql="CREATE TABLE IF NOT EXISTS [AlarmTable] ([AlarmSerialNo] INTEGER(4) NOT NULL ON CONFLICT ABORT COLLATE BINARY DEFAULT (0),[AlarmID] INTEGER(4) COLLATE BINARY DEFAULT (0), [AlarmType] INTEGER(4) DEFAULT (0),[AlarmLevel] INTEGER(4) DEFAULT (2), [AlarmBeginTime] VARCHAR(20),[AlarmClearTime] VARCHAR(20),[AlarmParam] VARCHAR(100), CONSTRAINT [] PRIMARY KEY ([AlarmSerialNo] COLLATE BINARY, [AlarmID]));";
    ASSERT_EQ(MP_SUCCESS, CreateTable(create_sql));

    //insert test
    alarm_Info_t  alarmInfo;
    alarmInfo.iAlarmSN = 7;
    alarmInfo.iAlarmID = "52625429";
    alarmInfo.iAlarmType = 1;
    alarmInfo.strEndTime = 1;
    mp_time time;
    CMpTime::Now(time);
    mp_string strNowTime = CMpTime::GetTimeString(time);
    alarmInfo.strStartTime = 1;
    alarmInfo.iAlarmCategoryType = 1;
    alarmInfo.strAlarmParam = "test";
    ASSERT_EQ(MP_SUCCESS, AlarmDB::InsertAlarmInfo(alarmInfo));

    //get all test
    vector<alarm_Info_t> infoList;
    AlarmDB::GetAllAlarmInfo(infoList);
    EXPECT_EQ(0, infoList.size());
    alarm_Info_t queryAlarmInfo;
    queryAlarmInfo.iAlarmCategoryType = 1;
    queryAlarmInfo.iAlarmID = "2";
    queryAlarmInfo.severity = 3;
    queryAlarmInfo.iAlarmSN = 4;
    queryAlarmInfo.iAlarmType = 5;
    queryAlarmInfo.strAlarmParam = "this is alarm param";
    queryAlarmInfo.strEndTime = 0;
    queryAlarmInfo.strStartTime = 1;
    // queryAlarmInfo = infoList.front();
   // EXPECT_EQ(queryAlarmInfo.iAlarmSN, 7);
   // EXPECT_EQ(queryAlarmInfo.iAlarmID,  "52625429");
   // EXPECT_EQ(queryAlarmInfo.iAlarmType, 1);
   // EXPECT_EQ(queryAlarmInfo.strEndTime, 1);
   // EXPECT_EQ(queryAlarmInfo.strAlarmParam, "test");

    //update test
    alarmInfo.strEndTime = 1;
    ASSERT_EQ(MP_SUCCESS, AlarmDB::UpdateAlarmInfo(alarmInfo));

    //getAlarmInfoBySNAndID
    alarmInfo.strEndTime = 1;
    ASSERT_EQ(MP_FAILED, AlarmDB::GetAlarmInfoBySNAndID(7, alarmInfo.iAlarmID, alarmInfo));
    EXPECT_EQ(alarmInfo.strEndTime, 1);
    
    //delete test
    ASSERT_EQ(MP_SUCCESS, AlarmDB::DeleteAlarmInfo(7, alarmInfo.iAlarmID));
    infoList.clear();
    EXPECT_EQ(infoList.size(), 0);
    AlarmDB::GetAllAlarmInfo(infoList);
    EXPECT_EQ(infoList.size(), 0);
    
    remove("zwgTest.db");
}

/*
*用例名称：验证发送告警、恢复告警、发送事件接口
*前置条件：alarmhandle发送告警/事件/告警清除接口均成功
*check点：alarmhandle的接口均被正常调用到，调试次数与实际相符
*/
TEST_F(CMpAlarmTest, TestAlarmMgr)
{
    enterCount = 0;
    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    stub_cmp.set(ADDR(AlarmHandle, Alarm), StubAlarmResult);
    stub_cmp.set(ADDR(AlarmHandle, Event), StubAlarmResult);
    stub_cmp.set(ADDR(AlarmHandle, ClearAlarm), StubAlarmResult);
    stub_cmp.set(ADDR(CIP, GetListenIPAndPort), StubGetListenIPAndPort);
    AlarmMgr::GetInstance().SendAlarm("0x64032D0002", "param1");
    AlarmMgr::GetInstance().ResumeAlarm("0x64032D0002", "param1");
    AlarmMgr::GetInstance().SendEvent("0x64032D0002", "param1");
    EXPECT_EQ(3, enterCount);

    stub_cmp.set(ADDR(CIP, GetListenIPAndPort), StubGetListenIPAndPortFailed);
    AlarmMgr::GetInstance().SendAlarm("0x64032D0002", "param1");
    AlarmMgr::GetInstance().ResumeAlarm("0x64032D0002", "param1");
    AlarmMgr::GetInstance().SendEvent("0x64032D0002", "param1");
    EXPECT_EQ(3, enterCount);
}

/*
*用例名称：验证发送告警、恢复告警、发送事件接口
*前置条件：alarmhandle发送告警/事件/告警清除接口均成功
*check点：alarmhandle的接口均被正常调用到，调试次数与实际相符
*/
TEST_F(CMpAlarmTest, TestAlarmMgrRetry)
{
    enterCount = 0;
    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    stub_cmp.set(&DoSleep, StubDoSleep);
    stub_cmp.set(ADDR(AlarmHandle, Alarm), StubAlarmRetry);
    stub_cmp.set(ADDR(AlarmHandle, Event), StubAlarmRetry);
    stub_cmp.set(ADDR(AlarmHandle, ClearAlarm), StubAlarmRetry);
    stub_cmp.set(ADDR(CIP, GetListenIPAndPort), StubGetListenIPAndPort);
    AlarmMgr::GetInstance().SendAlarm("0x64032D0002");
    AlarmMgr::GetInstance().ResumeAlarm("0x64032D0002");
    AlarmMgr::GetInstance().SendEvent("0x64032D0002");
    EXPECT_EQ(6, enterCount);
}

TEST_F(CMpAlarmTest, GetAllAlarmInfo_TEST)
{
    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    stub_cmp.set(&CDB::QueryTable, StubQueryTable);

    std::vector<alarm_Info_t> vecAlarmInfo;
    EXPECT_EQ(MP_SUCCESS, AlarmDB::GetAllAlarmInfo(vecAlarmInfo));
    EXPECT_EQ(1, vecAlarmInfo.size());
    EXPECT_EQ(1001, vecAlarmInfo.front().iAlarmSN);
}

TEST_F(CMpAlarmTest, GetAlarmInfo_TEST)
{
    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    stub_cmp.set(&CDB::QueryTable, StubQueryTable);

    mp_int32 iAlarmSN;
    mp_string iAlarmID;
    mp_string strAlarmParam;
    alarm_Info_t stAlarmInfo;

    std::vector<alarm_Info_t> vecAlarmInfo;
    EXPECT_EQ(MP_SUCCESS, AlarmDB::GetAlarmInfoBySNAndID(iAlarmSN, iAlarmID, stAlarmInfo));
    EXPECT_EQ(1001, stAlarmInfo.iAlarmSN);
    EXPECT_EQ(MP_SUCCESS, AlarmDB::GetCurrentAlarmInfoByAlarmID(iAlarmID, stAlarmInfo));
    EXPECT_EQ(1001, stAlarmInfo.iAlarmSN);
    EXPECT_EQ(MP_SUCCESS, AlarmDB::GetAlarmInfoByParam(iAlarmID, strAlarmParam, stAlarmInfo));
    EXPECT_EQ(1001, stAlarmInfo.iAlarmSN);
}

TEST_F(CMpAlarmTest, UpdateAllTrapInfo_TEST)
{
    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    stub_cmp.set(ADDR(AlarmDB, CheckTrapInfoTable), StubSuccess);
    stub_cmp.set(ADDR(AlarmDB, DeleteAllTrapServer), StubSuccess);
    stub_cmp.set(ADDR(CDB, ExecSql), StubSuccess);

    trap_server info;
    info.iPort = 9081;
    info.iVersion = 1;
    info.strServerIP = "192.168.1.1";
    std::vector<trap_server> vecStServerInfo;
    vecStServerInfo.push_back(info);

    EXPECT_EQ(MP_SUCCESS, AlarmDB::UpdateAllTrapInfo(vecStServerInfo));
}

TEST_F(CMpAlarmTest, CheckTrapInfoTable_TEST)
{
    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    stub_cmp.set(ADDR(CDB, QueryTable), StubSuccess);
    stub_cmp.set(ADDR(CDB, ExecSql), StubSuccess);

    EXPECT_EQ(MP_SUCCESS, AlarmDB::CheckTrapInfoTable());
}

TEST_F(CMpAlarmTest, AlarmHandle_Alarm)
{
    alarm_param_t alarmParam;
    alarmParam.iAlarmID = "0001";
    AlarmHandle alarmHandle;
    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    stub_cmp.set(ADDR(AlarmDB, GetAlarmInfoByParam), StubFailed);
    EXPECT_EQ(MP_FAILED, alarmHandle.Alarm(alarmParam));

    stub_cmp.set(ADDR(AlarmDB, GetAlarmInfoByParam), StubSuccess);
    stub_cmp.set(ADDR(AlarmHandle, NewAlarm), StubFailed);
    EXPECT_EQ(MP_FAILED, alarmHandle.Alarm(alarmParam));
}

TEST_F(CMpAlarmTest, AlarmHandle_ClearAlarm)
{
    alarm_param_t alarmParam;
    alarmParam.iAlarmID = "0001";
    AlarmHandle alarmHandle;

    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    stub_cmp.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubGetCurrentAlarmInfoByAlarmID);
    EXPECT_EQ(MP_SUCCESS, alarmHandle.ClearAlarm(alarmParam));
}

TEST_F(CMpAlarmTest, AlarmHandle_NewAlarm)
{
    alarm_param_t alarmParam;
    alarm_Info_t alarmInfo;
    AlarmHandle alarmHandle;

    stub_cmp.set(&CLogger::Log, StubCLoggerLog);
    stub_cmp.set(ADDR(AlarmDB, GetSN), StubFailed);
    EXPECT_EQ(MP_FAILED, alarmHandle.NewAlarm(alarmParam, alarmInfo));

    stub_cmp.set(ADDR(AlarmDB, GetSN), StubSuccess);
    stub_cmp.set(ADDR(AlarmHandle, UpdateAlmInfo), StubSuccess);
    stub_cmp.set(ADDR(AlarmDB, InsertAlarmInfo), StubFailed);
    EXPECT_EQ(MP_FAILED, alarmHandle.NewAlarm(alarmParam, alarmInfo));

    stub_cmp.set(ADDR(AlarmDB, InsertAlarmInfo), StubSuccess);
    stub_cmp.set(ADDR(AlarmDB, SetSN), StubFailed);
    EXPECT_EQ(MP_FAILED, alarmHandle.NewAlarm(alarmParam, alarmInfo));
}

TEST_F(CMpAlarmTest, AlarmHandle_SendAlarm_Http)
{
    alarm_Info_t alarmInfo;
    AlarmHandle alarmHandle;
    stub_cmp.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubFailed);
    EXPECT_EQ(MP_FAILED, alarmHandle.SendAlarm_Http(alarmInfo));

    stub_cmp.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    EXPECT_EQ(MP_FAILED, alarmHandle.SendAlarm_Http(alarmInfo));
}

TEST_F(CMpAlarmTest, AlarmHandle_BuildHttpRequest)
{
    HttpRequest req;
    mp_string ip;
    mp_string port;
    AlarmHandle alarmHandle;
    stub_cmp.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubFailed);
    EXPECT_EQ(MP_FAILED, alarmHandle.BuildHttpRequest(req, ip, port));

    stub_cmp.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    EXPECT_EQ(MP_FAILED, alarmHandle.BuildHttpRequest(req, ip, port));
}



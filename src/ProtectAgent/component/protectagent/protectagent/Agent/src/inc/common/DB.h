#ifndef _COMMON_DB_H_
#define _COMMON_DB_H_

#include <list>
#include "common/Types.h"
#include "common/CMpThread.h"
#include "sqlite/sqlite3.h"
#include <stdlib.h>
#include <sstream>

// sqlite数据库表名
static const mp_string TRAP_SERVER_TABLE  = "TrapInfoTable";   // 记录注册trap server信息表
static const mp_string ALARM_TABLE        = "AlarmTable";      // 记录上报告警信息表
static const mp_string ALARM_TYPE_TABLE   = "AlarmTypeTable";  // 记录流水号信息表
static const mp_string APP_STATUS_TABLE   = "AppStatusTable";  // 记录应用冻结状态信息表
static const mp_string FREEZE_OBJ_TABLE   = "FreezeObjTable";  // 记录冻结对象
static const mp_string g_MobilityTable    = "OMProtectInfo";   // 记录Mobility保护的信息
static const mp_string g_OMAlarmTable     = "OMAlarmTable";    // 记录Mobility的告警事件，生产者消费者模式
static const mp_string g_OMHWChangeTable  = "OMHWChangeTable"; // 记录Mobility的硬件变化事件
static const mp_string g_BusinessClient   = "BusinessClient";  // 记录BusinessClient的列表，用于Agent重启后自动连接business client
static const mp_string g_BackupParam      = "BackupParam";     // 记录备份时参数，用于多个任务间传递参数
static const mp_string JOBS               = "Jobs";            // 记录外部下发的任务
static const mp_string g_OracleDbInfo     = "OracleDbInfo";    // 记录外部下发的oracle数据库信息
static const mp_string g_PluginJobs       = "PluginJobs"; // 记录外部插件相关任务

// sqlite数据库各表单字段名称
// AlarmTable各字段名
static const mp_string TITLE_ALARM_SERIALNO = "AlarmSerialNo";
static const mp_string TITLE_ALARM_ID       = "AlarmID";
static const mp_string TITLE_ALARM_LEVEL    = "AlarmLevel";
static const mp_string TITLE_ALARM_TYPE     = "AlarmType";
static const mp_string TITLE_ALARM_CATEGORY = "AlarmCategoryType";
static const mp_string TITLE_START_TIME     = "AlarmBeginTime";
static const mp_string TITLE_END_TIME       = "AlarmClearTime";
static const mp_string TITLE_ALARM_PARAM    = "AlarmParam";
static const mp_string TITLE_TRAPSERVER     = "Trapserver";

// AlarmTypeTable各字段名称
static const mp_string TITLE_ALARM_SN = "AlarmSN";

// FreezeObjTable各字段名称
static const mp_string INSTANCE_NAME = "InstanceName";        // 数据库实例名称
static const mp_string DB_NAME       = "DBName";              // 数据库名称
static const mp_string BEGIN_STATUS  = "BeginStatus";         // 数据库开始状态
static const mp_string LOOPTIME      = "LoopTime";            // 监控轮询时间
static const mp_string USER          = "User";                // 数据库访问
static const mp_string MP            = "MP";                  // 数据库访问
static const mp_string JSON_DATA     = "JsonData";            // 请求的json消息
static const mp_string APPTYPE       = "AppType";             // 应用类型
static const mp_string BEGIN_TIME    = "BeginTime";           // 监控开始时间

// OMProtectInfo各字段名称
const static mp_string g_OMProtectInfoVMId              = "VMId";
const static mp_string g_OMProtectInfoOMAId             = "OMAId";
const static mp_string g_OMProtectInfoOMAIp             = "OMAIp";
const static mp_string g_OMProtectInfoOMAPort           = "OMAPort";
const static mp_string g_OMProtectInfoRPO               = "RPO";
const static mp_string g_OMProtectInfoGranularity       = "Granularity";
const static mp_string g_OMProtectInfoSize              = "ProtectSize";
const static mp_string g_OMProtectInfoHwInfo            = "HwInfo";

// OMAlarmInfo表的字段名称
const static mp_string g_OMAlarmInfoErrCode             = "ErrorCode";     // mp_uint64
const static mp_string g_OMAlarmInfoDesc                = "Desc";          // json format string
const static mp_string g_OMAlarmInfoID                  = "OMAlarmID";     // mp_uint64
const static mp_string g_OMAlarmInfoTime                = "OMAlarmTime";   // 告警发生的时间

const static mp_string g_OMHWChangeInfo                 = "HWChangeInfo";      // json format string
const static mp_string g_OMHWChangeTime                 = "HWChangeTime";      // 硬件变化发生的时间
const static mp_string g_OMHWNeedSend                   = "IsNeedSend";        // 表中硬件变化信息是否需要上传

// business clien表字段
const static mp_string g_BusiClientRole                 = "role";           // 微服务角色
const static mp_string g_BusiClientIP                   = "busiIP";         // 微服务IP
const static mp_string g_BusiClientPort                 = "busiPort";       // 微服务端口

// BackupParam fileds
const static mp_string g_ParamID                        = "ID";             // 参数ID
const static mp_string g_ParamKey                       = "key";            // 参数key
const static mp_string g_ParamValue                     = "value";          // 参数Value

// Jobs 表字段
const static mp_string g_ID = "ID";
const static mp_string g_Status = "status";
const static mp_string g_Step = "step";
const static mp_string g_SubStepStatus = "subStepStatus";
const static mp_string g_SubStep = "subStep";
const static mp_string g_ConnIP = "connIP";
const static mp_string g_ConnPort = "connPort";
const static mp_string g_InnerPID = "innerPID";
const static mp_string g_TaskType = "taskType";
const static mp_string g_MsgBody = "msgBody";

// g_OracleDBInfo 表字段
const static mp_string g_OracleDBInfo_DbName = "DbName";
const static mp_string g_OracleDBInfo_DbInstance = "DbInstance";
const static mp_string g_OracleDBInfo_DbUser = "DbUser";
const static mp_string g_OracleDBInfo_DbPassword = "DbPassword";
const static mp_string g_OracleDBInfo_ASMInstance = "ASMInstance";
const static mp_string g_OracleDBInfo_ASMUser = "ASMUser";
const static mp_string g_OracleDBInfo_ASMPassword = "ASMPassword";
const static mp_string g_OracleDBInfo_OracleUser = "OracleUser";
const static mp_string g_OracleDBInfo_GridUser = "GridUser";
const static mp_string g_OracleDBInfo_RunUserPwd = "RunUserPwd";
const static mp_string g_OracleDBInfo_AccessOracleHome = "AccessOracleHome";
const static mp_string g_OracleDBInfo_AccessOracleBase = "AccessOracleBase";

// PluginJobs 表和表字段
const static mp_string g_PluginJobs_AppType = "AppType";
const static mp_string g_PluginJobs_MainID = "MainID";
const static mp_string g_PluginJobs_SubID = "SubID";
const static mp_string g_PluginJobs_MainType = "MainType";
const static mp_string g_PluginJobs_SubType = "SubType";
const static mp_string g_PluginJobs_Status = "Status";
const static mp_string g_PluginJobs_MountPoints = "MountPoints";
const static mp_string g_PluginJobs_DmeIPS = "DmeIPS";
const static mp_string g_PluginJobs_GenerateTime = "GenerateTime";
const static mp_string g_PluginJobs_RunEnable = "RunEnable";

#define atoint32(x)  mp_int32(atoi(x))
#define atoint64(x)  mp_int64(atoll(x))
#define IntToString(i, s)        \
    do {                         \
        std::stringstream ss;    \
        ss << i;                 \
        ss >> s;                 \
    } while (0)

// 使用此宏后不需要分号
#define FREE_STMT_THEN_DISCONNECT_RETURN_MPFAILED(x) do                                        \
    {                                                                                         \
        COMMLOG(OS_LOG_ERROR, x);                                           \
        if (SQLITE_OK != sqlite3_finalize(&stmt)) {                                            \
            COMMLOG(OS_LOG_ERROR, "sqlite3_finalize failed");               \
        }                                                                                     \
        if (MP_SUCCESS != Disconnect()) {                                                     \
            COMMLOG(OS_LOG_ERROR, "Disconnect DB failed"); \
        }                                                                                     \
        return MP_FAILED;                                                                     \
    } while (0)

typedef enum {
    DB_PARAM_TYPE_INT32,
    DB_PARAM_TYPE_INT64,
    DB_PARAM_TYPE_UINT32,
    DB_PARAM_TYPE_UINT64,
    DB_PARAM_TYPE_STRING
} DbParamType;

struct AGENT_API DbParam {
    DbParamType m_type = DB_PARAM_TYPE_STRING;
    mp_string m_value;
    DbParam() {}
    DbParam(const DbParam& other)
    {
        m_type = other.m_type;
        m_value = other.m_value;
    }
    DbParam(DbParam& other)
    {
        m_type = other.m_type;
        m_value = other.m_value;
    }
    DbParam(DbParam&& other)
    {
        m_type = other.m_type;
        std::swap(m_value, other.m_value);
    }
    DbParam& operator=(const DbParam& other)
    {
        m_type = other.m_type;
        m_value = other.m_value;
        return *this;
    }

    DbParam(const mp_string& value)
    {
        m_type = DB_PARAM_TYPE_STRING;
        m_value = value;
    }
    DbParam(mp_string&& value)
    {
        m_type = DB_PARAM_TYPE_STRING;
        std::swap(m_value, value);
    }
    DbParam(mp_int32 value);
    DbParam(mp_int64 value);
    DbParam(mp_uint32 value);
    DbParam(mp_uint64 value);
};
// 使用预编译模式查询时的入参
class AGENT_API DbParamStream {
public:
    DbParamStream()
    {}
    ~DbParamStream()
    {}
    mp_void Clear()
    {
        m_ParamList.clear();
    }
    mp_bool Empty()
    {
        return m_ParamList.empty();
    }
    DbParam operator>>(DbParam& param);

    // 支持右值操作，连续<<
    DbParamStream& operator<<(const DbParam& param)
    {
        m_ParamList.emplace_back(param);
        return *this;
    }
    DbParamStream& operator<<(DbParam& param)
    {
        m_ParamList.emplace_back(param);
        return *this;
    }
    DbParamStream& operator<<(DbParam&& param)
    {
        m_ParamList.emplace_back(std::move(param));
        return *this;
    }

private:
    std::list<DbParam> m_ParamList;
};

class AGENT_API DBReader {
public:
    DBReader();
    ~DBReader();
    mp_string operator>>(mp_string& strResult);
    mp_string operator<<(mp_string& strResult);
    mp_void Clear();
    mp_bool Empty();

private:
    std::list<mp_string> m_lstResult;
};

class DB {
public:
    DB() : m_pDB(NULL), m_pTransDB(NULL) { }
    ~DB() { }

    // sql语句预编译方式
    mp_int32 ExecSql(const mp_string& dbFile, const mp_string& strSql, DbParamStream& dpl);
    mp_int32 QueryTable(const mp_string& dbFile, const mp_string& strSql, DbParamStream& dpl,
        DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount);

    // 事务操作
    mp_int32 BeginTrans(const mp_string& dbFile);
    mp_int32 RollbackTrans();
    mp_int32 CommitTrans();

private:
    mp_int32 Connect(const mp_string& dbFile, mp_bool isTrans = MP_FALSE);
    mp_int32 Disconnect(mp_bool isTrans = MP_FALSE);
    mp_int32 CheckNeedDisconnect(mp_bool isTrans);

    // ExecSql函数拆分，降低函数复杂度
    sqlite3_stmt* SqlPrepare(const mp_string& sql);
    mp_int32 SqlBind(sqlite3_stmt& stmt, DbParamStream& dps);
    mp_int32 SqlExecute(sqlite3_stmt& stmt);
    mp_int32 SqlQuery(sqlite3_stmt& stmt, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount);

    sqlite3* m_pDB;                // m_pDB的内存管理由sqlit自己保证，sqlite3_open时申请，sqlite3_close时释放
    std::list<mp_string> m_stringList;  // 用于存放sqlite_bind_text的字符串
    sqlite3* m_pTransDB;           // 用于事务的指针
};

// DWS特性会比较频繁的写sqlite数据库，为了避免影响原有的功能，拆分成两个数据库(sqlite是库级写锁)，对应CDB和DWSDB两个壳子
class AGENT_API CDB {
public:
    // sql语句预编译方式
    mp_int32 ExecSql(const mp_string& strSql, DbParamStream& dpl);
    mp_int32 QueryTable(
        const mp_string& strSql, DbParamStream& dpl, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount);
    // 事务操作
    mp_int32 BeginTrans();
    mp_int32 RollbackTrans();
    mp_int32 CommitTrans();
    static CDB& GetInstance(void)
    {
        return m_Instance;
    }

    ~CDB()
    {
        CMpThread::DestroyLock(&m_InstanceLock);
    }

private:
    CDB(CDB& cdb) {}
    CDB& operator=(const CDB& cdb);
    CDB()
    {
        CMpThread::InitLock(&m_InstanceLock);
    }

    static CDB m_Instance;
    thread_lock_t m_InstanceLock;
    DB m_db;
};

class AGENT_API DWSDB {
public:
    // sql语句预编译方式
    mp_int32 ExecSql(const mp_string &dbFile, const mp_string& strSql, DbParamStream& dpl);
    mp_int32 QueryTable(const mp_string &dbFile,
        const mp_string& strSql, DbParamStream& dpl, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount);
    // 事务操作，需要在外层调用处加锁
    mp_int32 ExecSqlNoLock(const mp_string &dbFile, const mp_string& strSql, DbParamStream& dpl);
    mp_int32 BeginTrans(const mp_string &dbFile);
    mp_int32 RollbackTrans();
    mp_int32 CommitTrans();

    static DWSDB& GetInstance()
    {
        return m_Instance;
    }

    ~DWSDB()
    {
        CMpThread::DestroyLock(&m_InstanceLock);
    }
private:
    DWSDB()
    {
        CMpThread::InitLock(&m_InstanceLock);
    }
    static DWSDB m_Instance;
    thread_lock_t m_InstanceLock;
    DB m_db;
};

#endif

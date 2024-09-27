/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleDefines.h
 * @brief  Contains function declarations OracleDefines
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef __AGENT_ORACLE_DEFINES_H__
#define __AGENT_ORACLE_DEFINES_H__

#include "common/Types.h"

// URL参数
static const mp_string RESPOND_ORACLE_PARAM_INSTNAME = "instName";
static const mp_string RESPOND_ORACLE_PARAM_DBNAME = "dbName";
static const mp_string RESPOND_ORACLE_PARAM_VERSION = "version";
static const mp_string RESPOND_ORACLE_PARAM_STATE = "state";
static const mp_string RESPOND_ORACLE_PARAM_ISASMDB = "isAsmInst";
static const mp_string RESPOND_ORACLE_PARAM_AUTHTYPE = "authType";
static const mp_string RESPOND_ORACLE_PARAM_DBROLE = "dbRole";
static const mp_string RESPOND_ORACLE_PARAM_ISCLUSTER = "isCluster";
static const mp_string RESPOND_ORACLE_PARAM_ORACLE_HOME = "oracleHome";
static const mp_string RESPOND_ORACLE_PARAM_DBUUID = "dbUUID";

static const mp_string RESPOND_ORACLE_PARAM_CONID = "conID";
static const mp_string RESPOND_ORACLE_PARAM_PDBNAME = "pdbName";
static const mp_string RESPOND_ORACLE_PARAM_PDBSTATUS = "status";

static const mp_string RESPOND_ORACLE_PARAM_ASMINSTNAME = "asmInstName";
static const mp_string RESPOND_ORACLE_PARAM_SEARCHARCHIVE = "isArchive";

static const mp_string RESPOND_ORACLE_PARAM_ARCHIVETHRESHOLD = "threshold";

static const mp_string RESPOND_ORACLE_PARAM_CDBTYPE = "type";

static const mp_string RESPOND_ORACLE_LUNID = "lunId";
static const mp_string RESPOND_ORACLE_UUID = "uuid";
static const mp_string RESPOND_ORACLE_ARRAYSN = "arraySn";
static const mp_string RESPOND_ORACLE_WWN = "wwn";
static const mp_string RESPOND_ORACLE_VOLTYPE = "volType";
static const mp_string RESPOND_ORACLE_VGNAME = "vgName";
static const mp_string RESPOND_ORACLE_DEVICENAME = "deviceName";
static const mp_string RESPOND_ORACLE_PVNAME = "pvName";
static const mp_string RESPOND_ORACLE_STORMAINTYPE = "deviceType";
static const mp_string RESPOND_ORACLE_STORSUBTYPE = "volType";
static const mp_string RESPOND_ORACLE_DEVICEPATH = "devicePath";
static const mp_string RESPOND_ORACLE_UDEVRULES = "udevRules";
static const mp_string RESPOND_ORACLE_LBA = "LBA";
static const mp_string RESPOND_ORACLE_ASMDISKGROUPS = "asmDiskGroups";
static const mp_string RESPOND_ORACLE_ASMDISKGROUP = "asmDiskGroup";
static const mp_string RESPOND_ORACLE_ISASM = "isASM";
static const mp_string REST_PARAM_ORACLE_STATE = "state";
static const mp_string REST_PARAM_ORACLE_IS_INCLUDE_ARCH = "isIncludeArchLog";
static const mp_string RESPOND_ORACLE_TS_CONNAME = "CONNAME";
static const mp_string RESPOND_ORACLE_TS_TSNAME = "TSNAME";
static const mp_string RESPOND_ORACLE_TS_FILENO = "FileNo";
static const mp_string RESPOND_ORACLE_TS_FILENAME = "FileName";

static const mp_string REST_PARAM_ORACLE_TIME = "time";
static const mp_string REST_PARAM_ORACLE_LOGTYPE = "logType";
static const mp_string REST_PARAM_ORACLE_ERRORCODE = "errorCode";
static const mp_string REST_PARAM_ORACLE_ERRORMESSAGE = "errorMessage";
static const mp_string REST_PARAM_ORACLE_INSTANCEARRAY = "instances";
static const mp_string RESPOND_ORACLE_PARAM_INST_LIST = "instNames";
static const mp_string RESPOND_ORACLE_VSS_WRITER_STATUS = "vssWriterStatus";
static const mp_string ORACLE_TASK_RESTORE = "restore";

// 脚本定义参数名称
static const mp_string ORACLE_SCRIPTPARAM_DBNAME = "AppName=";
static const mp_string ORACLE_SCRIPTPARAM_DBUUID = "DBUUID=";
static const mp_string ORACLE_SCRIPTPARAM_INSTNAME = "InstanceName=";
static const mp_string ORACLE_SCRIPTPARAM_DBUSERNAME = "UserName=";
static const mp_string ORACLE_SCRIPTPARAM_DBPASSWORD = "Password=";
static const mp_string ORACLE_SCRIPTPARAM_ORACLE_HOME = "OracleHome=";
static const mp_string ORACLE_SCRIPTPARAM_ASMUSERNAME = "ASMUserName=";
static const mp_string ORACLE_SCRIPTPARAM_ASMPASSWOD = "ASMPassword=";
static const mp_string ORACLE_SCRIPTPARAM_ASMINSTANCE = "ASMInstanceName=";
static const mp_string ORACLE_SCRIPTPARAM_TABLESPACENAME = "TableSpaceName=";
static const mp_string ORACLE_SCRIPTPARAM_FRUSHTYPE = "FrushType=";
static const mp_string ORACLE_SCRIPTPARAM_ARCHIVETHRESHOLD = "ArchiveThreshold=";
static const mp_string ORACLE_SCRIPTPARAM_ASMDISKGROUPS = "ASMDiskGroups=";
static const mp_string ORACLE_SCRIPTPARAM_ISASM = "IsASM=";
static const mp_string ORACLE_SCRIPTPARAM_ACTION = "Action=";  // 停止和开始动作 0:start  1:stop
static const mp_string ORACLE_SCIPRTPARAM_IS_INCLUDE_ARCH = "IsIncludeArchLog=";
static const mp_string ORACLE_SCIPRTPARAM_TRUNCATE_LOG_TIME = "TruncateLogTime=";
static const mp_string ORACLE_SCIPRTPARAM_PDBNAME = "PDBName=";
static const mp_string ORACLE_SCIPRTPARAM_CHANNEL = "Channel=";
static const mp_string ORACLE_SCIPRTPARAM_PITSCN = "pitScn=";
static const mp_string ORACLE_SCIPRTPARAM_PITTIME = "pitTime=";
static const mp_string ORACLE_SCIPRTPARAM_BACKUPLEVEL = "Level=";
static const mp_string ORACLE_SCIPRTPARAM_BACKUPQOS = "Qos=";
static const mp_string ORACLE_SCIPRTPARAM_TRUNCATE_LOG = "truncateLog=";
static const mp_string ORACLE_SCIPRTPARAM_TASKTYPE = "TaskType=";
static const mp_string ORACLE_SCIPRTPARAM_CHECKTYPE = "CheckType=";
static const mp_string ORACLE_SCIPRTPARAM_RECOVERTARGET = "recoverTarget=";
static const mp_string ORACLE_SCIPRTPARAM_RECOVERPATH = "recoverPath=";
static const mp_string ORACLE_SCIPRTPARAM_RECOVERORDER = "recoverOrder=";
static const mp_string ORACLE_SCIPRTPARAM_RECOVERNUM = "recoverNum=";
static const mp_string ORACLE_SCIPRTPARAM_QUERY_TASKMODE = "queryTaskMode=";
static const mp_string ORACLE_SCIPRTPARAM_STORTYPE = "storType=";
static const mp_string ORACLE_SCIPRTPARAM_DBTYPE = "dbType=";
static const mp_string ORACLE_SCIPRTPARAM_ENCALGO = "EncAlgo=";
static const mp_string ORACLE_SCIPRTPARAM_ENCKEY = "EncKey=";
static const mp_string ORACLE_SCIPRTPARAM_PFIILEPID = "pfilePID=";
static const mp_string ORACLE_SCIPRTPARAM_RESTOREBY = "RestoreBy=";
static const mp_string ORACLE_SCIPRTPARAM_ARCHIVELOG_KEEPDAYS = "ArchiveLogKeepDays=";
static const mp_string ORACLE_SCRIPTPARAM_RMANTASKTYPE = "RmanTaskType=";
static const mp_string ORACLE_SCRIPTPARAM_TASKINNERID = "TaskInnerID=";
static const mp_string ORACLE_SCRIPTPARAM_STARTDB = "startDb=";

static const mp_string ORACLE_SCRIPTPARAM_FREEZEDB = "0";
static const mp_string ORACLE_SCRIPTPARAM_THAWDB = "1";
static const mp_string ORACLE_SCRIPTPARAM_FREEZESTATUS = "2";
static const mp_string ORACLE_SCRIPTPARAM_ARCHIVEDB = "3";
static const mp_string ORACLE_SCRIPTPARAM_TRUNCATEARCHIVELOG = "4";
static const mp_string ORACLE_SCRIPTPARAM_GET_ARCHIVE_LOG_MODE = "5";
static const mp_string ORACLE_SCRIPTPARAM_CHECK_ORACLE = "6";

// 分割符定义暂时放此，稍后移植到公共类中
static const mp_string DBADAPTIVE_PRAMA_MUST = "must";
static const mp_string DBADAPTIVE_PRAMA_OPTION = "option";
static const mp_string DBADAPTIVE_PRAMA_ARCHIVE = "archive";

// 定义PDB状态
static const mp_uchar PDB_MOUNTED = 0;
static const mp_uchar PDB_READ_ONLY = 1;
static const mp_uchar PDB_READ_WRITE = 2;

static const mp_string INIT_PDB_STATUS_MOUNTED = "MOUNTED";
static const mp_string INIT_PDB_STATUS_READ_ONLY = "READ ONLY";
static const mp_string INIT_PDB_STATUS_READ_WRITE = "READ WRITE";

// windows下的脚本列表
static const mp_string WIN_ORACLE_INFO = "oracleinfo.bat";
static const mp_string WIN_ORACLE_LUN_INFO = "oracleluninfo.bat";
static const mp_string WIN_ORACLE_TEST = "oracletest.bat";
static const mp_string WIN_ORACLE_CONSISTENT = "oracleconsistent.bat";
static const mp_string WIN_ORACLE_ASMACTION = "oraasmaction.bat";
static const mp_string WIN_ORACLE_CHECK_ARCHIVE = "oraclecheckarchive.bat";
static const mp_string WIN_ORACLE_DB_ACTION = "oradbaction.bat";
static const mp_string WIN_ORACLE_CHECK_CDB = "oraclecheckcdb.bat";
static const mp_string WIN_ORACLE_QUERY_TABLESPACE = "oraclequerytablespace.bat";
static const mp_string WIN_ORACLE_QUERY_ASM = "oracleasm.bat";

static const mp_string STORAGE_TYPE_FS = "0";
static const mp_string STORAGE_TYPE_RAW = "1";
static const mp_string STORAGE_TYPE_ASMLIB = "2";
static const mp_string STORAGE_TYPE_ASMRAW = "3";
static const mp_string STORAGE_TYPE_ASMLINK = "4";
static const mp_string STORAGE_TYPE_ASMUDEV = "5";
static const mp_string STORAGE_TYPE_ASMWIN = "6";

static const mp_uchar VOLTYPE_NOVOL = 0;
static const mp_uchar ORACLE_QUERY_ARCHIVE_LOGS = 1;

static const mp_uchar NON_ARCHIVE_LOG_MODE = 0;
static const mp_uchar ARCHIVE_LOG_MODE = 1;

static const mp_uchar ORACLE_TYPE_NON_CDB = 0;
static const mp_uchar ORACLE_TYPE_CDB = 1;

static const mp_string ORA_INSTANCE_DATABASE_CONFIGFILE = "oracleinfo.cfg";
static const mp_string ORACLE_REG_KEY = "SOFTWARE\\ORACLE";

// oracle clean task type
static const mp_int32 ORA_CLEAN_TASK = 0;
static const mp_int32 ORA_DISMOUNT_TASK = 1;

// oracle backup storage type
static const mp_int32 ORA_STORTYPE_NAS = 0;
static const mp_int32 ORA_STORTYPE_ISCSI = 1;
static const mp_int32 ORA_STORTYPE_FC = 2;

// oracle DB dbtype
static const mp_int32 ORA_DBTYPE_ASM = 0;
static const mp_int32 ORA_DBTYPE_FS = 1;

// volume info
static const mp_string keyDataVol = "dataVolumes";
static const mp_string keyLogVol = "logVolumes";

// storage info
static const mp_string keyStorage = "storage";
static const mp_string keyTaskType = "taskType";
static const mp_string keyStorType = "storType";
static const mp_string keyStorageIp = "storageIp";
static const mp_string keyDataSharePath = "dataSharePath";
static const mp_string keyLogSharePath = "logSharePath";

// hostRole 1-storage host, 2-business host, 3-storage+buiness host
static const mp_string keyHostRole = "hostRole";

// oracle task init key
static const mp_string keyDbInfos = "dbInfo";
static const mp_string keyDbType = "dbType";

static const mp_string KEY_SCRIPTS = "scripts";

// dismount param key
static const mp_string keyVgDataName = "VGDATA=";
static const mp_string keyLvDataName = "LVDATA=";
static const mp_string keyDataMountPath = "PATHDATA=";
static const mp_string keyVgLogName = "VGLOG=";
static const mp_string keyLvLogName = "LVLOG=";
static const mp_string keyLogMountPath = "PATHLOG=";
static const mp_string keyDiskList = "DISKLIST=";
static const mp_string keyMetaDataPath = "METADATAPATH=";
static const mp_string keyScriptParamTaskType = "TASKTYPE=";
static const mp_string keyScriptParamStorType = "STORTYPE=";

// mount param key
static const mp_string keyDataShareMountPath = "dataShareMountPath=";
static const mp_string keyLogShareMountPath = "logShareMountPath=";
static const mp_string keyScriptParamDataOwnerIP = "dataOwnerIps=";
static const mp_string keyScriptParamDataOtherIP = "dataOtherIps=";
static const mp_string keyScriptParamLogOwnerIP = "logOwnerIps=";
static const mp_string keyScriptParamLogOtherIP = "logOtherIps=";
static const mp_string keyScriptParamDataturboIP = "dataturboIps=";
static const mp_string keyScriptParamAuthUser = "authUser=";
static const mp_string keyScriptParamAuthKey = "authKey=";
static const mp_string keyScriptParamDataturboName = "storageName=";
static const mp_string keyScriptParamLinkEncry = "linkEncryption=";

static const mp_string keyDbParams = "params";
static const mp_string keyChannel = "channel";
static const mp_string keyPitTime = "pitTime";
static const mp_string keyPitScn = "pitScn";
static const mp_string keyStartDB = "startDB";
static const mp_string keyRecoverTarget = "recoverTarget";
static const mp_string keyRecoverPath = "recoverPath";
static const mp_string keyRecoverOrder = "recoverOrder";
static const mp_string keyRecoverNum = "recoverNum";
static const mp_string keyPfile = "pfile";
static const mp_string keyPfileParams = "pfileParams";
static const mp_string keyRestoreBy = "RestoreBy";
static const mp_string keyLevel = "level";
static const mp_string keyLimitSpeed = "limitSpeed";
static const mp_string keyTruncateLog = "truncateLog";
static const mp_string keyArchiveLogKeepDays = "ArchiveLogKeepDays";
static const mp_string keyStorInfo = "storage";

static const mp_string g_EncAlgo = "EncAlgo";
static const mp_string g_EncKey = "EncKey";

// max string len
static const mp_int32 ORACLE_PLUGIN_MAX_STRING = 512;
static const mp_int32 ORACLE_PLUGIN_MAX_DBNAME = 8;
static const mp_int32 ORACLE_PLUGIN_MAX_DBINSTANCENAME = 128;
static const mp_int32 ORACLE_PLUGIN_MAX_NSERNAME = 30;
static const mp_int32 ORACLE_PLUGIN_MAX_DBTYPE = 5;
static const mp_int32 ORACLE_PLUGIN_MAX_INT32 = INT32_MAX;
static const mp_int32 ORACLE_PLUGIN_MAX_INTGENERAL = 10;
static const mp_int32 ORACLE_PLUGIN_MAX_RESTORYBY = 10;
static const mp_int32 ORACLE_PLUGIN_MAX_CHANNEL = 256;
static const mp_int32 ORACLE_PLUGIN_MAX_UUID = 36;
static const mp_int32 ORACLE_PLUGIN_MAX_TASKTYPE = 6;
static const mp_int32 ORACLE_PLUGIN_MAX_HOSTROLE = 3;
static const mp_int32 ORACLE_PLUGIN_MAX_SCRIPT = 256;
static const mp_int32 ORACLE_PLUGIN_MAX_LEVEL = 2;
typedef struct tag_oracle_instance_info {
    mp_string strInstName;     // 实例
    mp_string strDBName;       // 数据库名称
    mp_string strVersion;      // 数据库版本
    mp_int32 iState;           // 状态,0-在线,1-离线
    mp_int32 iIsASMDB;         // 是否是ASM数据库
    mp_int32 iArchiveLogMode;  // oracle的归档模式
#ifdef WIN32
    mp_int32 iVssWriterStatus;  // VssWriter状态,0-停止,1-启动,2-其他
#endif
    mp_string strOracleHome;  // ORACLE_HOME环境变量的值
    mp_int32 authType;        // 鉴权类型，确定OS认证是否开启
    mp_int32 dbRole;          // 数据库实例角色
    mp_string strDBUUID;       // 数据库dbid
} oracle_inst_info_t;

typedef struct tag_oracle_error_info {
    mp_int32 iErrorCode;    // 错误码
    mp_string strInstName;  // 实例
    // mp_string   strDBName;                  //数据库名称
} oracle_error_info_t;

typedef struct tag_oracle_dataBase_info {
    mp_string strInstName;
    mp_string strDBName;
    mp_string strDBUsername;
    mp_string strDBPassword;
    mp_string strASMInstance;
    mp_string strASMUserName;
    mp_string strASMPassword;
    mp_int32 iGetArchiveLUN;    // 查询oracle的lun信息是否查询归档日志所在lun， 1 -- 查询， 0 -- 不查询
    mp_int32 iIncludeArchLog;   // 启动时是否包含归档日志，1 -- 包含，0 -- 不包含
    mp_string strOracleHome;
    mp_string strArchThreshold;
    mp_string strTableSpaceName;
    mp_string strASMDiskGroup;
    mp_string strIsASM;
    mp_int32 dbType;            // 数据库类型
} oracle_db_info_t;

typedef struct tag_oracle_storage_script_info {
    mp_string strStorMainType;
    mp_string strStorSubType;
    mp_string strSystemDevice;
    mp_string strDeviceName;
    mp_string strDevicePath;
    mp_string strVgName;
    mp_string strASMDiskGroup;
    mp_string strUDEVRes;
    mp_string strUDEVName;
    mp_string strLBA;
} oracle_storage_script_info;

typedef struct tag_oracle_lun_info {
    mp_string strInstName;
    mp_string strDBName;
    mp_string strLUNId;
    mp_string strUUID;
    mp_string strArraySn;
    mp_string strWWN;
    mp_string strVgName;
    mp_string strDeviceName;
    mp_string strPvName;
    mp_int32 iStorMainType;
    mp_int32 iStorSubType;
    mp_string strDevicePath;
    mp_string strUDEVRules;
    mp_string strLBA;
    mp_string strASMDiskGroup;
} oracle_lun_info_t;

typedef struct tag_oracle_rsp_pdb_info {
    mp_int32 iConID;
    mp_string strPdbName;
    mp_int32 iStatus;
} oracle_pdb_rsp_info_t;

typedef struct tag_oracle_req_pdb_info {
    mp_string strOracleHome;
    mp_string strInstName;
    mp_string strPdbName;
    mp_string strDBUsername;
    mp_string strDBPassword;
} oracle_pdb_req_info_t;

#endif  // __AGENT_ORACLE_H__

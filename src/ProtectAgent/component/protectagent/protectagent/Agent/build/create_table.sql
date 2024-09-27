PRAGMA foreign_keys=OFF;
BEGIN TRANSACTION;

CREATE TABLE IF NOT EXISTS [AlarmTable] (
    [AlarmSerialNo] INTEGER(4) NOT NULL ON CONFLICT ABORT COLLATE BINARY DEFAULT (0), 
    [AlarmID] VARCHAR(64) COLLATE BINARY DEFAULT (0), 
    [AlarmLevel] INTEGER(4) DEFAULT (2), 
    [AlarmType] INTEGER(4) DEFAULT (0), 
    [AlarmCategoryType] INTEGER(4) DEFAULT (1), 
    [AlarmBeginTime] INTEGER(20), 
    [AlarmClearTime] INTEGER(20), 
    [AlarmParam] VARCHAR(100), 
    [Trapserver] VARCHAR(255), 
    CONSTRAINT [] PRIMARY KEY ([AlarmSerialNo] COLLATE BINARY, [AlarmID]));

CREATE TABLE IF NOT EXISTS [AlarmTypeTable] (
  [AlarmSN] INTEGER(4), 
  CONSTRAINT [] PRIMARY KEY ([AlarmSN]));
CREATE TABLE IF NOT EXISTS [TrapInfoTable] (
  [TrapServerIP] VARCHAR(30) NOT NULL ON CONFLICT ROLLBACK COLLATE BINARY DEFAULT (0), 
  [TrapPort] INTEGER(4) NOT NULL ON CONFLICT ROLLBACK COLLATE BINARY DEFAULT (162), 
  [SnmpVersion] INTEGER(1) NOT NULL ON CONFLICT ROLLBACK DEFAULT (1), AgentIP VARCHAR(30) NOT NULL DEFAULT (0), 
  CONSTRAINT [] PRIMARY KEY ([TrapServerIP] COLLATE BINARY, [TrapPort], [SnmpVersion]));
CREATE TABLE IF NOT EXISTS [AppStatusTable] (
  [Key] VARCHAR(100));
CREATE TABLE IF NOT EXISTS [FreezeObjTable] (
  [InstanceName] VARCHAR(1024), 
  [DBName] VARCHAR(64), 
  [BeginStatus] INTEGER(4), 
  [LoopTime] INTEGER(4), 
  [User] VARCHAR(1024), 
  [MP] VARCHAR(1024), 
  [JsonData] VARCHAR(1024), 
  [AppType] INTEGER(4), 
  [BeginTime] INT64(8));
  
CREATE TABLE IF NOT EXISTS [OMProtectInfo] (
  [VMId]  VARCHAR(64),
  [OMAId] VARCHAR(64),
  [OMAIp] VARCHAR(64),
  [OMAPort] INTEGER(4) DEFAULT(0),
  [RPO] INTEGER(4) DEFAULT(0),
  [Granularity] INTEGER(1),
  [ProtectSize] INTEGER(4) DEFAULT(0),
  [HwInfo] TEXT);

CREATE TABLE IF NOT EXISTS [OMAlarmTable] (
  [ErrorCode]  INT64(8),
  [OMAlarmID]  INT64(8) NOT NULL PRIMARY KEY, 
  [OMAlarmTime] VARCHAR(20),
  [Desc] TEXT);

CREATE TABLE IF NOT EXISTS [BusinessClient] (
  [role]  INTEGER(4),
  [busiIP]  VARCHAR(64),
  [busiPort] INTEGER(4));

--通过job表，不支持在线升级场景，升级时如果添加删除步骤会出现任务失败场景
CREATE TABLE IF NOT EXISTS [Jobs] (
  [ID] VARCHAR(64),
  [status] INTEGER(4),
  [step] VARCHAR(64),
  [subStepStatus] INTEGER(4),
  [subStep] VARCHAR(64),
  [connIp] VARCHAR(64),
  [connPort] INTEGER(4),
  [innerPID] VARCHAR(64),
  [taskType] INTEGER(4),
  [msgBody] VARCHAR(10240),
  [startTime] DATE(64),
  CONSTRAINT [] PRIMARY KEY ([ID], [step]));

CREATE TABLE IF NOT EXISTS [BackupParam] (
  [ID] VARCHAR(64),
  [key] VARCHAR(64),
  [value] VARCHAR(1024),
  CONSTRAINT [] PRIMARY KEY ([ID], [key]));

CREATE TABLE IF NOT EXISTS [OracleDbInfo] (
    [DbName] VARCHAR(64) NOT NULL, 
    [DbInstance] VARCHAR(64) NOT NULL, 
    [DbUser] VARCHAR(64), 
    [DbPassword] VARCHAR(10240), 
    [ASMInstance] VARCHAR(64), 
    [ASMUser] VARCHAR(64), 
    [ASMPassword] VARCHAR(10240),
    [OracleUser] VARCHAR(64),
    [GridUser] VARCHAR(64),
    [RunUserPwd] VARCHAR(10240),
    [AccessOracleHome] VARCHAR(2048),
    [AccessOracleBase] VARCHAR(2048));

CREATE TABLE IF NOT EXISTS [PluginJobs] (
    [AppType] VARCHAR(64) NOT NULL,
    [MainID] VARCHAR(64) NOT NULL,
    [SubID] VARCHAR(64),
    [MainType] INTEGER(4) NOT NULL,
    [SubType] INTEGER(4),
    [Status] INTEGER(4),
    [MountPoints] VARCHAR(10240),
    [DmeIPS] VARCHAR(1024),
    [GenerateTime] INT64(8),
    [RunEnable] INTEGER(4),
    CONSTRAINT [] PRIMARY KEY ([MainID], [SubID]));

COMMIT;

#
# This file is a part of the open-eBackup project.
# This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
# If a copy of the MPL was not distributed with this file, You can obtain one at
# http://mozilla.org/MPL/2.0/.
#
# Copyright (c) [2024] Huawei Technologies Co.,Ltd.
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#

from enum import Enum
from common.const import RpcParamKey
from common.env_common import get_install_head_path


class GetIPConstant:
    ADDRESS_FAMILY_AF_INET = 2
    LOCAL_HOST = "127.0.0.1"


class SubJobName:
    MOUNT = "mountSubJob"
    EXEC_BACKUP = "execSubJob"
    EXEC_LOG_BACKUP = "execLogBackup"
    EXEC_COPY_BINLOG = "execCopyBinlog"
    EXEC_REPORT_DATA_SIZE = "execReportDataSize"
    EXEC_MDS_BACKUP = "execMdsBackup"
    EXEC_MDS_DIFF_BACKUP = "execMdsDiffBackup"
    EXEC_MDS_BINLOG_BACKUP = "execMdsBinlogBackup"
    EXEC_FLUSH_LOG = "execFlushLog"
    EXEC_DATA_BACKUP = "execDataBackup"
    EXEC_DIFF_BACKUP = "execDiffBackup"
    EXEC_BINLOG_BACKUP = "execBinlogBackup"
    EXEC_START_BINLOG_BACKUP = "execStartBinlogBackup"
    EXEC_ACTIVE_BACKUP = "execActiveBackup"
    EXEC_SEQUENCE_BACKUP = "execSequenceBackup"
    EXEC_BINLOG_COPY = "execBinlogCopy"
    QUERY_COPY = "queryCopy"
    QUERY_SCAN_REOSITORIES = "queryScanRepositories"


class ManagerPriority:
    # 管理节点执行备份，恢复的优先级为降序排列，1为最高
    priority = 1


class RoleIniName:
    CLUSTERMANAGERINI = "clustermanager.ini"
    METADATASERVERINI = "metadataserver.ini"
    GTMINI = "gtm.ini"
    DBAGENTINI = "dbagent.ini"


class RoleIniSection:
    CLUSTERMANAGER = "clustermanager"
    METADATASERVER = "Metadataserver"
    GTMINI = "seq"
    DBAGENTINI = "backup_restore"


class RoleBackupDir:
    CLUSTERMANAGER = "backup_root_directory"
    METADATASERVER = "metadata_backup_dir"
    GTM = "seq_backup_dir"
    DBAGENT = "backup_rootdir"
    ACTIVE = "active_file_directory"


class FormatCapacity(int, Enum):
    BASE_SIZE = 1024
    MB_SIZE = BASE_SIZE
    GB_SIZE = BASE_SIZE * MB_SIZE
    TB_SIZE = BASE_SIZE * GB_SIZE
    PB_SIZE = BASE_SIZE * TB_SIZE


class MountBindPath:
    ROACH_META_FILE_PATH = "/mnt/databackup/GoldenDB-clusterInstance/meta"
    DATA_FILE_PATH = "/mnt/databackup/GoldenDB-clusterInstance/data"
    META_FILE_PATH = "/mnt/databackup/GoldenDB-clusterInstance/meta"
    DB_BIN_PATH = f'{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/'
    XTRABACKUP2 = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/xtrabackup2"
    MDS_PATH = f'{get_install_head_path()}/DataBackup/MDS_DATA'
    MDS_CONF_PATH = "/home/goldendb/etc/my.cnf"
    LIB_PATH = "/home/goldendb/zxmanager/lib/"


class GoldenJsonConstant:
    SUB_JOB = "subJob"
    JOB_NAME = "jobName"


class ErrorCode(int, Enum):
    # 增量转全量
    ERROR_INCREMENT_TO_FULL = 1577209901
    # 数据库版本不一致
    ERROR_DIFFERENT_VERSION = 1577209971
    # 集群节点数不一致
    ERROR_DIFFERENT_TOPO = 1577209972
    # 数据库用户名不一致
    ERROR_DIFFERENT_USER = 1577209973
    # 挂载路径失败，原因：内部错误导致挂载路径权限与请求有差异
    ERROR_MOUNT_PATH = 1577209974
    # 数据库状态异常
    ERR_DATABASE_STATUS = 1577210000
    # 集群用户不存在
    ERROR_USER_NOT_EXIST_CLUSTER = 0x5E0250D7
    # 恢复失败
    ERR_RESTORED = 1577210101
    # 不支持修改数据库名称
    ERROR_RENAME_DATABASE_NAME = 1577209901
    # 数据库不存在，返回参数 数据库名称
    ERROR_DB_NOT_EXIST = 1577213477
    # 认证信息错误
    ERROR_AUTH = 1577209942
    # 参数错误
    ERROR_PARAM = 1677929218
    DB_NODES_NOT_FULL_MATCH = 1677931026
    # 不能修改数据库为新的数据库
    ERR_RENAME_DATABASE = 1577213482
    # 系统异常，执行操作过程中，因系统异常，导致操作失败。无参数
    SYSTEM_ERROR = 1677929221
    # 不是所有数据库的服务都运行
    NOT_ALL_DB_SERVICE_RUNNING = 9
    # 增量转全量
    INC_TO_FULL = 0x5E02502D
    # 数据库不存在
    ERR_DB_NOT_EXIST = 1577210047
    # 部署类型错误
    ERR_DEPLOY_TYPE = 0x5E02508B
    # 设备版本不符合系统要求，版本不支持
    ERR_NOT_SUPPORT_VERSION = 1677935120
    # 数据库环境异常不支持备份恢复
    ERR_ENVIRONMENT = 1577213475
    # 账号密码错误
    LOGIN_FAILED = 1677929488
    # 数据库服务异常
    ERR_DB_SERVICES = 1577213479
    # 输入参数错误
    ERR_INPUT_STRING = 1677934101
    # 主机选择有误
    ERR_CHOSEN_HOSTS = 1577209936
    # 校验socket文件路径失败
    ERR_SOCKET_PATH = 10
    # cnf文件路径错误
    ERR_CONF_PATH = 11
    # keyring文件错误
    ERR_KEYRING_PATH = 12
    # log_bin日志未开启
    LOG_BIN_OFF_ERROR = 1577209945
    # 校验节点类型失败
    ERROR_NODE_TYPE = 1577209938
    # 校验分片结构不一致
    ERROR_GOLDENDB_STRUCTURE = 1577210032
    # 内部错误，DMC进行调度过程中，由于rest操作、构造结构校验或数据库操作结果异常，导致调度操作失败。无参数
    ERROR_INTERNAL = 1593987329  # 0x5F025101
    # 未配置实例级存储配置文件
    ERROR_STORAGE_CONFIG_FAIL = 1577213500
    # 实例存在异常
    ERROR_CLUSTER_ABNORMAL = 1577213501
    # 执行备份/恢复命令失败，执行命令（{0}）异常({1})
    EXEC_BACKUP_RECOVER_CMD_FAIL = 1577209989
    # GoldenDB不支持高版本往低版本恢复
    ERR_NEW_LOC_RST_VER_CONFLICT = 1677933074
    # 检查文件系统中副本异常
    ERR_BKP_CHECK = 1677873258
    # 用户密码错误
    ERROR_LOGIN_INFO = 1577209986


class GoldenSubJobName:
    SUB_VER_CHECK = "sub_ver_check"
    SUB_CHECK = "sub_check"
    SUB_EXEC = "sub_exec"
    SUB_BINLOG_MERGE = "sub_binlog_merge"


class V5SubJobStep:
    RESTORE_OFF = "restore_off"
    RESTORE_MANAGER = "restore_manager"
    PREPARE_DATA = "prepare_data"
    RESTORE_DATA = "restore_data"
    RESTORE_LOG_DATA = "restore_log_data"
    RESTORE_LOG_DATA_WITH_FIXED_TIME = "restore_log_data_with_fixed_time"
    WRITE_MASTER_INFO = "write_master_info"
    CHANGE_MASTER = "change_master"
    RESTORE_SEQUENCE = "restore_sequence"
    RESTORE_ACTIVE = "restore_active"
    RESTORE_ON = "restore_on"


class MasterSlavePolicy:
    MASTER = "master"
    SLAVE = "slave"


class LogLevel(int, Enum):
    INFO = 1
    WARN = 2
    ERROR = 3
    SERIOUS = 4


class GoldenDBPath:
    GoldenDB_FILESYSTEM_MOUNT_PATH = "/mnt/databackup/"
    GoldenDB_LINK_PATH = "/mnt/databackup/gbase"
    XTRBACKUP_golden = "xtrabackup"


class ClusterInfoStr:
    """
    解析goldenDB集群参数所用字段
    """
    CLUSTER_ID = "ClusterId:"
    CLUSTER_NAME = "ClusterName:"
    GROUP_ID = "group id:"
    DATABASE_NUM = "db num:"
    NODE_ID = "id:"
    NODE_NAME = "name:"
    NODE_IP = "db ip:"
    NODE_PORT = "db ip:"
    NODE_ROLE = "db role:"
    INSTANCE_TYPE = "instancetype:"
    NODE_IP_V5 = "ip:"
    NODE_PORT_V5 = "port:"
    MASTER_NODE_IP_V5 = "master ip:"
    MASTER_NODE_PORT_V5 = "master port:"
    SINGLE_GRP = "single group[0:NO,1:YES]"


class GoldenDBSubType:
    """
    goldenDB涉及的类型
    """
    TYPE = "Database"
    SUBTYPE_CLUSTER = "GoldenDB-cluster"
    SUBTYPE_CLUSTER_INSTANCE = "GoldenDB-clusterInstance"


class GoldenDBSql:
    """
    GoldenDB执行的SQL语句
    """
    SHOW_SOCKET = "show variables like 'socket';"
    SHOW_BIN_LOG = "show variables like '%log_bin%';"
    SHOW_LOG_LEVEL = "show variables like 'binlog_format';"
    SHOW_DATABASES = "show databases;"


class GoldenDBNodeType:
    """
    GoldenDB的节点类型
    """
    ZX_MANAGER_NODE = "managerNode"
    GTM_NODE = "gtmNode"
    DATA_NODE = "dataNode"


class GoldenDBSupportVersion(str, Enum):
    """
    GoldenDB支持的版本
    """
    VERSION_5_0_02 = "V5.0.02"
    VERSION_5_2 = "V5.2"
    VERSION_6_1_01 = "V6.1.01"
    VERSION_6_1_02 = "V6.1.02"
    VERSION_6_1_03 = "V6.1.03"


class GoldenDBNodeStatus:
    """
    GoldenDB在线状态
    """
    # 在线
    ONLINE = "1"
    # 离线
    OFFLINE = "0"
    # 异常
    ABNORMAL = "8"


class GoldenDBNodeService:
    """
    各个服务的列表
    """
    ZX_MANAGER_SERVICE_LIST = ("metadataserver", "proxymanager", "clustermanager", "ommagent")
    DATA_NODE_SERVICE_LIST = ("dbagent", "loadserver", "ommagent")
    ZX_GTM_SERVICE_LIST = ("gtm", "ommagent")
    SERVICE_DICT = {
        "managerNode": ZX_MANAGER_SERVICE_LIST, "dataNode": DATA_NODE_SERVICE_LIST,
        "gtmNode": ZX_GTM_SERVICE_LIST
    }
    gtm_info = {'1': [{'gtmId': '1', 'gtmIp': '51.20.109.48', 'port': '6026', 'masterFlag': '1'},
                      {'gtmId': '2', 'gtmIp': '51.20.109.49', 'port': '6026', 'masterFlag': '0'}]}
    VALID_INFO = 'qjuOr3j8aA4I5dMWulImcw=='


class GoldenDBMetaInfo:
    COPYINFO = "copy_info"
    GOLDENDBINFO = "goldendb_info"


class GoldenDBJsonConst:
    """
    GoldenDB的json字段名
    """
    EXTENDINFO = "extendInfo"
    CLUSTERTYPE = "clusterType"
    JOB = "job"
    JOBPARAM = "jobParam"
    BACKUPTYPE = "backupType"
    PROTECTOBJECT = "protectObject"
    SUBTYPE = "subType"
    APPENV = "protectEnv"
    ENDPOINT = "endpoint"
    AUTH = "auth"
    AUTHKEY = "authKey"
    AUTHPWD = "authPwd"
    INSTANCEPORT = "instancePort"
    NODES = "nodes"
    REPORITTORIES = "repositories"
    REPORITORYTYPE = "repositoryType"
    PATH = "path"
    BACKJOBRESLUT = "backupJobResult"
    COPY = "copy"
    COPYID = "copyId"
    TIMESTAMP = "timestamp"
    BACKUPTIME = "backupTime"
    BACKUPLMITE = "BackupLimit"
    NAME = "name"
    COPIES = "copies"
    TARGETOBJECT = "targetObject"
    JOBPARAM = "jobParam"
    RESTOREJOB = "restoreType"
    BACKUPTYPE = "backupType"
    RESTOREJOBRESULT = "restoreJobResult"
    PROGRESS = "progress"
    TASKID = "taskId"
    SUBTASKID = "subTaskId"
    TASKSTATUS = "taskStatus"
    DATASIZE = "dataSize"
    SPEED = "speed"
    LOGDETAIL = "logDetail"
    APPLICTION = "application"
    JOBTYPE = "jobType"
    LIVEMOUNT = "liveMount"
    TARGETENV = "targetEnv"
    ROLE = "role"
    ADVANCEPARAMS = "advanceParams"
    SUBJOB = "subJob"
    JOBNAME = "jobName"
    RESTORETIMESTAMP = "restoreTimestamp"
    TYPE = "type"
    TYPES = "types"
    REMOTEHOST = "remoteHost"
    MOUNTJOBID = "mountJobId"
    LOGINFO = "logInfo"
    LOGINFOPARAM = "logInfoParam"
    LOGLEVEL = "logLevel"
    BACKUPTASKSLA = "backupTask_sla"
    POLICYLIST = "policy_list"
    EXTPARAMETERS = "ext_parameters"
    CHANNELNUMBER = "channel_number"
    NEW_DATABASE_NAME = "newDatabaseName"
    VERSION = "version"
    DATA_DIR = "dataDir"
    LOG_BIN_INDEX_PATH = "logBinIndexPath"
    AVERAGE_SPEED = "averageSpeed"
    LOG_FLAG = "logFlag"
    LOG_FLAG_START_TIME = "logFlagStartTime"
    RESTORE_COPY_ID = "restoreCopyId"
    SERVICE_NAME = "serviceName"
    ID = "id"
    BACKUPHOSTSN = "backupHostSN"
    FIRSTFULLBACKUPTIME = "firstFullBackupTime"
    NEXTCAUSEPARAM = "next_cause_param"
    END_TIME = "endTime"
    GOLDENDB = "GoldenDB"
    CLUSTERINFO = "clusterInfo"
    GROUP = "group"
    OSUSER = "osUser"
    NODETYPE = "nodeType"
    PARENTUUID = "parentUuid"
    MYSQLNODES = "mysqlNodes"
    JOBINFO = "jobInfo"
    GTM = "gtm"


class ProgressInfo(str, Enum):
    START = "Start to do restore"
    FAILED = "Failed to do task, unique id: TU34IHS"
    SUCCEED = "Succeed to do task, unique id: TU34IHS"


class Env:
    USER_NAME = "application_auth_authKey"
    PASS_WORD = "application_auth_authPwd"


class SqliteServiceField(Enum):
    SQLITE_DATABASE_NAME = 'copymetadata.sqlite'
    SQLITE_DATATABLE_NAME = 'T_COPY_METADATA'
    TYPE_DIR = 'd'
    TYPE_FILE = 'f'


class Report:
    REPORT_INTERVAL = 30
    STS_CHECK_INTERVAL = 10
    TIME_OUT = 3600 * 1200


class LastCopyType:
    last_copy_type_dict = {
        1: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY],
        2: [RpcParamKey.FULL_COPY],
        3: [RpcParamKey.LOG_COPY],
        4: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY, RpcParamKey.LOG_COPY]
    }


class BackupStrategyPolicy:
    FULL = "full"
    INCREMENT = "inrc"


class ErrCodeMessage:
    """
    GoldenDB返回错误
    """
    DBTOOL_MDS_BACKUP_CAN_NOT_USE = "Please use dbtool -cm -backup"


class BackupResultMessage:
    """
    GoldenDB查询备份状态返回结果
    """
    BACKUP_IS_GOING = "backup is going"
    BACKUP_IS_DONE = "[100%] OK"
    BACKUP_NOT_EXIST = "no record"


class DbtoolTaskStatus:
    """
    GoldenDB备份/恢复状态
    """
    RUNNING = "running"
    COMPLETED = "completed"
    FAILED = "failed"


class RestoreResultMessage:
    """
    GoldenDB查询恢复状态返回结果
    """
    RESTORE_IS_GOING = "restore is going"
    RESTORE_IS_DONE = "OK"
    # Fail to download file DBCluster_1/LOGICAL_BACKUP/Sequence/sequence_history.json
    Sequence_history_NOT_EXIST = "sequence_history.json"


class ErrPattern:
    Select_Bkp_DB_Fail = "select backup db fail"
    Auth_Check_Fail = "authentication check fail"


class GoldendbLabel:
    pre_check_sla = "goldendb_backup_pre_check_sla_failed_label"
    active_tx_info_record_lost = "goldendb_active_tx_info_record_lost_label"


class ActTxInfoConsts:
    # 1: 标识符，标识这一行是活跃事务信息
    active_tx_info = "1"
    # 2: 标识符，标识这一行是集群中每个分片当时主DN节点的gtid位置信息
    gtid_position = "2"
    # 活跃事务信息最小长度为6
    min_act_tx_info_len = 6
    # 一致性时刻日期的索引
    consistent_date_idx = 2
    # 一致性时刻时间的索引
    consistent_time_idx = 3


class FileInfoDict:
    file_info_dict = {
        "active": {"ini_name": RoleIniName.CLUSTERMANAGERINI, "section": RoleIniSection.CLUSTERMANAGER,
                   "field": RoleBackupDir.ACTIVE},
        GoldenDBNodeType.GTM_NODE: {"ini_name": RoleIniName.GTMINI, "section": RoleIniSection.GTMINI,
                                    "field": RoleBackupDir.GTM},
        GoldenDBNodeType.DATA_NODE: {"ini_name": RoleIniName.DBAGENTINI, "section": RoleIniSection.DBAGENTINI,
                                     "field": RoleBackupDir.DBAGENT},
        GoldenDBNodeType.ZX_MANAGER_NODE: {"ini_name": RoleIniName.CLUSTERMANAGERINI,
                                           "section": RoleIniSection.CLUSTERMANAGER,
                                           "field": RoleBackupDir.CLUSTERMANAGER},
    }


class RepositoryDataTypeEnum(int, Enum):
    META_REPOSITORY = 0
    DATA_REPOSITORY = 1
    CACHE_REPOSITORY = 2
    LOG_REPOSITORY = 3
    INDEX_REPOSITORY = 4
    LOG_META_REPOSITORY = 5

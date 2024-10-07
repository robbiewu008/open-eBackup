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
from common.env_common import get_install_head_path


class GetIPConstant:
    ADDRESS_FAMILY_AF_INET = 2
    LOCAL_HOST = "127.0.0.1"


class RpcParamKey:
    APPLICATION = "application"
    TYPES = "types"
    FULL_COPY = "full"
    INCREMENT_COPY = "increment"
    LOG_COPY = "log"
    COPY_ID = "copyId"
    INPUT_FILE_PREFFIX = "rpcInput"
    OUTPUT_FILE_PREFFIX = "rpcOutput"
    RPC_TOOL = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/rpctool.sh"
    # 输入参数文件目录
    PARAM_FILE_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/tmp"
    # 输出结果文件目录
    RESULT_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/stmp"
    DB_BIN_PATH = f'{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/'


class SubJobName:
    MOUNT = "mountSubJob"
    EXEC_BACKUP = "execSubJob"
    EXEC_LOG_BACKUP = "execLogBackup"
    EXEC_COPY_BINLOG = "execCopyBinlog"
    EXEC_REPORT_DATA_SIZE = "execReportDataSize"


class RoleIniNameEnum:
    CLUSTERMANAGERINI = "clustermanager.ini"
    METADATASERVERINI = "metadataserver.ini"
    GTMINI = "gtm.ini"
    DBAGENTINI = "dbagent.ini"


class RoleBackupDirEnum:
    CLUSTERMANAGER = "backup_root_directory"
    METADATASERVER = "metadata_backup_dir"
    GTM = "seq_backup_dir"
    DBAGENT = "backup_rootdir"


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
    # 挂载路径失败
    ERROR_MOUNT_PATH = 1577209974
    # 数据库状态异常
    ERR_DATABASE_STATUS = 1577210000
    # 集群用户不存在
    ERROR_USER_NOT_EXIST_CLUSTER = 0x5E0250D7
    # 恢复失败
    ERR_RESTORED = 1577210101
    # 不支持修改数据库名称
    ERROR_RENAME_DATABASE_NAME = 1577209901
    # 某个服务未正常开启，返回参数 服务名称
    ERROR_SERVICE_1577213479 = 1577213479
    # 数据库不存在，返回参数 数据库名称
    ERROR_DB_NOT_EXIST = 1577213477
    # 认证信息错误
    ERROR_AUTH = 1577209942
    # 参数错误
    ERROR_PARAM = 1677929218
    DB_NODES_NOT_FULL_MATCH = 1677931026
    # 不能修改数据库为新的数据库
    ERR_RENAME_DATABASE = 1577213482
    # 系统异常
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
    # 内部错误
    ERROR_INTERNAL = 0x5F025101  # 1593987329
    # 未配置实例级存储配置文件
    ERROR_STORAGE_CONFIG_FAIL = 1577213500
    # 实例存在异常
    ERROR_CLUSTER_ABNORMAL = 1577213501
    # 执行备份/恢复命令失败
    EXEC_BACKUP_RECOVER_CMD_FAIL = 1577209989
    # GoldenDB不支持高版本往低版本恢复
    ERR_NEW_LOC_RST_VER_CONFLICT = 1677933074


class SubJobPolicy(int, Enum):
    LOCAL_NODE = 1,
    EVERY_NODE_ONE_TIME = 2,
    RETRY_OTHER_NODE_WHEN_FAILED = 3,
    FIXED_NODE = 4


class GoldenSubJobName:
    SUB_VER_CHECK = "sub_ver_check"
    SUB_CHECK = "sub_check"
    SUB_EXEC = "sub_exec"
    SUB_BINLOG_MERGE = "sub_binlog_merge"
    SUB_BINLOG_MOUNT = "sub_binlog_mount"


class MasterSlavePolicy:
    MASTER = "master"
    SLAVE = "slave"


class CommandReturnCode(Enum):
    ERROR = -1
    # plugin or proxy framework executes successfully
    SUCCESS = 0
    # plugin or proxy framework executes successfully, client need to perform this operation continue
    CONTINUE = 100
    # plugin or proxy framework is busy, client should perform this operation after a period of time
    BUSY = 101
    # an internal error occurred in the plugin or proxy framework
    INTERNAL_ERROR = 200


class NormalErr(int, Enum):
    NO_ERR = 0
    FALSE = -1
    WAITING = -2


class CMDResult(str, Enum):
    """
    执行命令的错误码
    """
    SUCCESS = "0"
    FAILED = "1"
    STORAGE_CONFIG_FAILED = "3"
    CLUSTER_ABNORMAL = "84"


class LogLevel(int, Enum):
    INFO = 1
    WARN = 2
    ERROR = 3
    SERIOUS = 4


class GoldenDBPath:
    GoldenDB_FILESYSTEM_MOUNT_PATH = "/mnt/databackup/"
    GoldenDB_LINK_PATH = "/mnt/databackup/gbase"


class GoldenDBCode(Enum):
    """
    返回给框架的code
    """
    SUCCESS = 0
    FAILED = 200


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
    VERSION_5_2 = "V5.2"
    VERSION_6_1 = "V6.1"


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


class SubJobType(int, Enum):
    PRE_SUB_JOB = 0
    GENERATE_SUB_JOB = 1
    BUSINESS_SUB_JOB = 2
    POST_SUB_JOB = 3


class ExecutePolicy(int, Enum):
    ANY_NODE = 0
    LOCAL_NODE = 1
    EVERY_NODE_ONE_TIME = 2
    RETRY_OTHER_NODE_WHEN_FAILED = 3
    FIXED_NODE = 4


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


class LastCopyType:
    last_copy_type_dict = {
        1: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY],
        2: [RpcParamKey.FULL_COPY],
        3: [RpcParamKey.LOG_COPY],
        4: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY, RpcParamKey.LOG_COPY]
    }

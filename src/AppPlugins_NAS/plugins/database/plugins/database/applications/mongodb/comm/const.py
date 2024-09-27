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

from common.const import ReportDBLabel


class AllStepEnum(str, Enum):
    ALLOW_BACKUP_IN_LOCAL_NODE = "AllowBackupInLocalNode"
    SUPPORT_RESOURCE = "SupportResource"
    QUERY_JOB_PERMISSION = "QueryJobPermission"
    CHECK_BACKUP_JOB_TYPE = "CheckBackupJobType"
    PRE_TASK = "BackupPrerequisite"
    BACKUP = "Backup"
    POST_TASK = "BackupPostJob"
    STOP_TASK = "AbortJob"
    QUERY_BACKUP_COPY = "QueryBackupCopy"
    GENERATOR_SUB_JOB = "GeneratorSubJob"
    CHECK_APPLICATION = "CheckApplication"
    QUERY_CLUSTER = "QueryCluster"
    RESTORE_PREREQUISITE = "RestorePrerequisite"
    ALLOW_RESTORE = "AllowRestoreInLocalNode"
    RESTORE = "Restore"
    RESTORE_POST = "RestorePostJob"
    DELETECOPY = "DelCopy"
    CHECK_COPY = "CheckCopy"
    DELIVE_TASK_STATUS = "DeliverTaskStatus"
    RESTORE_GEN_SUB = "RestoreGenSubJob"
    BACKUP_GEN_SUB = "BackupGenSubJob"


class Status:
    ONLINE = "ONLINE"
    OFFLINE = "OFFLINE"


class ErrorCode(int, Enum):
    # 错误场景：任务执行过程中，请求参数中部分参数非法，导致任务失败。
    # 原因：请求参数非法。
    # 建议：请收集日志并联系技术支持工程师协助解决。
    PARAMS_IS_INVALID = 1593988925
    # 分片集群的分片数量不一致恢复失败
    SHARD_NUMBER_NOT_MATCH = 1677873193
    # mongo工具不在全局变量中
    MONGO_TOOL_NOT_IN_ENV_PATH = 1677873194
    # 恢复存在节点实例运行中
    HAVE_INSTANCE_RUNNING = 1677873195
    # 执行恢复操作时，由于目标实例端口被占用，操作失败。
    INSTANCE_PORT_USED = 1577210054
    # 清理目标实例data目录失败
    CLEAN_DATA_DIR_ERROR = 1677873198
    # 复制数据到data目录失败
    COPY_DATA_ERROR = 1677873199
    # 修改目标实例data目录权限失败
    MODIFY_DATA_DIR_ERROR = 1677873200
    # 生成mongo启动Yaml文件失败
    PARSE_YAML_FILE_ERROR = 1677873201
    # 连接数据库失败
    CONNECT_TO_DB_ERROR = 1677873202
    # 未找到主节点
    PRIMARY_NODE_NOT_FOUND = 1677873203
    # 重建集群失败
    CLUSTER_INITIATE_ERROR = 1677873204
    # 启动实例失败
    INSTANCE_START_ERROR = 1677873205
    ERROR_INCREMENT_TO_FULL = 1577209901
    ERROR_DIFFERENT_VERSION = 1577210056
    ERROR_DIFFERENT_TOPO = 1577209972
    ERROR_DIFFERENT_USER = 1577209973
    ERROR_MOUNT_PATH = 1577209974
    UMOUNT_SNAPSHOT_FILE_ERROR = 1677873190
    ERR_DATABASE_STATUS = 1577210000
    ERROR_USER_NOT_EXIST_CLUSTER = 0x5E0250D7
    ERR_RESTORED = 1577210101
    # 某个服务未正常开启，返回参数 服务名称
    ERROR_SERVICE_1577213479 = 1577213479
    # 认证信息错误
    ERROR_AUTH = 1577209942
    # 参数错误
    ERROR_PARAM = 1677929218
    # 输入参数错误
    ERR_INPUT_STRING = 1677934101
    ERR_USERNAME_OR_PASSWORD = 1577209938
    DB_NODES_NOT_FULL_MATCH = 1677931026
    # 执行资源注册/备份操作时，由于数据库所属的集群不在线，操作失败。
    CLUSTER_NOT_ONLINE = 1577213478
    # 系统异常
    SYSTEM_ERROR = 1677929221
    # 数据库不存在
    ERR_DB_NOT_EXIST = 1577210047
    # 部署类型错误
    ERR_DEPLOY_TYPE = 0x5E02508B
    # 版本不支持
    ERR_NOT_SUPPORT_VERSION = 1677935120
    # 数据库环境异常不支持备份恢复
    ERR_ENVIRONMENT = 1577213475
    # 账号密码错误
    LOGIN_FAILED = 1677929488
    # 数据库服务异常
    ERR_DB_SERVICES = 1577213479
    # 主机选择有误
    ERR_CHOSEN_HOSTS = 1577209936
    # 操作失败
    OPERATE_FAILED = 1677929219
    # 原因：Red Hat Virtualization Manager的IP地址、端口号或域名填写错误。
    # 建议：请检查IP地址、端口号和域名是否正确。
    NOT_ALLOW_EXECUTE = 1677873168
    # 节点无运行实例
    NO_INSTANCE_RUNNING = 1677873176
    # 不支持的挂载方式
    NOT_SUPPORT_MOUNT = 1677873177
    # 仅支持逻辑卷快照
    ONLY_SUPPORT_LVM = 1677873179
    # 卷组空间不足
    OUT_OF_VG_SPACE = 1677873181
    # 不支持oplog
    NOT_SUPPORT_OPLOG = 1677873183
    # 创建快照失败
    FAILED_CREATE_SNAP = 1677873188
    # 复制失败
    FAILED_COPY_DATA = 1677873196
    # 生成备份信息失败
    FAILED_COPY_INFO = 1677873197
    # 上报备份信息失败
    FAILED_REPORT_COPY = 1677873206
    # 注册端口错误
    ENV_PORT_ERROR = 1677873170
    # 主机的认证方式错误
    ENV_AUTH_TYPE_ERROR = 1677873169
    # 主机认证用户权限不足
    ENV_HOST_ROLE_ERROR = 1677873161
    # 主机注册类型与实际类型不一致
    ENV_CLUSTER_TYPE_ERROR = 1677873160
    # 停止平衡失败
    FAILED_STOP_BALANCE = 1677873209
    # 日志备份失败
    FAILED_BACKUP_OPLOG = 1677873210
    # 日志副本数据目录不存在
    LOG_COPIES_DATA_PATH_NOT_EXIST = 1677873189
    # 执行oplog普通恢复失败
    EXECUTE_OPLOG_NORMAL_RESTORE_FAILED = 1677873207
    #  执行oplog时间点恢复失败
    EXECUTE_OPLOG_TIMESTAMP_RESTORE_FAILED = 1677873208
    # 原因：数据库用户角色权限不足。
    # 建议：请确保数据库的用户角色权限满足（{0}）角色权限。
    USER_ROLE_PERMISSION_ERROR = 1677873214
    # 原因：连接实例（{0}）认证失败。
    # 建议：请确保实例的认证方式、用户名、密码正确。
    INSTANCE_AUTH_ERROR = 1677873213
    # 原因：实例（{0}）连接失败。
    # 建议：请确保实例的主机IP、端口、网络连接、服务状态正确。
    INSTANCE_CONNECT_ERROR = 1677873212
    # 原因：启动实例用户执行MongoDB相关命令失败。
    # 建议：请检查用户是否具有MongoDB相关命令权限。
    INSTANCE_USER_EXECUTE_TOOL_ERROR = 1677873249
    # 原因：实例（{0}）的数据库/数据库工具的安装目录不正确，找不到（{1}）工具。
    # 建议：请确保实例的数据库/数据库工具的安装目录填写正确。
    # 场景：由于实例的数据库/数据库工具的安装目录不正确，找不到工具，操作失败。
    INVALID_BIN_PATH = 1677873248


class MongodbErrorCode(int, Enum):
    AUTH_ERROR = 18
    NOT_AUTHORIZED_ERROR = 13


class EnvName:
    DB_USER_NAME = "application_{}auth_authKey"
    DB_PASSWORD = "application_{}auth_authPwd"
    DB_AUTH_TYPE = "application_{}auth_authType"


class ParamField(str, Enum):
    STATUS = "status"
    COPY_ID_LIST = 'copyIdList'
    AUTH_CUSTOM_PARAMS = "authCustomParams"
    APP_ENV = "appEnv"
    JOB_INFO = "jobInfo"
    JOB_PRIORITY = "jobPriority"
    JOB_TYPE = "jobType"
    HOSTS = "hosts"
    POLICY = "policy"
    SUB_JOB_ID = "subJobId"
    BASE_COPY_ID = "baseCopyId"
    CLUSTER_INFO = "cluster_info"
    CUSTOM_PARAMS = "customParams"
    DEPENDENCIES = "dependencies"
    CLUSTER_UUID = "clusterUuid"
    ORIGIN_PROTECT_ENV = "originProtectEnv"
    ORIGIN_PROTECTED_ENV = "originProtectedEnv"
    PROTECT_DATABASE = "database"
    SYSTEM_DB_USER = "systemDbUser"
    SYSTEM_DB_PASSWORD = "systemDbPassword"
    SYSTEM_ID = "systemId"
    SYSTEM_DB_PORT = "systemDbPort"
    EXTEND_INFO = "extendInfo"
    CLUSTER_TYPE = "clusterType"
    JOB = "job"
    JOB_PARAM = "jobParam"
    BACKUP_TYPE = "backupType"
    PROTECT_OBJECT = "protectObject"
    SUB_TYPE = "subType"
    PROTECT_ENV = "protectEnv"
    PROTECTED_ENV = "protectedEnv"
    ENDPOINT = "endpoint"
    HOSTURL = "hostUrl"
    AGENTURL = "agentHost"
    SERVICE_PORT = "servicePort"
    PORT = "port"
    AUTH = "auth"
    AUTH_KEY = "authKey"
    AUTH_PWD = "authPwd"
    INSTANCE_PORT = "instancePort"
    CLUSTER_NODES = "clusterNodes"
    NODES = "nodes"
    REPOSITORIES = "repositories"
    REPOSITORY_TYPE = "repositoryType"
    PATH = "path"
    BACK_JOB_RESULT = "backupJobResult"
    COPY = "copy"
    COPY_ID = "copy_id"
    TIMESTAMP = "timestamp"
    BACKUP_TIME = "backupTime"
    BACKUP_LIMIT = "BackupLimit"
    NAME = "name"
    COPIES = "copies"
    AGENTS = "agents"
    TARGET_OBJECT = "targetObject"
    RESTORE_JOB = "restoreType"
    RESTORE_JOB_RESULT = "restoreJobResult"
    PROGRESS = "progress"
    TASK_ID = "taskId"
    SUB_TASK_ID = "subTaskId"
    TASK_STATUS = "taskStatus"
    DATA_SIZE = "dataSize"
    SPEED = "speed"
    LOG_DETAIL = "logDetail"
    APPLICATION = "application"
    LIVE_MOUNT = "liveMount"
    TARGET_ENV = "targetEnv"
    ROLE = "role"
    ADVANCE_PARAMS = "advanceParams"
    SUB_JOB = "subJob"
    JOB_NAME = "jobName"
    RESTORE_TIMESTAMP = "restoreTimestamp"
    TYPE = "type"
    TYPES = "types"
    REMOTE_HOST = "remoteHost"
    REMOTE_PATH = "remotePath"
    MOUNT_JOB_ID = "mountJobId"
    LOG_INFO = "logInfo"
    LOG_INFO_PARAM = "logInfoParam"
    LOG_LEVEL = "logLevel"
    BACKUP_TASK_SLA = "backupTask_sla"
    POLICY_LIST = "policy_list"
    EXT_PARAMETERS = "ext_parameters"
    CHANNEL_NUMBER = "channel_number"
    NEW_DATABASE_NAME = "newDatabaseName"
    VERSION = "version"
    DATA_DIR = "dataDir"
    AVERAGE_SPEED = "averageSpeed"
    LOG_FLAG = "logFlag"
    LOG_FLAG_START_TIME = "logFlagStartTime"
    RESTORE_COPY_ID = "restoreCopyId"
    SERVICE_NAME = "serviceName"
    ID = "id"
    IP = "ip"
    BACKUP_HOST_SN = "backupHostSN"
    FIRST_FULL_BACKUP_TIME = "firstFullBackupTime"
    NEXT_CAUSE_PARAM = "next_cause_param"
    HOST_AGENT = "hostAgent"
    BACKUP_ID = "backupId"
    MULTI_TENANT_SYSTEM = "multiTenantSystem"
    IS_LOCAL = "isLocal"
    AGENT_IP_LIST = "agentIpList"
    AGENT_UID = "agentUuid"
    JOB_ID = "jobId"
    FIRSTCLASSIFICATION = "firstClassification"
    HOSTSN_FILE_PATH = "/etc/HostSN/HostSN"
    SHARDING = "sharding"
    DATA_PATH = "dataPath"
    SINLE_DATA_PATH = "0"
    CLUSTER_INSTANCE_TYPE = "shardClusterType"
    CLUSTER_INSTANCE_NAME = "clusterInstanceName"
    PRIORITY = "priority"
    HOST = "host"
    ARBITER_ONLY = "arbiterOnly"
    CLUSTER_ID = "_id"
    INSTANCE_ID = "instanceId"
    MEMBERS = "members"
    LOG_COPY = "log"
    # 起始操作时间序号
    FIRST_LSN = "firstLsn"
    # 最后操作时间序号
    LAST_LSN = "lastLsn"
    FULL_COPY = "full"
    # 归档
    S3_ARCHIVE = "s3Archive"
    # 磁带
    TAPE_ARCHIVE = "tapeArchive"
    SHARD_CLUSTER_TYPE = "shardClusterType"
    REPLSET_GET_STATUS = "replSetGetStatus"
    GET_CMDLINE_OPTS = "getCmdLineOpts"
    STORAGE = "storage"
    DB_PATH = "dbPath"
    BIN_PATH = "binPath"
    MOBGODUMP_BIN_PATH = "mongodumpBinPath"
    FORWARD_SLASH = "/"


class TaskLabel(ReportDBLabel):
    # 生成子任务
    EXECUTE_GENERATE_SUBJOB_LABEL = "plugin_excute_generate_subjob_label"
    # "生成子任务失败。"
    GENERATE_SUBJOB_FAIL_LABEL = "plugin_generate_subjob_fail_label"
    # "生成子任务成功"
    GENERATE_SUBJOB_SUCCESS_LABEL = "plugin_generate_subjob_success_label"
    # "子任务（{0}）恢复失败。"


class ActionCode(int, Enum):
    SUCCESS = 0
    FAILED = -1
    WAITING = -2


class MongoRestoreType(str, Enum):
    SINGLE = "0"
    SHARD = "1"


class CMDResult(str, Enum):
    SUCCESS = "0"
    FAILED = "1"


class BackupType(str, Enum):
    INSTANCE = 'instance'
    REPLSET = "replset"
    SHARDING = "sharding"


class MongoDBCode(Enum):
    """
    返回给框架的code
    """
    SUCCESS = 0
    FAILED = 200


class RoleMode(int, Enum):
    MASTER_MODE = 1
    SLAVE_MODE = 2
    ARBITRATE_MODE = 7
    NO_MODE = 0


class PexpectResult:
    LOGIN = ["Enter password for mongo user:", "mongo user:", "Password", "Password:", "口令", "密码"]
    LOGIN_DATABASE_SUCCESS = ["mongo>"]


class MongoRoles(str, Enum):
    CONFIG_SVR = "configsvr"
    SHARDS_VR = "shardsvr"
    PARSED = "parsed"
    SHARDING = "sharding"
    REPLICATION = "replication"
    SINGLE = "single"
    SINGLE_NODE_REPL = "single_node_repl"
    MONGOS = "mongos"
    CONFIG = "config"
    SHARD = "shard"
    CLUSTER_ROLE = "clusterRole"
    CONFIG_DB = "configDB"
    LOW_CONFIG_DB = "configdb"


class MongoRolesStatus(int, Enum):
    NONE_TYPE = 0
    PRIMARY = 1
    SECENDARY = 2
    STARTUP = 4
    STARTUP2 = 5
    OTHER = 6
    ARBITER = 7


class MongoDBCopyDataRet:
    SUCCESS = 1
    RUNNING = 2
    FAILED = 3


class MongoSubJob(str, Enum):
    PRE_CHECK = "pre_check"
    SNAPSHOT = "snapshot"
    OPLOG = "oplog"
    REPORT_COPY_INFO = "report_copy_info"
    STOP_BALANCE = "stop_balance"
    RESUME_BALANCE = "resume_balance"
    PRE_RESTORE = "pre_restore"
    RESTORE_SINGLE_NODE = "restore_single_node"
    RESTORE_REPLSET_NODE = "restore_replset_node"
    RESTORE_REPLSET_INIT = "restore_replset_init"
    RESTORE_CONFIG_NODE = "restore_config_node"
    RESTORE_CONFIG_INIT = "restore_config_init"
    RESTORE_CLUSTER_NODE = "restore_cluster_node"
    RESTORE_CLUSTER_INIT = "restore_cluster_init"
    EXECUTE_LOG_RESTORE = "execute_log_restore"
    EXECUTE_CONFIG_LOG_RESTORE = "execute_config_log_restore"
    EXECUTE_SHARD_LOG_RESTORE = "execute_shard_log_restore"
    RESTORE_MONGOS_NODES = "restore_mongos_nodes"


class MongoTool(str, Enum):
    MONGODUMP = "mongodump"
    MONGORESTORE = "mongorestore"
    MONGO = "mongo"
    MONGOD = "mongod"
    MONGOS = "mongos"


class DefaultValue(Enum):
    SNAP_SIZE = 500.0
    SNAP_PER = 0.1
    SNAP_MIN_PER = 10
    SNAP_MAX_PER = 50
    SNAP_UNIT = "m"
    LOCAL_DB = "local"
    OPLOG_COLLECTION = "oplog.rs"
    BIND_ALL_IP = '0.0.0.0'


ROLE_MAP = {
    "1": "single",
    "2": "repl-P",
    "3": "repl_S",
    "4": "shard-P",
    "5": "shard-S",
    "7": "arbitrate"
}

CLUSTER_ROLE_MAP = {
    "single": "3",
    "single_node_repl": "3",
    "replication": "1",
    "shard": "2",
    "mongos": "2",
    "config": "2",
}

INSTANCE_DIRECTORY = "instance_directory"
LOG_DIRECTORY = "log_directory"
SINGLE_TYPE = "3"

TMP_MOUNT_PATH = "/mnt/databackup/mongodb_tmp/"

TMP_CONF_PATH = "/mnt/databackup/mongodb_restore_tmp/"

TMP_MONGODB_SOCK = "/tmp/mongodb-{}.sock"

REPORT_INTERVAL_SEC = 30

WHITE_LIST = []

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
from dataclasses import dataclass
from pydantic import BaseModel, Field

from common.const import CopyDataTypeEnum, RpcParamKey
from common.env_common import get_install_head_path


class TdsqlConstant:
    pass


class EnvName:
    IAM_USERNAME = ""
    IAM_PASSWORD = ""
    IAM_BELONG = ""


class EnvNameValue:
    IAM_USERNAME_BACKUP = "job_protectEnv_auth_authKey"
    IAM_PASSWORD_BACKUP = "job_protectEnv_auth_authPwd"
    IAM_USERNAME_RESTORE = "job_targetEnv_auth_authKey"
    IAM_PASSWORD_RESTORE = "job_targetEnv_auth_authPwd"
    IAM_OSADAUTHPORT = "job_extendInfo_OSADAuthPort"
    IAM_USERNAME_LIVEMOUNT = "job_copy_0_protectEnv_auth_authKey"
    IAM_PASSWORD_LIVEMOUNT = "job_copy_0_protectEnv_auth_authPwd"


class TdsqlSubJobName:
    SUB_OSS = "sub_oss"
    SUB_EXEC = "sub_exec"
    SUB_FLUSH_LOG = "sub_flush_log"
    SUB_RM_BINLOG = "sub_rm_binlog"
    SUB_BINLOG = "sub_binlog"
    SUB_GROUP_MOUNT_BIND = "sub_group_mount_bind"
    SUB_GROUP_BINLOG = "sub_group_binlog"
    SUB_GROUP_EXEC = "sub_group_exec"


class TdsqlBackupStatus:
    SUCCEED = "Completed"
    RUNNING = "Running"
    FAILED = "Failed"


class TdsqlRestoreStatus:
    SUCCEED = "Completed"
    RUNNING = "Running"
    FAILED = "Failed"


class TdsqlPath:
    TDSQL_FILESYSTEM_MOUNT_PATH = "/mnt/databackup/"
    TDSQL_LINK_PATH = f"{get_install_head_path()}/DataBackup/"


class LastCopyType:
    last_copy_type_dict = {
        1: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY],
        2: [RpcParamKey.FULL_COPY],
        3: [RpcParamKey.LOG_COPY],
        4: [RpcParamKey.FULL_COPY, RpcParamKey.INCREMENT_COPY, RpcParamKey.LOG_COPY]
    }


class MountType:
    FUSE = 'fuse'


class ErrorCode(int, Enum):
    ERROR_INCREMENT_TO_FULL = 1577209901
    ERROR_DIFFERENT_VERSION = 1577209971
    ERROR_DIFFERENT_TOPO = 1577209972
    ERROR_DIFFERENT_USER = 1577209973
    ERROR_MOUNT_PATH = 1577209974
    ERR_DATABASE_STATUS = 1577210000
    ERROR_USER_NOT_EXIST_CLUSTER = 0x5E0250D7
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
    SYSTEM_ID_NOT_EXIST = 0
    DB_NODES_NOT_FULL_MATCH = 1677931026
    DB_ENV_ERROR = 2
    ACCESS_DB_ERROR = 3
    TENANT_MODEL_NOT_SUPPORT = 4
    DB_NOT_EXIST = 5
    # 不能修改数据库为新的数据库
    ERR_RENAME_DATABASE = 1577213482
    # 系统异常
    SYSTEM_ERROR = 1677929221
    # 不是所有数据库的服务都运行0x5E02502D
    NOT_ALL_DB_SERVICE_RUNNING = 9
    # 增量转全量
    INC_TO_FULL = 0x5E02502D
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
    # 输入参数错误
    ERR_INPUT_STRING = 1677934101
    # 主机选择有误
    ERR_CHOSEN_HOSTS = 1577209936
    # region 不一致
    ERR_REGION = 1577210146
    # 资源接入异常 0x5E025120
    ERR_RESOURCE_ABILITY = 1577210144
    # 原因：不存在全量/增量备份副本或不存在日志备份副本。建议：请对该备件集执行全量/增量备份后重试。
    NOT_EXIT_WAL_BACKUP_FILE_AND_SNAPSHOT_BACKUP = 1577209911
    # 执行备份/恢复命令失败
    EXEC_BACKUP_RECOVER_CMD_FAIL = 1577209989
    # 挂载端口被占用
    ERR_LIVE_MOUNT_PORT_USED = 1677873226
    # tdsqlsys_repl密码错误
    ERR_TDSQLSYS_REPL_PWD = 1677873246


class NodeInfo(BaseModel):
    # 以下由pm传递
    # ip、port 形式："172.0.0.1:80"，唯一标记
    node_host: str = Field(description="node ip and port", alias="nodeHost")
    set_id: str = Field(description="instance id", alias="setId")
    agent_uuid: str = Field(description="agent uuid", alias="agentUuid")
    priority: int = Field(description="节点优先级", alia="priority")
    # 以下由备份执行中写入
    is_exec_node: int = Field(default=0, description="是否有备份权限，0否1是", alias="isExecNode")
    last_modified_time: str = Field(default="", description="上次修改时间", alias="lastModTime")
    ever_backup: int = Field(default=0, description="0无1有", alias="everBackup")
    is_completed: int = Field(default=0, description="0未1已完成2已失败", alias="isCompleted")
    # 以下由接口返回
    is_master: int = Field(default=0, description="0从1主", alias="isMaster")
    is_alive: int = Field(default=0, description="节点是否存活，0活1挂", alias="isAlive")


class BackupPath:
    BACKUP_PRE = "/data/tdsql_run"
    BACK_TOOLS_POST = "xtrabackup/innobackupex"


class TdsqlRestoreSubJobName:
    SUB_EXEC_PREPARE = "sub_job_prepare"
    SUB_CHECK_MYSQL_VERSION = "sub_job_check"
    SUB_EXEC_RESTORE = "sub_job_exec"
    SUB_EXEC_CREATE_INSTANCE = "sub_job_cre_instance"


class TdsqlClusterGroupRestoreSubJobName:
    SUB_EXEC_MOUNT = "sub_exec_mount"
    SUB_CHECK_MYSQL_VERSION = "sub_check_mysql_version"
    SUB_CHECK_HOST_AGENT = "sub_check_host_agent"
    SUB_EXEC_RESTORE = "sub_exec_restore"
    SUB_EXEC_UMOUNT = "sub_exec_umount"


class TdsqlJsonConst:
    """
    Tdsql的json字段名
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
    TDSQL = "Tdsql"
    CLUSTERINFO = "clusterInfo"
    GROUP = "group"
    OSUSER = "osUser"
    NODETYPE = "nodeType"
    PARENTUUID = "parentUuid"
    MYSQLNODES = "mysqlNodes"
    JOBINFO = "jobInfo"
    GTM = "gtm"


@dataclass
class JobInfo:
    """
    任务相关参数信息
    """
    copy_id: str
    job_id: str
    # 恢复或者备份
    type: str = None
    pid: str = None
    usr: str = None
    env_path: str = None
    sub_job_id: str = None
    sub_job_type: str = None
    cache_path: str = None
    meta_path: str = None
    meta_rep: dict = None
    cache_rep: dict = None
    data_reps: list = None
    metadata_destination: str = None
    backup_type: int = None
    progress: int = None
    task_status: int = None
    backup_result: int = None
    storage_id: str = None
    protect_env: dict = None
    backup_tool_type: int = None
    res_name: str = None
    nodes: list = None
    pro_obj_extend_info: dict = None
    pro_obj: dict = None
    storage_esn: set = None
    agents: list = None
    failed_agents: list = None
    cluster_agents: list = None
    host_agents: list = None
    # 此次备份是否开启源端重删
    open_source_delete: bool = False


@dataclass
class BackupParam:
    # 备份组装参数
    file_name: str
    host: str
    port: str
    backup_type: str
    backup_tools: str
    defaults_file: str
    socket: str
    user: str
    parallel: int
    use_memory: int
    target_dir: str
    base_dir: str
    mysql_version: str
    path: str = None
    url: str = None
    set_id: str = None


@dataclass
class RestoreParam:
    # 恢复组装参数
    node_ips: list
    group_id: str
    restore_time: str
    request_url: str
    env_variable: str
    job_extend_info: dict


@dataclass
class HostParam:
    url: str
    ip: str
    port: str


@dataclass
class ConnectParam:
    socket: str
    ip: str
    port: str


class TdsqlBackTypeConstant:
    FULL = "FULL"
    INCREMENTAL = "INCREMENTAL"
    INCREMENTAL_FOREVER = "INCREMENTAL_FOREVER"
    LOG = "LOG"

    def log_format(self):
        """
        功能： 日志格式化返回
        """
        return f"pid: {self.pid}, job_id: {self.job_id}, sub_job_id: {self.sub_job_id}"

    def log_usr_env_path_format(self):
        return f"usr: {self.usr}, env_path: {self.env_path}"


class ActionResponse(BaseModel):
    code: int = Field(default=0, description="执行结果")
    body_err: int = Field(None, description="错误码", alias='bodyErr')
    message: str = Field(default='', description="错误信息")
    body_err_params: list = Field(None, description="错误码具体参数", alias="bodyErrParams")


class TDSQLVersionPath:
    """
    各个服务的列表
    """
    VERSION_PATH_DICT = {"5.7.36": "percona-5.7.17", "8.0.24": "mysql-server-8.0.24"}


class ArchiveType:
    archive_array = [CopyDataTypeEnum.TAP_ARCHIVE.value, CopyDataTypeEnum.S3_ARCHIVE.value]


class MySQLVersion:
    MARIADB = "mariadb"
    MYSQL = "mysql"
    PERCONA = "percona"
    MARIADB_START = "mariadb-"
    MYSQL_START = "mysql-"
    PERCONA_START = "percona-"


class InstanceConfigInfo(BaseModel):
    """
    记录实例的配置信息
    """
    dbversion: str = Field(description="实例的数据库版本", alias="dbversion")
    machine: str = Field(description="实例的机型", alias="machine")
    cpu: str = Field(description="实例的CPU核数", alias="cpu")
    memory: str = Field(description="实例的内存大小", alias="memory")
    data_disk: str = Field(description="实例的数据磁盘大小", alias="data_disk")
    log_disk: str = Field(description="实例的日志磁盘大小", alias="log_disk")

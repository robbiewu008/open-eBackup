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
from common.env_common import get_install_head_path, adaptation_win_path


class BackupTypeEnum(int, Enum):
    # 备份类型
    FULL_BACKUP = 1
    INCRE_BACKUP = 2
    DIFF_BACKUP = 3
    LOG_BACKUP = 4


class ParamConstant:
    # 输入参数文件目录
    PARAM_FILE_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/tmp"
    # 输出结果文件目录
    RESULT_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/stmp"
    # bin目录
    BIN_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin"
    # agent id 文件所在位置
    HOST_SN_FILE_PATH = "/etc/HostSN/HostSN"
    INPUT_FILE_PREFFIX = "rpcInput_"
    OUTPUT_FILE_PREFFIX = "rpcOutput_"

    # windows输入参数文件目录
    WINDOWS_PARAM_FILE_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/tmp"
    # windows输出结果文件目录
    WINDOWS_RESULT_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/stmp"


class RepositoryDataTypeEnum(int, Enum):
    META_REPOSITORY = 0
    DATA_REPOSITORY = 1
    CACHE_REPOSITORY = 2
    LOG_REPOSITORY = 3
    INDEX_REPOSITORY = 4
    LOG_META_REPOSITORY = 5


class RepositoryNameEnum(str, Enum):
    META = "meta"
    DATA = "data"
    CACHE = "cache"
    LOG = "log"


class CopyDataTypeEnum(str, Enum):
    # 副本类型
    FULL_COPY = "full"
    INCREMENT_COPY = "increment"
    DIFF_COPY = "diff"
    LOG_COPY = "log"
    S3_ARCHIVE = "s3Archive"
    TAP_ARCHIVE = "tapeArchive"
    CLONE = "clone"


class SubJobStatusEnum(int, Enum):
    INITIALIZING = 0
    RUNNING = 1
    ABORTING = 2
    COMPLETED = 3
    ABORTED = 4
    ABORTED_FAILED = 5
    FAILED = 6
    FAILED_NOTRY = 7
    PARTIAL_COMPLETED = 13


class ExecuteResultEnum(int, Enum):
    SUCCESS = 0
    CONTINUE = 100
    BUSY = 101
    INTERNAL_ERROR = 200


class DeployType(int, Enum):
    INVALID_TYPE = -1
    SINGLE_TYPE = 1
    CLUSTER_TYPE = 3
    SHARDING_TYPE = 4
    DISTRIBUTED_TYPE = 5


class Indexes(int, Enum):
    BEFORE_START = -1
    START_POS = 0


class RoleType(int, Enum):
    NONE_TYPE = 0
    PRIMARY = 1
    STANDBY = 2
    SHARD = 3


class ClusterCatalogue:
    AZ_STATUS = 'AZ Status'
    HOST_STATUS = 'Host Status'
    CM_STATUS = 'Cluster Manager Status'
    ETCD_STATUS = 'ETCD Status'
    INSTANCE_STATUS = 'Instances Status in Group'
    MANAGE_IP = 'Manage IP'
    QUERY_INFO = 'Query Action Info'


class JobData:
    PID = None
    JOB_ID = None
    SUB_JOB_ID = None
    HOSTSN = None
    CMD = None


class SysData:
    SYS_STDIN = ''


class BackupLimit(Enum):
    # any cluste node can be execute
    NO_LIMIT = 0
    # only master node can be execute
    ONLY_MASTER = 1
    # only slave node can be execute
    ONLY_SLAVE = 2


class AuthType(Enum):
    NO_AUTO = 0
    OS_PASSWORD = 1
    APP_PASSWORD = 2
    LADP = 3
    AKSK = 4
    KERBEROS = 5
    TOKEN = 6
    OAUTH2 = 7
    OTHER = 8


class EnvInfoPrefix:
    AUTH_TYPE = "job_protectEnv_auth_authType"
    PASS_PREFIX = "job_protectEnv_auth_authPwd"
    EXTEND_INFO_PREFIX = "job_protectEnv_auth_extendInfo"
    OBJ_USERNAME_PREFIX = "job_protectObject_auth_authKey"
    OBJ_AUTH_TYPE = "job_protectObject_auth_authType"
    OBJ_PASS_PREFIX = "job_protectObject_auth_authPwd"
    RESTORE_AUTH_TYPE = "job_targetObject_auth_authType"
    RESTORE_USERNAME = "job_targetObject_auth_authKey"
    RESTORE_PASS = "job_targetObject_auth_authPwd"
    NODE_RESTORE_AUTH_TYPE = "job_targetEnv_nodes_0_auth_authType"
    NODE_RESTORE_USERNAME = "job_targetEnv_nodes_0_auth_authKey"
    NODE_RESTORE_PASS = "job_targetEnv_nodes_0_auth_authPwd"


class RestoreType(int, Enum):
    NORMAL_RESTORE = 1  # 普通恢复
    INSTANT_RESTORE = 2  # 即时恢复
    FINE_GRAINED_RESTORE = 3  # 细粒度恢复
    INVALID_RESTORE_TYPE = 4  # 非法类型


class RestoreTypeEnum(int, Enum):
    # 恢复类型
    FULL_RESTORE = 1
    INCRE_RESTORE = 2
    DIFF_RESTORE = 3
    LOG_RESTORE = 4


class RepoProtocalType(int, Enum):
    CIFS = 0
    NFS = 1
    S3 = 2
    BLOCK = 3
    LOCAL_DIR = 4
    TAPE = 5


class SubJobTypeEnum(int, Enum):
    PRE_SUB_JOB = 0
    GENERATE_SUB_JOB = 1
    BUSINESS_SUB_JOB = 2
    POST_SUB_JOB = 3


class SubJobPolicyEnum(int, Enum):
    ANY_NODE = 0
    LOCAL_NODE = 1
    EVERY_NODE_ONE_TIME = 2
    RETRY_OTHER_NODE_WHEN_FAILED = 3
    FIXED_NODE = 4
    EVERY_NODE_ONE_TIME_SKIP_OFFLINE = 5


class SubJobPriorityEnum(int, Enum):
    JOB_PRIORITY_1 = 1
    JOB_PRIORITY_2 = 2
    JOB_PRIORITY_3 = 3
    JOB_PRIORITY_4 = 4
    JOB_PRIORITY_5 = 5
    JOB_PRIORITY_6 = 6
    JOB_PRIORITY_7 = 7
    JOB_PRIORITY_8 = 8
    JOB_PRIORITY_9 = 9


class DBLogLevel(int, Enum):
    INFO = 1
    WARN = 2
    ERROR = 3
    SERIOUS_WARN = 4


class ReportDBLabel:
    # 子任务（{0}）备份失败。
    BACKUP_SUB_FAILED = "plugin_backup_subjob_fail_label"
    # 子任务（{0}）恢复失败。
    RESTORE_SUB_FAILED = "plugin_restore_subjob_fail_label"
    # 子任务（{0}）即时挂载失败。
    LIVE_MOUNT_FAILED = "plugin_live_mount_subjob_fail_label"
    # 执行前置任务成功。
    PRE_REQUISIT_SUCCESS = "plugin_execute_prerequisit_task_success_label"
    # 执行前置任务失败。
    PRE_REQUISIT_FAILED = "plugin_execute_prerequisit_task_fail_label"
    # 开始执行备份挂载子任务（{0}）。
    BACKUP_SUB_START_PREPARE = "plugin_start_backup_prepare_subjob_label"
    # 开始执行备份子任务（{0}）。
    BACKUP_SUB_START_COPY = "plugin_start_backup_copy_subjob_label"
    # 开始执行备份卸载子任务（{0}）。
    BACKUP_SUB_START_UMOUNT = "plugin_start_backup_umount_subjob_label"
    # 开始执行恢复挂载子任务（{0}）。
    RESTORE_SUB_START_PREPARE = "plugin_start_restore_prepare_subjob_label"
    # 开始执行恢复子任务（{0}）。
    RESTORE_SUB_START_COPY = "plugin_start_restore_copy_subjob_label"
    # 开始执行恢复卸载子任务（{0}）。
    RESTORE_SUB_START_UMOUNT = "plugin_start_restore_umount_subjob_label"
    # 子任务（{0}）执行成功。
    SUB_JOB_SUCCESS = "plugin_task_subjob_success_label"
    # 子任务（{0}）执行失败。
    SUB_JOB_FALIED = "plugin_task_subjob_fail_label"
    # 数据保护代理主机（{0}）执行后置任务（{1}）失败。
    POST_TASK_FAIL = "agent_execute_post_task_fail_label"
    # 副本校验成功。
    COPY_VERIFICATION_SUCCESS = "plugin_task_copy_verification_success_label"
    # 副本校验失败。
    COPY_VERIFICATION_FALIED = "plugin_task_copy_verification_fail_label"
    # 销毁即时挂载失败。
    CANCEL_LIVE_MOUNT_FAILED = "dme_databases_cancel_livemount_failed_label"
    # 子任务（{0}）备份成功，共备份文件数量：（{1}），共备份数据量：（{2}）。
    BACKUP_SUB_JOB_SUCCESS = "plugin_backup_subjob_success_label"


class EnumPathType(int, Enum):
    INVALID_TYPE = 0
    DIR_TYPE = 1
    FILE_TYPE = 2
    LINK_TYPE = 3


class CMDResult(str, Enum):
    SUCCESS = "0"
    FAILED = "1"
    INFORMIX_CERTIFICATE_ABOUT_TO_EXPIRE = ["147", "149"]


class CMDResultInt(int, Enum):
    SUCCESS = 0
    FAILED = 1


archive_name = ["s3Archive", "tapeArchive"]

DELETING_PATH_WHITE_LIST = [
    "/mnt/databackup/",
    f"{get_install_head_path()}/DataBackup/",
    "/mnt/"
]


class IPConstant:
    LOCAL_HOST = "127.0.0.1"


class BackupJobResult:
    # 备份任务结果
    SUCCESS = 0
    FAIL = 1


class RpcToolInterface(str, Enum):
    CREATE_RESOURCE = "CreateResource"  # 创建共享资源
    QUERY_RESOURCE = "QueryResource"  # 查询共享资源
    UPDATE_RESOURCE = "UpdateResource"  # 更新共享资源
    DELETE_RESOURCE = "DeleteResource"  # 删除共享资源
    LOCK_RESOURCE = "LockResource"  # 锁定共享资源
    UNLOCK_RESOURCE = "UnLockResource"  # 解锁共享资源
    REPORT_JOB_DETAIL = "ReportJobDetails"  # 上报任务详情
    REPORT_COPY_INFO = "ReportCopyAdditionalInfo"  # 上报副本信息
    QUERY_PREVIOUS_COPY = "QueryPreviousCopy"  # 查询最新副本
    MOUNT_REPO = "MountRepositoryByPlugin"  # 挂载文件系统
    UNMOUNT_REPO = "UnMountRepositoryByPlugin"  # 去挂载文件系统
    ADD_IP_WHITE_LIST = "AddIpWhiteList"  # 通过UBC添加IP白名单


class RpcParamKey:
    APPLICATION = "application"
    TYPES = "types"
    FULL_COPY = "full"
    LOG_COPY = "log"
    INCREMENT_COPY = "increment"
    DIFF_COPY = "diff"
    PERMANENT_INCREMENTAL_COPY = "foreverIncrement"
    COPY_ID = "copyId"
    JOB_ID = "jobId"
    INPUT_FILE_PREFFIX = "rpcInput_"
    OUTPUT_FILE_PREFFIX = "rpcOutput_"
    QUERY_PREVIOUS_CPOY = "QueryPreviousCopy"
    REPORT_COPY_INFO = 'ReportCopyAdditionalInfo'
    REPORT_JOB_DETAILS = "ReportJobDetails"
    LOCK_JOB_RESOURCE = "LockJobResource"
    # 查询脚本
    RPC_TOOL = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/rpctool.sh"
    # 输入参数文件目录
    PARAM_FILE_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/tmp"
    # 输出结果文件目录
    RESULT_PATH = f"{get_install_head_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/stmp"

    # Windows下查询脚本
    WINDOWS_RPC_TOOL = f"{adaptation_win_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/bin/rpctool.bat"
    # Window下输入参数文件目录
    WINDOWS_PARAM_FILE_PATH = f"{adaptation_win_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/tmp"
    # Window下结果文件目录
    WINDOWS_RESULT_PATH = f"{adaptation_win_path()}/DataBackup/ProtectClient/Plugins/GeneralDBPlugin/stmp"


class RepositoryName:
    DATA_REPOSITORY = 'data_repository'
    META_REPOSITORY = 'meta_repository'
    CACHE_REPOSITORY = 'cache_repository'
    LOG_REPOSITORY = 'log_repository'


class PathConstant:
    HOST_SN_FILE_PATH = "/etc/HostSN/HostSN"
    WINDOWS_HOST_SN_FILE_PATH = f'{adaptation_win_path()}/DataBackup/ProtectClient/ProtectClient-E/conf/HostSN'
    XBSA_PATH = f'{get_install_head_path()}/DataBackup/ProtectClient/interfaces/xbsa'
    FILE_CLIENT_PATH = f'{get_install_head_path()}/DataBackup/FileClient/bin/file_admin_client'


class JsonProperty:
    JOB = "job"
    PROTECTENV = "protectEnv"
    APPENV = "appEnv"
    TARGETENV = "targetEnv"
    EXTENDINFO = "extendInfo"
    CLUSTERTYPE = "clusterType"


class FilePath:
    # 路径黑名单
    PATH_BLACK_LIST = r"^/$|^/bin$|^/bin/.*|^/boot$|^/boot/.*|^/dev$|^/dev/.*|^/etc$|^/etc/.*|" \
                      "^/lib$|^/lib/.*|^/lib64$|^/lib64/.*|^/lost+found$|^/lost+found/.*|^/media$|^/media/.*|" \
                      "^/proc$|^/proc/.*|^/root$|^/run$|" \
                      "^/sbin$|^/sbin/.*|^/selinux$|^/selinux/.*|^/sys$|^/sys/.*|" \
                      "^/usr$|^/usr/bin$|^/usr/include$|^/usr/lib$|^/usr/local$|" \
                      "^/usr/local/bin$|^/usr/local/include$|^/usr/local/sbin$|^/usr/local/share$|" \
                      "^/usr/sbin$|^/usr/share$|^/usr/src$|^/var$"
    # 路径白名单
    PATH_WHITE_LIST = r"^{}/DataBackup|^/mnt/databackup|^/mnt|^/srv|^/home|^/opt|^/data".format(get_install_head_path())

    # 读取文件黑名单
    READ_BLACK_LIST = r"^/dev/random$|^/dev/urandom$"


class Encoding:
    INTERNAL_ENCODING = "utf-8"


class Progress:
    START = 0
    RUNNING = 50
    END = 100

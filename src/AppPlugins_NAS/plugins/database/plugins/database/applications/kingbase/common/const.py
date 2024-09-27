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

import pexpect


class KbConst:
    # recovery_target_time时间格式
    RECOVERY_TARGET_TIME_FORMATTER = "%Y-%m-%d %H:%M:%S"

    QUERY_PROGRESS_TIME = 5
    PROGRESS_FIVE = 5
    PROGRESS_FIFTY = 50
    WAIT_SIX_SECONDS = 60
    PROGRESS_NINETY = 90
    PROGRESS_ONE_HUNDRED = 100
    WAIT_TEN_SECONDS = 10
    WAIT_EIGHTY_SECONDS = 80
    KINGBASE_CONF_FILE_NAME = "kingbase.conf"
    CLUSTER_CONF_FILE_NAME = "es_rep.conf"
    RECOVERY_SIGNAL_FILE_NAME = "recovery.signal"
    # 数据库data目录恢复前需删除文件名称
    DELETE_FILE_NAMES_OF_DATA_DIR = ("kingbase.pid", "kingbase.opts", "backup_label.old")
    # 重做集群备库前需删除的配置参数
    DELETE_PARAMETER_OF_STANDBY = ("restore_command", "recovery_target_time",
                                   "recovery_target_action", "recovery_end_command")
    KINGBASE_HBA_CONF_FILE = "sys_hba.conf"
    KINGBASE_AUTO_CONF_FILE = "kingbase.auto.conf"
    # 恢复前需要备份的配置文件列表
    NEED_BAK_CFG_FILES = (KINGBASE_CONF_FILE_NAME, KINGBASE_HBA_CONF_FILE, KINGBASE_AUTO_CONF_FILE,
                          CLUSTER_CONF_FILE_NAME)
    # 恢复成功后需要删除备份的配置文件
    DELETE_FILE_NAMES_OF_BAK_CONF = ("kingbase.auto.conf.bak", "kingbase.conf.bak", "sys_hba.conf.bak")
    # 路径黑名单
    PATH_BLACK_LIST = r"^/$|^/bin$|^/bin/.*|^/boot$|^/boot/.*|^/dev$|^/dev/.*|^/etc$|^/etc/.*|" \
                      "^/lib$|^/lib/.*|^/lib64$|^/lib64/.*|^/lost+found$|^/lost+found/.*|^/media$|^/media/.*|" \
                      "^/proc$|^/proc/.*|^/root$|^/run$|" \
                      "^/sbin$|^/sbin/.*|^/selinux$|^/selinux/.*|^/srv$|^/srv/.*|^/sys$|^/sys/.*|" \
                      "^/usr$|^/usr/bin$|^/usr/include$|^/usr/lib$|^/usr/local$|" \
                      "^/usr/local/bin$|^/usr/local/include$|^/usr/local/sbin$|^/usr/local/share$|" \
                      "^/usr/sbin$|^/usr/share$|^/usr/src$|^/var$"
    # 路径白名单
    PATH_WHITE_LIST = r"^/mnt/databackup/"
    # 数据库自动WAL检查点之间的最长时间
    CHECK_POINT_TIME_OUT = 86400
    # 备份停止命令的最长执行时间
    STOP_PG_BACKUP_TIME_OUT = 300
    # 网络接口信息AddressFamily.AF_INET为2表示是IP地址信息
    ADDRESS_FAMILY_AF_INET = 2
    # 非恢复后首次全量备份
    NOT_THE_FIRST_BACKUP_AFTER_RESTORE = '0'
    # 单次日志备份最大文件数量
    MAX_FILE_NUMBER_OF_LOG_BACKUP = 1000
    # king_base实例类型
    KINGBASE_INSTANCE_TYPE = "KingBaseInstance"
    # data仓挂载目录
    MOUNT_POINT_PREFIX = "/mnt/databackup/kingbase"
    # backup目录名
    BIN_DIR_NAME = "bin"
    # kingbase数据存放目录：安装目录/data
    DATA_DIR_NAME = "data"
    # 副本目录名：{_repo_path}/backup
    BACKUP_DIR_NAME = "backup"
    # 归档日志目录名：{_repo_path}/archive
    ARCHIVE_DIR_NAME = "archive"
    # kingbase目录名
    KINGBASE_DIR_NAME = "kingbase"
    # PostgreSQL 流复制和故障转移工具
    REPMGR = "repmgr"
    # sys_rman 备份恢复工具
    SYS_RMAN_NAME = "sys_rman"
    # sys_ctl
    SYS_CTL = "sys_ctl"
    # ksql
    KSQL = "ksql"
    SYSRMAN_CONF_FILE_NAME = "sys_rman.conf"
    SYS_BACKUP_CONF_FILE_NAME = "sys_backup.conf"
    ARCHIVE_INFO_FILE_NAME = "archive.info"
    ARCHIVE_INFO_COPY_FILE_NAME = "archive.info.copy"
    BACKUP_INFO_FILE_NAME = "backup.info"
    BACKUP_INFO_COPY_FILE_NAME = "backup.info.copy"
    MAX_PARALLEL_PROCESS = 999
    MIN_PARALLEL_PROCESS = 1
    IS_NEW_COPY = "Y"


class CmdRetCode(str, Enum):
    EXEC_SUCCESS = "0"
    OP_NOT_PERMITTED = "1"
    NO_SUCH_FILE = "2"
    NO_SUCH_PROCESS = "3"
    EXEC_ERROR = "4"
    CONFIG_ERROR = "5"


class BodyErrCode(Enum):
    """
    返回给框架的错误码
    """
    PLUGIN_CANNOT_BACKUP_ERR = 0x64032B0C   # 插件无法执行该备份任务
    INC_TO_FULL_ERR = 0x5E02502D
    INVALID_PARAMETER_ERR = 0x5F025102
    # 数据库未开启归档模式
    ARCHIVE_MODE_ENABLED = 0x5E0250C3
    # 原因：不存在全量/增量备份副本或不存在日志备份副本。建议：请对该备件集执行全量/增量备份后重试。
    NOT_EXIT_WAL_BACKUP_FILE_AND_SNAPSHOT_BACKUP = 0x5E025037
    # 原因：数据库已执行过恢复。建议：请重新执行全量备份后再进行日志备份。
    ERR_RESTORED = 0x5E0250F5


class PexpectResult:
    OS_LOGIN_RESULT = [pexpect.TIMEOUT, pexpect.EOF, "登录", "Last login"]
    DB_LOGIN_PASSWORD = [pexpect.TIMEOUT, pexpect.EOF, "Password", "Password:", "口令", "密码"]
    HTML_RESULT = [pexpect.TIMEOUT, pexpect.EOF, "<p>"]
    LOGIN_DATABASE_SUCCESS = [pexpect.TIMEOUT, pexpect.EOF, "=#"]
    EXECUTE_CMD_RESULT = [pexpect.TIMEOUT, pexpect.EOF, "$"]


class BackupStatus:
    COMPLETED = 1
    RUNNING = 2
    FAILED = 3


class DirAndFileNameConst:
    TABLE_SPACE_INFO_FILE = "KB_TABLE_SPACE.info"
    TABLE_SPACE_INFO_DIR = "kb_table_space"
    COPY_FILE_INFO = "CopyInfoFile.info"


class TableSpaceKey:
    NAME_CN = "名称"
    NAME_EN = "Name"
    LOCATION_CN = "所在地"
    LOCATION_EN = "Location"


class BackupProgressEnum(str, Enum):
    PRE_TASK_PROGRESS = "BackupPrerequisiteProgress"
    BACKUP_PROGRESS = "BackupProgress"
    POST_TASK_PROGRESS = "BackupPostJobProgress"
    STOP_TASK_PROGRESS = "AbortJobProgress"


class ConfigKeyStatus:
    """配置文件中key的状态
    """
    # 被注释
    ANNOTATED = "annotated"
    # 已配置
    CONFIGURED = "configured"
    # 不存在
    NOT_EXIST = "not_exist"


class DatabaseMode:
    PG = "pg"
    ORACLE = "oracle"


class BackupType:
    FULL = "full"
    INCR = "incr"

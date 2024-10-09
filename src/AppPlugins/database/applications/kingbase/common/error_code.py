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

class ErrorCode:
    # 校验成功
    SUCCESS = 0

    # 用户不存在
    USER_IS_NOT_EXIST = 1577209963

    # 连接kingbase失败
    CHECK_CONNECTIVITY_FAILED = 1577209942

    # IP地址、端口、用户名或密码错误
    AUTH_INFO_INCORRECT = 1577213583

    # 用户名或密码错误
    LOGIN_FAILED = 1677929488

    # 校验集群失败
    CHECK_CLUSTER_FAILED = 1577209956

    # 获取集群节点失败
    GET_CLUSTER_NODE_FAILED = 1577209966

    # 获取版本信息失败
    GET_VERSION_FAILED = 1577209944

    # 客户端路径不存在
    CLIENT_PATH_IS_NOT_EXIST = 1577209968

    # 执行恢复前数据库实例正在运行
    DB_RUNNING_ERR_BEFORE_RESTORE = 1577210053

    # 目标实例数据目录空间不足
    NO_ENOUGH_SPACE_BEFORE_RESTORE = 1577210055

    # 副本的OS用户和目标实例的OS用户不一致
    OS_USER_NOT_EQUAL_BEFORE_RESTORE = 1577210057

    # 目标实例数据目录不可读写
    TARGET_NOT_RW_BEFORE_RESTORE = 1577210058

    # 执行启动数据库命令失败
    EXEC_START_DB_CMD_FAILED = 1577210060

    # 恢复后数据库状态异常
    DB_STATUS_ERR_AFTER_RESTORE = 1577210061

    # 恢复表空间失败
    RESTORE_TABLE_SPACE_FAILED = 1577213466

    # 恢复时，表空间目录与原位置表空间目录权限不一致
    TB_DIR_IS_NOT_EXIST_OR_PERMISSION_ERROR = 1577213467

    # 备份表空间失败
    BACKUP_TABLE_SPACE_FAILED = 1577213468

    # 归档模式配置错误
    ARCHIVE_MODE_CONFIG_ERROR = 1577213470

    # 备份失败
    BACKUP_FAILED = 1577213471

    # 访问数据库的操作系统用户名有误
    OS_USERNAME_ERROR = 1577209868

    # 数据保护代理内部错误
    AGENT_INTERNAL_ERROR = 1577209867

    # 当前集群实例的主备信息与前一个全量副本的主备信息不一致，请先进行全量备份后，再执行日志备份
    CLUSTER_ROLE_INFO_HAS_CHANGED = 1577209939

    # 日志副本不连续，请执行全量备份后重试
    LOG_INCONSISTENT = 1577213480

    # 目标位置配置文件中设置的max_connections值必须大于等于原位置的max_connections的值
    TARGET_MAX_CONNECTIONS_MUST_GREATER_THAN_OR_EQUAL_TO_ORIGINAL = 1577213503

    # 备份工具返回失败
    BACKUP_TOOL_FAILED = 1577213505

    # 归档日志目录不存在
    ARCHIVE_LOG_PATH_IS_NOT_EXIST = 1677931381

    # 挂载目录不存在
    MOUNT_FAIL = 1677931382


class BodyErr:
    """
    返回给框架的错误码
    """
    SUCCESS = 0
    ERR_PLUGIN_CANNOT_BACKUP = 0x64032B0C
    ERR_INC_TO_FULL = 0x5E02502D
    ERROR_COMMON_INVALID_PARAMETER = 0x5F025102
    ERROR_FILE_NOT_EXIST = 0x5E025078
    ERROR_AGENT_DIR_NOT_EXIST = 0x5E02503F
    ERROR_MOUNT_PATH = 0x5E025076

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


class BodyErr:
    """
    返回给框架的错误码
    """
    ERR_PLUGIN_CANNOT_BACKUP = 0x64032B0C
    ERR_INC_TO_FULL = 0x5E02502D
    ERR_LOG_TO_FULL = 0x5E025E4D
    ERROR_COMMON_INVALID_PARAMETER = 0x5F025102
    ERROR_FILE_NOT_EXIST = 0x5E025078
    ERROR_AGENT_DIR_NOT_EXIST = 0x5E02503F
    ERROR_MOUNT_PATH = 0x5E025076
    ERROR_USER_NOT_EXIST = 0x5E02507A
    ERROR_USER_NOT_EXIST_CLUSTER = 0x5E0250D7

    # 集群节点不属于同一集群
    ERROR_NODE_IN_CLUSTER = 0x5E02507D
    # 未选择所有节点
    ERROR_NOT_ALL_NODES = 0x5E02507F

    # 集群类型与应用群类型不匹配
    ERROR_CLUSTER_TYPE_FAILED = 0x5E02508B

    # 集群的环境变量文件路径不存在或集群用户无权限访问
    ERROR_ENVPATH_NOT_EXIST = 0x5E0250C4

    # 在执行数据库应用资源注册操作时，由于数据库操作系统用户名不一致、环境变量文件路径不匹配或者所选节点未检测到数据库应用
    ERROR_DB_REGISTER_INFO = 0x5E025095

    # 未开启归档
    ERROR_ARCHIVE_MODE = 0x5E0250C3

    # 未配置归档文件夹或配置错误
    ERROR_ARCHIVE_DIR = 0x64033375

    # 选择的集群信息与需要的集群信息不匹配
    ERROR_CLUSTER_NODES_INCONSISTENT = 0x640333A3


class OpenGaussErrorCode:
    # 连接服务失败
    CONNECT_OPENGAUSS_SERVER_FAILED = 1577209942

    # 执行命令失败
    EXECUTE_OPENGAUSS_CMD_FAILED = 1577209943

    # 获取数据库列表失败
    GET_DATABASES_FAILED = 1577209946

    # 校验集群失败
    CHECK_CLUSTER_FAILED = 1577209956

    # 配置参数错误 操作失败
    VAILD_PARAM_EXECUTE = 1577209947

    # 获取版本信息失败
    GET_DATABASE_VERSION_FAILED = 1577209944

    OPERATION_FAILED = 1677929219

    # 版本不同
    ERROR_DIFFERENT_VERSION = 1577209971

    # 拓扑结构不同
    ERROR_DIFFERENT_TOPO = 1577209972

    # 用户名有误
    ERROR_DIFFERENT_USER = 1577209973

    # 挂载路径有误
    ERROR_MOUNT_PATH = 1577209974

    # 数据库状态异常
    ERR_DATABASE_STATUS = 1577210002

    # enable_cbm_tracking配置错误
    ERR_ENABLE_CBM_TRACKING = 0x5E02509E

    # 会话超时异常
    ERR_SESSION_TIMEOUT = 0x5E02509C

    # 数据复制模式为非异步模式集群在降级状态下不允许实例备份
    ERR_NO_ASYSC_DEGRADE = 0x5E025E12

    # 执行openGauss恢复操作时，由于数据库启动命令执行失败，操作失败。
    ERR_DATABASE_START = 1577212934

    # 未开启归档
    ERROR_ARCHIVE_MODE = 1577210051

    # 未配置归档文件夹或配置错误
    ERROR_ARCHIVE_DIR = 1677931381

    # 无错误码
    NO_ERR = None


class OpenGaussCode:
    """
    返回给框架的code
    """
    SUCCESS = 0
    FAILED = 200


class NormalErr(int, Enum):
    NO_ERR = 0
    FALSE = -1
    WAITING = -2

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
from enum import Enum


class ResourceAlarmConst(object):
    # 告警标记key
    LINK_STATUS_ALARM_FLAG_KEY = "resource:{resource_id}:alarm:linkstatus:flags"

    # 告警ID key
    LINK_STATUS_ALARM_ID_KEY = "resource:{resource_id}:alarm:linkstatus:id"

    # 告警锁key
    LINK_STATUS_ALARM_LOCK_KEY = "resource:{resource_id}:alarm:linkstatus:lock"


class GaussDBVersionEnum(str, Enum):
    """GaussDB version 类别"""
    GAUSS_DB_T = "GaussDB T"


class ResourceABTypeEnum(str, Enum):
    GAUSS_DB_CLIENT_TYPE = 20
    GAUSS_DB_ENGINE_TYPE = 1
    GAUSS_DB_INSTANCE_TYPE = 1000


class Cluster(str, Enum):
    MAX_CLUSTER_NAME_LEN = 128


class HostMigrateConstants:
    # 任务类型
    PROTECT_AGENT = "ProtectAgent"

    # 复制集群
    REPLICATION_CLUSTER_TYPE = 0

    # 迁移调度轮询时间
    INTERVAL = 60

    # 主机迁移任务时间周期
    ONE_HOUR = 3600

    # 任务异常2天强制结束
    TWO_DAY = 172800

    # token 过期时间3天
    THREE_DAY = 259200

    # 公共请求头
    COMMON_HEADERS = {'Content-Type': 'application/json'}

    # 正常的集群状态
    TARGET_CLUSTER_STATUS_NORMAL = 27

    # 异常的集群状态
    TARGET_CLUSTER_STATUS_ABNORMAL = 28

    # 访问目标集群端口
    TARGET_ClUSTER_PORT = 25081


class HostMigrateJobSteps(object):
    # 准备创建迁移主机迁移任务
    HOST_MIGRATE_READY_CREATE_CREATE = "host_migrate_ready_create_label"

    # 迁移任务排队中
    HOST_MIGRATE_PENDING = "host_migrate_pending_label"

    # 迁移任务开始
    HOST_MIGRATE_POST_TASK_FINISH = "host_migrate_post_task_label"

    # 迁移任务完成
    HOST_MIGRATE_TASK_FINISH = "host_migrate_task_finish_label"

    # 删除本地主机资源
    HOST_MIGRATE_CLEAR_RESOURCE = "host_migrate_clear_resource_label"

    # 删除本地主机资源失败
    HOST_MIGRATE_CLEAR_FAILED_RESOURCE = "host_migrate_clear_failed_resource_label"

    # 迁移任务成功
    HOST_MIGRATE_TASK_SUCCESS = "host_migrate_task_success_label"

    # 迁移任务失败
    HOST_MIGRATE_FAILED = "host_migrate_failed_label"

    # 准备创建迁移主机任务失败
    HOST_MIGRATE_READY_CREATE_FAILED = "host_migrate_ready_create_failed_label"


class EnvironmentRemoveConstants:
    # 环境删除的redis key前缀
    DELETE_ENV_KEY_PREFIX = 'DELETE_ENV_FLAG:ENV_ID:'

    # 锁最大持有时间(单位：秒)
    LOCK_TIME_OUT = 120

    # 最长等待获取锁时间(单位：秒)
    LOCK_WAIT_TIME_OUT = 3

    # 资源状态更新的最长等待获取锁时间(单位：秒)
    LOCK_WAIT_TIME_OUT_RESOURCE_STATUS_UPDATE = 60


class VMwareRetryConstants:
    # 重试次数
    RETRY_TIMES = 3

    # 下次重试等待时间
    WAIT_TIME = 5

    # 等待时间系数(如2则下次等待时间翻倍)
    BACKOFF = 1


class ResourceConstants:
    # citation前缀
    CITATION = '$citations'

    # citation分隔符
    CITATION_SEPERATOR = '_'

    # children
    CHILDREN = 'children'

    # 资源名称正则表达式
    RESOURCE_NAME_PATTERN = "^[\u4e00-\u9fa5a-zA-Z_]{1}[\u4e00-\u9fa5a-zA-Z_0-9-]{0,63}$"


class VMWareCertConstants:
    # ca证书文件名
    CERT_NAME = "cert_name"
    # 吊销列表文件名
    CRL_NAME = "crl_name"
    # ca证书文件大小
    CERT_SIZE = "cert_size"
    # 吊销列表文件大小
    CRL_SIZE = "crl_size"
    # ca证书
    CERTIFICATION = "certification"
    # 吊销列表
    REVOCATION_LIST = "revocation_list"
    # TLS兼容性开关
    TLS_COMPATIBLE = "tls_compatible"
    # TLS安全算法套件
    CIPHERS = "DHE-RSA-AES128-GCM-SHA256:" \
              "DHE-RSA-AES256-GCM-SHA384:" \
              "DHE-DSS-AES128-GCM-SHA256:" \
              "DHE-DSS-AES256-GCM-SHA384:" \
              "ECDHE-ECDSA-AES128-GCM-SHA256:" \
              "ECDHE-ECDSA-AES256-GCM-SHA384:" \
              "ECDHE-RSA-AES128-GCM-SHA256:" \
              "ECDHE-RSA-AES256-GCM-SHA384:" \
              "ECDHE-RSA-CHACHA20-POLY1305:" \
              "TLS_DHE_RSA_WITH_AES_128_CCM:" \
              "TLS_DHE_RSA_WITH_AES_256_CCM:" \
              "dhe_rsa_chacha20_poly1305_sha_256:" \
              "TLS_ECDHE_ECDSA_WITH_AES_128_CCM:" \
              "TLS_ECDHE_ECDSA_WITH_AES_256_CCM:" \
              "ECDHE-ECDSA-CHACHA20-POLY1305"
    # 证书已吊销code
    REVOCATION_CODE = 23
    # 吊销列表已过期code
    EXPIRED_CRL_CODE = 12
    # 吊销列表不匹配证书code
    INVALID_CRL_CODE = 3
    # 吊销列表长度限制
    CRL_MAX_SIZE = 5 * 1024
    # 证书长度限制
    CERT_MAX_SIZE = 1024 * 1024
    # 吊销列表过期告警ID
    CRL_EXPIRED_ID = "0x6403320006"
    # 证书过期告警ID
    CERT_EXPIRED_ID = "0x6403320007"


class VMWareScanConstants:
    # 扫描最短间隔
    MIN_SCAN_INTERVAL = 3600

    # 扫描最大间隔
    MAX_SCAN_INTERVAL = 72 * 3600


class DmaProxy:
    host = "protectengine-e-dma"
    port = 30071


class VMWareStorageConstants:
    # 数据库中保存v-center的存储信息
    STORAGES = "storages"


class CopyExpireConstants:
    # 锁最大持有时间(单位：秒)
    LOCK_TIME_OUT = 120

    # 最长等待获取锁时间(单位：秒)
    LOCK_WAIT_TIME_OUT = 3

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
from app.common.enums.rbac_enum import ResourceSetTypeEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.security.role_dict import RoleEnum


class CommonConstants:
    COMMON_DATE_FORMAT = "%Y-%m-%d %H:%M:%S"
    COMMON_DATE_FORMAT_WITH_T = "%Y-%m-%dT%H:%M:%S"
    DEE_DATE_FORMAT = "%Y%m%d%H%M%S"

    # agent 版本号的长度
    AGENT_VERSION_LENTH = 4

    # agent 补丁版本号的长度
    AGENT_SPC_VERSION_LENTH = 5

    # 补丁号
    SPC_PRE_FIX = "SPC"

    # agent 版本号前缀的长度
    AGENT_VERSION_PREFIX_LENTH = 3

    # 接口返回新包agent的数据个数
    NEW_AGENT_PACKAGE_INFO_LENTH = 0

    # 默认时区
    DEFAULT_TIMEZONE = 'Asia/Shanghai'

    # 管理员用户
    ADMIN = 'admin'

    # 业务用户
    DATAPROTECT_ADMIN = 'dataprotect_admin'


class AgentConstants:
    # 主机不可升级
    AGENT_NOT_UPGRADEABLE = "0"

    # 主机可升级
    AGENT_UPGRADEABLE = "1"


class DBRetryConstants:
    # 重试次数
    RETRY_TIMES = 5

    # 下次重试等待时间
    WAIT_TIME = 60

    # 等待时间系数(如2则下次等待时间翻倍)
    BACKOFF = 1


class SecurityConstants:
    SSL_CIPHERS = "ECDHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES128-GCM-SHA256"
    INTERNAL_CERT_DIR = "/opt/OceanProtect/infrastructure/cert/internal"
    INTERNAL_CA_FILE = f"{INTERNAL_CERT_DIR}/ca/ca.crt.pem"
    INTERNAL_CERT_FILE = f"{INTERNAL_CERT_DIR}/internal.crt.pem"
    INTERNAL_KEY_FILE = f"{INTERNAL_CERT_DIR}/internal.pem"
    INTERNAL_KEYFILE_PWD_FILE = f"{INTERNAL_CERT_DIR}/internal_cert"
    REDIS_CERT_DIR = "/opt/OceanProtect/infrastructure/cert/redis"
    REDIS_CA_FILE = f"{REDIS_CERT_DIR}/ca/ca.crt.pem"
    REDIS_CERT_FILE = f"{REDIS_CERT_DIR}/redis.crt.pem"
    REDIS_KEY_FILE = f"{REDIS_CERT_DIR}/redis.pem"

    # 依赖的KMC动态库
    LIBKMCV3_SO_PATH = "/usr/lib64/libkmcv3.so"
    MODULE_NAME = "protectmanager"
    DEFAULT_DOMAIN_ID = 0
    KMC_MASTER_KS_PATH = "/opt/OceanProtect/protectmanager/kmc/master.ks"
    KMC_BACKUP_KS_PATH = "/kmc_conf/..data/backup.ks"


class ServiceConstants:
    HOSTNAME_SUFFIX = ".dpa.svc.cluster.local"

    DATA_ENABLE_ENGINE = "dataenableengine-server"
    DATA_ENABLE_ENGINE_PORT = 8083
    DATA_ENABLE_ENGINE_HOSTNAME = f"{DATA_ENABLE_ENGINE}{HOSTNAME_SUFFIX}"
    DATA_ENABLE_ENGINE_URL_PREFIX = f"https://{DATA_ENABLE_ENGINE_HOSTNAME}:{DATA_ENABLE_ENGINE_PORT}"

    DEE_PARSER = "dataenableengine-server"
    DEE_PARSER_PORT = 8085
    DEE_PARSER_HOSTNAME = f"{DEE_PARSER}{HOSTNAME_SUFFIX}"
    DEE_PARSER_URL_PREFIX = f"https://{DEE_PARSER_HOSTNAME}:{DEE_PARSER_PORT}"

    INFRASTRUCTURE = "infrastructure"
    INFRASTRUCTURE_PORT = 8088
    INFRASTRUCTURE_HOSTNAME = f"{INFRASTRUCTURE}{HOSTNAME_SUFFIX}"
    INFRASTRUCTURE_URL_PREFIX = f"https://{INFRASTRUCTURE_HOSTNAME}:{INFRASTRUCTURE_PORT}"

    PM_PROTECTION_SERVICE = "pm-protection-service"
    PM_PROTECTION_SERVICE_PORT = 30092
    PM_PROTECTION_SERVICE_HOSTNAME = f"{PM_PROTECTION_SERVICE}{HOSTNAME_SUFFIX}"
    PM_PROTECTION_SERVICE_URL_PREFIX = f"https://{PM_PROTECTION_SERVICE_HOSTNAME}:{PM_PROTECTION_SERVICE_PORT}"

    PM_SYSTEM_BASE = "pm-system-base"
    PM_SYSTEM_BASE_PORT = 30081
    PM_SYSTEM_BASE_HOSTNAME = f"{PM_SYSTEM_BASE}{HOSTNAME_SUFFIX}"
    PM_SYSTEM_BASE_URL_PREFIX = f"https://{PM_SYSTEM_BASE_HOSTNAME}:{PM_SYSTEM_BASE_PORT}"

    PROTECTENGINE_E_DMA = "protectengine-e-dma"
    PROTECTENGINE_E_DMA_PORT = 30070
    PROTECTENGINE_E_DMA_HOSTNAME = f"{PROTECTENGINE_E_DMA}{HOSTNAME_SUFFIX}"
    PROTECTENGINE_E_DMA_URL_PREFIX = f"https://{PROTECTENGINE_E_DMA_HOSTNAME}:{PROTECTENGINE_E_DMA_PORT}"

    UBC = "dme-ubc"
    UBC_PORT = 8089
    UBC_HOSTNAME = f"{UBC}{HOSTNAME_SUFFIX}"
    UBC_URL_PREFIX = f"https://{UBC_HOSTNAME}:{UBC_PORT}"

    OSA = "dme-openstorageapi"
    OSA_PORT = 30173
    OSA_HOSTNAME = f"{OSA}{HOSTNAME_SUFFIX}"
    OSA_URL_PREFIX = f"https://{OSA_HOSTNAME}:{OSA_PORT}"

    MULTI_CLUSTER_CONF = "multicluster-conf"
    REDIS_CLUSTER = "REDIS_CLUSTER"
    ZK_CLUSTER = "ZK_CLUSTER"


class GaussdbCertConfig:
    # gaussdb cert config
    CONN_ARGS = {
        "sslmode": "verify-ca",
        "sslrootcert": "/opt/OceanProtect/infrastructure/cert/internal/ca/ca.crt.pem"
    }
    AUTOCOMMIT = "AUTOCOMMIT"


class CallbackStatus:
    SUCCESS = "success"
    FAIL = "fail"


class ReplicationConstants:
    REPLICATION_TARGET_TYPE = "replication_target_type"
    REP_MODE_ALL_COPY = "ALL_COPY"
    REP_MODE_SPECIFIED_COPY = "SPECIFIED_COPY"
    REP_MODE_MANUAL = "MANUAL"


class RBACConstants:
    # 忽略的映射字段
    IGNORE_FIELDS_LIST = ["resource_set_id", "labelName", 'labelList', "user_id"]
    # 内置角色名称集合
    DEFAULT_IN_ROLE_NAME_LIST = [
        RoleEnum.ROLE_SYS_ADMIN.value, RoleEnum.ROLE_AUDITOR.value, RoleEnum.ROLE_DR_ADMIN.value,
        RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_DEVICE_MANAGER
    ]
    # 只分域不分权资源类型
    ONLY_IN_DOMAIN_RESOURCE_TYPE_LIST = [
        ResourceSetTypeEnum.JOB.value, ResourceSetTypeEnum.ALARM.value, ResourceSetTypeEnum.JOB_LOG.value,
        ResourceSetTypeEnum.EXERCISE.value
    ]
    REQUIRED_TYPE_LIST = [
        ResourceSetTypeEnum.JOB.value, ResourceSetTypeEnum.COPY.value
    ]


class RestoreConstants:
    RESTORE_OBJECT_MAX_NUM_LIMIT = 1000
    RESTORE_OBJECT_MAX_BYTE = 1000 * 1024


class MultiClusterKafkaTopicConstants:
    KAFKA_DEFAULT_TOPIC_PARTITIONS = 6
    KAFKA_DEFAULT_TOPIC_REPLICATION_FACTOR = 3
    MIN_IN_SYNC_REPLICAS = 2


class PermanentBackupConstants:
    # 实际是永久增量类型，但是配置成了difference_increment的应用
    DIFFERENCE_INCREMENT_APP_SUBTYPE = [ResourceSubTypeEnum.HBaseBackupSet.value,
                                        ResourceSubTypeEnum.HiveBackupSet.value,
                                        ResourceSubTypeEnum.HDFSFileset.value,
                                        ResourceSubTypeEnum.KubernetesStatefulSet.value,
                                        ResourceSubTypeEnum.VirtualMachine.value,
                                        ResourceSubTypeEnum.HCSCloudHost.value,
                                        ResourceSubTypeEnum.FusionCompute.value,
                                        ResourceSubTypeEnum.FUSION_ONE_COMPUTE.value,
                                        ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE.value,
                                        ResourceSubTypeEnum.APSARA_STACK.value,
                                        ResourceSubTypeEnum.HYPER_V_VM.value,
                                        ResourceSubTypeEnum.Oracle.value,
                                        ResourceSubTypeEnum.CNWARE_VM.value,
                                        ResourceSubTypeEnum.NUTANIX_VM.value]

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

from decimal import Decimal
from enum import Enum, IntEnum
from typing import Optional

from k8s.common.kubernetes_client.struct import (ClusterAuthentication,
                                                 Namespace, Rule, Resource, ConsistencyRules, Repositories)
from pydantic import BaseModel, Field


class K8sResourceKeyName:
    """
    K8s资源接入用户key
    """
    APPLICATIONS_AUTH_EXTENDINFO_TOKEN = "applications_0_auth_extendInfo_token_"
    APPLICATIONS_AUTH_AUTHTYPE = "applications_0_auth_authType_"
    APPENV_AUTH_AUTHTYPE = "appEnv_auth_authType_"
    APPENV_AUTH_AUTHPWD = "appEnv_auth_authPwd_"
    APPENV_AUTH_EXTENDINFO_TOKEN = "appEnv_auth_extendInfo_token_"
    APPENV_AUTH_EXTENDINFO_CONFIG = "appEnv_auth_extendInfo_configKey_"
    APPENV_AUTH_EXTENDINFO_CERTIFICATE_AUTHORITY_DATA = "appEnv_auth_extendInfo_certificateAuthorityData_"


class K8sBackupKeyName:
    """
    K8s资源接入用户key
    """
    PROTECTENV_AUTH_AUTHTYPE = "job_protectEnv_auth_authType_"
    PROTECTENV_AUTH_AUTHPWD = "job_protectEnv_auth_authPwd_"
    PROTECTENV_AUTH_EXTENDINFO_TOKEN = "job_protectEnv_auth_extendInfo_token_"
    PROTECTENV_AUTH_EXTENDINFO_CONFIG = "job_protectEnv_auth_extendInfo_configKey_"
    PROTECTENV_AUTH_EXTENDINFO_CERTIFICATE_AUTHORITY_DATA = "job_protectEnv_auth_extendInfo_certificateAuthorityData_"
    TARGETENV_AUTH_AUTHTYPE = "job_targetEnv_auth_authType_"
    TARGETENV_AUTH_EXTENDINFO_TOKEN = "job_targetEnv_auth_extendInfo_token_"
    TARGETENV_AUTH_EXTENDINFO_CONFIG = "job_targetEnv_auth_extendInfo_configKey_"
    TARGETENV_AUTH_EXTENDINFO_CERTIFICATE_AUTHORITY_DATA = "job_targetEnv_auth_extendInfo_certificateAuthorityData_"


class K8sType:
    """
    K8s涉及的类型
    """
    TYPE_CLUSTER = "KubernetesCommon"


class K8sSubType:
    """
    K8s子任务涉及的类型
    """
    SUBTYPE_CLUSTER = "KubernetesClusterCommon"
    SUBTYPE_NAMESPACE = "KubernetesNamespaceCommon"
    SUBTYPE_DATASET = "KubernetesDatasetCommon"


class K8sJobKind(str, Enum):
    NAMESPACE = 'Namespace'
    PVC = 'PersistentVolumeClaim'
    WORKLOAD = [
        'Pod',
        'StatefulSet',
        'DaemonSet',
        'Deployment'
    ]


class Super(IntEnum):
    WORKLOAD = 0


class ActionType(IntEnum):
    BACKUP = 1
    RESTORE = 2


class RestoreType(IntEnum):
    OVERWRITE = 0
    DEFAULT = 1
    IGNORE_EXIST = 2
    OVERWRITE_OLDER = 3


class RestoreCommandParam(IntEnum):
    IGNORE_EXIST = 1
    OVERWRITE = 2


class MetadataStatus(IntEnum):
    PRE = 0
    POST = 1
    PVC = 2


class UserIdentity(IntEnum):
    ROOT = 0


class ConditionsInfo(BaseModel):
    kind: str
    params_for_workload: Optional[str]
    params_for_pvc: Optional[dict]


class QueryClusterResponse(BaseModel):
    uuid: str = Field(None, description='id')
    name: str = Field(None, description='name')
    cluster_type: str = Field(None, description='type')
    sub_type: str = Field(default=None, description='subType', alias='subType')
    endpoint: str = Field(None, description='endpoint')
    extend_info: dict = Field({}, description='extend_info', alias='extendInfo')


NFS_DATA_PATH = '/mnt/target/backup-data/'
NFS_META_PATH = '/mnt/target/meta/'
NFS_CACHE_PATH = '/mnt/target/cache/'
SOURCE_MOUNT_PATH = '/mnt/source/'
TARGET_PREFIX = '/mnt/source/'
# 路径白名单
PATH_WHITE_LIST = r"^/mnt/databackup/"
# 进度上报路径
PROGRESS_REPORT_PATH = 'progress'
PROGRESS_FILE_NAME = '1_backup_stats.json'
STORAGE_CLASS_NAME = 'storageClassName'
SNAPSHOT_GROUP = 'snapshot.storage.k8s.io'
VOLUME_SNAPSHOT_CLASS_KIND = "VolumeSnapshotClass"
VOLUME_SNAPSHOT_CLASS_PLURAL = 'volumesnapshotclasses'
VOLUME_SNAPSHOT_KIND = "VolumeSnapshot"
VOLUME_SNAPSHOT_PLURAL = 'volumesnapshots'
HUAWEI_CSI_CONFIGMAP = 'huawei-csi-configmap'
NAMESPACE_KUBE_SYSTEM = 'kube-system'
TASK_ID_LABEL = 'taskid'
DPA_POD_LABEL = "dataprotect.backup.io/tag"
EXEC_COMMAND_TIMEOUT = 6000
HTTP_REQUEST_TIMEOUT = 6000
NAS_FILESYSTEM_METRICS_VALUE = Decimal('1.5')

CONSISTENCY_LABEL_SWITCH_SELECTOR = 'hook.dataprotect.backup.io/switch=on'
CONSISTENCY_ANNOTATION_PRE_CONTAINER = 'pre.hook.dataprotect.backup.io/container'
CONSISTENCY_ANNOTATION_PRE_COMMAND = 'pre.hook.dataprotect.backup.io/command'
CONSISTENCY_ANNOTATION_POST_CONTAINER = 'post.hook.dataprotect.backup.io/container'
CONSISTENCY_ANNOTATION_POST_COMMAND = 'post.hook.dataprotect.backup.io/command'

VOLUME_SNAPSHOT_CONTENT_KIND = "VolumeSnapshotContent"
VOLUME_SNAPSHOT_CONTENT_PLURAL = "volumesnapshotcontents"
PVC_KIND = "PersistentVolumeClaim"
SNAPSHOT_VERSION_PATTERN = 'Snapshot.storage(.+?)Api'
PVC_VERSION_PATTERN = 'Storage(.+?)Api'

NAMESPACE = 'dpa'
DORADO_TASK_TIMEOUT_SEC = 300
TASK_QUERY_INTERVAL_SEC = 1
PVC_PREFIX_LENGTH = 12
SP_PVC_PREFIX_LENGTH = 21
PVC_PENDING_TIMEOUT = 60 * 60 * 24
CONSISTENT_SCRIPT_TIMEOUT = 60 * 60

DATA_REPO = 'data'
META_REPO = 'meta'
CACHE_REPO = 'cache'
# 工作负载类型和复数映射
WORKLOAD_PLURAL_MAP = {
    'Job': 'jobs',
    'CronJob': 'cronjobs',
    'StatefulSet': 'statefulsets',
    'ReplicaSet': 'replicasets',
    'Deployment': 'deployments',
    'DaemonSet': 'daemonsets',
    'DeploymentConfig': 'deploymentconfigs'
}

MAX_RETRY_TIME = 5
# 核心资源没有group属性，添加默认组恢复时好区分
OCEAN_PROTECT_GROUP = "oceanProtect_group"
# 不需要恢复资源的group/version/kind
NO_NEED_RESTORE_RESOURCE_GROUP_AND_VER_AND_KIND = [
    'events.k8s.io/v1|Event', 'project.openshift.io/v1|Project',
    'admissionregistration.k8s.io/v1|ValidatingWebhookConfiguration',
    'admissionregistration.k8s.io/v1|MutatingWebhookConfiguration'
]
# k8s的工作负载StatefulSet
STATEFULSET = 'StatefulSet'
# k8s的工作负载ReplicaSet
REPLICASET = 'ReplicaSet'
# k8s的工作负载Deployment
DEPLOYMENT = 'Deployment'
# openshift的工作负载DeploymentConfig
DEPLOYMENTCONFIG = 'DeploymentConfig'
# k8s的工作负载DaemonSet
DAEMONSET = 'DaemonSet'
# k8s的工作负载Job
JOB = 'Job'
# k8s的工作负载CronJob
CRONJOB = 'CronJob'


class ResourceRequirements:
    LIMITS = {'memory': '3Gi'}
    REQUESTS = {'memory': '128Mi', 'cpu': '0.5'}


class PREFIX:
    # {PVC_NAME_PREFIX}-{任务id后12位}-{pvc-uid后12位}
    PVC_NAME_PREFIX = "dpa-pvc-backup"
    # {VOLUME_SNAPSHOT_NAME_PREFIX}-{taskId后12位}-{pvc的uid后12位}
    # 后期 {VOLUME_SNAPSHOT_NAME_PREFIX}-{子任务taskId后12位}-{pvc的uid后12位}
    VOLUME_SNAPSHOT_NAME_PREFIX = "dpa-backup"
    # {VOLUME_SNAPSHOT_CONTENT_NAME_PREFIX}-{存储snapshot名字}
    VOLUME_SNAPSHOT_CONTENT_NAME_PREFIX = "dpa-content"
    # {VOLUME_SNAPSHOT_CLASS_PREFIX}-{驱动名称}
    VOLUME_SNAPSHOT_CLASS_NAME_PREFIX = "dpa-vsc"
    SNAPSHOT_GROUP_NAME_PREFIX = "k8s"
    JOB_NAME_PREFIX = 'dpa-backup-job'
    LOGIC_CHECK_PREFIX = 'dpa-logic-check'
    SNAPSHOT_PVC_NAME_PREFIX = 'dpa-pvc-backup'
    BACKUP_POD_NAME_PREFIX = 'dpa-k8s'
    RESTORE_POD_PREFIX = 'dpa-restore-job'


class K8SProtectError(Exception):
    def __init__(self, error_code, error_desc):
        super().__init__(error_code, error_desc)
        self._error_info = {'code': error_code, 'desc': error_desc}
        self._error = str(self._error_info)

    def __str__(self):
        return self._error

    @property
    def code(self):
        return self._error_info['code']

    @property
    def desc(self):
        return self._error_info['desc']


class ConsistencyRulesWithAuthentication(BaseModel):
    rule: Optional[Rule]
    cluster_authentication: ClusterAuthentication
    namespace: Namespace


class ConsistencyRuleType(IntEnum):
    PRE_RULE = 0
    POST_RULE = 1


class BackupPodType(IntEnum):
    BACKUP_PVC_TYPE = 1
    OBTAIN_WHITE_LIST_TYPE = 2


class JobInfo(BaseModel):
    resource: Resource
    consistency_rules: Optional[ConsistencyRules]
    qos: Optional[str]
    sub_id: Optional[str]
    task_id: str
    request_id: Optional[str]
    copy_id: Optional[str]
    data_repo: Repositories
    cache_repo: Repositories
    meta_repo: Repositories
    image_name: str
    pod_nums: int
    advanced_params: Optional[dict]
    action_type: ActionType
    job_type: int  # jobType: {Backup: 1 FULL, 2 INC; Restore: 1 Ignore, 2 Overwrite}
    node_selector: Optional[str]
    change_env_dict: Optional[dict]
    change_sc_dict: Optional[dict]
    task_timeout: Optional[int]
    consistent_timeout: Optional[int]


class SnapShotInfo(BaseModel):
    name: str
    size: str
    storage_class_name: str
    source_name: str
    source_uid: str
    access_mode: str


class NFSInfo(BaseModel):
    server: str
    path: str


class RestorePolicy(IntEnum):
    OVERWRITE_EXIST = 0
    IGNORE_IF_EXIST = 1


class Progress:
    """
    进度上报常量，按照前置、备份中、后置三个阶段上报，规定可上报进度分别为30/40/30
    """
    PROGRESS_THIRTY = 30
    PROGRESS_FORTY = 40
    PROGRESS_ZERO = 0


class Event:
    PVC_IS_DELETED = "PVC is being deleted"
    NODE_HAS_NO_DRIVER = "does not contain driver"


class K8sReportLabel:
    DATASET_WITH_NO_DATA_NEED_TO_SKIP_RESTORE = "k8s_dataset_skip_restore_label"
    NO_CSI_DRIVER_NODE = "k8s_node_without_csi_driver_label"
    # {{pod名称}}的数据库前置一致性脚本执行超时，超时时间为{（超时时间）}，请用户执行后置脚本解锁数据库
    PRE_CONSISTENT_SCRIPT_EXEC_TIMEOUT = "k8s_pre_consistent_script_exec_timeout_label"
    # {{pod名称}}的数据库后置一致性脚本执行超时，超时时间为{（超时时间）}，请用户执行后置脚本解锁数据库
    POST_CONSISTENT_SCRIPT_EXEC_TIMEOUT = "k8s_post_consistent_script_exec_timeout_label"
    # {{pod名称}}的数据库前置一致性脚本执行失败
    PRE_CONSISTENT_SCRIPT_EXEC_FAIL = "k8s_pre_consistent_script_exec_fail_label"
    # {{pod名称}}的数据库后置一致性脚本执行失败，请用户执行后置脚本解锁数据库
    POST_CONSISTENT_SCRIPT_EXEC_FAIL = "k8s_post_consistent_script_exec_fail_label"
    # {{pod名称}}的数据库前置一致性脚本执行成功
    PRE_CONSISTENT_SCRIPT_EXEC_SUCCESS = "k8s_pre_consistent_script_exec_success_label"
    # {{pod名称}}的数据库后置一致性脚本执行成功
    POST_CONSISTENT_SCRIPT_EXEC_SUCCESS = "k8s_post_consistent_script_exec_success_label"
    # 时间点{打快照的时间戳}对pvc{单个pvc}打快照
    PVC_CREATE_SNAPSHOT_SUCCESS = "k8s_pvc_create_snapshot_success_label"
    # 已注册的CSI集群的镜像名称{镜像名称}不合法，需要重新注册
    PVC_PRE_REGISTERED_IMAGE_NAME_ILLEGAL = "k8s_pvc_pre_registered_image_name_illegal_label"

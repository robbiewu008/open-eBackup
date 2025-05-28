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

import re
from enum import Enum, IntEnum
from typing import Optional, List

from pydantic import BaseModel, validator, Field

from k8s.logger import log


class Token(BaseModel):
    address = ""
    port = ""
    token_info = ""
    certificateAuthorityData = ""


class AuthType(IntEnum):
    TOKEN = 6
    CONFIGFILE = 8


class K8sResType(IntEnum):
    CONFIGMAP = 1
    SECRET = 2
    PVC = 3


class ClusterAuthentication(BaseModel):
    auth_type: AuthType
    token: Optional[Token]
    kube_config: Optional[dict]
    is_verify_ssl: str = Field(None, description='is_verify_ssl')
    id: str = Field(None, description='pid')


class SubRule(BaseModel):
    pod_selector_list: List[str]
    container: str
    action_list: List[str]

    @validator('action_list', each_item=True)
    def check_squares(cls, v):
        m = re.findall(r'[|;&$><`\\]', v)
        if m:
            log.error(f'action:{v} is not ok. m:{m}')
            raise AssertionError
        else:
            log.debug(f'action:{v} is ok. m{m}')
        return v


class Rule(BaseModel):
    name: str
    id: str
    sub_rule_list: List[SubRule]


class ConsistencyRules(BaseModel):
    pre_rule: Optional[Rule]
    post_rule: Optional[Rule]


class BackupProxy(BaseModel):
    node_selector: str
    current_task_num: int
    back_proxy_image_tag: str


class BackupApplicationAuth(BaseModel):
    user: str
    password: str


class BackupApplicationInfo(BaseModel):
    application_type: int
    backup_application_auth: BackupApplicationAuth


class WorkLoadType(Enum):
    POD = 'Pod'
    REPLICASET = 'ReplicaSet'
    STATEFULSET = 'StatefulSet'
    DEPLOYMENT = 'Deployment'
    DAEMONSET = 'DaemonSet'
    JOB = 'Job'
    CRONJOB = 'CronJob'


class ApplicationType(int, Enum):
    MYSQL = 0
    COMMON = 1


class DataSet(BaseModel):
    name: str
    id: Optional[str]
    labels: Optional[str]


class Namespace(BaseModel):
    name: str
    id: Optional[str]


class Resource(BaseModel):
    cluster_name: str = Field(None, description='cluster_name')
    backup_type: str = Field(None, description='backup_type')
    namespace: Namespace = Field(None, description='namespace')
    dataset: DataSet = Field(None, description='dataset')
    cluster_authentication: ClusterAuthentication = Field(None, description='cluster_authentication')


class Repositories(BaseModel):
    logical_ip_list: List[str]
    remote_path: str
    local_path: str


class ResourceInfo(BaseModel):
    namespace: str = Field(None, description='namespace')
    groups: str = Field(None, description='groups')
    version: str = Field(None, description='version')
    plural: str = Field(None, description='plural')
    kind: str = Field(None, description='kind')
    items = []


class BackTaskStatus(BaseModel):
    pod_num: int = Field(0, description='pod_num')
    total_pod_num: int = Field(0, description='total_pod_num')
    snap_shot_dict = {}
    all_pvc_list = []


class PodPhase(str, Enum):
    # pvc完成状态，同对应的pod状态
    SUCCEEDED = "Succeeded"
    RUNNING = 'Running'
    PENDING = 'Pending'
    FAILED = 'Failed'
    UNKNOWN = 'Unknown'


class ReportParam(BaseModel):
    cache_path: str = Field('', description='cache_path')
    pvc_name_list = []
    req_id: str
    job_id: str
    sub_job_id: str


class WorkloadInfo(BaseModel):
    pvc: list
    kind: str
    name: str
    api_version: str
    replicas: int = Field(0, description='replicas')


class PatchInfo(BaseModel):
    group: str
    version: str
    namespace: str
    plural: str
    name: str
    body: dict

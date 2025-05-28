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

import time
from enum import IntEnum

from kubernetes.client import ApiException

from k8s.common.const import JobInfo, MAX_RETRY_TIME, WORKLOAD_PLURAL_MAP, STATEFULSET, REPLICASET, DEPLOYMENT, \
    DEPLOYMENTCONFIG, DAEMONSET, JOB, CRONJOB
from k8s.common.kubernetes_client.k8s_api_class_custom_objects import ResourceCustomObjects
from k8s.common.kubernetes_client.struct import PatchInfo
from k8s.logger import log


def get_patch_replicas_body(num):
    body = {
        'spec': {
            'replicas': num
        }
    }
    return body


def get_job_or_cron_job_patch_body(suspend):
    body = {
        'spec': {
            'suspend': suspend
        }
    }
    return body


def get_daemon_set_patch_body_down():
    body = {
        "spec": {
            "template": {
                "spec": {
                    "nodeSelector": {
                        "non-existing": "true"
                    }
                }
            }
        }
    }
    return body


def get_daemon_set_patch_body_up():
    body = {
        "spec": {
            "template": {
                "spec": {
                    "nodeSelector": None
                }
            }
        }
    }
    return body


def get_daemon_set_patch_body(param):
    if param:
        return get_daemon_set_patch_body_up()
    else:
        return get_daemon_set_patch_body_down()


class PatchType(IntEnum):
    SHUTDOWN = 0
    UP = 1


class WorkloadManager:

    @staticmethod
    def update_workload_info(job_info: JobInfo, workload_info_dict):
        workload_api = ResourceCustomObjects(job_info.resource.cluster_authentication)
        for workload_name, workload_info in workload_info_dict.items():
            group_api = workload_info.api_version.split('/')
            group = group_api[0]
            version = group_api[1]
            try:
                replicas = workload_api.get_with_namespace(group=group, version=version,
                                                           namespace=job_info.resource.namespace.name,
                                                           plural=WORKLOAD_PLURAL_MAP.get(workload_info.kind),
                                                           name=workload_name).get('spec').get('replicas')
                workload_info_dict[workload_name].replicas = replicas
            except ApiException as ex:
                if ex.status == 404:
                    log.exception(f"Workload {workload_name} does not exist, continue.")
                continue
        return workload_info_dict

    @staticmethod
    def shut_down_replicas(job_info: JobInfo, workload_info_dict):
        workload_api = ResourceCustomObjects(job_info.resource.cluster_authentication)
        workload_check_list = []
        for _ in range(MAX_RETRY_TIME):
            WorkloadManager._patch_workload_by_kind(PatchType.SHUTDOWN, workload_info_dict, workload_api, job_info)
            workload_check_list = WorkloadManager._check_replicas(PatchType.SHUTDOWN, workload_info_dict,
                                                                  workload_api, job_info)
            if not workload_check_list:
                break
        return workload_check_list

    @staticmethod
    def restore_replicas(job_info: JobInfo, workload_info_dict):
        workload_api = ResourceCustomObjects(job_info.resource.cluster_authentication)
        workload_check_list = []
        for _ in range(MAX_RETRY_TIME):
            WorkloadManager._patch_workload_by_kind(PatchType.UP, workload_info_dict, workload_api, job_info)
            workload_check_list = WorkloadManager._check_replicas(PatchType.UP, workload_info_dict,
                                                                  workload_api, job_info)
            if not workload_check_list:
                break
        return workload_check_list

    @staticmethod
    def _check_replicas(flag, workload_info_dict, workload_api, job_info):
        workload_check_list = list(workload_info_dict.keys())
        time_limit = 30
        while time_limit > 0:
            log.info(f'Workload is been checking:{workload_check_list}, '
                     f'workload_info_dict is {workload_info_dict}, task id:{job_info.task_id}')
            WorkloadManager._get_workload_status(flag, workload_info_dict, job_info, workload_api,
                                                 workload_check_list)
            if not workload_check_list:
                break
            time_limit -= 5
            time.sleep(5)
        log.info(f"Workload check list is {workload_check_list}, task id:{job_info.task_id}")
        return workload_check_list

    @staticmethod
    def _get_workload_status(flag, workload_info_dict, job_info, workload_api, workload_check_list):
        for workload_name, workload_info in workload_info_dict.items():
            if workload_name not in workload_check_list:
                continue
            group_api = workload_info.api_version.split('/')
            group = group_api[0]
            version = group_api[1]
            log.info(
                f'Check workload name {workload_name}, group:{group}, version:{version}, kind:{workload_info.kind}, '
                f'namespace:{job_info.resource.namespace.name}, '
                f'task id:{job_info.task_id}')
            try:
                workload = workload_api.get_with_namespace(group=group, version=version,
                                                           namespace=job_info.resource.namespace.name,
                                                           plural=WORKLOAD_PLURAL_MAP.get(workload_info.kind),
                                                           name=workload_name)
                if workload_info.kind in [JOB, CRONJOB]:
                    target_suspend = True
                    if flag == PatchType.UP:
                        target_suspend = False
                    suspend = workload.get('spec').get('suspend')
                    if target_suspend == suspend:
                        log.info(f"Workload {workload_name} has shut down, task id:{job_info.task_id}")
                        workload_check_list.remove(workload_name)
                    else:
                        log.warning(f'Workload {workload_name} is still shutting down, task id:{job_info.task_id}')
                if workload_info.kind == DAEMONSET:
                    WorkloadManager._check_daemon_set_patch(workload, workload_name, job_info, workload_check_list)
                if workload_info.kind in [STATEFULSET, REPLICASET, DEPLOYMENT, DEPLOYMENTCONFIG]:
                    target_replicas = 0
                    if flag == PatchType.UP:
                        target_replicas = workload_info.replicas
                    replicas = workload.get('spec').get('replicas', 0)
                    if replicas == target_replicas:
                        log.info(f"Workload {workload_name} has shut down, task id:{job_info.task_id}")
                        workload_check_list.remove(workload_name)
                    else:
                        log.warning(f'Workload {workload_name} is still shutting down, task id:{job_info.task_id}')
            except ApiException as ex:
                if ex.status == 404:
                    log.exception(f"Workload {workload_name} does not exist, continue.")
                workload_check_list.remove(workload_name)

    @staticmethod
    def _check_daemon_set_patch(workload, workload_name, job_info, workload_check_list):
        current_number_scheduled = workload.get('status').get('currentNumberScheduled', 0)
        number_miss_scheduled = workload.get('status').get('numberMisscheduled', 0)
        if current_number_scheduled == number_miss_scheduled:
            log.info(f"Workload {workload_name} has shut down, task id:{job_info.task_id}")
            workload_check_list.remove(workload_name)
        else:
            log.warning(f'Workload {workload_name} is still shutting down, task id:{job_info.task_id}')

    @staticmethod
    def _patch_workload_by_kind(flag, workload_info_dict, workload_api, job_info):
        for workload_name, workload_info in workload_info_dict.items():
            group_api = workload_info.api_version.split('/')
            group = group_api[0]
            version = group_api[1]
            try:
                log.info(f'Patch workload name {workload_name}, group:{group}, version:{version}, '
                         f'namespace:{job_info.resource.namespace.name}, '
                         f'task id:{job_info.task_id}')
                body = WorkloadManager._patch_workload(workload_info, flag)
                patch_info = PatchInfo(group=group, version=version,
                                       namespace=job_info.resource.namespace.name,
                                       plural=WORKLOAD_PLURAL_MAP.get(workload_info.kind),
                                       name=workload_name,
                                       body=body)
                workload_api.patch_with_namespace(patch_info)
            except ApiException as ex:
                if ex.status == 404:
                    log.exception(f"Workload {workload_name} patch failed, continue patch.")

    @staticmethod
    def _patch_workload(workload_info, flag):
        body = {}
        if workload_info.kind in [STATEFULSET, REPLICASET, DEPLOYMENT, DEPLOYMENTCONFIG]:
            target_replica = 0
            if flag == PatchType.UP:
                target_replica = workload_info.replicas
            body = get_patch_replicas_body(target_replica)
        elif workload_info.kind == DAEMONSET:
            body = get_daemon_set_patch_body(flag)
        elif workload_info.kind in [JOB, CRONJOB]:
            param = True
            if flag == PatchType.UP:
                param = False
            body = get_job_or_cron_job_patch_body(param)
        return body

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

import os
import shutil

from k8s.common.kubernetes_client.k8s_api_class_core.k8s_pod_api import ResourcePod
from k8s.common.label_const import LabelConst
from k8s.logger import log
from k8s.common.const import JobInfo
from kubernetes.client import ApiException


class BasicJob:

    def __init__(self, task_id, ns_name, image_name, job_info: JobInfo):
        self.task_id = task_id
        self.ns_name = ns_name
        self.image_name = image_name
        self.cache_path = job_info.cache_repo.local_path
        self._job_info = job_info
        return

    def _move_and_clean_cache(self, cache_path):
        log_path = os.getenv("GENERALDB_LOG_PATH")
        try:
            file_list = os.listdir(cache_path)
            for file in file_list:
                if os.path.isdir(os.path.join(cache_path, file)) and self._move_dir(cache_path, file, log_path):
                    log.info(f"Move dir {file} succeed!")
                    continue
                elif os.path.isfile(os.path.join(cache_path, file)) and self._move_file(cache_path, file, log_path):
                    log.info(f"Move file {file} succeed!")
                    continue
                log.error(f"Action error! File name: {file}")
            return True
        except Exception as err:
            log.exception(f'Clean task failed:{err}.')
            return False

    def _move_dir(self, cache_path, file, log_path):
        try:
            cache_file_path = os.path.join(cache_path, file)
            shutil.copytree(cache_file_path, log_path)
            shutil.rmtree(cache_file_path)
            return not os.path.isdir(cache_file_path)
        except FileNotFoundError as err:
            log.exception(f'Move cache fir {file} Failed! Err:{err}.ID:{self.task_id}.')
            return False

    def _move_file(self, cache_path, file, log_path):
        try:
            shutil.copy(os.path.join(cache_path, file), log_path)
            os.remove(os.path.join(cache_path, file))
            return os.path.isfile(os.path.join(cache_path, file))
        except FileNotFoundError as err:
            log.exception(f'Move cache file {file} Failed! Err:{err}.ID:{self.task_id}.')
            return False

    def _clean_pod(self):
        log.debug("Start _clean_pod_and_pvc!")
        pod_items, back_pod_instance = self._get_backup_pod()
        try:
            # 删除Pod
            for pod in pod_items:
                back_pod_instance.delete(pod.metadata.name, pod.metadata.namespace)
            return True
        except ApiException as e:
            log.exception(f"Delete backup pod failed!, err: {e}")
            return False

    def _get_backup_pod(self):
        log.info(f"Start _get_backup_pod")
        pod_label = f'{LabelConst.DPA_BACKUP_POD_COMMON_KEY}={LabelConst.DPA_BACKUP_POD_COMMON_VALUE},' \
                    f'{LabelConst.DPA_BACKUP_POD_TASK_KEY}={self.task_id}'
        back_pod_instance = ResourcePod(self._job_info.resource.cluster_authentication)
        pod_items = back_pod_instance.list(self._job_info.resource.namespace.name,
                                           label_selector=pod_label).items
        if not pod_items:
            log.warning(f"Get no backup pod! Task id:{self._job_info.task_id}")
            return [], back_pod_instance
        log.info(f"Get back pod! Pod nums:{len(pod_items)}")
        return pod_items, back_pod_instance

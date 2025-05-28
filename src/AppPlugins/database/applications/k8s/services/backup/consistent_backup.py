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

import json
import time

import k8s.common.const as const
from common.common_models import LogDetail, SubJobDetails
from common.const import DBLogLevel, SubJobStatusEnum
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_pod_api import ResourcePod
from k8s.common.report.report_manager import report_job_details
from k8s.logger import log


class ConsistentBackup:
    def __init__(self, kubernetes_back_info: const.JobInfo):
        self._kubernetes_back_info = kubernetes_back_info
        self._k8s_pod_api = ResourcePod(self._kubernetes_back_info.resource.cluster_authentication)
        self._namespace = self._kubernetes_back_info.resource.namespace.name
        self._exec_pods = self._get_pods_with_switch_hook_on()
        self.task_id = self._kubernetes_back_info.task_id
        self.sub_id = self._kubernetes_back_info.sub_id
        self.req_id = self._kubernetes_back_info.request_id
        self.timeout = self._kubernetes_back_info.consistent_timeout

    @staticmethod
    def _get_container_name(pre: bool, pod):
        annotations = pod.metadata.annotations
        if not annotations:
            return pod.spec.containers[0].name
        container_key = const.CONSISTENCY_ANNOTATION_PRE_CONTAINER \
            if pre else const.CONSISTENCY_ANNOTATION_POST_CONTAINER
        container_name = annotations.get(container_key, None)
        # 如果没有配置，默认取第一个
        if not container_name:
            container_name = pod.spec.containers[0].name
        return container_name

    @staticmethod
    def _get_exec_command(pre: bool, pod):
        annotations = pod.metadata.annotations
        if not annotations:
            return []
        command_key = const.CONSISTENCY_ANNOTATION_PRE_COMMAND if pre else const.CONSISTENCY_ANNOTATION_POST_COMMAND
        command_str = annotations.get(command_key, '[]')
        return json.loads(command_str)

    def exec_consistent_rule(self, pre: bool):
        """
        执行一致性备份
        :return: None
        """
        if not self._exec_pods:
            log.info(f"Not exist pod in namespace: {self._namespace}")
            return True
        result_dict = {}
        for exec_pod in self._exec_pods:
            pod_name = exec_pod.metadata.name
            resp = None
            try:
                # exec_command 执行的命令，list格式
                exec_command = self._get_exec_command(pre, exec_pod)
                prefix = "pre" if pre else "post"
                if not exec_command:
                    log.warn(f"{prefix} command is null.")
                    continue
                container_name = self._get_container_name(pre, exec_pod)
                resp = self._exec_command_with_pod(exec_pod, container_name, exec_command)
                if self.is_script_exec_finish(resp, pod_name, prefix, result_dict):
                    log.info("Skip outer for.")
                    continue
                # 一致性脚本执行超时
                log_info = const.K8sReportLabel.PRE_CONSISTENT_SCRIPT_EXEC_TIMEOUT if prefix == "pre" \
                    else const.K8sReportLabel.POST_CONSISTENT_SCRIPT_EXEC_TIMEOUT
                log.error(f"Pod:{pod_name} exec {prefix} script timeout {self.timeout}, task is {self.task_id}")
                self.report_job(log_info, [pod_name, str(self.timeout)], DBLogLevel.ERROR.value)
                result_dict.update({pod_name: False})
            except Exception:
                result_dict.update({pod_name: False})
                log.exception(f"Exec script fail task is {self.task_id}.")
            finally:
                if resp:
                    resp.close()
        if False in result_dict.values():
            return False
        return True

    def is_script_exec_finish(self, resp, pod_name, prefix, result_dict):
        start = time.time()
        while time.time() - start <= self.timeout:
            # 超时等待5秒
            if resp.is_open():
                resp.write_stdin("date\n")
            res = resp.readline_stdout(timeout=5)
            if res is None:
                continue
            log.info(f"pod: {pod_name} exec res: {res}, time: {time.time() - start}.")
            if res == 200 or res == '200':
                log.info(f"Pod:{pod_name} exec {prefix} script success, task is {self.task_id}")
                log_info = const.K8sReportLabel.PRE_CONSISTENT_SCRIPT_EXEC_SUCCESS if prefix == "pre" \
                    else const.K8sReportLabel.POST_CONSISTENT_SCRIPT_EXEC_SUCCESS
                self.report_job(log_info, [pod_name], DBLogLevel.INFO.value)
                result_dict.update({pod_name: True})
                return True
            if res == 0 or res == '0':
                log_info = const.K8sReportLabel.PRE_CONSISTENT_SCRIPT_EXEC_FAIL if prefix == "pre" \
                    else const.K8sReportLabel.POST_CONSISTENT_SCRIPT_EXEC_FAIL
                log.error(f"Pod:{pod_name} exec {prefix} script fail, task is {self.task_id}")
                self.report_job(log_info, [pod_name], DBLogLevel.ERROR.value)
                result_dict.update({pod_name: False})
                return True
        return False

    def report_job(self, log_info, log_info_param, log_level):
        log_detail = LogDetail(logInfo=log_info,
                               logLevel=log_level,
                               logInfoParam=log_info_param)
        report_job_details(self.req_id,
                           SubJobDetails(taskId=self.task_id, subTaskId=self.sub_id, logDetail=[log_detail],
                                         progress=const.Progress.PROGRESS_ZERO,
                                         taskStatus=SubJobStatusEnum.RUNNING.value))

    def _get_pods_with_switch_hook_on(self):
        if not self._kubernetes_back_info.resource.dataset.labels:
            label_selector = const.CONSISTENCY_LABEL_SWITCH_SELECTOR
        else:
            label_selector = f'{const.CONSISTENCY_LABEL_SWITCH_SELECTOR},' \
                             f'{self._kubernetes_back_info.resource.dataset.labels}'
        pod_list = self._k8s_pod_api.list(self._namespace, label_selector=label_selector)
        return pod_list.items

    def _exec_command_with_pod(self, pod, container_name, command):
        return self._k8s_pod_api.sync_pod_exec(pod.metadata.name, self._namespace, command, container_name)

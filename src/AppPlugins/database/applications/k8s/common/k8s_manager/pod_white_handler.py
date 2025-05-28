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
import uuid

from common.const import RpcToolInterface
from common.exception.common_exception import ErrCodeException
from common.util.exec_utils import exec_overwrite_file
from db2.comm.util.common_util import Db2CommonUtil
from k8s.common.error_code import ErrorCode
from k8s.common.k8s_manager.backup_pod_manager import BackupPodManager, CreatePodInfo
from k8s.common.const import BackupPodType, JobInfo, PREFIX, HTTP_REQUEST_TIMEOUT
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_pod_api import ResourcePod
from k8s.common.label_const import LabelConst
from k8s.common.utils import validate_ip_str
from k8s.logger import log


class PodWhiteListHandler:
    @staticmethod
    def create_temp_pod_and_add_ip_white_list(job_info):
        node_dict = BackupPodManager.get_node_dict(job_info)
        log.info(f"node dict===={node_dict}")
        create_pod_info_list = PodWhiteListHandler._get_creat_pod_info_list(job_info, node_dict)

        res = BackupPodManager.create_backup_pods(job_info, create_pod_info_list)

        log.info(f'Whitelist pod res:{res}, task id:{job_info.task_id}')

        all_ip = []

        for create_pod_info in create_pod_info_list:
            for storage_ip in job_info.data_repo.logical_ip_list:
                log.info(f'Whitelist to storage ip:{storage_ip}, task id:{job_info.task_id}')
                ip_list = PodWhiteListHandler._get_backup_pod_host_ip_list(create_pod_info.pod_name,
                                                                           job_info.resource.cluster_authentication,
                                                                           job_info.resource.namespace.name,
                                                                           storage_ip)
                log.info(f"Get backup pod ip:{ip_list}. Task id:{job_info.task_id}")
                if not ip_list:
                    log.warning(f'Storage ip {storage_ip} cannot reach! Task id:{job_info.task_id}')
                    continue
                for ips_str in ip_list:
                    log.info(f'Add ip {ips_str} into whitelist! Task id:{job_info.task_id}')
                    PodWhiteListHandler._send_msg_to_ubc_add_whitelist(job_info.task_id,
                                                                       ips_str, job_info.cache_repo.local_path)
                    all_ip.append(ips_str)
            BackupPodManager.del_backup_pods(job_info.resource.cluster_authentication,
                                             job_info.resource.namespace.name,
                                             [create_pod_info.pod_name])
        if not all_ip:
            log.error(f'Network cannot reach storage ip! Task id:{job_info.task_id}')
            raise ErrCodeException(err_code=ErrorCode.ERROR_NETWORK_CONNECT_TIMEOUT)
        return True

    @staticmethod
    def _get_backup_pod_host_ip_list(pod_name, cluster_authentication, ns_name, storage_ip):
        log.info(f"Pod name is {pod_name}")
        if not validate_ip_str(storage_ip):
            log.error(f'Error! Storage ip {storage_ip } has unacceptable character!!!')
            raise ErrCodeException(err_code=ErrorCode.PARAM_FAILED)
        backup_pod = ResourcePod(cluster_authentication)
        cmd_get_ips = f"sh /opt/script/exec_get_valid_ip_list.sh {storage_ip}"
        exec_command = ["/bin/sh", "-c", cmd_get_ips]
        ips = backup_pod.async_pod_exec(pod_name, ns_name, exec_command, "agent")
        log.info(f"Get pod ip list is {ips}")
        ip_list = ips.strip().split(',')
        return ip_list

    @staticmethod
    def _get_creat_pod_info_list(job_info: JobInfo, node_dict):
        create_pod_info_list = []
        for node_name in node_dict.keys():
            pod_name = f'{PREFIX.JOB_NAME_PREFIX}-{job_info.task_id}-{str(uuid.uuid4())}'
            labels = {
                LabelConst.DPA_BACKUP_POD_COMMON_KEY: LabelConst.DPA_BACKUP_POD_COMMON_VALUE,
                LabelConst.DPA_BACKUP_POD_TYPE_KEY: LabelConst.DPA_BACKUP_POD_TYPE_WHITE_LIST,
                LabelConst.DPA_BACKUP_POD_TASK_KEY: job_info.task_id,
                LabelConst.DPA_BACKUP_POD_NODE_KEY: node_name
            }
            create_pod_info = CreatePodInfo(node_name=node_name, pvc_name_list=[], pod_name=pod_name,
                                            command=["/bin/sh", "-c", "while true;do sleep 1;done"],
                                            pod_type=BackupPodType.OBTAIN_WHITE_LIST_TYPE, labels=labels)
            create_pod_info_list.append(create_pod_info)
        return create_pod_info_list

    @staticmethod
    def _send_msg_to_ubc_add_whitelist(task_id, ip_list_str, cache_repo_path):
        log.info("Begin to send msg to ubc.")
        tmp_id = uuid.uuid4()
        input_path = os.path.realpath(os.path.join(cache_repo_path,
                                                   f"ip_list_input{task_id}_P{tmp_id}"))
        output_path = os.path.realpath(os.path.join(cache_repo_path,
                                                    f"ip_list_output{task_id}_P{tmp_id}"))
        params = dict()
        params["jobId"] = task_id
        params["ipListStr"] = ip_list_str
        try:
            exec_overwrite_file(input_path, params)
            if not Db2CommonUtil.exec_rpc_tool_cmd(RpcToolInterface.ADD_IP_WHITE_LIST, input_path, output_path):
                raise Exception("Send msg to ubc add whitelist failed.")
        finally:
            for tmp_path in (input_path, output_path):
                if os.path.exists(tmp_path):
                    os.remove(tmp_path)
        return

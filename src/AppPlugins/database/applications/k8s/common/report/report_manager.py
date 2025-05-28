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
import os
import re
import stat
import time
from threading import Thread

from common.common import execute_cmd
from common.common_models import SubJobDetails
from common.const import SubJobStatusEnum, ParamConstant
from k8s.common.const import PROGRESS_REPORT_PATH, PROGRESS_FILE_NAME
from k8s.common.kubernetes_client.k8s_api_class_core.k8s_persistent_volume_claim_api import \
    ResourcePersistentVolumeClaim
from k8s.logger import log


def report_job_details(pid, job_detail):
    """
    功能描述：主动上报任务详情
    参数：无
    @pid: pid
    @job_detail: 任务详情
    返回值：无
    """
    input_file = ParamConstant.RESULT_PATH + pid + str(time.time()) + '_input'
    output_file = ParamConstant.RESULT_PATH + pid + '_output'
    json_str = json.dumps(job_detail.dict(by_alias=True))
    flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
    modes = stat.S_IWUSR | stat.S_IRUSR | stat.S_IXUSR
    with os.fdopen(os.open(input_file, flags, modes), 'w') as jsonfile:
        jsonfile.write(json_str)
    cmd = f'sh {ParamConstant.BIN_PATH}/rpctool.sh ReportJobDetails' \
          f' {input_file} {output_file}'
    execute_cmd(cmd)
    os.remove(input_file)
    os.remove(output_file)


class ReportManger:
    def __init__(self, report_param=None, status=SubJobStatusEnum.RUNNING):
        self.data_size = None
        self.report_param = report_param
        self.status = status
        self.progress_status = SubJobStatusEnum.RUNNING

    def update_process(self, method, args, thread_name):
        param_dict = {
            "cache_path": self.report_param.cache_path, "pvc_name_list": self.report_param.pvc_name_list,
            "req_id": self.report_param.req_id, "job_id": self.report_param.job_id,
            "sub_job_id": self.report_param.sub_job_id
        }
        log.info(f"Start update process, param is {param_dict}, task id is {self.report_param.job_id}")
        rp_thread = Thread(target=method, kwargs=args, name=thread_name)
        rp_thread.start()

    def report_job_detail(self, interval_sec=20):
        log.info(f"Start report sub job detail, taskId is {self.report_param.job_id}, interval_sec is {interval_sec}")
        data_size = 0
        while self.status == SubJobStatusEnum.RUNNING:
            data_size, progress = self.statistics_of_backup_data()
            sub_job_detail = SubJobDetails(taskId=self.report_param.job_id,
                                           subTaskId=self.report_param.sub_job_id,
                                           progress=0, dataSize=data_size,
                                           taskStatus=SubJobStatusEnum.RUNNING.value)
            report_job_details(self.report_param.req_id, sub_job_detail)
            log.info(f"Finish report sub job detail, datasize is {data_size}, task id is {self.report_param.job_id}")
            time.sleep(interval_sec)

    def statistics_of_backup_data(self):
        log.info(f"Start cal data size and statistical progress, task id is {self.report_param.job_id}")
        # 遍历获取pvc的名字，读取pvc进度的数据，累加复制的数据量
        all_data = 0
        all_progress = 0
        for pvc_name in self.report_param.pvc_name_list:
            parent_directory = os.path.dirname(self.report_param.cache_path)
            # 拼接进度文件路径，存储端cache仓/progress/pvc名字/1_backup_stats.json
            process_file_path = os.path.join(parent_directory, PROGRESS_REPORT_PATH, pvc_name, PROGRESS_FILE_NAME)
            if not os.path.exists(process_file_path):
                log.info(f"Pvc process file not exist, pvc is {pvc_name}, task id is {self.report_param.job_id}")
                continue
            with open(process_file_path, "r") as file:
                content = file.read()
                match_current = re.search(r'"noOfBytesCopied":(\d+)', content)
                match_total = re.search(r'"noOfBytesToBackup":(\d+)', content)
                if match_current and match_total:
                    # 已复制的字节数
                    no_of_bytes_copied = int(match_current.group(1))
                    # 总的字节数
                    no_of_bytes_to_backup = int(match_total.group(1))
                    all_data += no_of_bytes_copied
                    tmp_progress = 1 if no_of_bytes_to_backup == 0 else no_of_bytes_copied / no_of_bytes_to_backup
                    all_progress += tmp_progress
                else:
                    log.error(f"Not found noOfBytesCopied or noOfBytesToBackup, pvc is {pvc_name}, "
                              f"task id is {self.report_param.job_id}")
        # 返回备份总数据量以及备份进度,单位转换字节 -> kb
        all_data = round(all_data / 1024, 2)
        all_progress = round(all_progress / len(self.report_param.pvc_name_list), 2)
        log.info(f"All data is {all_data} kb, progress is {all_progress}, task id is {self.report_param.job_id}")
        return all_data, all_progress

    def check_pvc_status_and_report_progress(self, pvc_name, cluster_auth, namespace):
        log.info(f"Start check pvc status, taskId is {self.report_param.job_id}, namespace is {namespace}, "
                 f"pvc_name is {pvc_name}")
        k8s_pvc_api = ResourcePersistentVolumeClaim(cluster_auth)
        start_time = time.time()
        self.update_process(method=self.report_progress, args={}, thread_name='check_pvc_status_and_report_progress')
        try:
            while True:
                pvc_info = k8s_pvc_api.read(name=pvc_name, namespace=namespace)
                status = pvc_info.status.phase
                end_time = time.time()
                interval_time = end_time - start_time
                time_out_flag = interval_time > 60 * 60 * 24
                if status == 'Bound' or time_out_flag:
                    self.progress_status = SubJobStatusEnum.COMPLETED
                    break
                time.sleep(2)
            log.info(
                f"End check pvc status, taskId is {self.report_param.job_id}, namespace is {namespace}, "
                f"pvc_name is {pvc_name}, status is {status}, interval_time is {interval_time}")
        except Exception as err:
            self.progress_status = SubJobStatusEnum.FAILED
            log.exception(f'Check pvc status failed! task id:{self.report_param.job_id}.Err:{err}')

    def report_progress(self, interval_sec=20):
        log.info(f"Start report sub job detail, taskId is {self.report_param.job_id}, interval_sec:{interval_sec}")
        while self.status == SubJobStatusEnum.RUNNING:
            sub_job_detail = SubJobDetails(taskId=self.report_param.job_id,
                                           subTaskId=self.report_param.sub_job_id,
                                           progress=0,
                                           taskStatus=SubJobStatusEnum.RUNNING.value)
            report_job_details(self.report_param.req_id, sub_job_detail)
            log.info(f"Finish report sub job detail, task id is {self.report_param.job_id}")
            time.sleep(interval_sec)

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
import time
from abc import abstractmethod, ABCMeta

from common import common as res_base_common
from common.common_models import SubJobDetails, LogDetail
from common.const import RoleType
from common.const import SubJobStatusEnum, DBLogLevel, ReportDBLabel, ExecuteResultEnum
from common.logger import Logger
from common.number_const import NumberConst
from kingbase.common.models import RestoreProgress
from kingbase.common.util.resource_util import get_sys_rman_configuration_item, extract_ip
from kingbase.restore.kb_restore_process import KingbaseRestoreProcess
from kingbase.restore.kb_restore_service import KingbaseRestoreService

LOGGER = Logger().get_logger("kingbase.log")


class RestoreBase(metaclass=ABCMeta):
    """Kingbase恢复任务执行基类
    """

    def __init__(self, pid, job_id, sub_job_id, job_dict):
        self.pid = pid
        self.job_id = job_id
        self.sub_job_id = sub_job_id
        self.job_dict = job_dict

    @abstractmethod
    def exec_pre_task(self):
        """执行前置任务
        """
        LOGGER.info("Start executing restore prerequisite ...")
        cache_path = KingbaseRestoreService.get_cache_mount_path(self.job_dict)
        # 集群、单实例恢复前置任务进度文件文件名分别为：QueryRestoreProcess、QueryRestorePreProcess
        if KingbaseRestoreService.is_restore_cluster(self.job_dict):
            process_stage = "QueryRestoreProcess"
        else:
            process_stage = "QueryRestorePreProcess"
        KingbaseRestoreService.write_progress_info(cache_path, process_stage,
                                                   RestoreProgress(progress=NumberConst.ZERO, message="pre task begin"))
        # 1.恢复前检查数据库服务是否已停止
        if not KingbaseRestoreProcess.check_db_status_before_restore(self.pid, self.job_dict, cache_path,
                                                                     process_stage):
            return

        # 2.检查目标实例安装目录所属用户是否是副本数据所属的系统用户
        if not KingbaseRestoreProcess.check_sys_user_before_restore(self.pid, self.job_dict, cache_path, process_stage):
            return

        # 3.检查副本数据所属的系统用户是否可读写目标实例数据目录的父目录
        if not KingbaseRestoreProcess.check_rw_permission_before_restore(self.pid, self.job_dict, cache_path,
                                                                         process_stage):
            return
        LOGGER.info(f"Execute restore prerequisite task success")
        KingbaseRestoreService.record_task_result(self.pid, "Execute restore prerequisite task success",
                                                  code=ExecuteResultEnum.SUCCESS.value)
        KingbaseRestoreService.write_progress_info(cache_path, process_stage,
                                                   RestoreProgress(progress=NumberConst.HUNDRED,
                                                                   message="restore pre check completed"))

    @abstractmethod
    def exec_pre_subtask(self):
        """执行前置子任务
        """
        LOGGER.info("Start executing restore prerequisite subtask...")
        cache_path = KingbaseRestoreService.get_cache_mount_path(self.job_dict)
        if KingbaseRestoreService.is_restore_cluster(self.job_dict):
            process_stage = "QueryRestoreProcess"
        else:
            process_stage = "QueryRestorePreProcess"
        KingbaseRestoreService.write_progress_info(cache_path, process_stage,
                                                   RestoreProgress(progress=NumberConst.ZERO,
                                                                   message="pre subtask begin"))
        # 1.恢复前检查数据库服务是否已停止
        if not KingbaseRestoreProcess.check_db_status_before_restore(self.pid, self.job_dict, cache_path,
                                                                     process_stage):
            return

        # 2.检查目标实例安装目录所属用户是否是副本数据所属的系统用户
        if not KingbaseRestoreProcess.check_sys_user_before_restore(self.pid, self.job_dict, cache_path, process_stage):
            return

        # 3.检查副本数据所属的系统用户是否可读写目标实例数据目录的父目录
        if not KingbaseRestoreProcess.check_rw_permission_before_restore(self.pid, self.job_dict, cache_path,
                                                                         process_stage):
            return

        # 4.集群需要检查副本中目标位置的max_connections是否大于等于目标位置的max_connections
        if not KingbaseRestoreProcess.check_target_max_connections_is_greater_than_or_equal_to_original(
                self.pid, self.job_dict, cache_path, process_stage):
            return
        LOGGER.info("Success exec_pre_subtask")

    @abstractmethod
    def exec_task(self):
        """执行任务
        """
        pass

    @abstractmethod
    def exec_init_sys_rman_task(self):
        """
        执行sys_rman工具初始化任务，只能在repo节点
        """
        db_install_path, db_data_path = KingbaseRestoreService.get_db_install_and_data_path(self.job_dict)
        repo_ip = get_sys_rman_configuration_item(db_install_path, self.job_id, "_repo_ip")
        host_ips = extract_ip()
        tgt_role = KingbaseRestoreService.get_current_node_role(self.job_dict)
        if tgt_role == str(RoleType.PRIMARY.value) and repo_ip in host_ips:
            LOGGER.info("Current node is primary node and repo node!")
            db_system_user = KingbaseRestoreService.get_db_system_user(self.job_dict)
            KingbaseRestoreService.init_sys_rman_task(self.job_dict, db_install_path, db_system_user, self.job_id)
        cache_path = KingbaseRestoreService.get_cache_mount_path(self.job_dict)
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestorePostProcess",
                                                   RestoreProgress(progress=NumberConst.HUNDRED,
                                                                   message="completed"))

    @abstractmethod
    def exec_post_task(self):
        """执行后置任务
        """
        LOGGER.info("Start executing restore post task...")
        cache_path = KingbaseRestoreService.get_cache_mount_path(self.job_dict)
        restore_timestamp = self.job_dict.get("extendInfo", {}).get("restoreTimestamp")
        if restore_timestamp:
            merged_path = os.path.realpath(os.path.join(cache_path, "merged_log_copies"))
            KingbaseRestoreService.delete_path(merged_path)
        KingbaseRestoreService.write_progress_info(cache_path, "QueryRestorePostProcess",
                                                   RestoreProgress(progress=NumberConst.HUNDRED, message="completed"))
        return

    @abstractmethod
    def report_progress(self, task_name):
        """上报进度
        """
        LOGGER.info(f"Start querying progress ... task name: {task_name}")
        sub_job_details = SubJobDetails(taskId=self.job_id, subTaskId=self.sub_job_id, progress=NumberConst.ZERO,
                                        logDetail=list(), taskStatus=SubJobStatusEnum.RUNNING.value)
        cache_path = KingbaseRestoreService.get_cache_mount_path(self.job_dict)
        process_cxt = KingbaseRestoreService.read_process_info(cache_path, task_name)
        progress, message, err_code = process_cxt.get("progress"), process_cxt.get("message", ""), \
            process_cxt.get("err_code")
        err_param = process_cxt.get("err_param")
        LOGGER.info(f"Executing {task_name} task, progress: {progress}, message: {message}, process_cxt:{process_cxt}")
        if "completed" in message:
            task_status = SubJobStatusEnum.COMPLETED.value
        elif "failed" in message:
            task_status = SubJobStatusEnum.FAILED.value
        else:
            task_status = SubJobStatusEnum.RUNNING.value
        sub_job_details.progress = progress
        sub_job_details.task_status = task_status
        if err_code:
            if not self.sub_job_id and task_name == "QueryRestorePreProcess":
                report_label = ReportDBLabel.PRE_REQUISIT_FAILED
            else:
                report_label = ReportDBLabel.RESTORE_SUB_FAILED
            job_detail = LogDetail(logInfo=report_label,
                                   logInfoParam=[self.sub_job_id],
                                   logTimestamp=int(time.time()),
                                   logDetail=err_code,
                                   logDetailParam=err_param if err_param else [],
                                   logLevel=DBLogLevel.ERROR.value)
            sub_job_details.log_detail.append(job_detail)
        LOGGER.info(f"Executing query progress task success. Task Name: {task_name}, "
                    f"Details: {sub_job_details.dict(by_alias=True)}")
        res_base_common.output_result_file(self.pid, sub_job_details.dict(by_alias=True))

    @abstractmethod
    def abort_task(self):
        """中止任务
        """
        pass

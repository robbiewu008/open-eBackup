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

from common.common import get_local_ips
from common.common_models import LogDetail
from common.const import ExecuteResultEnum, SubJobStatusEnum, DBLogLevel
from common.util.scanner_utils import scan_dir_size
from mongodb import LOGGER
from mongodb.comm.const import ErrorCode, Status
from mongodb.comm.utils import copy_info_report
from mongodb.job_manager import JobManager
from mongodb.service.backup.instance_backup import InstanceBackup
from mongodb.service.backup.replset_backup import ReplSetBackup
from mongodb.service.backup.shard_backup import ShardBackup
from mongodb.service.base_service import MetaServiceWorker


class BackupTask(MetaServiceWorker):

    def __init__(self, job_manager: JobManager, backup_param):
        super().__init__(job_manager, backup_param)
        self.pid = job_manager.pid
        self.job_id = job_manager.job_id
        self.sub_job_id = job_manager.sub_job_id
        self.param = backup_param
        self.backup_obj = self.create_backup_obj()

    def get_protect_obj_type(self):
        """
        备份对象类型
        :return:
        """
        cluster_mode = self.param.get_resource_type()
        cluster_map = {
            "3": "single",
            "2": "shard",
            "1": "replication",
            "0": "replication"
        }
        return cluster_map.get(cluster_mode)

    def create_backup_obj(self):
        cluster_type = self.get_protect_obj_type()
        return self._create_backup_obj(cluster_type)

    def get_instance(self):
        """
        获取当前节点上的在线mongo实例，没有实例当前节点不可执行
        :return:
        """
        instances = self.backup_obj.get_local_online_instances()
        if not instances:
            msg = "No assigned instance running on this node, job id: %s, sub job id: %s." % (
                self.job_id, self.sub_job_id)
            LOGGER.error(msg)
            self.update_action_result(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                                      body_err_code=ErrorCode.NO_INSTANCE_RUNNING.value, msg=msg)

    def check_instance_role(self):
        """
        获取当前节点上运行实例的角色，非可备份角色，当前节点不可执行
        :return:
        """
        inst_role = self.backup_obj.get_local_role()
        if not inst_role:
            LOGGER.error("Local instance role is %s, not allow backup on this role, job id: %s, sub job id: %s.",
                         inst_role, self.job_id, self.sub_job_id)
            self.update_action_result(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                                      body_err_code=ErrorCode.NO_INSTANCE_RUNNING.value)

    def check_status(self):
        """
        检查当前节点的所有实例状态，实例不在线不可备份
        :return:
        """
        status = self.backup_obj.get_local_insts_status()
        if status != Status.ONLINE:
            LOGGER.error("Local instance status is %s, not allow backup on this role, job id: %s, sub job id: %s.",
                         status, self.job_id, self.sub_job_id)
            self.update_action_result(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                                      body_err_code=ErrorCode.NO_INSTANCE_RUNNING.value)

    def check_lvm(self):
        """
        全量备份检查逻辑卷
        :return:
        """
        code, msg, err_param = self.backup_obj.check_lvm()
        if code:
            LOGGER.error(
                "Check local instance lvm failed. errcode:%s, msg:%s, job id: %s, sub job id: %s, err_param: %s.",
                code, msg, self.job_id, self.sub_job_id, err_param)
            log_detail = LogDetail(logInfo="plugin_backup_subjob_fail_label",
                                   logInfoParam=[self.sub_job_id], logDetail=code,
                                   logDetailParam=err_param, logDetailInfo=[msg], logLevel=DBLogLevel.ERROR.value)
            self.update_report_result(SubJobStatusEnum.FAILED.value, 30, log_details=[log_detail])

    def check_oplog(self):
        """
        检查oplog
        :return:
        """
        code, msg = self.backup_obj.check_oplog()
        if code:
            LOGGER.error("Check local instance oplog failed. errcode: %s, msg: %s, job id: %s , sub job id: %s.", code,
                         msg, self.job_id, self.sub_job_id)
            self.update_action_result(code=ExecuteResultEnum.INTERNAL_ERROR.value, body_err_code=code, msg=msg)

    def check_backup_type(self):
        """
        备份类型检查
        :return:
        """
        code, msg = self.backup_obj.check_backup_type()
        if code:
            self.update_action_result(code=ExecuteResultEnum.INTERNAL_ERROR.value, body_err_code=code, msg=msg)

    def gen_sub_jobs(self):
        """
        拆分备份子任务
        :return:
        """
        sub_jobs = self.backup_obj.gen_sub_job()
        if not sub_jobs or len(sub_jobs) < 1:
            code = ErrorCode.OPERATE_FAILED.value
            msg = "Gen sub job failed."
            LOGGER.error("Backup failed. errcode:%s, msg:%s, job id: %s.", code, msg, self.param.job_id)
            self.update_action_result(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                                      body_err_code=code, msg=msg)
        self.return_result = sub_jobs
        self.update_result()

    def execute_backup(self):
        """
        执行备份子任务
        :return:
        """
        code, msg = self.backup_obj.backup()
        if code:
            LOGGER.error("Backup failed. errcode: %s, msg: %s, job id: %s, sub job id: %s.", code, msg, self.job_id,
                         self.sub_job_id)
            self.update_action_result(code=ExecuteResultEnum.INTERNAL_ERROR.value, body_err_code=code, msg=msg)

    def report_copy_info(self):
        # 上报副本信息之前上报备份任务进度(数据量给UBC)
        LOGGER.debug("Sub job name is :%s", self.param.get_sub_job_name())
        dirs = self.backup_obj.gen_back_dir()
        LOGGER.info("Gen back dirs is: %s, jod id: %s", dirs, self.job_id)
        flag, data_size = scan_dir_size(self.job_id, dirs)
        LOGGER.info("MongoDB backup data size is: %s, jod id: %s, flag: %s", data_size, self.job_id, flag)
        progress = 100
        if not flag:
            data_size = 0
            LOGGER.error("Query copy data failed, data_size:%s jod id: %s", self.job_id, data_size)
        # status=6表示成功
        self.update_report_result(SubJobStatusEnum.COMPLETED.value, progress, data_size)
        LOGGER.info("Update report backup task progress completed.")
        copy_info = self.backup_obj.gen_copy_info()
        if not copy_info:
            LOGGER.error("Generate copy info failed, job id: %s , sub job id: %s.", self.job_id, self.sub_job_id)
            self.update_action_result(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                                      body_err_code=ErrorCode.FAILED_COPY_INFO.value)
            return
        result = copy_info_report(self.job_id, copy_info)
        if not result:
            LOGGER.error("Report backup failed with no report tool result, job id: %s , sub job id: %s.", self.job_id,
                         self.sub_job_id)
            self.update_action_result(code=ExecuteResultEnum.INTERNAL_ERROR.value,
                                      body_err_code=ErrorCode.FAILED_REPORT_COPY.value)
            return
        code = result.get("code", 1)
        if code:
            errcode = result.get("bodyErr", 0)
            msg = result.get("message", "")
            LOGGER.error("Report backup failed with errcode: %s, msg: %s, job id: %s , sub job id: %s.", errcode, msg,
                         self.job_id, self.sub_job_id)
            self.update_action_result(
                code=ExecuteResultEnum.INTERNAL_ERROR.value,
                body_err_code=ErrorCode.FAILED_REPORT_COPY.value,
                msg=msg
            )
            return

    def post_backup(self):
        """
        功能描述： 后置任务
        参数：
        @job_info：job_info
        """
        code, msg, err_param = self.backup_obj.post_backup()
        nodes_info = self.param.get_nodes_info()
        local_host = get_local_ips()
        insts = []
        for node, _ in nodes_info.items():
            if node in local_host:
                insts.extend(node)
        if not insts and local_host and len(local_host) > 0:
            insts.extend(local_host[0])
        if code:
            LOGGER.error("Execute post job failed. errcode: %s, msg: %s, job id: %s, sub job id: %s, host ip: %s.",
                         code, msg, self.job_id, self.sub_job_id, ''.join(get_local_ips()))
            log_detail = LogDetail(logInfo="agent_execute_post_task_fail_label",
                                   logInfoParam=[''.join(insts), self.sub_job_id], logDetail=code,
                                   logDetailParam=err_param, logDetailInfo=[msg], logLevel=DBLogLevel.ERROR.value)
            self.update_report_result(SubJobStatusEnum.FAILED.value, 100, log_details=[log_detail])

    def _create_backup_obj(self, cluster_type):
        backup_obj_map = {
            "single": InstanceBackup,
            "config": ReplSetBackup,
            "replication": ReplSetBackup,
            "shard": ShardBackup,
        }
        backup_obj = backup_obj_map.get(cluster_type)
        return backup_obj(self.pid, self.param)

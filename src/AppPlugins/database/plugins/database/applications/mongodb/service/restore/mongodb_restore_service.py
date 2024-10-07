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

from common.common_models import LogDetail
from common.const import DBLogLevel, SubJobStatusEnum
from mongodb import LOGGER
from mongodb.comm.const import ErrorCode, MongoDBCode, MongoRoles, ParamField
from mongodb.job_manager import JobManager
from mongodb.service.base_service import MetaServiceWorker
from mongodb.service.restore.mongodb_replset_restore import ReplSetRestore
from mongodb.service.restore.mongodb_shard_restore import ShardRestore
from mongodb.service.restore.mongodb_single_restore import SingleInstanceRestore


class RestoreTask(MetaServiceWorker):

    def __init__(self, job_manager: JobManager, param_obj):
        super().__init__(job_manager, param_obj)
        self.pid = job_manager.pid
        self.job_id = job_manager.job_id
        self.sub_job_id = job_manager.sub_job_id
        self.param = param_obj
        self.restore_obj = self.create_restore_obj()
        self.restore_type = self.param.get_copy_type()

    def create_restore_obj(self):
        """
        功能描述： 创建恢复对象类型
        """
        cluster_type = self.get_protect_obj_type()
        return self.get_restore_obj(cluster_type)

    def get_restore_obj(self, cluster_type):
        """
        功能描述： 创建恢复对象类型
        """
        restore_obj_map = {
            MongoRoles.SINGLE.value: SingleInstanceRestore,
            MongoRoles.CONFIG.value: ReplSetRestore,
            MongoRoles.REPLICATION.value: ReplSetRestore,
            MongoRoles.SHARD.value: ShardRestore,
        }
        restore_obj = restore_obj_map.get(cluster_type)
        return restore_obj(self.pid, self.param)

    def get_protect_obj_type(self):
        """
        恢复对象类型
        :return:
        """
        cluster_mode = self.param.get_resource_type()
        cluster_map = {
            "3": MongoRoles.SINGLE.value,
            "2": MongoRoles.SHARD.value,
            "1": MongoRoles.REPLICATION.value,
            "0": MongoRoles.REPLICATION.value
        }
        return cluster_map.get(cluster_mode)

    def check_shard_number(self):
        """
        功能描述： 检查当前节点的实例分片的数量是否一致
        """
        # 判断是否所有的shard均为复制集
        if self.get_protect_obj_type() == "shard":
            target_shard_number = len(self.param.get_target_shard_role_info())
            local_shard_number = len(self.param.get_local_shard_role_info())
            if not target_shard_number or target_shard_number != local_shard_number:
                msg = "Check shard number error, target cluster shard number is %s." % target_shard_number
                LOGGER.error("Check shard number error, job id: %s.", self.job_id)
                self.update_action_result(code=MongoDBCode.FAILED.value,
                                          body_err_code=ErrorCode.SHARD_NUMBER_NOT_MATCH.value, msg=msg,
                                          err_param=[local_shard_number])

    def check_oplog(self):
        """
        功能描述： 检查当前节点的实例中是否存在oplog
        """
        # 若是全量副本恢复则此处无需校验，直接返回, 判断是否所有的shard均为复制集
        if self.get_protect_obj_type() == "shard" and self.restore_type != ParamField.LOG_COPY:
            for shard in self.param.get_target_shard_role_info():
                if not shard and shard.get("host").split(",") < 2:
                    msg = "Check oplog error."
                    LOGGER.error("Check oplog error, job id: %s.", self.job_id)
                    self.update_action_result(code=MongoDBCode.FAILED.value,
                                              body_err_code=ErrorCode.NOT_SUPPORT_OPLOG.value, msg=msg)

    def check_mongo_tool(self):
        """
        功能描述： 检查当前节点的实例中是否存在mongorestore工具
        """
        body_err_code, body_err_param, msg = self.restore_obj.check_mongo_tool()
        LOGGER.debug("Func check_mongo_tool finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func check_mongo_tool error, body_err_code: %s, msg: %s, job id: %s.", body_err_code, msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value,
                                      body_err_code=body_err_code, msg=msg, err_param=[body_err_param])

    def gen_sub_jobs(self):
        """
        功能描述： 拆分恢复子任务
        :return:
        """
        sub_jobs = self.restore_obj.gen_sub_job()
        if len(sub_jobs) == 0:
            msg = "Gen sub jobs failed, job id: %s.", self.job_id
            LOGGER.error(msg)
            return self.update_action_result(code=MongoDBCode.FAILED.value,
                                             body_err_code=ErrorCode.OPERATE_FAILED.value, msg=msg)
        self.return_result = sub_jobs
        return self.update_result()

    def get_copy_info(self):
        """
        功能描述： 检查日志副本恢复对应的副本链路信息
        :return:
        """
        pass

    def pre_restore(self):
        """
        功能描述： 单实例执行恢复子任务前置校验
        :return:
        """
        body_err_code, msg, err_param = self.restore_obj.pre_restore()
        LOGGER.debug("Func pre_restore finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func pre_restore error, body_err_code: %s, msg: %s, job id: %s.", body_err_code, msg,
                         self.job_id)
            log_detail = LogDetail(logInfo="plugin_restore_subjob_fail_label",
                                   logInfoParam=[self.sub_job_id], logDetail=body_err_code,
                                   logDetailParam=err_param, logDetailInfo=[msg], logLevel=DBLogLevel.ERROR.value)
            self.update_report_result(SubJobStatusEnum.FAILED.value, 30, log_details=[log_detail])

    def restore_single_node(self):
        """
        功能描述： 单实例执行恢复子任务
        :return:
        """
        body_err_code, msg = self.restore_obj.restore_single_node()
        LOGGER.debug("Func restore_single_node finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func restore_single_node error, body_err_code: %s, msg: %s, job id: %s.", body_err_code, msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=body_err_code, msg=msg)

    def restore_replset_node(self):
        """
        功能描述： 单实例执行恢复子任务
        :return:
        """
        body_err_code, msg = self.restore_obj.restore_replset_node()
        LOGGER.debug("Func restore_replset_node finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func restore_replset_node error, body_err_code: %s, msg: %s, job id: %s.", body_err_code,
                         msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=body_err_code, msg=msg)

    def restore_replset_init(self):
        """
        功能描述： 单实例执行恢复子任务
        :return:
        """
        body_err_code, msg = self.restore_obj.restore_replset_init()
        LOGGER.debug("Func restore_replset_init finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func restore_replset_init error, body_err_code: %s, msg: %s, job id: %s.", body_err_code, msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=body_err_code, msg=msg)

    def restore_config_node(self):
        """
        功能描述： shard集群config角色恢复主节点
        :return:
        """
        body_err_code, msg = self.restore_obj.restore_config_node()
        LOGGER.debug("Func restore_config_node finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func restore_config_node error, body_err_code: %s, msg: %s, job id: %s.", body_err_code,
                         msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=body_err_code, msg=msg)

    def restore_config_init(self):
        """
        功能描述： shard集群config角色恢复主节点执行重建集群（仅判读后的新位置执行）
        :return:
        """
        body_err_code, msg = self.restore_obj.restore_config_init()
        LOGGER.debug("Func restore_config_init finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func restore_config_init error, body_err_code: %s, msg: %s, job id: %s.", body_err_code, msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=body_err_code, msg=msg)

    def restore_cluster_node(self):
        """
        功能描述： 执行集群恢复子任务主节点
        :return:
        """
        body_err_code, msg = self.restore_obj.restore_cluster_node()
        LOGGER.debug("Func restore_cluster_node finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func restore_cluster_node error, body_err_code: %s, msg: %s, job id: %s.", body_err_code,
                         msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=body_err_code, msg=msg)

    def restore_cluster_init(self):
        """
        功能描述： 执行集群恢复子任务重建集群（仅判读后的新位置执行）
        :return:
        """
        body_err_code, msg = self.restore_obj.restore_cluster_init()
        LOGGER.debug("Func restore_cluster_init finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func restore_cluster_init error, body_err_code: %s, msg: %s, job id: %s.", body_err_code, msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=body_err_code, msg=msg)

    def execute_log_restore(self):
        """
        功能描述： 执行集群恢复子任务时间点恢复
        :return:
        """
        body_err_code, msg = self.restore_obj.execute_log_restore()
        LOGGER.debug("Func execute_log_restore finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func execute_log_restore error, body_err_code: %s, msg: %s, job id: %s.", body_err_code, msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=body_err_code, msg=msg)

    def restore_mongos_nodes(self):
        """
        功能描述： 执行shard集群所有mongos节点恢复
        :return:
        """
        body_err_code, msg = self.restore_obj.restore_mongos_nodes()
        LOGGER.debug("Func restore_mongos_nodes finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func restore_mongos_nodes error, body_err_code: %s, msg: %s, job id: %s.", body_err_code, msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=body_err_code, msg=msg)

    def handle_sub_job_not_exists(self):
        """
        功能描述： 执行子任务找不到的问题
        :return:
        """
        LOGGER.error("Func handle_sub_job_not_exists error, job id: %s.", self.job_id)
        self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=ErrorCode.OPERATE_FAILED.value)

    def post_restore(self):
        """
        功能描述： 后置任务
        """
        body_err_code, msg = self.restore_obj.post_restore()
        LOGGER.debug("Func post_restore finished, job id: %s, pid: %s.", self.job_id, self.pid)
        if body_err_code:
            LOGGER.error("Func post_restore error, body_err_code: %s, msg: %s, job id: %s.", body_err_code, msg,
                         self.job_id)
            self.update_action_result(code=MongoDBCode.FAILED.value, body_err_code=body_err_code, msg=msg)

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

from common.const import ReportDBLabel
from mongodb import LOGGER
from mongodb.comm.const import EnvName, AllStepEnum, MongoSubJob, TaskLabel
from mongodb.job_manager import JobManager
from mongodb.param.restore_param import RestoreParam
from mongodb.service.base_service import (
    MetaService,
    AllowInterfaceMixin,
    PrerequisiteInterfaceMixin,
    ExecuteInterfaceMixin,
    PostInterfaceMixin)
from mongodb.service.restore.mongodb_restore_service import RestoreTask
from mongodb.service.restore.mongodb_single_restore import MetaRestore


class RestoreService(MetaService):
    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)
        self.param = self.gen_param_obj()
        EnvName.DB_USER_NAME = "job_targetEnv_{}auth_authKey"
        EnvName.DB_PASSWORD = "job_targetEnv_{}auth_authPwd"
        EnvName.CUSTOM_SETTINGS = "job_targetEnv_{}auth_authType"
        self.restore_manager = RestoreTask(self.job_manager, self.param)

    def gen_param_obj(self):
        """
        功能描述： 获取参数对象
        """
        param = RestoreParam(self.param_dict)
        return param

    def get_steps(self):
        """
        功能描述： 步骤
        """
        pass


@JobManager.register(AllStepEnum.ALLOW_RESTORE)
class AllowRestoreService(RestoreService, AllowInterfaceMixin):

    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)

    def get_steps(self):
        """
        功能描述：
        1、允许恢复的前置校验
        2、以agent为单位进行分发，只确定是否下发任务
        3、检查agent上所有的实例，只要存在需要操作
        """
        return []


@JobManager.register(AllStepEnum.RESTORE_PREREQUISITE)
class PrerequisiteRestoreService(RestoreService, PrerequisiteInterfaceMixin):

    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)

    def get_steps(self):
        """
        功能描述： 恢复的基本前置校验
        1、检查agent是否满足要求
        2、任意节点抢占
        """
        steps = [
            # 分片集群检查
            self.restore_manager.check_shard_number,
            # 日志恢复检查
            self.restore_manager.check_oplog,
            self.restore_manager.check_mongo_tool,
        ]
        return steps


@JobManager.register(AllStepEnum.RESTORE_GEN_SUB)
class GenSubRestoreService(RestoreService):

    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)
        self.need_report = False
        self.log_detail.update({
            "failed": TaskLabel.GENERATE_SUBJOB_FAIL_LABEL,
            "success": TaskLabel.GENERATE_SUBJOB_SUCCESS_LABEL,
            "start": TaskLabel.EXECUTE_GENERATE_SUBJOB_LABEL
        })

    def get_steps(self):
        """
        功能描述： 拆分恢复子任务
                1、任意节点抢占、哪个agent抢占到需要自己校验
        """
        steps = [
            self.restore_manager.gen_sub_jobs
        ]
        return steps


@JobManager.register(AllStepEnum.RESTORE)
class RestoreExecuteService(RestoreService, ExecuteInterfaceMixin):

    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)
        self.log_detail.update({
            "failed": ReportDBLabel.RESTORE_SUB_FAILED,
            "start": ReportDBLabel.RESTORE_SUB_START_COPY,
            "success": ReportDBLabel.SUB_JOB_SUCCESS
        })

    def get_steps(self):
        """
        功能描述： 执行恢复子任务
        1、各个节点均需要验证
        2、实例级别处理
        config 123 1
        shard /123 1/   1   /1234 1/
        mogos 12
        """
        restore_manager = self.restore_manager
        func_restore_sub_job = {
            # 单实例、复制集群、shard集群所有实例均执行
            MongoSubJob.PRE_RESTORE.value: restore_manager.pre_restore,
            # 仅单实例适用
            MongoSubJob.RESTORE_SINGLE_NODE.value: restore_manager.restore_single_node,

            # 复制集主实例
            MongoSubJob.RESTORE_REPLSET_NODE.value: restore_manager.restore_replset_node,
            # 复制集主实例（new位置）
            MongoSubJob.RESTORE_REPLSET_INIT.value: restore_manager.restore_replset_init,

            # 分片集群config所有实例均执行
            # 分片集群config主节点执行
            MongoSubJob.RESTORE_CONFIG_NODE.value: restore_manager.restore_config_node,
            # 分片集群config 主实例执行（new位置）
            MongoSubJob.RESTORE_CONFIG_INIT.value: restore_manager.restore_config_init,
            # 分片集群各个shard角色主实例：主 从 仲裁
            MongoSubJob.RESTORE_CLUSTER_NODE.value: restore_manager.restore_cluster_node,
            # 分片集群各个shard角色主实例（new位置）
            MongoSubJob.RESTORE_CLUSTER_INIT.value: restore_manager.restore_cluster_init,
            # 复制集群，分片集群各个shard角色主实例、此处需要用实时primary
            MongoSubJob.EXECUTE_LOG_RESTORE.value: restore_manager.execute_log_restore,
            # 分片集群config 主实例执行、此处需要用实时primary (日志恢复)
            MongoSubJob.EXECUTE_CONFIG_LOG_RESTORE.value: restore_manager.execute_log_restore,
            # 分片集群各个shard角色主实例、此处需要用实时primary
            MongoSubJob.EXECUTE_SHARD_LOG_RESTORE.value: restore_manager.execute_log_restore,
            # 分片集群route角色所有实例
            MongoSubJob.RESTORE_MONGOS_NODES.value: restore_manager.restore_mongos_nodes
        }
        sub_job_name = self.param.get_sub_job_name()
        LOGGER.debug("Mongodb sub job restore, start sub job name: %s, job id: %s.", sub_job_name, self.param.job_id)
        if isinstance(sub_job_name, str) and func_restore_sub_job.get(sub_job_name):
            return [func_restore_sub_job.get(sub_job_name)]
        else:
            return [self.restore_manager.handle_sub_job_not_exists]


@JobManager.register(AllStepEnum.RESTORE_POST)
class PostRestoreService(RestoreService, PostInterfaceMixin):

    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)

    def get_steps(self):
        """
        功能描述： 执行恢复子任务后置处理
        1、agent下发下来，
        判断哪些实例在这个节点上
        2、
        """
        return [self.restore_manager.post_restore]

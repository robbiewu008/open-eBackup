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

from common import common
from common.util.exec_utils import su_exec_rm_cmd
from mongodb import LOGGER
from mongodb.comm.const import ErrorCode, MongoDBCode, ParamField, MongoRoles
from mongodb.comm.const import MongoRolesStatus, TMP_CONF_PATH, MongoTool
from mongodb.service.restore.mongodb_full_restore import FullRestore
from mongodb.service.restore.mongodb_oplog_restore import OplogRestore


class MetaRestore:
    def __init__(self, pid, param_obj):
        self.param = param_obj
        self.snapshot_restore = FullRestore(pid, param_obj)
        self.cmd = self.snapshot_restore.cmd
        self.oplog_restore = OplogRestore(pid, param_obj, self.cmd)
        self._local_instances = self.param.get_local_insts_info()
        self.job_id = self.param.job_id
        self.inst_handles = []

    def get_local_role_nodes(self, role_type):
        nodes = self.param.get_local_role_insts(role_type)
        return nodes

    def check_mongo_tool(self):
        """
        功能描述： 校验日志恢复是否存在mongorestore工具是否在全局变量中
        :return:
        """
        # 检查恢复工具mongod是否在全局变量中
        body_err_code, body_err_param, msg = self.snapshot_restore.check_mongo_tool(MongoTool.MONGOD.value)
        if body_err_code:
            return body_err_code, body_err_param, msg
        # 检查恢复类型是否为日志恢复, 若为日志恢复需要校验此参数是否在全局变量中
        restore_type = self.param.get_copy_type
        if restore_type != ParamField.LOG_COPY:
            return MongoDBCode.SUCCESS.value, "", ""
        body_err_code, body_err_param, msg = self.snapshot_restore.check_mongo_tool(MongoTool.MONGORESTORE.value)
        if body_err_code:
            return body_err_code, body_err_param, msg
        return MongoDBCode.SUCCESS.value, "", ""

    def process_replset_base_restore(self, nodes):
        for node in nodes:
            body_err_code, msg = self.snapshot_restore.replset_node_start_new_process(node)
            if body_err_code:
                return body_err_code, msg
        return MongoDBCode.SUCCESS.value, ""

    def gen_sub_job(self):
        """
        功能描述： 拆分子任务
        :return:
        """
        pass

    def pre_restore(self):
        """
        功能描述： 执行恢复子任务1
        :return:
        """
        # 基本恢复检查
        # 检查各个节点实例服务是否已经停止
        if self.snapshot_restore.check_instance_status():
            return ErrorCode.HAVE_INSTANCE_RUNNING.value, "Check instance status error.", []
        # 检查恢复目标目录data权限，判断目录是否存在软连接问题
        if self.snapshot_restore.check_dir_rw_permission():
            return ErrorCode.OPERATE_FAILED.value, "Check dir rw permission error.", []
        # 检查恢复目标用户的是否满足要求
        ret, instance_user = self.snapshot_restore.check_instance_user_status()
        if ret:
            return ErrorCode.INSTANCE_USER_EXECUTE_TOOL_ERROR.value, "Check instance user status error.", [
                instance_user]
        # 修改恢复目标用户的sock地址
        self.snapshot_restore.modify_tmp_sock_permission()
        # 检查副源实例数据库版本是否与目标实例的版本号匹配
        if self.snapshot_restore.check_copy_and_target_version():
            return ErrorCode.ERROR_DIFFERENT_VERSION.value, "Check copy and target version are different.", []
        # 检查port是否被占用
        if self.snapshot_restore.check_port_used():
            return ErrorCode.INSTANCE_PORT_USED.value, "Check port is used error.", []
        return MongoDBCode.SUCCESS.value, "", []

    def restore_single_node(self):
        """
        功能描述： 执行单实例恢复子任务
        :return:
        """
        nodes = self._local_instances
        if len(nodes) == 1 and nodes[0]:
            body_err_code, msg = self.snapshot_restore.single_node_start_process(nodes[0])
            if body_err_code:
                return body_err_code, msg
            else:
                return MongoDBCode.SUCCESS.value, msg
        else:
            return ErrorCode.OPERATE_FAILED.value, "Node is not exist."

    def restore_replset_node(self):
        """
        功能描述： 执行复制集主实例恢复任务
        :return:
        """
        LOGGER.debug("Restore primary node, start cluster primary node process, job id: %s", self.param.job_id)
        nodes = self.get_local_role_nodes(MongoRolesStatus.PRIMARY.value)
        body_err_code, msg = self.process_replset_base_restore(nodes)
        if body_err_code:
            return body_err_code, msg
        LOGGER.debug("Restore secondary node , start cluster secondary node process, job id: %s", self.param.job_id)
        nodes = self.get_local_role_nodes(MongoRolesStatus.SECENDARY.value)
        body_err_code, msg = self.process_replset_base_restore(nodes)
        if body_err_code:
            return body_err_code, msg
        LOGGER.debug("Restore fault node , start cluster fault node process, job id: %s", self.param.job_id)
        nodes = self.get_local_role_nodes(MongoRolesStatus.NONE_TYPE.value)
        body_err_code, msg = self.process_replset_base_restore(nodes)
        if body_err_code:
            return body_err_code, msg
        LOGGER.debug("Restore arbiter node , start cluster arbiter node process, job id: %s", self.param.job_id)
        nodes = self.get_local_role_nodes(MongoRolesStatus.ARBITER.value)
        for node in nodes:
            body_err_code, msg = self.snapshot_restore.arbiter_node_process(node)
            if body_err_code:
                return body_err_code, msg
        LOGGER.debug("Restore replset nodes finished, job id: %s.", self.job_id)
        return MongoDBCode.SUCCESS.value, ""

    def restore_replset_init(self):
        """
        功能描述： 执行复制集主实例重建恢复子任务
        :return:
        """
        LOGGER.info("Mongo restore replset init, job id: %s.", self.job_id)
        primary_nodes = self.get_local_role_nodes(MongoRolesStatus.PRIMARY.value)
        secendary_nodes = self.get_local_role_nodes(MongoRolesStatus.SECENDARY.value)
        secendary_nodes.extend(self.get_local_role_nodes(MongoRolesStatus.NONE_TYPE.value))
        # 预防没有主节点,然后用secondary节点去执行init
        primary_node = None
        if secendary_nodes and len(secendary_nodes) >= 1:
            primary_node = secendary_nodes[0]
        if primary_nodes and len(primary_nodes) >= 1:
            primary_node = primary_nodes[0]
        if not primary_node:
            LOGGER.info("Job %s, primary node is not find", self.job_id)
            return ErrorCode.PRIMARY_NODE_NOT_FOUND.value, "Primary node is not find."
        all_instances = self.param.get_all_instances_dic_info()
        body_err_code, msg = self.snapshot_restore.initiate_cluster(primary_node, all_instances)
        if body_err_code:
            return body_err_code, msg
        return MongoDBCode.SUCCESS.value, ""

    def restore_config_node(self):
        """
        功能描述： 执行恢复子任务
        :return:
        """
        pass

    def restore_config_init(self):
        """
        功能描述： 执行恢复子任务
        :return:
        """
        pass

    def restore_cluster_node(self):
        """
        功能描述： 执行恢复子任务
        :return:
        """
        pass

    def restore_cluster_init(self):
        """
        功能描述： 执行恢复子任务
        :return:
        """
        pass

    def get_mongo_primary_node(self):
        nodes = self._local_instances
        LOGGER.info("Start get mongo primary node.")
        for node in nodes:
            node_inst_uri = node.get(ParamField.HOSTURL, "")
            LOGGER.info("Oplog Node inst uri: %s", node_inst_uri)
            ret, primary_node = self.oplog_restore.check_and_find_replset_primary_role(node)
            if ret:
                return primary_node
        LOGGER.info("Job id: %s is not mongo replset primary role", self.job_id)
        return {}

    def execute_log_restore(self):
        """
        功能描述： 执行恢复子任务
        :return:
        功能描述：执行日志恢复逻辑
        :return: status_code, msg
        """
        restore_type = self.param.get_copy_type()
        LOGGER.info("Oplog restore type is %s", restore_type)
        if restore_type != ParamField.LOG_COPY:
            return MongoDBCode.SUCCESS.value, "Skip log execute restore because restore type is not log"
        LOGGER.info("Check and get replset primary role.")
        primary_node = self.get_mongo_primary_node()
        if not primary_node:
            LOGGER.warn("Can not find replset primary role, job id: %s", self.job_id)
            return MongoDBCode.SUCCESS.value, "can not find replset primary role"
        primary_node_type = primary_node.get(ParamField.SHARD_CLUSTER_TYPE, "")
        LOGGER.info("Primary node type is :%s, job id: %s.", primary_node_type, self.job_id)
        # 副本集或单节点副本集支持日志恢复
        if primary_node_type == MongoRoles.REPLICATION.value or primary_node_type == MongoRoles.SINGLE_NODE_REPL.value:
            body_err_code, msg = self.oplog_restore.oplog_replset_start_process(primary_node)
            if body_err_code:
                return body_err_code, msg
            else:
                return MongoDBCode.SUCCESS.value, msg
        elif primary_node_type == MongoRoles.SHARD.value:
            LOGGER.info("Job:%s, Start Shard oplog restore.", self.job_id)
            body_err_code, msg = self.oplog_restore.oplog_shard_start_process(primary_node)
            if body_err_code:
                return body_err_code, msg
            else:
                return MongoDBCode.SUCCESS.value, msg

        return MongoDBCode.FAILED.value, "Execute log restore falied"

    def restore_mongos_nodes(self):
        """
        功能描述： 恢复mongos nodes
        :return:
        """
        pass

    def post_restore(self):
        """
        功能描述： 后置任务,删除生成的conf启动文件
        参数：
        """
        LOGGER.info('Start post restore! job id: %s', self.job_id)
        conf_dir = os.path.join(TMP_CONF_PATH, self.job_id)
        if not su_exec_rm_cmd(conf_dir, check_white_black_list_flag=False):
            msg = f"Mongo post restore failed! job id: {self.job_id}"
            LOGGER.error()
            return ErrorCode.OPERATE_FAILED.value, msg
        return MongoDBCode.SUCCESS.value, "Post restore success!"


class SingleInstanceRestore(MetaRestore):
    def __init__(self, pid, param_obj):
        super().__init__(pid, param_obj)
        single_type = param_obj.param_dict.get("job", {}).get("targetEnv", {}).get("extendInfo", {}).get("singleType",
                                                                                                         "single")
        self.support_oplog = True if single_type == MongoRoles.SINGLE_NODE_REPL else False

    def gen_sub_job(self):
        """
        功能描述： 拆分子任务
        :return:
        """
        jobs = []
        nodes = self._local_instances
        if self.support_oplog:
            # 单节点副本集的恢复采用副本集的方法拆分子任务,开启oplog,支持日志副本恢复
            self.snapshot_restore.gen_replset_sub_job(nodes, jobs)
        else:
            self.snapshot_restore.gen_single_sub_job(nodes, jobs)
        return jobs

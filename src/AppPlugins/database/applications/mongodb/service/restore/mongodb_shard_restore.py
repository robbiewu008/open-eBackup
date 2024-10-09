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

from common.util.exec_utils import su_exec_rm_cmd
from mongodb import LOGGER
from mongodb.comm.const import ErrorCode, MongoDBCode, MongoSubJob, ParamField, MongoRoles, MongoTool
from mongodb.comm.utils import sort_nodes, get_mkdir_user
from mongodb.service.restore.mongodb_single_restore import MetaRestore


class ShardRestore(MetaRestore):
    def __init__(self, pid, param_obj):
        super().__init__(pid, param_obj)
        self.restore_type = param_obj.get_copy_type()
        self.pid = pid
        self.config_nodes = self.parse_local_node_list("config")
        self.shard_nodes = self.parse_local_node_list("shard")
        self.mongos_nodes = self.parse_local_node_list("mongos")

    def gen_sub_job(self):
        """
        功能描述： 拆分子任务
        :return:
        """
        jobs = []
        self.snapshot_restore.gen_mongos_sub_job(jobs)
        return jobs

    def restore_config(self, *args):
        for node in sort_nodes(self.config_nodes):
            ret, msg = self.restore_data(node, "0", args)
            if ret:
                return ret, msg
        return MongoDBCode.SUCCESS.value, ""

    def restore_config_node(self):
        ret, msg = self.restore_config("replication", "sharding")
        return ret, msg

    def restore_cluster_node(self):
        ret, msg = self.restore_shard("replication", "sharding")
        return ret, msg

    def restore_config_init(self):
        """
        进入config主节点实例，执行rs.init初始化实例信息，仅恢复到新位置执行
        :return:
        """
        return self.base_restore_init("config")

    def execute_log_restore(self):
        sub_job_name = self.param.get_sub_job_name()
        log_restore_func_map = {
            MongoSubJob.EXECUTE_CONFIG_LOG_RESTORE.value: self.execute_config_log_restore,
            MongoSubJob.EXECUTE_SHARD_LOG_RESTORE.value: self.execute_shard_log_restore
        }
        execute_func = log_restore_func_map.get(sub_job_name)
        if execute_func is None:
            return ErrorCode.OPERATE_FAILED.value, "Not support sub job."
        code, msg = execute_func()
        return code, msg

    def get_primary_node(self, cluster_type):
        nodes = self._local_instances
        LOGGER.info("Start get mongo primary node.")
        primary_nodes = []
        for node in nodes:
            node_role = node.get(ParamField.SHARD_CLUSTER_TYPE, "")
            if node_role != cluster_type:
                continue
            node_inst_uri = node.get(ParamField.HOSTURL, "")
            LOGGER.info("Oplog Node inst uri: %s", node_inst_uri)
            ret, primary_node = self.oplog_restore.check_and_find_replset_primary_role(node)
            if ret:
                primary_nodes.append(primary_node)
        return primary_nodes

    def execute_config_log_restore(self):
        """
        恢复配置实例日志，仅日志恢复需要在主节点执行
        :return:
        """
        if self.restore_type.value != "log":
            return MongoDBCode.SUCCESS.value, ""
        primary_nodes = self.get_primary_node("config")
        for primary in primary_nodes:
            if primary.get(ParamField.SHARD_CLUSTER_TYPE) == MongoRoles.CONFIG.value:
                body_err_code, msg = self.oplog_restore.oplog_replset_start_process(primary)
                if body_err_code:
                    return body_err_code, msg

        return MongoDBCode.SUCCESS.value, ""

    def execute_shard_log_restore(self):
        if self.restore_type.value != "log":
            return MongoDBCode.SUCCESS.value, ""
        primary_nodes = self.get_primary_node("shard")
        for primary in primary_nodes:
            if primary.get(ParamField.SHARD_CLUSTER_TYPE) == MongoRoles.SHARD.value:
                body_err_code, msg = self.oplog_restore.oplog_shard_start_process(primary)
                if body_err_code:
                    return body_err_code, msg

        return MongoDBCode.SUCCESS.value, ""

    def restore_shard(self, *args):
        for node in sort_nodes(self.shard_nodes):
            shard_idx = node.get("shardIndex")
            ret, msg = self.restore_data(node, str(shard_idx), args)
            if ret:
                return ret, msg
        return MongoDBCode.SUCCESS.value, ""

    def restore_cluster_init(self):
        return self.base_restore_init("shard")

    def base_restore_init(self, cluster_type):
        """
        shard主节点执行rs.init初始化实例信息，仅恢复到新位置需要执行
        :return:
        """
        primary_nodes = []
        nodes = self.param.get_local_insts_info()
        for node in nodes:
            if node.get("stateStr") == "PRIMARY" and node.get("shardClusterType") == cluster_type:
                primary_nodes.append(node)
        all_nodes = self.param.get_nodes_info()
        for node in primary_nodes:
            self.restore_init_primary_node(all_nodes, cluster_type, node)
        return MongoDBCode.SUCCESS.value, "mongo local instance start success."

    def restore_init_primary_node(self, all_nodes, cluster_type, node):
        instance_infos = []
        for _, instance_info in all_nodes.items():
            for instance in instance_info:
                if instance.get("shardClusterType") != cluster_type:
                    continue
                if instance.get("instanceNameInfos") != node.get("instanceNameInfos"):
                    continue
                instance_infos.append(instance)
        self.snapshot_restore.initiate_cluster(node, instance_infos)

    def restore_mongos_nodes(self):
        """
        子任务：启动mongos，每个节点都需要执行，无顺序要求
        :return:
        """
        for node in self.mongos_nodes:
            LOGGER.debug("Start mongos restore, job id: %s", self.job_id)
            conf_path = self.snapshot_restore.build_parsed_data_to_conf(node=node, mongo_type=MongoTool.MONGOS.value)
            if not conf_path:
                return ErrorCode.PARSE_YAML_FILE_ERROR.value, "Mongo local instance start conf file path is not exist."
            mongos_bin_dir = self.param.get_mongo_bin_path(ParamField.BIN_PATH.value, MongoTool.MONGOS.value, node)
            ret, out_msg = self.cmd.start_up_mongos_instance(mongos_bin_dir, conf_path)
            if not ret:
                LOGGER.error("Failed to start the routing server, err: %s. pid: %s, job id: %s", out_msg, self.pid,
                             self.job_id)
                return ErrorCode.INSTANCE_START_ERROR.value, ""
            with self.param.db_command(node) as mongo:
                mongo.admin_command("balancerStart")
        LOGGER.debug("Execute mongos success, pid: %s, job id: %s", self.pid, self.job_id)
        return MongoDBCode.SUCCESS.value, ""

    def restore_data(self, node: dict, relative_path: str, args):
        ret, ret_msg = self.snapshot_restore.handle_single_node_pre_process(node, relative_path)
        if ret:
            return ret, ret_msg
        origin_path = node.get("dataPath")
        lock_file = os.path.join(origin_path, "mongod.lock")
        # 启动单实例对象信息;
        ret, ret_msg = self.start_up_instance_for_args(node=node, lock_file=lock_file, args=args)
        if not ret:
            return ErrorCode.INSTANCE_START_ERROR.value, ret_msg
        # 删除local数据库信息
        ret, ret_msg = self.snapshot_restore.drop_local_database_update_info(node=node, relative_path=relative_path)
        if ret:
            return ErrorCode.CLUSTER_INITIATE_ERROR.value, ret_msg
        # 重新启动数据库信息
        body_err_code, msg = self.snapshot_restore.delete_mongod_lock_file(node)
        if body_err_code:
            return body_err_code, msg
        ret, ret_msg = self.start_up_instance_for_args(node=node, lock_file=lock_file)

        cluster_type = node.get("shardClusterType")
        node_type = node.get("stateStr")
        if not ret:
            LOGGER.error("Failed to start instance, node: %s:%s. msg: %s, pid: %s, job id: %s", cluster_type, node_type,
                         ret_msg, self.pid, self.job_id)
            return ErrorCode.INSTANCE_START_ERROR.value, ""
        LOGGER.debug("Succeeded in starting instance, node: %s:%s.pid: %s, job id: %s", cluster_type, node_type,
                     self.pid, self.job_id)
        return MongoDBCode.SUCCESS.value, ret_msg

    def start_up_instance_for_args(self, node, lock_file, args=None):
        su_exec_rm_cmd(lock_file, check_white_black_list_flag=False)
        if args:
            conf_path = self.snapshot_restore.build_parsed_data_to_conf(*args, node=node)
        else:
            conf_path = self.snapshot_restore.build_parsed_data_to_conf(node=node)
        if not conf_path:
            return ErrorCode.PARSE_YAML_FILE_ERROR.value, "Mongo local instance start conf file path is not exist."
        user = get_mkdir_user(self.param.get_db_path(node))
        _, instance_user = self.param.get_start_instance_user()
        user = instance_user if instance_user else user
        mongod_bin_dir = self.param.get_mongo_bin_path(ParamField.BIN_PATH.value, MongoTool.MONGOD.value, node)
        return self.cmd.start_up_instance(mongod_bin_dir, conf_path, user)

    def parse_local_node_list(self, cluster_type: str):
        """
        解析节点信息，获取本端节点信息
        :return:
        """
        ret_list = []
        for node in self._local_instances:
            if node.get("shardClusterType") != cluster_type:
                continue
            ret_list.append(node)
        LOGGER.debug("Node list length(%s) in local host", len(ret_list))
        return ret_list

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

import os.path

from mongodb import LOGGER
from mongodb.comm.const import MongoDBCode, ErrorCode, ParamField, MongoRoles, DefaultValue, MongoTool
from mongodb.comm.utils import check_node_server_type


class OplogRestore:
    """
        oplog恢复基础功能实现类
    """

    def __init__(self, pid, param_obj, cmd):
        self.param = param_obj
        self.pid = pid
        self.job_id = self.param.job_id
        self.cmd = cmd

    def execute_oplog_normal_restore(self, node, log_copies_data_paths):
        """
        功能描述： 执行oplog普通恢复: mongorestore -h ip:port --oplogReplay "日志副本1-n的元数据路径"
        :param node: 主节点
        :param log_copies_data_paths: 日志副本路径列表
        :return: 错误码
        """
        host_url = node.get(ParamField.HOSTURL, "")
        LOGGER.info("Job id: %s log copies data paths length is: %s.", self.job_id, len(log_copies_data_paths))
        try:
            for log_copie_path in log_copies_data_paths:
                LOGGER.debug("Job id: %s log copies data, log copy path is: %s.", self.job_id, log_copie_path)
                if not os.path.exists(log_copie_path):
                    LOGGER.error("Job id: %s Oplog copie data path is not exist, log copy path: %s.", self.job_id,
                                 log_copie_path)
                    return ErrorCode.LOG_COPIES_DATA_PATH_NOT_EXIST.value, "Oplog copy data path is not exist."
                mongorestore_bin_dir = self.param.get_mongo_bin_path(ParamField.MOBGODUMP_BIN_PATH.value,
                                                                     MongoTool.MONGORESTORE.value, node)
                ret, res_cont = self.cmd.oplog_restore_copy(mongorestore_bin_dir, host_url, log_copie_path)
                LOGGER.debug("Job id: %s execute mongorestore command res_cont: %s", self.job_id, res_cont)
                if not ret:
                    LOGGER.error("Job: %s excute oplog restore cmd result failed.", self.job_id)
                    return ErrorCode.EXECUTE_OPLOG_NORMAL_RESTORE_FAILED.value, "execute oplog normal restore failed."
        except Exception:
            LOGGER.error("Job id: %s execute oplog normal restore raise exception.", self.job_id)
            return ErrorCode.EXECUTE_OPLOG_NORMAL_RESTORE_FAILED.value, "execute oplog normal restore failed."
        return MongoDBCode.SUCCESS.value, ""

    def oplog_replset_start_process(self, primary_node):
        """
        执行副本日志恢复
        :param primary_node:
        :return:
        """
        LOGGER.info("Job id: %s Start oplog replset restore.", self.job_id)
        # 获取到node节点上所有的副本信息
        sorted_oplog_copie_dirs = self.param.get_oplog_copie_dirs()
        if not sorted_oplog_copie_dirs:
            return ErrorCode.EXECUTE_OPLOG_NORMAL_RESTORE_FAILED.value, "Mongo oplog data repository is empty."
        log_copies_data_path = self.get_replset_copies_chain_info(sorted_oplog_copie_dirs, primary_node)
        if not log_copies_data_path:
            return ErrorCode.LOG_COPIES_DATA_PATH_NOT_EXIST.value, "Get replet copies chain path is empty.."
        err_code, msg = self.execute_oplog_normal_restore(primary_node, log_copies_data_path)
        if err_code:
            return err_code, msg
        return MongoDBCode.SUCCESS.value, ""

    def check_and_find_replset_primary_role(self, node):
        """
        检查primary节点hostUrl连接到实例, rs.status()命令实时查
        :param node: 节点
        :return: 检验结果
        """
        LOGGER.info("Job id: %s Start check replset primary role.", self.job_id)
        host_url = node.get(ParamField.HOSTURL, "")
        with self.param.db_command(node) as mongo:
            status = mongo.get_replset_status()
            for cnf_member in status['members']:
                if cnf_member.get("stateStr") == "PRIMARY" and cnf_member.get("name") == host_url:
                    return True, node
            LOGGER.error("Job id: %s Check replset role is primary failed.", self.job_id)
        return False, {}

    def check_shard_primary_role(self, node):
        """
        1.如果是复制集: 检查primary节点(根据(node中的hostUrl信息)连接进去到实例, rs.status()命令实时查一下)
        2.如果是Shard: 需要检查是否是shard角色()
        :param node:
        :return:
        """
        LOGGER.info("Job id: %s Start check shard primary role.", self.job_id)
        with self.param.db_command(node) as mongo:
            line_opts = mongo.get_cmd_line_opts()
            server_type = check_node_server_type(line_opts)
        if server_type != MongoRoles.SHARD:
            return ErrorCode.EXECUTE_OPLOG_TIMESTAMP_RESTORE_FAILED.value, "Node server type is not shard."
        LOGGER.info("Job id: %s Check node server type is shard.", self.job_id)
        return MongoDBCode.SUCCESS.value, ""

    def oplog_shard_start_process(self, primary_node):
        LOGGER.info("Job id: %s Start oplog shard restore.", self.job_id)
        body_err_code, msg = self.oplog_replset_start_process(primary_node)
        if body_err_code:
            return body_err_code, msg
        return MongoDBCode.SUCCESS.value, ""

    def get_replset_copies_chain_info(self, sorted_oplog_copie_dirs, node):
        """
        功能描述： 获取oplog元数据的路劲
        元数据目录 + log_directory + copy_id + "0" + local + oplog.rs.bson
        """
        LOGGER.info("Job id: %s Start get sorted copies chain path.", self.job_id)
        log_copies_data_path = []
        shard_index = node.get("shardIndex", "0")
        if not shard_index:
            shard_index = "0"
        for copie_dir in sorted_oplog_copie_dirs:
            oplog_file = os.path.join(copie_dir, shard_index, DefaultValue.LOCAL_DB.value,
                                      "oplog.rs.bson")
            log_copies_data_path.append(oplog_file)
        return log_copies_data_path

    def get_restore_timestamp(self):
        return self.param.get(ParamField.JOB, {}).get(ParamField.EXTEND_INFO, {}) \
            .get(ParamField.RESTORE_TIMESTAMP, "")

    def execute_oplog_timestamp_restore(self, node, log_copies_data_paths, sorted_copies):
        """
        功能描述： 执行oplog时间点恢复 (最后一个时间点的命令)
        {Mongodump工具中mongorestore所在目录}  -h localhost:27018 --oplogReplay   --oplogLimit {需要恢复到的日志副本时间戳}:{
        该时间戳对应的操作序列号}   "{日志副本n的元数据路径}"
        """
        host_url = node.get(ParamField.HOSTURL, "")
        last_log_copies_data_path = log_copies_data_paths[-1]
        # endLsn "1623547821:14"
        last_copie_end_lsn = sorted_copies[-1].get(ParamField.LAST_LSN)
        last_copie_timestamp = last_copie_end_lsn.split(":")[0]
        LOGGER.info("Job id: %s Mongo oplog last copie timestamp:%s.", last_copie_timestamp, self.job_id)
        mongorestore_bin_dir = self.param.get_mongo_bin_path(ParamField.MOBGODUMP_BIN_PATH.value,
                                                             MongoTool.MONGORESTORE.value, node)
        ret, res_cont = self.cmd.oplog_restore_timestamp(mongorestore_bin_dir, host_url, last_copie_end_lsn,
                                                         last_log_copies_data_path)
        if not ret:
            return ErrorCode.EXECUTE_OPLOG_NORMAL_RESTORE_FAILED.value, \
                   f"Execute oplog timestamp restore failed: {res_cont}"
        return MongoDBCode.SUCCESS.value, ""

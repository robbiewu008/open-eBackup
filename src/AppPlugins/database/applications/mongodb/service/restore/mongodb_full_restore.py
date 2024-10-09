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
import os.path
import stat
import time

import yaml

from common import common
from common.common import check_port_is_used, check_del_file
from common.common_models import SubJobModel
from common.const import EnumPathType, SubJobPolicyEnum, SubJobTypeEnum
from common.file_common import check_file_or_dir, change_path_permission
from common.util.backup import backup, query_progress
from common.util.exec_utils import exec_mkdir_cmd, su_exec_rm_cmd
from common.util.common_utils import get_uid_by_os_user, get_gid_by_os_user
from mongodb import LOGGER
from mongodb.comm.cmd import Cmd
from mongodb.comm.const import MongoDBCode, ErrorCode, MongoDBCopyDataRet, TMP_CONF_PATH, MongoRolesStatus, \
    MongoSubJob, MongoRoles, MongoTool, INSTANCE_DIRECTORY, TMP_MONGODB_SOCK
from mongodb.comm.const import ParamField
from mongodb.comm.exception_err import DBAuthenticationError, DBOperationError, DBConnectionError
from mongodb.comm.mongod_relationship import MongodCommandRelationship
from mongodb.comm.mongos_relationship import MongosCommandRelationship
from mongodb.comm.utils import get_mkdir_user, check_real_path


class FullRestore:
    """
        全量快照恢复基础功能实现类
    """

    def __init__(self, pid, param_obj):
        self.param = param_obj
        self.pid = pid
        self.instance_directory = INSTANCE_DIRECTORY
        self.cmd = Cmd(pid)
        self._local_instances = self.param.get_local_insts_info()
        self.job_id = self.param.job_id

    @classmethod
    def build_sub_job(cls, job_id, job_name, job_priority, agent_id, job_info=""):
        sub_job = SubJobModel(
            jobId=job_id,
            subJobId="",
            jobType=SubJobTypeEnum.BUSINESS_SUB_JOB.value,
            jobName=job_name,
            jobPriority=job_priority, policy=SubJobPolicyEnum.FIXED_NODE.value, ignoreFailed=False,
            execNodeId=agent_id, jobInfo=job_info).dict(by_alias=True)
        return sub_job

    @classmethod
    def get_data_path_file_permission(cls, data_path):
        """
        获取实例原来路径的文件权限
        :param data_path:
        :return:
        """
        # 获取文件权限
        return os.stat(data_path).st_mode

    @classmethod
    def get_data_path_user_and_group(cls, data_path):
        """
        获取实例原来路径的用户和组
        :param data_path:
        :return:
        """
        stat_info = os.stat(data_path)
        return stat_info.st_uid, stat_info.st_gid

    @classmethod
    def get_parsed_data_filed_paths(cls, dictionary, path):
        """
        获取到指定路径的dict
        :param dictionary:
        :param path: 'storage.journal.enabled',
        :return: dict
        """
        for item in path.split("."):
            dictionary = dictionary[item]
        return dictionary

    @classmethod
    def change_file_own_and_mod(cls, data_path_permission_num, file_path, gid, uid):
        """
        更改文件的用户,用户组,文件权限为恢复目标路径
        :param data_path_permission_num:
        :param file_path:
        :param gid:
        :param uid:
        :return:
        """
        if not check_real_path(file_path):
            raise Exception("change_file_own_and_mod failed")
        stat_info = os.stat(file_path)
        if stat_info.st_uid != uid or stat_info.st_gid != gid:
            os.lchown(file_path, uid, gid)
        file_path_permission_num = os.stat(file_path).st_mode
        if data_path_permission_num != file_path_permission_num:
            change_path_permission(file_path, mode=data_path_permission_num)

    def check_mongo_tool(self, mongo_tool_type):
        """
        功能描述：检查复制集群是否存在mongorestore全局变量
        """
        mongodump_bin_dir = self.param.get_mongo_bin_path(ParamField.MOBGODUMP_BIN_PATH.value,
                                                          MongoTool.MONGORESTORE.value, self._local_instances[0])
        try:
            ret, result = self.cmd.check_mongo_tool(mongodump_bin_dir)
        except Exception as ex:
            LOGGER.error("Check mongo tool failed, mongo_tool_type: %s, ex: %s, type: %s, job id: %s.",
                         mongo_tool_type, ex, type(ex), self.job_id)
            return ErrorCode.MONGO_TOOL_NOT_IN_ENV_PATH.value, mongo_tool_type, "Check mongo tool failed, ex: %s." % ex
        if not ret:
            LOGGER.error("Check mongo tool failed, mongo_tool_type: %s, result: %s, job id: %s.",
                         mongo_tool_type, result, self.job_id)
            return ErrorCode.MONGO_TOOL_NOT_IN_ENV_PATH.value, mongo_tool_type, \
                "Check mongo tool failed, ret: %s." % result
        return MongoDBCode.SUCCESS.value, "", ""

    def gen_single_sub_job(self, nodes, sub_jobs, priority=1):
        for node in nodes:
            shard_cluster_type = node.get(ParamField.CLUSTER_INSTANCE_TYPE, "")
            node_url = node.get(ParamField.HOSTURL, "")
            agent_id = node.get(ParamField.ID, "")
            node_ip, _ = node_url.split(":")
            job_info = "%s %s" % (node_ip, shard_cluster_type)
            pre_job = self.build_sub_job(self.job_id, MongoSubJob.PRE_RESTORE.value, priority, agent_id, job_info)
            sub_jobs.append(pre_job)
            sub_job = self.build_sub_job(self.job_id, MongoSubJob.RESTORE_SINGLE_NODE.value, priority + 1, agent_id,
                                         job_info)
            sub_jobs.append(sub_job)

    def gen_mongos_sub_job(self, sub_jobs):
        for uuid, _ in self.param.get_target_env_nodes().items():
            LOGGER.debug("Gen shard cluster sub jobs, uuid : %s, job id: %s.", uuid, self.job_id)
            pre_job = self.build_sub_job(self.param.job_id, MongoSubJob.PRE_RESTORE.value, 1, uuid)
            sub_jobs.append(pre_job)
            restore_config_node = self.build_sub_job(self.param.job_id, MongoSubJob.RESTORE_CONFIG_NODE.value, 2, uuid)
            sub_jobs.append(restore_config_node)
            if uuid in self.param.get_config_primary():
                LOGGER.debug("Gen shard cluster sub job:restore_config_init, uuid : %s, job id: %s.", uuid, self.job_id)
                restore_config_init = self.build_sub_job(self.param.job_id, MongoSubJob.RESTORE_CONFIG_INIT.value, 3,
                                                         uuid)
                sub_jobs.append(restore_config_init)
            execute_config_log_restore = self.build_sub_job(self.param.job_id,
                                                            MongoSubJob.EXECUTE_CONFIG_LOG_RESTORE.value, 4, uuid)
            sub_jobs.append(execute_config_log_restore)
            restore_cluster_node = self.build_sub_job(self.param.job_id, MongoSubJob.RESTORE_CLUSTER_NODE.value, 5,
                                                      uuid)
            sub_jobs.append(restore_cluster_node)
            if uuid in self.param.get_shard_primary():
                LOGGER.debug("Gen shard cluster sub job:restore_cluster_init, uuid : %s, job id: %s.", uuid,
                             self.job_id)
                restore_cluster_init = self.build_sub_job(self.param.job_id, MongoSubJob.RESTORE_CLUSTER_INIT.value, 6,
                                                          uuid)
                sub_jobs.append(restore_cluster_init)
            execute_shard_log_restore = self.build_sub_job(self.param.job_id,
                                                           MongoSubJob.EXECUTE_SHARD_LOG_RESTORE.value, 7, uuid)
            sub_jobs.append(execute_shard_log_restore)
            restore_mongos_nodes = self.build_sub_job(self.param.job_id, MongoSubJob.RESTORE_MONGOS_NODES.value, 8,
                                                      uuid)
            sub_jobs.append(restore_mongos_nodes)

    def gen_replset_sub_job(self, nodes, sub_jobs, priority=1):
        agent_ids = set()
        primary_agent_ids = set()
        secondary_agent_ids = set()
        arbiter_agent_ids = set()
        for node in nodes:
            agent_id = node.get(ParamField.ID, "")
            agent_ids.add(agent_id)
            role = node.get(ParamField.ROLE, str(MongoRolesStatus.NONE_TYPE.value))
            if role == str(MongoRolesStatus.PRIMARY.value):
                primary_agent_ids.add(agent_id)
            elif role == str(MongoRolesStatus.ARBITER.value):
                arbiter_agent_ids.add(agent_id)
            else:
                secondary_agent_ids.add(agent_id)
        LOGGER.debug(
            "Replication cluster gen subs, primary agent ids: %s, secondary ids: %s, arbiter ids: %s. job_id:%s",
            list(primary_agent_ids), list(secondary_agent_ids), list(arbiter_agent_ids), self.job_id)
        for agent_id in agent_ids:
            pre_restore_job = self.build_sub_job(self.job_id, MongoSubJob.PRE_RESTORE.value, priority, agent_id)
            sub_jobs.append(pre_restore_job)
            restore_replset_node_job = self.build_sub_job(self.job_id, MongoSubJob.RESTORE_REPLSET_NODE.value,
                                                          priority + 1, agent_id)
            sub_jobs.append(restore_replset_node_job)
            execute_log_restore_job = self.build_sub_job(self.job_id, MongoSubJob.EXECUTE_LOG_RESTORE.value,
                                                         priority + 3, agent_id)
            sub_jobs.append(execute_log_restore_job)
        if len(primary_agent_ids) >= 1:
            agent_id = list(primary_agent_ids)[0]
            restore_replset_init_job = self.build_sub_job(self.job_id, MongoSubJob.RESTORE_REPLSET_INIT.value,
                                                          priority + 2, agent_id)
            sub_jobs.append(restore_replset_init_job)
        else:
            if len(secondary_agent_ids) < 1:
                return False
            agent_id = list(secondary_agent_ids)[0]
            restore_replset_init_job = self.build_sub_job(self.job_id, MongoSubJob.RESTORE_REPLSET_INIT.value,
                                                          priority + 2, agent_id)
            sub_jobs.append(restore_replset_init_job)
        return True

    def backup_db_file(self, origin_data_path, data_path):
        """
        复制备份数据到data_path
        """
        if not os.path.exists(origin_data_path) or not data_path:
            LOGGER.error("Backup db file failed, data path not exists, job id: %s.", self.job_id)
            return False
        # 判断文件夹是否存在,不存在创建
        if not os.path.exists(data_path):
            exec_mkdir_cmd(data_path)
        # 获取到job_id
        result = backup(self.param.job_id, origin_data_path, data_path)
        if not result:
            LOGGER.error("End to backup db file failed, job id: %s.", self.job_id)
            return False
        # 每间隔2s，查看一次数据拷贝情况，超过2000s，默认数据拷贝超时
        query_progress_interval = 2
        interval = 0
        while True:
            interval += 1
            time.sleep(query_progress_interval)
            status, progress, data_size = query_progress(self.param.job_id)
            if status != MongoDBCopyDataRet.RUNNING:
                LOGGER.debug("Backup not running, interval: %s, job id: %s.", interval, self.param.job_id)
                return True

    def get_local_instances_status(self, online_instance, offline_instance, auth_failed_instance):
        """
        功能描述： 转换本地实例状态信息
        :return:
        """
        for inst in self._local_instances:
            uri = inst.get(ParamField.HOSTURL)
            try:
                with self.param.db_command(inst) as mongo:
                    mongo.connect()
            except DBConnectionError:
                offline_instance.append(uri)
            except DBAuthenticationError:
                auth_failed_instance.append(uri)
            else:
                online_instance.append(uri)

    def handle_single_node_pre_process(self, node, agent_path=ParamField.SINLE_DATA_PATH):
        """
        功能描述： 启动实例前置操作
        """
        # 清理目标位置data目录
        flag, msg = self.clean_data_dir(node)
        if not flag:
            return ErrorCode.CLEAN_DATA_DIR_ERROR.value, msg
        # 拷贝全量副本数据到目标实例的数据目录下
        body_err_code, msg = self.copy_data_to_dir(node, agent_path)
        if body_err_code:
            return body_err_code, msg
        # 修改目录权限：宿主的权限
        body_err_code, msg = self.modify_dir_permission(node)
        if body_err_code:
            return body_err_code, msg
        # 检查mongod.lock文件是否存在，有则删除mongod.lock文件
        body_err_code, msg = self.delete_mongod_lock_file(node)
        if body_err_code:
            return body_err_code, msg
        return MongoDBCode.SUCCESS.value, ""

    def single_node_start_process(self, node):
        """
        功能描述： 以重新启动的方式启动复制集节点(原位置)
        启动：主 、从、仲裁启动实例
        """
        # 启动实例前置操作
        body_err_code, msg = self.handle_single_node_pre_process(node)
        if body_err_code:
            return body_err_code, msg
        # 全部默认不开启认证，注释掉keyFile,auth等参数
        conf_file_path = self.build_parsed_data_to_conf(node=node)
        # 启动此节点
        body_err_code, msg = self.start_local_instance(conf_file_path, node)
        if body_err_code:
            return body_err_code, msg
        # 检查实例进程状态
        body_err_code, msg = self.connect_instance(node)
        if body_err_code:
            return body_err_code, msg
        return MongoDBCode.SUCCESS.value, ""

    def replset_node_start_new_process(self, node):
        """
        功能描述： 以重建的方式启动复制集节点(新位置)
        """
        # 修改配置参数前置处理
        body_err_code, msg = self.handle_single_node_pre_process(node)
        if body_err_code:
            return body_err_code, msg
        # 全部默认不开启认证，注释掉replSet,security等参数
        conf_file_path = self.build_parsed_data_to_conf("replication", node=node)
        # 启动此节点
        body_err_code, msg = self.start_local_instance(conf_file_path, node)
        if body_err_code:
            return body_err_code, msg
        # 进入此节点, 删除local数据库, 使用本机关闭实例
        try:
            with self.param.db_command(node) as mongo:
                mongo.drop_local_database()
            LOGGER.info(f'replset node_start_new_process drop local database')
            user = get_mkdir_user(self.param.get_db_path(node))
            _, instance_user = self.param.get_start_instance_user()
            user = instance_user if instance_user else user
            mongod_bin_dir = self.param.get_mongo_bin_path(ParamField.BIN_PATH.value, MongoTool.MONGOD.value, node)
            self.cmd.shutdown_instance(mongod_bin_dir, user, self.param.get_db_path(node))
        except DBConnectionError:
            LOGGER.error("DBConnectionError to handle db, job_id: %s", self.job_id)
            return ErrorCode.CONNECT_TO_DB_ERROR.value, msg
        except DBAuthenticationError:
            msg = "DBAuthenticationError to handle db, job_id: %s" % self.job_id
            LOGGER.error("DBAuthenticationError to handle db, job_id: %s", self.job_id)
            return ErrorCode.ERROR_AUTH.value, msg
        except DBOperationError as ex:
            msg = "DBOperationError to handle node, job_id: %s, error: %s" % (self.job_id, ex)
            return ErrorCode.CONNECT_TO_DB_ERROR.value, msg
        except Exception as ex:
            msg = "Exception to connect node, job_id: %s, error: %s" % (self.job_id, ex)
            return ErrorCode.OPERATE_FAILED.value, msg
        # 默认不开启认证，放开replSet参数, 继续保留注释keyFile,auth等参数
        LOGGER.debug("Start to build parsed data to conf, job_id: %s", self.job_id)
        conf_file_path = self.build_parsed_data_to_conf(node=node)
        # 启动此节点
        LOGGER.debug("Start to start_local_instance, job_id: %s", self.job_id)
        body_err_code, msg = self.delete_mongod_lock_file(node)
        if body_err_code:
            return body_err_code, msg
        body_err_code, msg = self.start_local_instance(conf_file_path, node)
        LOGGER.debug("End to start_local_instance, job_id: %s", self.job_id)
        if body_err_code:
            return body_err_code, msg
        # 检查实例进程状态
        error_code, msg = self.connect_instance(node)
        LOGGER.debug("End to connect_instance, job_id: %s", self.job_id)
        if error_code:
            return error_code, msg
        return MongoDBCode.SUCCESS.value, ""

    def arbiter_node_process(self, node):
        """
        功能描述： 仲裁节点的实例启动模式
        """
        # 清理目标位置data目录
        flag, msg = self.clean_data_dir(node)
        if not flag:
            return ErrorCode.CLEAN_DATA_DIR_ERROR.value, msg
        # 修改目录权限：宿主的权限
        body_err_code, msg = self.modify_dir_permission(node)
        if body_err_code:
            return body_err_code, msg
        # 全部默认不开启认证，注释掉keyFile,auth等参数
        conf_file_path = self.build_parsed_data_to_conf(node=node)
        # 启动此节点
        body_err_code, msg = self.start_local_instance(conf_file_path, node)
        if body_err_code:
            return body_err_code, msg
        # 检查实例进程状态
        body_err_code, msg = self.connect_instance(node)
        if body_err_code:
            return body_err_code, msg
        return MongoDBCode.SUCCESS.value, ""

    def check_instance_status(self):
        """
        功能描述： 检查实例状态是否均处于离线状态
        """
        online_instance = []
        offline_instance = []
        auth_failed_instance = []
        self.get_local_instances_status(online_instance, offline_instance, auth_failed_instance)
        if online_instance or auth_failed_instance:
            LOGGER.error("Check instance status error, online instance: %s, auth failed instance: %s, "
                         "offline_instance: %s, job_id: %s.",
                         online_instance, auth_failed_instance, offline_instance, self.job_id)
            return True
        return False

    def check_copy_and_target_version(self):
        """
        功能描述： 检查源副本与目标环境的版本号是否一致
        """
        mongod_bin_dir = self.param.get_mongo_bin_path(ParamField.BIN_PATH.value,
                                                       MongoTool.MONGOD.value, self._local_instances[0])
        ret, res_cont = self.cmd.check_mongo_version(mongod_bin_dir)
        if not ret:
            return True
        version_split = res_cont.split(" ")
        if not version_split or len(version_split) <= 2:
            return True
        version = version_split[2].split(".")
        if not version or len(version) <= 2:
            return True
        target_env_version = ".".join(version[0:2])[1:]
        local_version = self.param.get_local_instance_version()
        target_database_version = self.param.get_target_instance_version()
        env_version_is_same = target_env_version and target_env_version == local_version
        database_version_is_same = target_database_version and target_database_version == local_version
        if env_version_is_same and database_version_is_same:
            return False
        LOGGER.error("Check versions are different, copy instance version: %s, target mongod version: %s, "
                     "target instance version: %s, job_id: %s.",
                     local_version, target_env_version, target_database_version, self.job_id)
        return True

    def check_port_used(self):
        """
        功能描述： 检查端口号是否被占用
        """
        nodes = self._local_instances
        for node in nodes:
            node_url = node.get(ParamField.HOSTURL, ":")
            _, tmp_port = node_url.split(":")
            if check_port_is_used(int(tmp_port)):
                LOGGER.error("Port %s is used in %s. main job_id :%s.", tmp_port, node_url, self.job_id)
                return True
        return False

    def check_instance_user_status(self):
        """
        功能描述： 检查恢复目标用户的是否满足要求
        :return:
        """
        is_exist, instance_user = self.param.get_start_instance_user()
        if not is_exist:
            return True, instance_user
        if instance_user:
            for node in self._local_instances:
                mongod_bin_dir = self.param.get_mongo_bin_path(ParamField.BIN_PATH.value, MongoTool.MONGOD.value, node)
                result, error_param = self.cmd.check_instance_user_exist(mongod_bin_dir, instance_user)
                if not result:
                    return True, instance_user
        return False, instance_user

    def modify_tmp_sock_permission(self):
        """
        功能描述： 检查目录是否存在，软连接，读写权限
        :return:
        """
        nodes = self._local_instances
        _, instance_user = self.param.get_start_instance_user()
        for node in nodes:
            if node.get("shardClusterType") == "mongos":
                continue
            data_path = self.param.get_db_path(node)
            uid, gid = self.get_data_path_user_and_group(data_path)
            uid, gid = (get_uid_by_os_user(instance_user), get_gid_by_os_user(instance_user)) if instance_user else (
                uid, gid)
            url = node.get(ParamField.HOSTURL, "")
            url_list = url.split(":")
            if len(url_list) != 2:
                continue
            port = url_list[1]
            sock = TMP_MONGODB_SOCK.format(port)
            if os.path.exists(sock):
                data_path_permission_num = self.get_data_path_file_permission(sock)
                # 更改文件夹权限,用户,用户组
                self.change_file_own_and_mod(data_path_permission_num, sock, gid, uid)
        return True

    def check_dir_rw_permission(self):
        """
        功能描述： 检查目录是否存在，软连接，读写权限
        :return:
        """
        nodes = self._local_instances
        for node in nodes:
            if node.get("shardClusterType", "") == "mongos":
                continue
            # 检查目录是否存在，是否存在软连接
            db_path = self.param.get_db_path(node)
            if not db_path:
                LOGGER.error("Db path is empty, job id: %s.", self.job_id)
                return True
            path_type = check_file_or_dir(db_path)
            if path_type != EnumPathType.DIR_TYPE:
                LOGGER.error("Db path is invalid type: %s can not copy, job id: %s.", path_type, self.job_id)
                return True
            if path_type == EnumPathType.LINK_TYPE:
                LOGGER.error("Db path is link type can not copy, job id: %s.", self.job_id)
                return True
        return False

    def clean_data_dir(self, node):
        """
        功能描述：清理新实例data目录数据信息
        """
        url = node.get(ParamField.HOSTURL, "")
        LOGGER.info('Delete original data! url: %s, job id: %s', url, self.job_id)
        data_path = self.param.get_db_path(node)
        if not data_path or not os.path.exists(data_path):
            LOGGER.error("Check target instance data error, dir path: '%s', uri: '%s', job id: '%s'.",
                         data_path, url, self.job_id)
            return False, "Check target instance data error, dir path: '%s'." % data_path
        else:
            common.clean_dir(data_path)
            return True, ""

    def parse_copies(self):
        copies = self.param.get_copy()
        return copies

    def copy_data_to_dir(self, node, agent_path: str):
        """
        功能描述： 复制数据到data目录
        """
        data_repository = self.param.get_data_path()
        origin_copies_path = self.get_origin_copies_path(data_repository[0], self.param.get_copy_id(), agent_path)
        if not origin_copies_path:
            return ErrorCode.COPY_DATA_ERROR.value, "Origin copies path not found, job id: %s" % self.job_id
        data_path = self.param.get_db_path(node)
        LOGGER.debug("Copy data to new instance data path, node uri: '%s', copy %s to %s, job id: %s",
                     node.get(ParamField.HOSTURL, ""), origin_copies_path, data_path, self.job_id)
        if self.backup_db_file(origin_copies_path, data_path):
            return MongoDBCode.SUCCESS.value, ""
        else:
            return ErrorCode.COPY_DATA_ERROR.value, \
                "Copy data to new instance data path failed, job id: %s" % self.job_id

    def get_origin_copies_path(self, repository, copy_id, agent_path):
        """
        根据副本id(copies中的uuid)和repositories中的dataPath解析得到原副本路径
        :param repository: /mnt/databackup/MongoDB-cluster/bf7e2700-6a3c-44b6-b304-b2b107baa4cc/
                            data/clone_c9ed5040-e2e9-4023-aee4-3ea6f8d25fc5_restore_0/0.0.0.0
        :param copy_id: c9ed5040-e2e9-4023-aee4-3ea6f8d25fc5
        :return:
        """
        origin_copies_path = os.path.join(repository, self.instance_directory, copy_id, agent_path)
        LOGGER.info("Mongo origin copies path is %s, copy id: %s, job id: %s.", origin_copies_path, copy_id,
                    self.job_id)
        path = os.listdir(origin_copies_path)
        if not path:
            LOGGER.error("Mongo origin copies path is empty, job id: %s.", self.job_id)
            return ""
        return os.path.join(origin_copies_path, path[0] + "/")

    def get_file_paths(self, root_dir):
        """
        获取所有文件下的文件和文件夹
        :param root_dir:
        :return:
        """
        file_paths = []
        for root, dirs, files in os.walk(root_dir):  # 分别代表根目录、文件夹、文件
            for file in files:
                file_path = os.path.join(root, file)  # 获取文件绝对路径
                file_paths.append(file_path)
            for directory in dirs:
                dir_path = os.path.join(root, directory)
                file_paths.append(dir_path)
                self.get_file_paths(dir_path)
        return file_paths

    def modify_dir_permission(self, node):
        """
        功能描述： 修改dir权限，赋予宿主正确权限
        """
        _, instance_user = self.param.get_start_instance_user()
        data_path = self.param.get_db_path(node)
        # 校验合法的路径
        if not data_path or not os.path.exists(data_path):
            LOGGER.error("Data path is not exits: '%s', job id : %s.", data_path, self.job_id)
            msg = "Data path is not exits: '%s'" % data_path
            return ErrorCode.MODIFY_DATA_DIR_ERROR.value, msg
        log_path = self.param.get_data_log_path(node)
        if not log_path or not os.path.exists(log_path):
            LOGGER.error("Log path is not exits: '%s', job id : %s.", log_path, self.job_id)
            msg = "Log path is not exits: '%s'" % data_path
            return ErrorCode.MODIFY_DATA_DIR_ERROR.value, msg
        uid, gid = self.get_data_path_user_and_group(data_path)
        uid, gid = (get_uid_by_os_user(instance_user), get_gid_by_os_user(instance_user)) if instance_user else (
            uid, gid)
        data_path_permission_num = self.get_data_path_file_permission(data_path)
        # 更改文件夹权限,用户,用户组
        file_paths = self.get_file_paths(data_path)
        file_paths.append(data_path)
        file_paths.append(log_path)
        for file_path in file_paths:
            self.change_file_own_and_mod(data_path_permission_num, file_path, gid, uid)
        LOGGER.debug("Modify copy file permission success, node url: %s, job id: %s.", node.get(ParamField.HOSTURL, ""),
                     self.job_id)
        return MongoDBCode.SUCCESS.value, ""

    def delete_mongod_lock_file(self, node):
        """
        功能描述： 检查是否存在mongod.lock目录，若存在则删除mongod.lock目录
        """
        mongod_lock_file = os.path.join(self.param.get_db_path(node), "mongod.lock")
        if os.path.exists(mongod_lock_file):
            su_exec_rm_cmd(mongod_lock_file, check_white_black_list_flag=False)
        LOGGER.debug('Delete mongod.lock file success! job id: %s.', self.job_id)
        return MongoDBCode.SUCCESS.value, ""

    def build_parsed_data_to_conf(self, *args, node, mongo_type=MongoTool.MONGOD.value):
        """
        解析parsed数据生conf
        :param *args: 要删除的字段
        :return: conf文件
        """
        # 解析转换parsed data,字段对应
        # 保存cache仓临时路径,启动完实例删除此文件
        local_host = node.get(ParamField.HOSTURL, "")
        parsed = node.get(MongoRoles.PARSED)
        if parsed:
            parsed_json = self.check_and_update_parsed_field(args, parsed_json=parsed, mongo_type=mongo_type)
            # 通过parsed数据新建yaml格式的conf文件
            conf_file_path = self.create_yaml_conf_by_parsed_data(local_host, parsed_json)
            if mongo_type == MongoTool.MONGOS.value:
                return conf_file_path
            result = self.chown_conf_path_permissions(node, conf_file_path)
            if not result:
                return ""
            return conf_file_path
        LOGGER.error("Parsed param is not invalid, args: %s, node uri: %s, job id: %s", args,
                     node.get(ParamField.HOSTURL, ""), self.param.job_id)
        return ""

    def chown_conf_path_permissions(self, node, conf_file_path):
        user = get_mkdir_user(self.param.get_db_path(node))
        _, instance_user = self.param.get_start_instance_user()
        user = instance_user if instance_user else user
        return self.cmd.chown_conf_path_permissions(conf_file_path, user)

    def traverse_parsed_data(self, dic, path=None):
        """
        解析parsed数据中字段
        @param dic: parsed 字典
        @param path: 递归深度
        @return:
        """
        if not path:
            path = []
        if isinstance(dic, dict):
            for x in dic.keys():
                local_path = path[:]
                local_path.append(x)
                for b in self.traverse_parsed_data(dic[x], local_path):
                    yield b
        else:
            yield path, dic

    def get_parsed_data_fields(self, parsed):
        """
        得到所有parsed data中的路径
        @param parsed:
        @return: ['net.bindIp',
                 'net.port',
                 'processManagement.fork',
                 'security.authorization',
                 'security.keyFile',
                 'storage.dbPath',
                 'storage.journal.enabled',
                 'storage.wiredTiger.engineConfig.cacheSizeGB',
                 'systemLog.destination',
                 'systemLog.logAppend',
                 'systemLog.path']
        """
        parsed_data_fields = []
        for parsed_data in self.traverse_parsed_data(parsed):
            if len(parsed_data) != 0:
                path = ('.'.join(parsed_data[0]))
                parsed_data_fields.append(path)
        return parsed_data_fields

    def set_parsed_data_field(self, dictionary, path, set_key):
        """
        更新指定路径下多重字典的key
        :param dictionary:
        :param path: 'storage.journal.enabled',
        :param set_key: update key
        :return: dict
        """
        path = path.split(".")
        key = path[-1]
        dictionary = self.get_parsed_data_filed_paths(dictionary, ".".join(path[:-1]))
        set_key = set_key.split(".")
        set_key = set_key[-1]
        dictionary[set_key] = dictionary[key]
        del dictionary[key]

    def check_and_update_parsed_field(self, args, parsed_json, mongo_type=MongoTool.MONGOD.value):
        """
        校验parsed数据中的字段和mongod命令集合是否一一映射, 若字段不对应,则修改为映射表中的key
        :param mongo_type: mongod或者mongos
        :param args: 要删除的首字段
        :param parsed_json: parsed_json
        :return: {
                "config" : "/home/mongodb/single/mongod.conf",
                "net" : {
                    "bindIp" : "0.0.0.0",
                    "port" : 27102
                },
                "processManagement" : {
                    "fork" : "true"
                },
                "security" : {
                    "authorization" : "enabled",
                    "keyFile" : "/home/mongodb/mongo/keyfile"
                },
                "storage" : {
                    "dbPath" : "/home/mongodb/single/data",
                    "journal" : {
                        "enabled" : "true"
                    },
                    "wiredTiger" : {
                        "engineConfig" : {
                            "cacheSizeGB" : 1
                        }
                    }
                },
                "systemLog" : {
                    "destination" : "file",
                    "logAppend" : "true",
                    "path" : "/home/mongodb/single/log/single.log"
                }
            }
        """
        # 实例要注释掉 "security" 字段, 统一不认证
        parsed_json = json.loads(parsed_json)
        parsed_data_fields = self.get_parsed_data_fields(parsed_json)
        data = MongodCommandRelationship.data
        if mongo_type == MongoTool.MONGOS.value:
            data = MongosCommandRelationship.data
        for parsed_field_path in parsed_data_fields:
            if parsed_field_path in data:
                set_item = data.get(parsed_field_path)
                self.set_parsed_data_field(parsed_json, parsed_field_path, set_item)
        if len(args) > 0:
            for filed in args:
                if str(filed) in parsed_json:
                    del parsed_json[filed]
        if "security" in parsed_json:
            del parsed_json["security"]
        if "config" in parsed_json:
            del parsed_json["config"]
        LOGGER.info("Parsed json data: %s, job id: %s.", parsed_json, self.job_id)
        return parsed_json

    def create_yaml_conf_by_parsed_data(self, local_host, parsed_json):
        """
        根据parsed data生成yaml
        :param local_host: "ip:port"
        :param parsed_json:
        :return: yaml的绝对路径
        """
        LOGGER.info("Start create conf file, job id: %s", self.job_id)
        local_host = "_".join(local_host.split(":")[0].split(".")) + "_" + local_host.split(":")[1]
        timestamp_str = str(int(time.time()))
        conf_file_name = local_host + "_" + timestamp_str + "_mongod.conf"
        conf_file_path = os.path.join(TMP_CONF_PATH, self.job_id, conf_file_name)
        conf_file_path_dir = os.path.join(TMP_CONF_PATH, self.job_id)
        if not os.path.exists(TMP_CONF_PATH):
            exec_mkdir_cmd(TMP_CONF_PATH)
        change_path_permission(TMP_CONF_PATH, mode=0o755)
        if not os.path.exists(conf_file_path_dir):
            exec_mkdir_cmd(conf_file_path_dir)
            change_path_permission(conf_file_path_dir, mode=0o755)
        # 注意根据具体业务的需要设置文件读写方式
        flags = os.O_WRONLY | os.O_CREAT | os.O_EXCL
        # 注意根据具体业务的需要设置文件权限
        modes = stat.S_IWUSR | stat.S_IRUSR
        with os.fdopen(os.open(conf_file_path, flags, modes), 'w', encoding='utf-8') as fout:
            yaml.dump(parsed_json, fout, allow_unicode=True, indent=4)
        LOGGER.info("Create yaml format conf file: %s success, job id: %s.", conf_file_path, self.job_id)
        return conf_file_path

    def drop_local_database_update_info(self, node, relative_path):
        # 进入此节点, 删除local数据库, 使用本机关闭实例
        try:
            with self.param.db_command(node) as mongo:
                LOGGER.info("Start to drop local database, relative_path: %s, job id: %s", relative_path, self.job_id)
                mongo.drop_local_database()
                if relative_path == "0":
                    mongo.update_config_info(self.param.get_shard_list())
                else:
                    mongo.update_shard_info(self.param.get_config_list())
            LOGGER.info("drop_local_database success")
            user = get_mkdir_user(self.param.get_db_path(node))
            _, instance_user = self.param.get_start_instance_user()
            user = instance_user if instance_user else user
            LOGGER.info("get instance user success")
            mongod_bin_dir = self.param.get_mongo_bin_path(ParamField.BIN_PATH.value, MongoTool.MONGOD.value, node)
            self.cmd.shutdown_instance(mongod_bin_dir, user, self.param.get_db_path(node))
            LOGGER.info(
                "End shutdown server, relative_path: %s, job id: %s", relative_path, self.job_id)
        except DBConnectionError:
            return ErrorCode.CONNECT_TO_DB_ERROR.value, "Error to connect db."
        except DBAuthenticationError:
            return ErrorCode.ERROR_AUTH.value, "DB authentication is error."
        except Exception:
            return ErrorCode.ERR_DB_SERVICES.value, "Drop or shut down db error."
        return MongoDBCode.SUCCESS.value, "Drop local database success."

    def start_local_instance(self, conf_file_path, node):
        """
        功能描述： 启动实例.默认mongod 配置了全局变量
        """
        user = get_mkdir_user(self.param.get_db_path(node))
        mongod_bin_dir = self.param.get_mongo_bin_path(ParamField.BIN_PATH.value, MongoTool.MONGOD.value, node)
        _, instance_user = self.param.get_start_instance_user()
        user = instance_user if instance_user else user
        if conf_file_path:
            # 执行启动命令 mongod的路径 -f conf文件路径, 0-执行成功
            result, error_param = self.cmd.start_up_instance(mongod_bin_dir, conf_file_path, user)
        else:
            LOGGER.error("Mongo local instance start failed, conf path is empty, job id: %s.", self.job_id)
            return ErrorCode.PARSE_YAML_FILE_ERROR.value, "Mongo local instance start conf file path is not exist."
        # 异常处理
        if result:
            LOGGER.debug("Mongo local instance start success, conf path: %s, job id: %s.", conf_file_path, self.job_id)
            return MongoDBCode.SUCCESS.value, "mongo local instance start success."
        else:
            LOGGER.error("Mongo local instance start failed, conf path: %s, job id: %s.", conf_file_path, self.job_id)
            return ErrorCode.INSTANCE_START_ERROR.value, "mongo local instance start failed."

    def connect_instance(self, node):
        """
        连接认证
        """
        url = node.get(ParamField.HOSTURL, "")
        try:
            with self.param.db_command(node) as mongo:
                mongo.connect()
        except DBConnectionError as e:
            msg = "Unable to connect to %s! Error: %s, job id: %s." % url % e % self.job_id
            return ErrorCode.CONNECT_TO_DB_ERROR.value, msg
        except DBAuthenticationError as e:
            msg = "Auth info to %s! Error: %s, job id: %s." % url % e % self.job_id
            return ErrorCode.ERROR_AUTH.value, msg
        except Exception as e:
            msg = "Connect to %s failed! Error: %s, job id: %s." % url % e % self.job_id
            return ErrorCode.CONNECT_TO_DB_ERROR.value, msg
        LOGGER.debug("End to check connect instance: %s,  job_id: %s", url, self.job_id)
        return MongoDBCode.SUCCESS.value, ""

    def initiate_cluster(self, node, intiate_instances):
        """
        功能描述： 初始化集群
        rs.initiate({
        "_id":"rs2",
        "members":[
        {"_id":0,"host":"xxx:xxx",priority:1},
        {"_id":1,"host":"xxx:xxx",priority:1},
        {"_id":2,"host":"xxx:xxx",priority:1},
        {"_id":3,"host":"xxx:xxx",arbiterOnly:true}
        ]
        })
        """
        url = node.get(ParamField.HOSTURL.value, "")
        cluster_instance_name = ""
        members = []
        for instance in intiate_instances:
            priority = instance.get(ParamField.PRIORITY.value, "1")
            cluster_id = instance.get(ParamField.INSTANCE_ID.value)
            if instance.get(ParamField.CLUSTER_INSTANCE_TYPE) == MongoRoles.SINGLE_NODE_REPL:
                # 如果是单节点副本集,只有一个节点,且为主节点,id与priority设置默认值即可
                cluster_id = "0"
                if not priority:
                    priority = "1"
            if not cluster_id:
                return ErrorCode.CONNECT_TO_DB_ERROR.value, "cluster id is not exist"
            member = {
                ParamField.CLUSTER_ID.value: int(cluster_id),
                ParamField.HOST.value: instance.get(ParamField.HOSTURL.value, ""),
                ParamField.PRIORITY.value: int(priority)
            }
            cluster_instance_name = instance.get(ParamField.CLUSTER_INSTANCE_NAME.value, "")
            if instance.get(ParamField.ROLE.value) == str(MongoRolesStatus.ARBITER.value):
                member[ParamField.ARBITER_ONLY.value] = True
            members.append(member)
        initiate_param = {ParamField.CLUSTER_ID.value: cluster_instance_name, ParamField.MEMBERS.value: members}
        LOGGER.debug("Mongo initiate cluster, initiate_param: %s, job id: %s.", initiate_param, self.job_id)
        try:
            with self.param.db_command(node) as mongo:
                mongo.initiate_cluster(initiate_param)
        except DBConnectionError as ex:
            msg = "Unable to connect to %s! Error: %s" % (url, ex)
            return ErrorCode.CONNECT_TO_DB_ERROR.value, msg
        except DBAuthenticationError as ex:
            msg = "Auth info to url: %s! Error: %s" % (url, ex)
            return ErrorCode.ERROR_AUTH.value, msg
        except Exception as ex:
            msg = "DB Operation Error to url: %s! Error: %s" % (url, ex)
            return ErrorCode.CLUSTER_INITIATE_ERROR.value, msg
        LOGGER.info("Mongo restore replSet initiate server success! job id: %s", self.job_id)
        return MongoDBCode.SUCCESS.value, ""

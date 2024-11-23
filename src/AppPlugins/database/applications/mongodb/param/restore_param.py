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

import contextlib
import json
import operator
import os
from json import JSONDecodeError

from common.common import get_local_ips
from common.const import RepositoryName, RepositoryDataTypeEnum
from common.parse_parafile import ParamFileUtil
from common.util.check_user_utils import check_os_user
from mongodb import LOGGER
from mongodb.comm.const import ParamField, ROLE_MAP, MongoRolesStatus, LOG_DIRECTORY
from mongodb.comm.mongo_executor import DB
from mongodb.comm.utils import get_base_nodes_info
from mongodb.param import BaseParam


class RestoreParam(BaseParam):
    restore_field = (
        ParamField.COPIES,
        ParamField.JOB_PARAM,
        ParamField.EXTEND_INFO,
        ParamField.TARGET_ENV,
        ParamField.TARGET_OBJECT,
        ParamField.JOB_ID
    )

    def __init__(self, param_dict):
        super().__init__(param_dict)
        """
        :param copies: 副本信息
        :param job_param: job_param参数
        :param repositories: 数据仓
        :param target_env: env信息
        :param job_id: job_id
        """
        self.copies = []
        self.job_param = {}
        self.repositories = []
        self.target_env = {}
        self.target_object = {}
        self.job_id = ""
        self._parse_default_field(self.job, RestoreParam.restore_field)
        self.repositories_map = self.get_restore_repository_dict()
        self.instance_user = self.extend_info.get("start_instance_user", "")

    def get_copy(self):
        return {} if not self.copies else self.copies[0]

    def get_start_instance_user(self):
        is_exist = True
        if self.instance_user:
            is_exist = check_os_user(self.instance_user)
        return is_exist, self.instance_user

    def get_copy_id(self):
        copy_list = self.get_all_copy()
        for copy in copy_list:
            copy_type = copy.get(ParamField.TYPE.value, "")
            LOGGER.info("Func get copy id cop type: %s", copy_type)
            if copy_type == ParamField.S3_ARCHIVE.value or copy_type == ParamField.TAPE_ARCHIVE.value:
                extend_dict = copy.get("extendInfo", {})
                copy_id = extend_dict.get("extendInfo", {}).get("copy_id")
                return copy_id
            if copy_type == ParamField.FULL_COPY.value:
                extend_dict = copy.get("extendInfo", {})
                return extend_dict.get(ParamField.COPY_ID.value, "")
        return ""

    def get_copy_agent_id(self):
        copy = self.get_copy()
        nodes = copy.get("protectEnv", {}).get("nodes", [])
        for node in nodes:
            return node.get(ParamField.ID, "")

    def get_copy_agent_url(self):
        copy = self.get_copy()
        protect_env_extend_info = copy.get("protectEnv", {}).get("extendInfo", {})
        return protect_env_extend_info.get("serviceIp", "") + ":" + protect_env_extend_info.get("servicePort", "")

    def get_restore_repository_dict(self):
        copy = self.get_copy()
        return ParamFileUtil.parse_backup_path(copy.get(ParamField.REPOSITORIES, []))

    def get_data_path(self):
        return self.repositories_map.get(RepositoryName.DATA_REPOSITORY)

    def get_data_repo(self):
        for repo in self.repositories:
            if repo.get(ParamField.REPOSITORY_TYPE) == RepositoryDataTypeEnum.DATA_REPOSITORY:
                return repo
        return self.repositories

    def get_cache_path(self):
        return self.repositories_map.get(RepositoryName.CACHE_REPOSITORY)

    def get_log_path(self):
        return self.repositories_map.get(RepositoryName.LOG_REPOSITORY)

    def get_meta_path(self):
        return self.repositories_map.get(RepositoryName.META_REPOSITORY)

    def get_sub_job_name(self):
        return self.sub_job.get(ParamField.JOB_NAME, "")

    def get_sub_job_id(self):
        return self.sub_job.get(ParamField.SUB_JOB_ID, "")

    def get_resource_type(self):
        """
        获取恢复对象资源类型：实例/复制集群/分片集群
        """
        extend_info = self.get_target_obj_extend_info()
        resource_type = extend_info.get(ParamField.CLUSTER_TYPE)
        return resource_type

    def get_local_node_data_path(self, agent_id, host_url):
        copy = self.get_copy()
        lvms = copy.get("extendInfo", {}).get("lvms", [])
        for lvm in lvms:
            if agent_id != lvm.get(ParamField.ID, ""):
                continue
            for inst in lvm.get("inst", []):
                if host_url == inst.get("hostUrl", ""):
                    return inst.get("repl_data_path", "")
        return ""

    def get_all_instances_dic_info(self):
        all_nodes = []
        nodes_info = self.get_nodes_info()
        for _, info in nodes_info.items():
            all_nodes = all_nodes + info
        return all_nodes

    def get_nodes_info(self):
        """
        获取所有待恢复节点信息
        :return:
        """
        extend_info = self.get_target_env_extend_info()
        return self.get_copy_or_target_nodes_info(extend_info)

    def get_copy_nodes_info(self):
        """
        获取所有待恢复节点信息
        :return:
        """
        extend_info = self.get_copy().get("protectEnv").get("extendInfo")
        return self.get_copy_or_target_nodes_info(extend_info)

    def get_copy_or_target_nodes_info(self, extend_info):
        cluster_nodes_string = extend_info.get(ParamField.CLUSTER_NODES.value, "")
        try:
            nodes_info = json.loads(cluster_nodes_string)
        except JSONDecodeError as e:
            raise ValueError from e
        nodes = self.target_env.get(ParamField.NODES.value, [])
        return get_base_nodes_info(nodes_info, nodes)

    def get_target_env_extend_info(self):
        """
        获取targetEnv下extendInfo信息
        """
        return self.target_env.get(ParamField.EXTEND_INFO)

    def get_target_obj_extend_info(self):
        """
        获取targetObject下extendInfo信息
        """
        return self.target_object.get(ParamField.EXTEND_INFO)

    def get_local_insts_info(self):
        nodes_info = self.get_nodes_info()
        local_host = get_local_ips()
        all_info = []
        for node, info in nodes_info.items():
            if node in local_host:
                all_info = all_info + info
        return all_info

    def get_primary_inst(self):
        for inst in self.get_local_insts_info():
            role = inst.get(ParamField.ROLE)
            if role == MongoRolesStatus.PRIMARY:
                return inst
        return {}

    def get_primary_uri(self):
        inst = self.get_primary_inst()
        return inst.get(ParamField.HOSTURL, "")

    def get_local_role_insts(self, role_type):
        nodes = []
        for inst in self.get_local_insts_info():
            role = inst.get(ParamField.ROLE)
            if role == str(role_type):
                nodes.append(inst)
        return nodes

    def get_local_uris(self):
        uris = []
        for node in self.get_local_insts_info():
            node_uri = node.get(ParamField.HOSTURL)
            if node_uri:
                uris.append(node_uri)
        return uris

    def get_local_role(self):
        roles = []
        for node in self.get_local_insts_info():
            role_id = node.get(ParamField.ROLE)
            role = ROLE_MAP.get(role_id)
            if role:
                roles.append(role)
        return roles

    def get_local_inst_port(self):
        extend_info = self.get_target_obj_extend_info()
        service_port = extend_info.get(ParamField.SERVICE_PORT)
        return service_port

    def get_target_instance_version(self):
        return self.get_target_env_extend_info().get(ParamField.VERSION, "")

    def get_local_instance_version(self):
        return self.get_copy().get(ParamField.PROTECT_ENV).get(ParamField.EXTEND_INFO, {}).get(ParamField.VERSION, "")

    def get_db_path(self, node):
        return node.get(ParamField.DATA_PATH)

    def get_data_log_path(self, node):
        parsed_json = json.loads(node.get("parsed", ""))
        return parsed_json.get("systemLog", "").get("path", "")

    def get_target_shard_role_info(self):
        """
        获取所有分片整合节点信息
        :return:
        """
        sharding = self.get_target_shards_info()
        path = list()
        for shard in sharding:
            if shard.get("shardClusterType") == "mongos":
                continue
            instance_name_infos = shard.get("instanceNameInfos", "")
            if instance_name_infos not in path:
                path.append(instance_name_infos)
                continue
            if instance_name_infos in path and not shard.get("stateStr"):
                path.append(instance_name_infos)
        return path

    def get_target_shards_info(self):
        """
        获取所有分片整合节点信息
        :return:
        """
        extend_info = self.get_target_obj_extend_info()
        sharding = extend_info.get(ParamField.CLUSTER_NODES, "")
        try:
            shards = json.loads(sharding)
        except JSONDecodeError as e:
            raise ValueError from e
        return shards

    def get_local_shard_info(self):
        """
        获取所有分片整合节点信息
        :return:
        """
        extend_info = self.get_copy().get("protectEnv").get("extendInfo")
        sharding = extend_info.get(ParamField.CLUSTER_NODES, "")
        try:
            sharding = json.loads(sharding)
        except JSONDecodeError as e:
            raise ValueError from e
        return sharding

    def get_local_shard_role_info(self):
        """
        获取所有分片整合节点信息
        :return:
        """
        sharding = self.get_local_shard_info()
        path = list()
        for shard in sharding:
            if shard.get("shardClusterType") == "mongos":
                continue
            instance_name_infos = shard.get("instanceNameInfos", "")
            if instance_name_infos not in path:
                path.append(instance_name_infos)
                continue
            if instance_name_infos in path and not shard.get("stateStr"):
                path.append(instance_name_infos)
        return path

    def get_target_env_nodes(self):
        """
        获取当前所有nodes节点;信息
        :return:
        """
        node_list = self.target_env.get("nodes", {})
        node_info = {}
        for node in node_list:
            if not node_info.get(node.get("extendInfo", {}).get("agentUuid", "")):
                node_info.update({
                    node.get("extendInfo", {}).get("agentUuid", ""): node
                })
        return node_info

    def get_config_primary(self):
        end_port = list()
        uuid_list = list()
        sharding = self.get_target_shards_info()
        for shard in sharding:
            if shard.get("shardClusterType", "") != "config":
                continue
            if shard.get("stateStr", "") == "PRIMARY":
                end_port.append(shard.get(ParamField.AGENTURL, "").split(":")[0])
        node_list = self.get_target_env_nodes()
        for uuid, node in node_list.items():
            agent_list = set(node.get("extendInfo").get("agentIpList").split(",")) & set(end_port)
            if agent_list and len(agent_list) > 0:
                uuid_list.append(uuid)
        return uuid_list

    def get_shard_primary(self):
        end_port = list()
        uuid_list = list()
        sharding = self.get_target_shards_info()
        for shard in sharding:
            if shard.get("shardClusterType", "") != "shard":
                continue
            if shard.get("stateStr", "") == "PRIMARY" or shard.get("stateStr", "") == "":
                end_port.append(shard.get(ParamField.AGENTURL, "").split(":")[0])
        node_list = self.get_target_env_nodes()
        for uuid, node in node_list.items():
            agent_list = set(node.get("extendInfo").get("agentIpList").split(",")) & set(end_port)
            if agent_list and len(agent_list) > 0:
                uuid_list.append(uuid)
        return uuid_list

    @contextlib.contextmanager
    def db_command(self, node):
        """
        功能描述： 转换本地实例状态信息
        :return:
        """
        host_url = node.get(ParamField.HOSTURL)
        config = {"username": None, "password": None, "auth_type": 0}
        bin_path = node.get('extendInfo').get('binPath')
        mongo = DB(host_url, config, direct_connection=True, bin_path=bin_path)
        try:
            yield mongo
        except Exception as ex:
            LOGGER.error("Job id: %s, create db command failed, uri: %s.", self.job_id, host_url)
            raise ex
        finally:
            mongo.close()

    def get_all_copy(self):
        copies = [] if not self.copies else self.copies
        return copies

    def get_copy_type(self):
        """
        获取所有分片整合节点信息
        :return:
        """
        copy_list = self.get_all_copy()
        for copy in copy_list:
            if copy.get(ParamField.TYPE, "") == ParamField.LOG_COPY:
                return ParamField.LOG_COPY
        return ParamField.FULL_COPY

    def get_shard_list(self):
        """
        功能描述： 转换本地实例状态信息
        :return:
        """
        local_sharding = self.get_local_shard_info()
        target_sharding = self.get_target_shards_info()
        tmp_shard_info = {}
        for local in local_sharding:
            if local.get("shardClusterType", "") != "shard":
                continue
            if local.get("stateStr", "") == "PRIMARY":
                tmp_shard_info.update(
                    {local.get("shardIndex"): local.get("clusterInstanceName") + "/" + local.get("instanceNameInfos")})
                continue
            if local.get("stateStr", "") == "":
                tmp_shard_info.update(
                    {local.get("shardIndex"): local.get("shardInstanceName") + "/" + local.get("hostUrl")})
        shard_info = {}
        for target in target_sharding:
            if target.get("shardClusterType", "") != "shard":
                continue
            if target.get("stateStr", "") == "PRIMARY":
                if tmp_shard_info.get(target.get("shardIndex")):
                    shard_info.update({target.get("shardInstanceName"): target.get(
                        "clusterInstanceName") + "/" + target.get("instanceNameInfos")})
                continue
            if target.get("stateStr", "") == "":
                if tmp_shard_info.get(target.get("shardIndex")):
                    shard_info.update(
                        {target.get("shardInstanceName"): target.get("shardInstanceName") + "/" + target.get(
                            "hostUrl")})
        if len(tmp_shard_info) != len(shard_info):
            LOGGER.warn("Tmp shard info: %s ,shard info length: %s are different.", len(tmp_shard_info),
                        len(shard_info))
            return {}
        return shard_info

    def get_config_list(self):
        """
        功能描述： 转换本地实例状态信息
        :return:
        """
        target_sharding = self.get_target_shards_info()
        for target in target_sharding:
            if target.get("shardClusterType", "") != "config":
                continue
            if target.get("stateStr", "") == "PRIMARY":
                return target.get("clusterInstanceName") + "/" + target.get("instanceNameInfos")
        return ""

    def get_oplog_copie_dirs(self):
        """
        根据timestamp字段排序copies
        :return: 返回日志副本的路径
        """
        copies = self.get_all_copy()
        # 根据timestamp字段排序copies
        sorted_copies = sorted(copies, key=operator.itemgetter(ParamField.TIMESTAMP))
        # "extendInfo" --> lastLsn --> "0"(非shard副本数据仓) --> [1684939520, 1]
        sorted_oplog_copie_dirs = []
        for copy in sorted_copies:
            for repositorie in copy.get(ParamField.REPOSITORIES):
                if repositorie.get(ParamField.REPOSITORY_TYPE) == RepositoryDataTypeEnum.LOG_REPOSITORY:
                    copie_dir = repositorie.get(ParamField.PATH)[0]
                    copy_id = copie_dir.split("/")[-1]
                    LOGGER.info("Oplog copy dir is %s, copy_id: %s, job id: %s.", copie_dir, copy_id, self.job_id)
                    copy_path = os.path.join(copie_dir, LOG_DIRECTORY, copy_id)
                    sorted_oplog_copie_dirs.append(copy_path)
        LOGGER.info("Job: %s, Sorted oplog copy dirs: %s ", self.job_id, sorted_oplog_copie_dirs)
        return sorted_oplog_copie_dirs

    def get_mongo_bin_path(self, bin_path_type, mongo_tool, node):
        mongo_bin_dir = node.get("extendInfo", {}).get(bin_path_type, "")
        if mongo_bin_dir:
            return mongo_bin_dir + ParamField.FORWARD_SLASH + mongo_tool
        else:
            return mongo_tool

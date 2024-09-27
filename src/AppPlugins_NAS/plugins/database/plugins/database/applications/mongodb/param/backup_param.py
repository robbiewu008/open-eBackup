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
from common.const import RepositoryName, RepositoryDataTypeEnum
from common.parse_parafile import ParamFileUtil
from mongodb import LOGGER
from mongodb.comm.const import DefaultValue
from mongodb.comm.const import ParamField, ROLE_MAP
from mongodb.comm.utils import loads, expansion, get_base_nodes_info
from mongodb.param import BaseParam


class BackupParam(BaseParam):
    backup_field = (
        ParamField.COPY,
        ParamField.JOB_PARAM,
        ParamField.REPOSITORIES,
        ParamField.PROTECT_ENV,
        ParamField.PROTECT_OBJECT,
        ParamField.JOB_ID
    )

    def __init__(self, param_dict):
        super().__init__(param_dict)
        self.copy = []
        self.job_param = {}
        self.repositories = []
        self.job_id = ""
        self.protect_env = {}
        self.protect_object = {}
        self._parse_default_field(self.job, BackupParam.backup_field)
        self.repositories_map = self.get_backup_repository_dict()

    def get_copy(self):
        return {} if not self.copy else self.copy[0]

    def get_lvm_percent(self):
        """
        创建逻辑卷空间比例参数
        :return: int: 10-50
        """
        param_dict = self.param_dict
        extend_info = param_dict.get("job", {}).get("extendInfo")
        lvm_percent = extend_info.get("create_lvm_percent", DefaultValue.SNAP_MIN_PER.value)
        LOGGER.info("Create lvm percent: %s", int(lvm_percent))
        return int(lvm_percent)

    def get_copy_id(self):
        copy = self.get_copy()
        return copy.get(ParamField.ID, "")

    def get_backup_repository_dict(self):
        return ParamFileUtil.parse_backup_path(self.repositories)

    def get_data_path(self):
        return self.repositories_map.get(RepositoryName.DATA_REPOSITORY)

    def get_data_repo(self):
        for repo in self.repositories:
            if repo.get(ParamField.REPOSITORY_TYPE) == RepositoryDataTypeEnum.DATA_REPOSITORY:
                return repo
        return self.repositories

    def get_log_repo(self):
        for repo in self.repositories:
            if repo.get(ParamField.REPOSITORY_TYPE) == RepositoryDataTypeEnum.LOG_REPOSITORY:
                return repo
        return self.repositories

    def get_cache_repo(self):
        return self.repositories_map.get(RepositoryName.CACHE_REPOSITORY)

    def get_log_path(self):
        return self.repositories_map.get(RepositoryName.LOG_REPOSITORY)

    def get_meta_path(self):
        return self.repositories_map.get(RepositoryName.META_REPOSITORY)

    def get_sub_job_name(self):
        return self.sub_job.get(ParamField.JOB_NAME, "")

    def get_sub_job_id(self):
        return self.sub_job.get(ParamField.SUB_JOB_ID, "")

    def get_backup_type(self):
        """
        获取备份类型：全量/增量/
        """
        return self.job_param.get(ParamField.BACKUP_TYPE)

    def get_resource_type(self):
        """
        获取备份资源类型：实例/数据库
        """
        extend_info = self.get_protect_obj_extend_info()
        resource_type = extend_info.get(ParamField.CLUSTER_TYPE)
        return resource_type

    def get_cluster_info(self):
        extend_info = self.get_protect_env_extend_info()
        cluster_nodes_string = extend_info.get(ParamField.CLUSTER_NODES, "")
        cluster_insts = loads(cluster_nodes_string)
        return cluster_insts

    def get_nodes_key_map(self):
        """
        获取所有节点信息
        :return:
        """
        nodes = self.protect_env.get(ParamField.NODES, [])
        node_key_map = {}
        for count, node in enumerate(nodes):
            service_port = node.get(ParamField.EXTEND_INFO, {}).get("servicePort", "")
            if service_port:
                node_key_map.update({service_port: str(count)})
        return node_key_map

    def get_nodes_info(self):
        """
        获取所有节点信息
        :return:
        """
        cluster_insts = self.get_cluster_info()
        nodes = self.protect_env.get(ParamField.NODES, [])
        return get_base_nodes_info(cluster_insts, nodes)

    def get_all_primary_inst(self):
        all_inst = self.get_nodes_info()
        primary_insts = []
        for inst in expansion(all_inst.values()):
            if inst.get(ParamField.ROLE) == "1":
                primary_insts.append(inst)
        return primary_insts

    def get_protect_env_extend_info(self):
        """
        获取portectEnv下extendInfo信息
        """
        return self.protect_env.get(ParamField.EXTEND_INFO)

    def get_protect_obj_extend_info(self):
        """
        获取portectObject下extendInfo信息
        """
        return self.protect_object.get(ParamField.EXTEND_INFO, {})

    def get_local_insts_info(self):
        nodes_info = self.get_nodes_info()
        local_host = get_local_ips()
        insts = []
        for node, info in nodes_info.items():
            if node in local_host:
                insts.extend(info)
        return insts

    def get_primary_inst(self):
        insts = self.get_local_primary_insts()
        return {} if not insts else insts[0]

    def get_local_primary_insts(self):
        insts = []
        for inst in self.get_local_insts_info():
            role = inst.get(ParamField.ROLE)
            if role == "1":
                insts.append(inst)
        return insts

    def get_primary_agent_id(self):
        inst = self.get_primary_inst()
        agent_id = inst.get(ParamField.ID, "")
        return agent_id

    def get_primary_uri(self):
        inst = self.get_primary_inst()
        return inst.get(ParamField.HOSTURL, "")

    def get_primary_uris(self):
        primary_insts = self.get_local_primary_insts()
        return [inst.get(ParamField.HOSTURL, "") for inst in primary_insts]

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
        extend_info = self.get_protect_obj_extend_info()
        service_port = extend_info.get(ParamField.SERVICE_PORT)
        return service_port

    def get_local_agent_id(self):
        insts = self.get_local_insts_info()
        return "" if not insts else insts[0].get("id", "")

    def get_mongo_bin_path(self, bin_path_type, mongo_tool, node):
        mongo_bin_dir = node.get(ParamField.EXTEND_INFO.value, {}).get(bin_path_type, "")
        if mongo_bin_dir:
            return mongo_bin_dir + ParamField.FORWARD_SLASH + mongo_tool
        else:
            return mongo_tool

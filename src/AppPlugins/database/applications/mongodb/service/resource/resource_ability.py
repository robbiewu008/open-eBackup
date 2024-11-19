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
import socket

from common.common import get_local_ips
from common.const import AuthType, EnumPathType
from common.file_common import check_file_or_dir
from mongodb import LOGGER
from mongodb.comm.cmd import Cmd
from mongodb.comm.const import RoleMode, ErrorCode, MongoDBCode, MongoRoles, ParamField, MongoTool
from mongodb.comm.exception_err import DBConnectionError, DBAuthenticationError, DBOperationError
from mongodb.comm.mongo_executor import DB, get_mongo_bind_ip


def is_intrusive(role: int):
    """
    判断角色结果
    """
    if role == 1:
        return RoleMode.MASTER_MODE.value
    if role == 2:
        return RoleMode.SLAVE_MODE.value
    if role == 7:
        return RoleMode.ARBITRATE_MODE.value
    return RoleMode.NO_MODE.value


class Resource:
    def __init__(self, job_manager, param):
        self.job_manager = job_manager
        self.param = param
        env2 = self.param.app_env
        extend_info = env2.get("extendInfo")
        service_port = extend_info["servicePort"]
        service_ip = extend_info["serviceIp"]
        self.bin_path = extend_info.get(ParamField.BIN_PATH.value, "")
        self.mongodump_bin_path = extend_info.get(ParamField.MOBGODUMP_BIN_PATH.value, "")
        self.url_arbiter = str(service_ip) + ":" + str(service_port)
        self.port = int(service_port)
        self.cmd = Cmd(self.job_manager.pid)
        self.username = self.cmd.get_db_user()
        password = self.cmd.get_db_pwd()
        self.auth_type = self.cmd.get_db_user_auth_type()
        bind_ip = get_mongo_bind_ip(service_port, extend_info["serviceIp"])
        self.url = str(bind_ip) + ":" + str(service_port)
        self.db = DB(self.url, {"username": self.username, "password": password, "auth_type": self.auth_type},
                     direct_connection=True, bin_path=self.bin_path)
        self._node_info = {
            "endpoint": "",
            "extendInfo": "",
            "name": "",
            "role": RoleMode.SLAVE_MODE.value,
            "subType": "MongoDB-cluster",
            "type": "",
            "id": ""
        }

    @classmethod
    def parse_host_str(cls, host_str: str):
        if not host_str:
            return list()
        if "/" in host_str:
            tmp_splits = host_str.split("/")
            return tmp_splits[1].split(",") if len(tmp_splits) else list()
        return list()

    def get_auth_type(self):
        """
        获取认证信息
        """
        return self.auth_type

    def connect_cluster(self):
        """
        连接认证
        """
        LOGGER.info(f"Begin to connect cluster from pid :{self.job_manager.pid}")
        try:
            self.db.connect()
        except DBConnectionError as e:
            LOGGER.error(f"DBConnectionError, unable to connect to {self.db.uri}! Error: {e}")
            return False, ErrorCode.INSTANCE_CONNECT_ERROR.value, self.url_arbiter
        except DBAuthenticationError as e:
            LOGGER.error(f"DBAuthenticationError, unable to connect to {self.db.uri}! Error: {e}")
            return False, ErrorCode.INSTANCE_AUTH_ERROR.value, self.url_arbiter
        return True, MongoDBCode.SUCCESS.value, ""

    def get_local_host(self, shard_cluster_type: str):
        """
        获取本地主机url
        """
        if shard_cluster_type == MongoRoles.MONGOS or shard_cluster_type == MongoRoles.SINGLE:
            return self.url
        status = self.db.admin_command("replSetGetStatus")
        ips = get_local_ips()
        ips.append(socket.gethostname())
        for cnf_member in status['members']:
            if self.check_local_instance(ips, cnf_member.get("name")):
                return cnf_member.get("name")
        return self.url

    def get_agent_host(self):
        """
        获取主机url
        """
        return self.url_arbiter

    def get_nodes_agent_list(self, shard_cluster_type):
        """
        获取当前分片类型列表
        """
        agent_list = list()
        if shard_cluster_type == MongoRoles.MONGOS:
            list_shards = self.db.admin_command("listShards")
            shards = list_shards.get("shards", [])
            for shard in shards:
                agent_list.extend(self.parse_host_str(shard.get("host", "")))
            line_opts = self.db.admin_command("getCmdLineOpts")
            cfg_db = line_opts.get(MongoRoles.PARSED, {}).get(MongoRoles.SHARDING, {}).get(MongoRoles.CONFIG_DB, "")
            agent_list.extend(self.parse_host_str(cfg_db))
        if shard_cluster_type == MongoRoles.REPLICATION:
            status = self.db.admin_command("replSetGetStatus")
            for cnf_member in status['members']:
                name = cnf_member.get("name")
                if name:
                    agent_list.append(name)
        return ",".join(agent_list)

    def get_version(self):
        """
        获取集群版本
        """
        return self.db.server_version()

    def get_cluster_type(self):
        """
        获取集群版本
        """
        return self.param.application.get("extendInfo").get("cluster_type")

    def get_agent_shard_list(self):
        """
        获取集群版本
        """
        list_shards = self.db.admin_command("listShards")
        shards = list_shards.get("shards", [])
        agent_list = list()
        for shard in shards:
            LOGGER.info(f"Read or write dir is wrong! shard: {shard}")
            agent_data = "%s/%s" % (shard.get("_id", ""), shard.get("host", "").split("/")[1])
            agent_list.append(agent_data)
        return ";".join(agent_list)

    def auth_password_user(self):
        """
        认证密码用户
        """
        try:
            status = self.db.admin_command("getCmdLineOpts")
        except DBOperationError as e:
            return False, ErrorCode.INSTANCE_AUTH_ERROR.value, self.url_arbiter
        auth_type_no_auto_matches = self.auth_type == str(AuthType.NO_AUTO.value)
        parsed_status_exists = status.get("parsed")
        security_disabled = not parsed_status_exists.get("security") or parsed_status_exists.get("security").get(
            "authorization") == "disabled"
        if auth_type_no_auto_matches and parsed_status_exists and security_disabled:
            return True, MongoDBCode.SUCCESS.value, ""
        auth_type_password_matches = self.auth_type == str(AuthType.APP_PASSWORD.value)
        security_enabled = status.get("parsed").get("security").get("authorization") == "enabled"
        if auth_type_password_matches and parsed_status_exists and security_enabled:
            return True, MongoDBCode.SUCCESS.value, ""
        return False, ErrorCode.INSTANCE_AUTH_ERROR.value, self.url_arbiter

    def auth_user_role(self):
        """
        认证用户权限是否满足要求
        """
        if not self.username:
            return True, MongoDBCode.SUCCESS.value, ""
        shard_cluster_type = self.get_node_server_type()
        if shard_cluster_type == MongoRoles.SHARD or shard_cluster_type == MongoRoles.REPLICATION:
            status = self.db.admin_command("replSetGetStatus")
            ips = get_local_ips()
            ips.append(socket.gethostname())
            for cnf_member in status['members']:
                if self.check_local_instance(ips, cnf_member.get("name")) and cnf_member.get("stateStr") == "ARBITER":
                    return True, MongoDBCode.SUCCESS.value, ""
        users = self.db.admin_command({"usersInfo": self.username})
        if not users["users"]:
            return False, ErrorCode.USER_ROLE_PERMISSION_ERROR.value, self.url_arbiter.split(":")[0]
        user = users["users"][0]
        if not user["roles"]:
            return False, ErrorCode.USER_ROLE_PERMISSION_ERROR.value, "[root, clusterAdmin]"
        for role in user['roles']:
            if role.get("role") and role.get("db"):
                if role.get("db") == "admin" and (role.get("role") == "root" or role.get("role") == "clusterAdmin"):
                    return True, MongoDBCode.SUCCESS.value, ""
        return False, ErrorCode.USER_ROLE_PERMISSION_ERROR.value, "[root, clusterAdmin]"

    def check_mongo_user_and_path(self, url, path):
        """
        检查bin路径和mongodump路径是否一致逻辑 抽取公共函数
        """
        LOGGER.info("Begin check user and path")
        if not self.cmd.check_mongo_user_and_path(path):
            LOGGER.error("dont pass check_mongo_user_and_path")
            return False, ErrorCode.PARAMS_IS_INVALID.value, "Not suitable user."
        # 路径下的mongo/mongos/mongod是否存在
        LOGGER.info("Begin check_mongo_tool ")
        ret, result = self.cmd.check_mongo_tool(path)
        if not ret:
            LOGGER.error(f"Can not find mongo help in this node, url: {url}, result: {result}")
            return False, ErrorCode.INVALID_BIN_PATH.value, \
                self.url_arbiter + ",mongo/mongos/mongod/mongodump/mongorestre"
        return True, MongoDBCode.SUCCESS.value, \
            self.url_arbiter + ",mongo/mongos/mongod/mongodump/mongorestre"

    def check_bin_path(self, url):
        """
        检查bin路径和mongodump路径是否一致
        """
        # 路径存在性校验, 合法性校验, 安全性校验
        mongo_bin_path = ""
        if self.bin_path:
            if not self.cmd.check_mongo_user_and_path(self.bin_path):
                return False, ErrorCode.PARAMS_IS_INVALID.value, "Not suitable user."
            bin_path_type = check_file_or_dir(self.bin_path)
            if bin_path_type != EnumPathType.DIR_TYPE:
                LOGGER.error(f"Mongo bin path is invalid type: {bin_path_type}, url: {url} can not register")
                return False, ErrorCode.INVALID_BIN_PATH.value, \
                    self.url_arbiter + ",mongo/mongos/mongod/mongodump/mongorestre"
            mongo_bin_path = self.bin_path + ParamField.FORWARD_SLASH.value
        mongo_tools = [MongoTool.MONGO.value, MongoTool.MONGOS.value, MongoTool.MONGOD.value]
        for mongo_tool in mongo_tools:
            ret, code, result = self.check_mongo_user_and_path(url, mongo_bin_path + mongo_tool)
            if not ret:
                return ret, code, result
        mongo_tool_bin_path = ""
        if self.mongodump_bin_path:
            if not self.cmd.check_mongo_user_and_path(self.mongodump_bin_path):
                return False, ErrorCode.PARAMS_IS_INVALID.value, "Not suitable user."
            mongodump_path_type = check_file_or_dir(self.mongodump_bin_path)
            if mongodump_path_type != EnumPathType.DIR_TYPE:
                LOGGER.error(f"Mongo dump bin path is invalid type:{mongodump_path_type}, url:{url} can not register.")
                return False, ErrorCode.INVALID_BIN_PATH.value, \
                    self.url_arbiter + ",mongo/mongos/mongod/mongodump/mongorestre"
            mongo_tool_bin_path = self.mongodump_bin_path + ParamField.FORWARD_SLASH.value
        # 路径下mongodump/mongorestore是否存在
        mongodump_tools = [MongoTool.MONGODUMP.value, MongoTool.MONGORESTORE.value]
        for mongodump_tool in mongodump_tools:
            ret, code, result = self.check_mongo_user_and_path(url, mongo_tool_bin_path + mongodump_tool)
            if not ret:
                return ret, code, result
        return True, MongoDBCode.SUCCESS.value, \
            self.url_arbiter + ",mongo/mongos/mongod/mongodump/mongorestre"

    def check_local_instance(self, ips: list, host_str: str):
        """
        检查本地主机是否一致
        """
        host_url = host_str.split(":")
        if host_url[0] in ips and self.url_arbiter.split(":")[1] == host_url[1]:
            return True
        return False

    def get_config_path(self):
        """
        查询备份配置文件目录信息
        """
        line_opts = self.db.admin_command("getCmdLineOpts")
        if line_opts.get(MongoRoles.PARSED).get("config"):
            return line_opts.get(MongoRoles.PARSED).get("config")
        return ""

    def get_start_cmd(self):
        """
        查询备份配置文件目录信息
        """
        line_opts = self.db.admin_command("getCmdLineOpts")
        if line_opts.get("argv"):
            return json.dumps(line_opts.get("argv"))
        return ""

    def get_config_info(self):
        """
        查询备份配置文件所有信息
        """
        line_opts = self.db.admin_command("getCmdLineOpts")
        if line_opts.get("parsed"):
            return json.dumps(line_opts.get("parsed"))
        return ""

    def get_ret_instance_name(self, shard_cluster_type: str):
        """
        查询集群实例名称
        """
        if shard_cluster_type == MongoRoles.MONGOS or shard_cluster_type == MongoRoles.SINGLE:
            return ""
        status = self.db.admin_command("replSetGetStatus")
        if status.get("set"):
            return status.get("set")
        return ""

    def get_env_nodes(self):
        """
        查询节点信息集合
        """
        member = {}
        status = self.db.admin_command("replSetGetStatus")
        for cnf_member in status['members']:
            name = cnf_member.get("name")
            if self.url_arbiter == name and cnf_member.get("stateStr") == "ARBITER":
                self._node_info["extendInfo"] = {
                    "exist_nodes": "0"
                }
                member[cnf_member.get("name")] = self._node_info
                return list(member.values())
        config = self.db.admin_command("replSetGetConfig")
        config = config['config']
        # 字典追加
        member = {}
        instance_name_list = list()
        for cnf_member in config['members']:
            node_info = {
                "endpoint": "",
                "extendInfo": "",
                "name": "",
                "role": "0",
                "subType": "MongoDB-cluster",
                "type": "",
                "id": ""
            }
            node_info["name"] = cnf_member.get("host")
            node_info["extendInfo"] = {
                "priority": str(cnf_member.get("priority")).split(".")[0],
                "votes": cnf_member.get("votes")
            }
            instance_name_list.append(cnf_member.get("host", ""))
            if node_info["name"] not in member:
                member[node_info["name"]] = node_info
        for cnf_member in status['members']:
            name = cnf_member.get("name")
            if name in member:
                member[name]["extendInfo"].update({
                    "health": str(cnf_member.get("health")).split(".")[0],
                    "stateStr": cnf_member.get("stateStr"),
                    "_id": cnf_member.get("_id"),
                    "role": str(is_intrusive(cnf_member.get("state"))),
                    "instance_name_list": ",".join(instance_name_list)
                })
        return list(member.values())

    def get_data_path(self):
        """
        查询备份数据信息
        """
        shard_cluster_type = self.get_node_server_type()
        if shard_cluster_type == MongoRoles.MONGOS.value:
            return ""
        line_opts = self.db.admin_command(ParamField.GET_CMDLINE_OPTS.value)
        db_path = line_opts.get(MongoRoles.PARSED.value, {}).get(ParamField.STORAGE.value, {}).get(
            ParamField.DB_PATH.value, "")
        if db_path:
            return db_path
        return ""

    def get_node_server_type(self):
        """
        查询分片集群的服务类型 实例为single和复制集集群replication
        """
        config_svr = MongoRoles.CONFIG_SVR
        shards_vr = MongoRoles.SHARDS_VR
        line_opts = self.db.admin_command("getCmdLineOpts")
        # 追加 ok 状态为 0 FALSE 和 1 TRUE
        if not line_opts.get(MongoRoles.PARSED).get(MongoRoles.SHARDING):
            if line_opts.get(MongoRoles.PARSED).get(MongoRoles.REPLICATION):
                # 判断是否为单节点副本集类型
                if self.check_single_nodel_repl():
                    return MongoRoles.SINGLE_NODE_REPL
                return MongoRoles.REPLICATION
            return MongoRoles.SINGLE
        if line_opts.get(MongoRoles.PARSED).get(MongoRoles.SHARDING).get(MongoRoles.CONFIG_DB):
            return MongoRoles.MONGOS
        if line_opts.get(MongoRoles.PARSED).get(MongoRoles.SHARDING).get(MongoRoles.CLUSTER_ROLE):
            if config_svr == line_opts.get(MongoRoles.PARSED).get(MongoRoles.SHARDING).get(MongoRoles.CLUSTER_ROLE):
                return MongoRoles.CONFIG
            elif shards_vr == line_opts.get(MongoRoles.PARSED).get(MongoRoles.SHARDING).get(MongoRoles.CLUSTER_ROLE):
                return MongoRoles.SHARD
        return ""

    def check_single_nodel_repl(self):
        repl_set_status_info = self.db.admin_command("replSetGetStatus")
        members = repl_set_status_info.get("members", [])
        # 如果成员节点只有一个，说明集群状态为单节点副本集实例
        if len(members) == 1:
            return True
        return False

    def close(self):
        """
        关闭节点
        """
        self.db.close()

    def clean(self):
        """
        清理密码
        """
        self.db.clean()

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

import re

from common.const import DeployType
from openGauss.common.base_cmd import VastBaseCmd, GaussCmd
from openGauss.common.common import get_hostname, get_host_ip, path_check, get_ids_by_name, check_injection_char, \
    set_uuid, get_hostname_by_addr
from openGauss.common.const import SyncMode, SubApplication, logger, NodeRole, Status, VB_DEFAULT_PORT, \
    NODE_FIELD
from openGauss.resource.cluster_node import ClusterNode


class GaussCluster:
    def __init__(self, auth, env_path, db_version=None):
        self.prim_node = None
        self.nodes = []
        self.all_cluster_info = []
        self.host_name = get_hostname()
        self._status_detail = None
        self._cluster_name = None
        self._status = None
        self.auth = auth
        self.env_path = env_path
        self._version = db_version
        self._cmd_obj = None

    @property
    def cmd_obj(self):
        if self._cmd_obj is None:
            self._cmd_obj = self._create_cmd_obj()
        return self._cmd_obj

    @property
    def db_version(self):
        if not self._version:
            self._get_db_version(self.cmd_obj)
        return self._version

    @property
    def cluster_state(self):
        """
        get cluster_state.
        """
        if self._status is None:
            self._status = self._get_status()
        return self._status

    @property
    def cluster_name(self):
        """get cluster_name.
        """
        if self._cluster_name is None:
            self._cluster_name = self._get_cluster_name()
        return self._cluster_name

    @property
    def deploy_type(self):
        """
        get deploy_type from all cluster information lines
        deploy_type = 1  ->  single
        deploy_type = 3  ->  cluster
        """
        if not self.nodes:
            self.get_cluster_nodes()
        if self.nodes:
            return DeployType.CLUSTER_TYPE.value if len(self.nodes) > 1 else DeployType.SINGLE_TYPE.value
        return DeployType.INVALID_TYPE.value

    @property
    def sync_state(self):
        return self._get_sync_state()

    def check_env_path(self):
        if self.env_path and not path_check(self.env_path):
            logger.error("Env path invalid or not exist.")
            return False
        return True

    def check_db_user(self):
        if not check_injection_char(self.auth):
            logger.error("Invalid user")
            return False
        try:
            uid, _ = get_ids_by_name(self.auth)
        except KeyError as e:
            logger.error("User not exist.")
            return False
        return True

    def check_db_version(self):
        version = self.db_version
        if not version:
            return False
        version_title = version.split(" ")[0]
        ver_tuple = (SubApplication.VASTBASE, SubApplication.OPENGAUSS, SubApplication.MOGDB, SubApplication.CMDB)
        return version_title in ver_tuple

    def check_endpoint(self, nodes):
        """
        校验集群节点一至性检验
        :param nodes: input nodes ip list
        :return: bool
        """
        self.get_cluster_nodes()
        return len(nodes) == len(self.nodes)

    def get_cluster_nodes(self):
        """通过解析查询详情命令结果生成节点信息
        """
        if self.nodes:
            return
        if not self.db_version:
            logger.error("Get cluster nodes failed with unknown db version.")
            return
        if SubApplication.VASTBASE in self.db_version:
            status = self._get_vb_status_all()
            parse_status_func = self._gen_vb_nodes_info
        else:
            status = self._get_gs_status_view()
            parse_status_func = self._gen_gs_nodes_info
        parse_status_func(status)
        if self.prim_node is None:
            self._get_primary_node()

    def get_cluster_nodes_detail(self):
        """通过解析查询详情命令结果生成节点信息
        """
        ret, cont = self.cmd_obj.get_status_detail()
        if not ret:
            logger.error(f'Execute show status detail failed with ret: {ret}.')
        return ret, cont

    def get_endpoint_by_hostname(self):
        """
        get IP address by it's hostname
        """
        temp_ip = get_host_ip(self.host_name)
        if not temp_ip:
            logger.error("Failed get host ip by host name")
            return ""
        return temp_ip

    def get_instance_port(self):
        if not self.nodes:
            self.get_cluster_nodes()
        node = self.nodes[0]
        port = node.instance_port
        return port

    def get_instance_data_path(self):
        if not self.nodes:
            self.get_cluster_nodes()
        node = self.nodes[0]
        data_path = node.data_path
        return data_path

    def get_system_identifier(self):
        db_path = self.get_instance_data_path()
        ret, cont = self.cmd_obj.get_control_data(db_path)
        if not ret:
            logger.error("Failed execute cmd to get system identifier")
            return ""
        system_identifier = re.search("system identifier:\s+(\d+)", cont)
        if not system_identifier:
            system_identifier = re.search(u"数据库系统标识符:\s+(\d+)", cont)
        return system_identifier.group(1) if system_identifier else ""

    def get_replicon_info(self):
        data_path = self._get_vb_node_data_path()
        num = 1
        replicons = set()
        while num:
            rep = f"replconninfo{num}"
            ret, replicon_info = self.cmd_obj.get_replcon_info(data_path, rep)
            if not ret:
                break
            rep = re.search(
                f"The value of parameter.*\s+{rep}='localhost=(.*)\s+localport.*remotehost=(.*)\s+remoteport.*'",
                replicon_info)
            if rep:
                localhost = rep.group(1)
                remotehost = rep.group(2)
                replicons.update({localhost, remotehost})
            num += 1
        return replicons

    def _get_db_version(self, cmd_obj):
        if cmd_obj is None:
            logger.error("Get db version failed with none type cmd. ")
            return ""
        ret, cont = cmd_obj.get_db_version_info()
        if not ret:
            logger.error("Get db version failed with cmd execute result.")
            return ""
        version_parten = re.compile("\((.*)\s.*[Bb]uild")
        search_obj = version_parten.search(cont)
        db_version = "" if search_obj is None else search_obj.group(1)
        if db_version:
            self._version = db_version
        if SubApplication.CMDB in cont:
            self._version = SubApplication.CMDB + " 2.0.0"
        return db_version

    def _create_cmd_obj(self):
        cmd = None
        if not self.check_db_user():
            logger.error(f"Create cmd_obj failed with invalid user")
            return cmd
        if not self.check_env_path():
            logger.error(f"Create cmd obj failed with invalid env_path :{self.env_path} ")
            return cmd
        cmd = GaussCmd(self.auth, self.env_path)
        db_version = self._get_db_version(cmd)
        if SubApplication.VASTBASE in db_version:
            cmd = VastBaseCmd(self.auth, self.env_path)
        return cmd

    def _get_primary_node(self):
        if not self.nodes:
            return
        for node in self.nodes:
            if node.instance_role == NodeRole.PRIMARY:
                self.prim_node = node
                break
        else:
            self.prim_node = self.nodes[0]

    def _get_gs_status_all(self):
        ret, cont = self.cmd_obj.get_status_all()
        if not ret:
            logger.error("Execute show all status failed with ret:%s.", ret)
            return ""
        return cont

    def _get_gs_status_detail(self):
        ret, cont = self.cmd_obj.get_status_detail()
        if not ret:
            logger.error("Execute show detail status failed with ret:%s.", ret)
            return ""
        return cont

    def _get_gs_status_view(self):
        ret, cont = self.cmd_obj.get_view_info()
        if not ret:
            logger.error("Execute show status view detail failed with ret:%s.", ret)
            return ""
        return cont

    def _parse_status_all_info(self, all_info: str):
        repl_parten = "node_ip.*:\s+(.*)[^-]*receiver_replay_location.*:\s(.*)\s.*\n"
        repls = re.findall(repl_parten, all_info)
        if not repls:
            return
        node_detail = {}
        for repl_info in repls:
            node_ip, repl = repl_info
            node_detail[node_ip] = repl
        for node in self.nodes:
            if node.node_ip in node_detail:
                node.receiver_replay_location = node_detail[node.node_ip]
        return

    def _parse_detail_status(self, cont):
        data_node_state = re.search("Datanode\s+State[^\d]*(\d+[\D\d]*)", cont)
        if not data_node_state:
            logger.info("Not find data node state detail, maybe cluster not running")
            return
        detail_info = data_node_state.group(1)
        parten_field = re.compile(r"(\d+)\s+([\w.-]+)\s+.*\s+(\d+)\s+\S*\s+(\w)\s+(\w+)\s+(\w+.*)")
        text = parten_field.findall(detail_info)
        nodes_detail = {}
        for node_info in text:
            node_fields = [field.strip() for field in node_info if field]
            _, node_name, instance_id, _, instance_role, instance_state = node_fields
            instance_role = NodeRole.PRIMARY if instance_role == NodeRole.PRIMARY else NodeRole.STANDBY
            instance_state = Status.NORMAL if instance_state == Status.NORMAL else Status.UNAVAILABLE
            nodes_detail[node_name] = (instance_id, instance_role, instance_state)
        for node in self.nodes:
            fields = nodes_detail.get(node.node_name)
            if fields:
                node.instance_id, node.instance_role, node.instance_state = nodes_detail.get(node.node_name)

    def _gen_gs_nodes_info(self, output):
        """
        从命令结果文本解析数据节点信息
        :param output:
        :return:
        """
        if not output:
            logger.error("Can't parse gs node without content.")
            return
        text = [line for line in output.split("\n") if line]
        for line in text:
            if ":" not in line:
                continue
            field, value = map(lambda x: x.strip(), line.split(":", maxsplit=1))
            if field not in NODE_FIELD:
                continue
            if field == NODE_FIELD[0]:
                node = ClusterNode()
                node.set_field_value(field, value)
                self.nodes.append(node)
            else:
                if self.nodes:
                    self.nodes[-1].set_field_value(field, value)
        detail_cont = self._get_gs_status_detail()
        if detail_cont:
            self._parse_detail_status(detail_cont)
        all_status_info = self._get_gs_status_all()
        if all_status_info:
            self._parse_status_all_info(all_status_info)

    def _get_vb_status_all(self):
        node_detail = {}
        nodes_set = self.get_replicon_info()
        if not nodes_set:
            logger.info("Not found replication param, it may install as single")
            return node_detail

        dcs_server = self._get_vb_dcs()
        if not dcs_server:
            logger.info("Not found dcs service, vastbase cluster not be managed  with dcs. ")
            return node_detail

        dcs_status = re.search("Active:\s+\(?(\w+)\)?", dcs_server).group(1)
        if not dcs_status == "active":
            logger.info("Dcs not active, can't get cluster nodes detail")
            return node_detail

        has_server = self._get_vb_has()
        if not has_server:
            logger.info("Not found has service, vastbase cluster not be managed  with has. ")
            return node_detail

        has_service = re.search("=?(\/.*has/bin.*)\s+(\/.*has.*\w+)\s+.*\n", has_server)
        if not has_service:
            logger.error("Not found has service with status content")
            return node_detail
        has_bin_path = has_service.group(1)
        has_etc_path = has_service.group(2)
        ret, cont = self.cmd_obj.get_hasctl_cont(has_bin_path, has_etc_path)
        if not ret:
            logger.error("Get has list info failed.")
            return node_detail
        node_detail = self._parse_hasctl_cont(cont)
        for node_ip in nodes_set:
            if node_ip not in node_detail:
                node_detail.update(self._set_default_stopped_vb_node(node_ip))
        return node_detail

    def _parse_hasctl_cont(self, cont):
        self._status_detail = {}
        nodes_pat = re.compile("(\|.*\|)")
        node_list = nodes_pat.findall(cont)
        if len(node_list) < 2:
            return self._status_detail
        for node_cont in node_list[1:]:
            _, node_name, node_ip, role, status, *_ = map(lambda x: x.strip(), node_cont.split("|"))
            status = Status.NORMAL if status == "running" else Status.UNAVAILABLE
            role = NodeRole.PRIMARY if role == "Leader" else NodeRole.STANDBY
            self._status_detail[node_ip] = (node_name, role, status)
        return self._status_detail

    def _get_vb_has(self):
        ret, cont = self.cmd_obj.get_has_service()
        if not ret:
            logger.error("Has service not found, make sure it install.")
            return ""
        return cont

    def _get_vb_dcs(self):
        ret, cont = self.cmd_obj.get_dcs_service()
        if not ret:
            logger.error("Dcs service not found, make sure it install.")
            return ""
        return cont

    def _gen_vb_nodes_info(self, node_detail):
        """
        从命令结果文本解析海量数据节点信息
        """
        if not node_detail:
            local_host = self.get_endpoint_by_hostname()
            role = NodeRole.PRIMARY
            state = self._get_vb_node_status()
            node_detail = self._set_default_stopped_vb_node(local_host, role, state)
        data_path = self._get_vb_node_data_path()
        db_port = str(self._get_vb_node_port(data_path))
        for node_ip, detail in node_detail.items():
            node_name, role, state = detail
            node = ClusterNode()
            node.node_ip = node_ip
            node.node_name = node_name
            node.data_path = data_path
            node.instance_port = db_port
            node.instance_state = state
            node.instance_role = role
            node.instance_id = set_uuid(node.node_ip, node.instance_port)[-4:]
            self.nodes.append(node)

    def _get_vb_node_port(self, data_path):
        port = VB_DEFAULT_PORT
        ret, cont = self.cmd_obj.get_inst_port(data_path)
        if not ret:
            logger.error("Get instance port with execute command failed.err:%s", cont)
            return port
        instance_port = re.search("The value of parameter.*\s+port='?(\d+)'?", cont)
        if instance_port:
            port_str = instance_port.group(1)
            try:
                port = int(port_str)
            except TypeError:
                logger.error("Vailed type to translate the port to int.")
            except ValueError:
                logger.error("Vailed port value to int.")
        return port

    def _get_vb_node_data_path(self):
        ret, cont = self.cmd_obj.get_data_path()
        if not ret:
            logger.error("Failed execute cmd to get node data path.")
            return ""
        if not cont:
            logger.error("No data path in env,check the pgdata.")
        return cont.strip("\n")

    def _get_status(self):
        if SubApplication.VASTBASE in self.db_version:
            cluster_status = self._get_vb_cluster_status()
        else:
            cluster_status = self._get_gs_cluster_status()
        return cluster_status

    def _get_gs_cluster_status(self):
        ret, status_cont = self.cmd_obj.get_status()
        if not ret:
            logger.error("Failed execute cmd to get cluster state.")
            return "Unavailable"
        status_pattern = re.search("cluster_state\s+:\s+(\w+)", status_cont)
        if not status_pattern:
            logger.error("Failed parse cluster state with exec ret content")
            return "Unavailable"
        return status_pattern.group(1)

    def _get_vb_cluster_status(self):
        if not self.nodes:
            self.get_cluster_nodes()
        is_normal = True
        has_primary = False
        status = Status.UNAVAILABLE
        for node in self.nodes:
            if node.instance_role == NodeRole.PRIMARY:
                has_primary = True
            if node.instance_state != Status.NORMAL:
                if node.instance_role == NodeRole.PRIMARY:
                    return status
                is_normal = False
        if not has_primary:
            return status

        status = Status.NORMAL if is_normal else Status.DEGRADED
        return status

    def _get_vb_node_status(self):
        data_path = self._get_vb_node_data_path()
        if not data_path:
            logger.error("Failed to get data path from env,make sure it was in PGDATA.")
            return Status.UNAVAILABLE
        ret, query_detail = self.cmd_obj.get_query_info(data_path)
        if not ret:
            logger.error("Query node status failed,make sure the db is running on right PGDATA")
            return Status.UNAVAILABLE
        return Status.NORMAL

    def _get_cluster_name(self):
        if SubApplication.VASTBASE in self.db_version:
            identify = self.get_system_identifier()
            # 取系统应用标识后4位
            cluster_name = f"{SubApplication.VASTBASE}_{identify[-4:]}"
            return cluster_name
        ret, cont = self.cmd_obj.get_status()
        cluster_state_patern = re.compile("cluster_name.*:\s?(\w+)")
        if not ret:
            logger.error("Failed to get cluster name.")
            return ""
        cluster_state = cluster_state_patern.search(cont)
        return "" if not cluster_state else cluster_state.group(1)

    def _set_default_stopped_vb_node(self, node_ip, node_role=None, node_status=None):
        if node_ip == self.get_endpoint_by_hostname():
            node_name = self.host_name
        else:
            node_name = get_hostname_by_addr(node_ip)
        node_role = node_role if node_role else NodeRole.STANDBY
        node_status = node_status if node_status else Status.UNAVAILABLE
        default_node_detail = {node_ip: (node_name, node_role, node_status)}
        return default_node_detail

    def _get_sync_state(self):
        if not self.nodes:
            self.get_cluster_nodes()
        default_sync_state = SyncMode.SINGER
        data_path = self.get_instance_data_path()
        ret, sync_state_cont = self.cmd_obj.get_sync_state(data_path)
        if not ret:
            logger.error("Get sync status failed")
            return default_sync_state
        sync_pat = re.search("The value of parameter.*\s+synchronous_commit='?(\w+)'?", sync_state_cont)
        sync_detail = re.search("The details for synchronous_commit.*\s+synchronous_commit='?(\w+)'?",
                                sync_state_cont)
        if not sync_pat and not sync_detail:
            return default_sync_state
        if sync_pat:
            sync_on = sync_pat.group(1)
        else:
            sync_on = sync_detail.group(1)
        if sync_on != "off":
            sync_state = SyncMode.SYNC
        else:
            sync_state = SyncMode.ASYNC if len(self.nodes) > 1 else default_sync_state
        return sync_state

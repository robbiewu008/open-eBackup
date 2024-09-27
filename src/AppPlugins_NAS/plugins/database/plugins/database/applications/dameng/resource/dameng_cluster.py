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
import stat

from common.common import exter_attack
from common.const import JobData, AuthType, RoleType, ParamConstant
from common.logger import Logger
from dameng.commons.common import get_hostsn, IniParses, get_env_value, del_file, matching_dameng_field, \
    parse_matching_output, open_grep
from dameng.commons.const import MPP_FLAG, ARCH_FLAG, DM_FILE_PATH, ArrayIndex, \
    ConfigureFlag, ExecCmdResult, ClusterNodeMode
from dameng.commons.dameng_tool import DmSqlTool
from dameng.commons.dm_ctlcvt_tool import DmCtlcvt
from dameng.commons.dm_param_parse import verifying_special_characters
from dameng.commons.path_operation import dameng_user_mkdir
from dameng.commons.query_information import query_db_status, get_db_name, get_oguid, get_version
from dameng.resource.damengsource import DamengSource

LOGGER = Logger().get_logger("dameng.log")


class DamengCluster(DamengSource):

    def __init__(self):
        self.inst_port = 0
        self.user_info = {}
        self.dmini_path = ''
        self.status_flag = "0"
        self.big_version = ''

    @staticmethod
    def get_cluster_flag(dmini_path):
        """
        获取是否是集群节点参数
        :param dmini_path:ini文件路径
        :return:
        """
        LOGGER.info("Start get cluster flag.")
        # 1. 获取归档信息
        result = matching_dameng_field("ARCH_INI", dmini_path)
        if len(result) != 1:
            LOGGER.error(f"Failed to query archive information.")
            return (ConfigureFlag.NOT_EXIST, ConfigureFlag.NOT_EXIST)
        arch_flag = result[0]
        if arch_flag != ConfigureFlag.EXIST:
            LOGGER.error(f"Archive is not enabled.")
            arch_flag = ConfigureFlag.NOT_EXIST

        # 2. 获取集群配置
        result = matching_dameng_field("MPP_INI", dmini_path)
        if len(result) != 1:
            LOGGER.error(f"Failed to obtain MPP_INI information.")
            return (arch_flag, ConfigureFlag.NOT_EXIST)
        mpp_flag = result[0]
        if mpp_flag != ConfigureFlag.EXIST:
            LOGGER.error(f"The node is not a cluster node.")
            mpp_flag = ConfigureFlag.NOT_EXIST
        return (arch_flag, mpp_flag)

    @staticmethod
    def get_mppconfigfile_path(dmini_path):

        LOGGER.info("Start get configfile path.")
        result = matching_dameng_field("SYSTEM_PATH", dmini_path)
        if len(result) != 1:
            LOGGER.error(f"Get SYSTEM_PATH fail.")
            return ('', '')
        system_path = result[0]
        result = matching_dameng_field("CONFIG_PATH", dmini_path)
        if len(result) != 1:
            LOGGER.error(f"Get CONFIG_PATH fail.")
            return ('', '')
        config_path = result[0]
        return (system_path, config_path)

    @staticmethod
    def refactor_malini(mal_path):

        LOGGER.info("Start refactor dmmal.ini.")
        config_path = os.path.join(mal_path, "dmmal.ini")
        result = open_grep("MAL_INST", config_path, ignore_case=True)
        if not result:
            return False
        flags = os.O_WRONLY | os.O_EXCL
        modes = stat.S_IWUSR | stat.S_IRUSR
        file_path = os.path.join(ParamConstant.RESULT_PATH, f"{JobData.PID}dmmal.ini")
        if not os.path.exists(file_path):
            flags = flags | os.O_CREAT
        with os.fdopen(os.open(file_path, flags, modes), 'w') as fout:
            fout.truncate(0)
            for mal_info in result:
                fout.write(f"{mal_info}\n")
        return True

    def get_primary_info(self, dmini_path):

        LOGGER.info("Start get primary and arch info.")
        # 1.
        primary_info = []
        file_path = self.get_mppconfigfile_path(dmini_path)
        if file_path == ('', ''):
            LOGGER.error("Get configfile fail.")
            return primary_info
        system_path, config_path = file_path
        src = os.path.join(system_path, "dmmpp.ctl")
        if not dameng_user_mkdir(DM_FILE_PATH):
            return primary_info
        dest = os.path.join(DM_FILE_PATH, f"dmmpp_{JobData.PID}.ini")
        return_code, out_info, err_info = DmCtlcvt().run_ctlcvt_tool(src=src, dest=dest, fun_type=1)
        if return_code != ExecCmdResult.SUCCESS:
            LOGGER.error(f'Cvt to ini fail.')
            return primary_info
        with open(dest) as file_data:
            file_list = file_data.readlines()
            for line in file_list:
                out_msg = parse_matching_output("inst_name", line)
                if not out_msg:
                    continue
                primary_name = out_msg
                primary_info.append(primary_name)
        del_file(dest)
        return primary_info

    def get_cluster_info(self, dmini_path):

        LOGGER.info("Start get cluster info.")
        # 1.
        cluster_info = []
        path_info = self.get_mppconfigfile_path(dmini_path)
        if path_info == ('', ''):
            LOGGER.error("Get configfile fail.")
            return cluster_info
        _, config_path = path_info

        if self.refactor_malini(config_path):
            ini_file_path = os.path.join(ParamConstant.RESULT_PATH, f'{JobData.PID}dmmal.ini')
            iniparses = IniParses(ini_file_path)
            all_ep_name = iniparses.get_all_ep()
            for ep_name in all_ep_name:
                inst_name = iniparses.get_ep_value(ep_name, "MAL_INST_NAME")
                inst_host = iniparses.get_ep_value(ep_name, "MAL_INST_HOST")
                inst_port = iniparses.get_ep_value(ep_name, "MAL_INST_PORT")

                ep_info = {
                    "name": inst_name,
                    "endpoint": inst_host,
                    "extendInfo": {"port": inst_port},
                    "id": ""
                }
                cluster_info.append(ep_info)
        return cluster_info

    def prepare_db_info(self, param_info_):

        self.inst_port = param_info_.get("application", {}).get("extendInfo", "").get("port", "")
        if not verifying_special_characters(self.inst_port):
            LOGGER.error("Failed to check port.")
            return {}
        if not self.inst_port:
            LOGGER.error("Get port fail.")
            return {}

        # 2. 设置数据库信息
        authtype = get_env_value(f"application_auth_authType_{JobData.PID}")
        if not authtype:
            LOGGER.error("Get authType fail.")
            return {}
        elif int(authtype) == AuthType.OS_PASSWORD.value:
            user_info = {
                "port": self.inst_port,
                "auth_type": AuthType.OS_PASSWORD,
                "single_or_cluser": 'single',
                "is_connect_other_version": False
            }
        elif int(authtype) == AuthType.APP_PASSWORD.value:
            env_user = f"application_auth_authKey_{JobData.PID}"
            env_pwd = f"application_auth_authPwd_{JobData.PID}"
            user_info = {
                "port": self.inst_port, "userkey": env_user, "pwdkey": env_pwd,
                "auth_type": AuthType.APP_PASSWORD, "single_or_cluser": 'single',
                "is_connect_other_version": False
            }
        else:
            LOGGER.error("Authtype error.")
            return {}

        dmsql_tool = DmSqlTool(user_info)
        self.big_version = dmsql_tool.get_db_real_version()
        user_info["big_version"] = self.big_version
        return user_info

    def get_resoursce_info(self):

        dmini_path_list = self.get_all_dmini_path()
        if not dmini_path_list:
            LOGGER.error("Get dmini path fail.")
            return []
        resoursce_msg = []
        for dmini_path in dmini_path_list:
            result = self.get_instancename_port(dmini_path)
            # 2：结果中包含实例名和端口
            if len(result) < 2:
                continue
            instance_name, port = result[0], result[1]
            if port == self.inst_port:
                resoursce_msg.append((port, instance_name, dmini_path))
                # 保证只有一个相同的端口
                if len(resoursce_msg) > ArrayIndex.INDEX_FIRST_1:
                    LOGGER.error("Instances with the same port exist.")
                    return []
        return resoursce_msg

    def get_all_node_info(self, dmini_path):
        LOGGER.info("Start get all node info.")
        # 1. 获取所有主节点实例名
        primary_info = self.get_primary_info(dmini_path)
        if primary_info == []:
            LOGGER.error("Get primary and arch info fail.")
            return []

        # 2. 组装集群所有节点信息
        all_node_info = self.get_cluster_info(dmini_path)
        for node in all_node_info:
            node_name = node.get("name", "")
            find_count = primary_info.count(node_name)
            node_extend = node.get("extendInfo", {})
            if find_count == ArrayIndex.INDEX_FIRST_0:
                node_extend["role"] = RoleType.STANDBY.value
            elif find_count == ArrayIndex.INDEX_FIRST_1:
                node_extend["role"] = RoleType.PRIMARY.value
            else:
                return []
        return all_node_info

    @exter_attack
    def get_resource(self, param_info_):

        cluster_resoursce = {
            "id": "",
            "name": "",
            "type": "",
            "subType": "",
            "endpoint": "",
            "role": 0,
            "nodes": [],
            "extendInfo": {}
        }
        LOGGER.info("Start get dameng cluster resource.")
        cluster_resoursce["id"] = get_hostsn()
        # 1. 准备数据库登录信息
        self.user_info = self.prepare_db_info(param_info_)
        cluster_resoursce.get("extendInfo", {})["bigVersion"] = self.big_version
        resoursce_info = self.get_resoursce_info()
        if resoursce_info == []:
            LOGGER.error("Resource not found.")
            return cluster_resoursce
        try:
            self.dmini_path = resoursce_info[ArrayIndex.INDEX_FIRST_0][ArrayIndex.INDEX_LAST_1]
        except IndexError:
            LOGGER.error("Index error.")
            return {}
        arch_info, mpp_info = self.get_cluster_flag(self.dmini_path)

        # 2.查询集群信息
        if arch_info != ARCH_FLAG or mpp_info != MPP_FLAG:
            LOGGER.error('Not mpp cluster.')
            return {}
        # 1. 获取所有主节点实例名
        all_node_info = self.get_all_node_info(self.dmini_path)
        if not all_node_info:
            return {}
        cluster_resoursce["nodes"] = all_node_info
        # 2. 查询当前节点实例状态
        status_info, mode = query_db_status(self.user_info)
        if status_info == 'OPEN':
            self.status_flag = '1'
        for node in cluster_resoursce.get('nodes'):
            if node.get("name", '') != resoursce_info[ArrayIndex.INDEX_FIRST_0][ArrayIndex.INDEX_FIRST_1]:
                continue
            if self.output_node_info(node, cluster_resoursce):
                node_extend_info = node.get("extendInfo", {})
                node_extend_info["role"] = RoleType.STANDBY.value
                if mode == ClusterNodeMode.NODE_MODE_PRIMARY:
                    node_extend_info["role"] = RoleType.PRIMARY.value
                return cluster_resoursce
        return {}

    def output_node_info(self, node_, cluster_resoursce_):

        node_extend_info = node_.get("extendInfo", {})
        node_extend_info["instanceStatus"] = self.status_flag
        path_info = self.dmini_path.strip("\"").split("/")
        if len(path_info) < ArrayIndex.INDEX_FIRST_2:
            return False
        db_path = path_info[ArrayIndex.INDEX_FIRST_0:ArrayIndex.INDEX_LAST_2]
        node_extend_info["dbPath"] = '/'.join(db_path)
        cluster_resoursce_.get("extendInfo", {})["version"] = get_version(self.user_info)
        result_type, db_name = get_db_name(self.user_info)
        if not result_type:
            LOGGER.error("Get db_name fail.")
            return False
        node_extend_info["dbName"] = db_name
        node_extend_info["dminiPath"] = self.dmini_path.strip("\"")
        oguid = get_oguid(self.user_info)
        node_extend_info["groupId"] = oguid
        node_["id"] = get_hostsn()
        return True
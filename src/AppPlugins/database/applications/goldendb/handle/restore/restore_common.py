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

from goldendb.logger import log
from goldendb.handle.common.const import GoldenDBJsonConst, GoldenJsonConstant, GoldenSubJobName


class GoldenDBRestoreCommon:
    def __init__(self, json_param):
        """
        功能描述：__init__ 构造函数 GoldenDB恢复参数获取类
        @param json_param: 恢复参数
        """
        self._json_param = json_param

    @staticmethod
    def get_sub_job_name(param):
        """
        获取sub job执行函数名称
        :param param:
        :return:
        """
        sub_name = param.get(GoldenJsonConstant.SUB_JOB, {}).get(GoldenJsonConstant.JOB_NAME, "")
        if sub_name not in (GoldenSubJobName.SUB_CHECK, GoldenSubJobName.SUB_EXEC, GoldenSubJobName.SUB_BINLOG_MERGE,
                            GoldenSubJobName.SUB_BINLOG_MOUNT, GoldenSubJobName.SUB_VER_CHECK):
            log.error(f"{sub_name} not found in sub jobs!")
            return ""
        return sub_name

    @staticmethod
    def get_cluster_info(resource_info):
        """
        获取集群extendInfo下实例信息
        @param resource_info: 源实例信息 或者目标实例信息
        @return:resource_cluster_info_dict 集群信息字典
        """
        resource_object_extend_info = resource_info.get(GoldenDBJsonConst.EXTENDINFO, {})
        if not resource_object_extend_info:
            log.error("Get target object or protect object extend info failed.")
            return {}
        resource_cluster_info = resource_object_extend_info.get(GoldenDBJsonConst.CLUSTERINFO, "")
        if not resource_cluster_info:
            log.error("Get target cluster info or protect cluster info extend info failed.")
            return {}
        resource_cluster_info_dict = json.loads(resource_cluster_info)
        if not resource_cluster_info_dict:
            log.error("Json load resource cluster info failed.")
        return resource_cluster_info_dict

    def get_protect_object(self):
        """
        获取源实例的资源信息
        @return: 源实例的资源信息
        """
        return self._json_param.get(GoldenDBJsonConst.JOB, {}).get(GoldenDBJsonConst.COPIES)[0] \
            .get(GoldenDBJsonConst.PROTECTOBJECT, {})

    def get_copy_version(self):
        """
        获取该副本对应集群的版本
        @return: 集群版本信息
        """
        return self._json_param.get(GoldenDBJsonConst.JOB, {}).get(GoldenDBJsonConst.COPIES)[0].get(
            GoldenDBJsonConst.EXTENDINFO, {}).get(GoldenDBJsonConst.VERSION, "V0")

    def get_target_object(self):
        """
        获取目标实例的资源信息
        @return: 目标实例信息字典
        """
        return self._json_param.get(GoldenDBJsonConst.JOB, {}).get(GoldenDBJsonConst.TARGETOBJECT, {})

    def get_manager_node_info(self):
        """
        获取管理节点的信息
        @return: 管理节点信息字典
        """
        target_env = self._json_param.get(GoldenDBJsonConst.JOB, {}).get(GoldenDBJsonConst.TARGETENV, {})
        if not target_env:
            log.error("Get target env failed.")
            return {}
        zxmanager_info = target_env.get(GoldenDBJsonConst.EXTENDINFO, {}).get(GoldenDBJsonConst.GOLDENDB, "")
        if not zxmanager_info:
            log.error("Get zx manager nodes info env failed.")
            return {}
        return json.loads(zxmanager_info)

    def get_cluster_instance_info(self):
        """
        获取集群extendInfo下实例信息
        @return: golden db集群实例信息
        """
        return GoldenDBRestoreCommon.get_cluster_info(self.get_target_object())

    def check_restore_golden_db_structure(self):
        """
        检查golden db源集群与目标集群结构不一致
        @return:检查结果 True相同 False不同
        """
        target_cluster_info_dict = GoldenDBRestoreCommon.get_cluster_info(self.get_target_object())
        protect_cluster_info_dict = GoldenDBRestoreCommon.get_cluster_info(self.get_protect_object())
        # 校验目标实例和源实例的分片数量是否一致
        if len(target_cluster_info_dict.get(GoldenDBJsonConst.GROUP, [])) \
                != len(protect_cluster_info_dict.get(GoldenDBJsonConst.GROUP, [])):
            log.error("The number of group list is not matched.")
            return False
        return True

    def get_target_cluster_id(self):
        """
        获取目标集群实例的cluster id
        @return: cluster id
        """
        cluster_info = self.get_cluster_instance_info()
        cluster_id = cluster_info.get(GoldenDBJsonConst.ID, "")
        if not cluster_id:
            log.error("Get cluster id failed.")
            return ""
        return cluster_id

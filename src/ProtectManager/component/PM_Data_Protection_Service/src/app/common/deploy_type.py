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
import os

from app.common import logger
from app.common.clients.client_util import InfrastructureHttpsClient, is_response_status_ok, parse_response_data
from app.common.enums.resource_enum import DeployTypeEnum
from app.common.exter_attack import exter_attack

LOGGER = logger.get_logger(__name__)


class DeployType(object):
    _deploy_type = ""

    @staticmethod
    @exter_attack
    def _get_from_config_map():
        url = f"/v1/infra/configmap/info?nameSpace=dpa&configMap=common-conf&configKey=deploy_type"
        response = InfrastructureHttpsClient().request("GET", url)
        LOGGER.info(f"get deploy type from configmap, response: {response.data}")
        if not is_response_status_ok(response):
            LOGGER.error(f"get infra/configmap/info error")
            return ""
        system_info = parse_response_data(response.data).get("data", [])
        if system_info:
            return system_info[0].get("deploy_type", "")
        LOGGER.error(f"parse response data error.")
        return ""

    def get_deploy_type(self):
        if self._deploy_type != "":
            return self._deploy_type
        # 从环境变量或configMap中获取
        self._deploy_type = os.getenv("DEPLOY_TYPE")
        if self._deploy_type not in DeployTypeEnum.__members__.values():
            self._deploy_type = self._get_from_config_map()
        return self._deploy_type

    def is_ocean_protect_type(self):
        if self._deploy_type == "":
            self.get_deploy_type()
        return self._deploy_type in [DeployTypeEnum.X9000.value, DeployTypeEnum.X8000.value, DeployTypeEnum.A8000.value,
                                     DeployTypeEnum.X6000.value, DeployTypeEnum.X3000.value]

    def is_cloud_backup_type(self):
        if self._deploy_type == "":
            self.get_deploy_type()
        return self._deploy_type in [DeployTypeEnum.CloudBackup.value, DeployTypeEnum.CloudBackupOld.value]

    def is_hyper_detect_deploy_type(self):
        if self._deploy_type == "":
            self.get_deploy_type()
        return self._deploy_type in [DeployTypeEnum.HyperDetect.value]

    def is_x3000_type(self):
        if self._deploy_type == "":
            self.get_deploy_type()
        return self._deploy_type == DeployTypeEnum.X3000.value

    def is_x9000_type(self):
        return self.get_deploy_type() == DeployTypeEnum.X9000.value

    def is_cyber_engine_deploy_type(self):
        if self._deploy_type == "":
            self.get_deploy_type()
        return self._deploy_type in [DeployTypeEnum.CYBER_ENGINE.value]

    def is_dependent(self):
        if self._deploy_type == "":
            self.get_deploy_type()
        return self._deploy_type in [DeployTypeEnum.DEPENDENT.value]

    def is_support_multi_job_deploy_type(self):
        # 支持如及时挂载等多个任务合在一个任务里的部署类型
        if self._deploy_type == "":
            self.get_deploy_type()
        return self._deploy_type in [DeployTypeEnum.X9000.value, DeployTypeEnum.X8000.value,
                                     DeployTypeEnum.A8000.value,
                                     DeployTypeEnum.X6000.value, DeployTypeEnum.X3000.value,
                                     DeployTypeEnum.PACIFIC.value, DeployTypeEnum.DEPENDENT.value]

    def is_not_support_rbac_deploy_type(self):
        # 不支持rbac的部署形态
        if self.get_deploy_type() == "":
            self.get_deploy_type()
        return self._deploy_type in [DeployTypeEnum.CloudBackup.value, DeployTypeEnum.CloudBackupOld.value,
                                     DeployTypeEnum.HyperDetect.value, DeployTypeEnum.CYBER_ENGINE.value]

    def is_not_support_dee_restful_deploy_type(self):
        # 不支持dee的parser接口的部署形态
        if self.get_deploy_type() == "":
            self.get_deploy_type()
        return self._deploy_type in [DeployTypeEnum.CloudBackup.value, DeployTypeEnum.CloudBackupOld.value,
                                     DeployTypeEnum.HyperDetect.value, DeployTypeEnum.CYBER_ENGINE.value]
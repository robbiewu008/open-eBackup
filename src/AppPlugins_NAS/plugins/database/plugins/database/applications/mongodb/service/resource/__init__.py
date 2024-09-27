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

from mongodb.comm.const import AllStepEnum, EnvName
from mongodb.job_manager import JobManager
from mongodb.param.resource_param import ResourceParam
from mongodb.service.base_service import MetaService, CheckApplicationInterfaceMixin, \
    SupportResourceInterfaceMixin
from mongodb.service.resource.mongodb_resource import MongoDBResource


class ResourceService(MetaService):
    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)
        self.param = self.gen_param_obj()
        EnvName.DB_USER_NAME = "appEnv_{}auth_authKey"
        EnvName.DB_PASSWORD = "appEnv_{}auth_authPwd"
        EnvName.DB_AUTH_TYPE = "appEnv_{}auth_authType"
        self.resource_manager = MongoDBResource(self.job_manager, self.param)

    def gen_param_obj(self):
        param = ResourceParam(self.param_dict)
        return param

    def get_steps(self):
        pass


@JobManager.register(AllStepEnum.CHECK_APPLICATION)
class CheckApplicationService(ResourceService, CheckApplicationInterfaceMixin):
    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)

    def get_steps(self):
        return [self.resource_manager.check_application]


@JobManager.register(AllStepEnum.QUERY_CLUSTER)
class QueryCluster(ResourceService, SupportResourceInterfaceMixin):
    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)

    def get_steps(self):
        return [self.resource_manager.query_node_all_cluster]


@JobManager.register(AllStepEnum.SUPPORT_RESOURCE)
class SupportResource(ResourceService, SupportResourceInterfaceMixin):
    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)

    def get_steps(self):
        pass

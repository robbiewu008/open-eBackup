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

import pwd
import grp
from pydantic import BaseModel, Field

from common.common import exter_attack, output_result_file
from tidb.logger import log
from tidb.common.safe_get_information import ResourceParam


class PermissionInfo(BaseModel):
    user: str = Field(default='root', description='user')
    group: str = Field(default='root', description='group')
    file_mode: str = Field(default='0700', description='mod', alias='fileMode')


class JobAbility:
    """
    任务相关接口
    """

    @staticmethod
    @exter_attack
    def query_job_permission(req_id, job_id, sub_id, data):
        log.info(f"step 2: execute QueryJobPermission interface pid:{req_id} job_id:{job_id}")
        param_inst = ResourceParam(req_id)
        param = param_inst.get_param()
        deploy_user = param.get("application", {}).get("extendInfo", {}).get("owner", "")
        log.info(f"User: {deploy_user}")
        if not deploy_user:
            # 获取不到集群用户，使用默认用户tidb
            log.error("Failed get cluster deploy user.")
            output = PermissionInfo(user="tidb", group="tidb", fileMode="0755")
            output_result_file(req_id, output.dict(by_alias=True))
            log.info(f"step 2: execute QueryJobPermission interface success")
            return True
        group_id = pwd.getpwnam(deploy_user).pw_gid
        group = grp.getgrgid(group_id).gr_name
        log.info(f"Group: {group}")
        output = PermissionInfo(user=deploy_user, group=group, fileMode="0755")
        output_result_file(req_id, output.dict(by_alias=True))
        log.info(f"step 2: execute QueryJobPermission interface success")
        return True

    @staticmethod
    @exter_attack
    def abort_job(req_id, job_id, sub_id, data):
        pass

    @staticmethod
    @exter_attack
    def pause_job(req_id, job_id, sub_id, data):
        pass

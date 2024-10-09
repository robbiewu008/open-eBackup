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

from common.common import exter_attack, output_result_file
from common.common_models import ActionResult, JobPermissionInfo
from common.const import ExecuteResultEnum
from common.parse_parafile import ParamFileUtil
from tdsql.handle.restore.exec_restore import Restore
from tdsql.handle.restore.restore_common import get_port_and_ip, remove_deploy_conf
from tdsql.logger import log


class JobAbility:
    """
    任务相关接口
    """

    @staticmethod
    @exter_attack
    def query_job_permission(req_id, job_id, sub_id, data):
        log.info(f"step 2: execute QueryJobPermission interface pid:{req_id} job_id:{job_id}")
        output = JobPermissionInfo(user="root", group="root", fileMode="0770")
        output_result_file(req_id, output.dict(by_alias=True))
        log.info(f"step 2: execute QueryJobPermission interface success")

    @staticmethod
    @exter_attack
    def abort_job(req_id, job_id, sub_id, data):
        param = ParamFileUtil.parse_param_file(req_id)
        log.info(f"abort job param is {param}")
        restore_type = param.get("job", {}).get("jobParam", {}).get("restoreType")
        log.info(f"restore_type is {restore_type}")
        if restore_type:
            restore_inst = Restore(req_id, job_id, sub_id, data, param)
            _, mysql_port, _ = get_port_and_ip(restore_inst.nodes)
            log.info(f"mysql_port is {mysql_port}")
            remove_deploy_conf(mysql_port)
        output_result_file(req_id, ActionResult(code=ExecuteResultEnum.SUCCESS).dict(by_alias=True))

    @staticmethod
    @exter_attack
    def pause_job(req_id, job_id, sub_id, data):
        pass

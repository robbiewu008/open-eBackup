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
import sys

from common.util.check_utils import is_valid_id
from openGauss.common.const import logger, ParamKey, ProtectSubObject
from common.const import ExecuteResultEnum, RepositoryDataTypeEnum
from common.common import output_result_file, exter_attack
from common.common_models import ActionResult
from openGauss.common.common import get_value_from_dict, safe_check_injection_char, check_path, get_repository_dir, \
    repositories_check
from common.parse_parafile import ParamFileUtil
from openGauss.backup.job_depatch import JobDepatch
from openGauss.common.opengauss_param import JsonParam


def param_check(pid, param):
    _, job_id = get_value_from_dict(param, ParamKey.JOB, ParamKey.JOB_ID)
    if not safe_check_injection_char(pid, job_id):
        logger.error(f"Pid or job id err. pid: {pid}, job id: {job_id}")
        return False
    _, env_file = get_value_from_dict(param, ParamKey.JOB, ParamKey.PROTECT_ENV, ParamKey.EXTEND_INFO,
                                      ParamKey.ENV_FILE)
    if env_file and not (os.path.isfile(env_file) and check_path(env_file)):
        logger.error(f"Bad env file. pid: {pid}, job id: {job_id}")
        return False
    _, object_name = get_value_from_dict(param, ParamKey.JOB, ParamKey.PROTECT_OBJECT, ParamKey.NAME)
    _, object_type = get_value_from_dict(param, ParamKey.JOB, ParamKey.PROTECT_OBJECT, ParamKey.SUB_TYPE)
    if object_type == ProtectSubObject.DATABASE and not safe_check_injection_char(object_name):
        logger.error(f"Bad object name. pid: {pid}, job id: {job_id}")
        return False
    ret, repositories = get_value_from_dict(param, ParamKey.JOB, ParamKey.REPOSITORIES)
    return repositories_check(repositories, RepositoryDataTypeEnum.DATA_REPOSITORY,
                              RepositoryDataTypeEnum.CACHE_REPOSITORY)


@exter_attack
def exec_job():
    if len(sys.argv) < 3:
        logger.error(f'Bad agrv!')
        return False
    interface_type = sys.argv[1]
    pid = sys.argv[2]
    # 添加id校验
    if not is_valid_id(pid):
        logger.warn(f'pid is invalid!')
        return False
    try:
        param = JsonParam.parse_param_with_jsonschema(pid)
    except Exception:
        logger.error(f'Failed to parse param file. self._pid: {pid}')
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Failed to parse param file")
        output_result_file(pid, output.dict(by_alias=True))
        return False
    if not param_check(pid, param):
        logger.error(f'Not safe param. pid: {pid}')
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Failed to parse param file")
        output_result_file(pid, output.dict(by_alias=True))
        return False
    ret, job_id = get_value_from_dict(param, ParamKey.JOB, ParamKey.JOB_ID)
    call_function = JobDepatch(pid, job_id, param).depatch.get(interface_type)
    if call_function is None:
        logger.error(f'Incorrect task type. job id: {job_id}')
        output = ActionResult(code=ExecuteResultEnum.INTERNAL_ERROR, message="Incorrect task type.")
        output_result_file(pid, output.dict(by_alias=True))
        return False
    ret, output = call_function()
    output_result_file(pid, output)
    return ret


if __name__ == "__main__":
    exec_job()
    sys.exit(ExecuteResultEnum.SUCCESS)

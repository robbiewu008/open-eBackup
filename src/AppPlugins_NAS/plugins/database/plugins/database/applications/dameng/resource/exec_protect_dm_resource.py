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

from dameng.commons.const import ArrayIndex, SysData
from dameng.commons.dm_param_parse import verifying_special_characters
from dameng.resource.damengsource import DamengSource
from dameng.resource.dameng_cluster import DamengCluster
from dameng.resource.dameng_application import DamengApplication
from dameng.resource.query_job_permission import QueryJobPermission

from common.const import JobData, ParamConstant
from common.common import output_execution_result
from common.logger import Logger
from common.parse_parafile import ParamFileUtil
from common.cleaner import clear

LOGGER = Logger().get_logger("dameng.log")


if __name__ == '__main__':
    LOGGER.info(f"Runing main...{sys.argv[2]}.")

    for line in sys.stdin:
        SysData.SYS_STDIN = line
        break
    if SysData.SYS_STDIN == '':
        LOGGER.error(f"Failed to obtain sys-parameters.")

    PARAM_FILE_PATH = DamengSource.check_param_file_path()
    RESULT_FILE_PATH = DamengSource.check_result_path()
    if not (PARAM_FILE_PATH and RESULT_FILE_PATH):
        clear(SysData.SYS_STDIN)
        sys.exit(1)
    JobData.PID = sys.argv[ArrayIndex.INDEX_FIRST_2]
    if not verifying_special_characters(JobData.PID):
        clear(SysData.SYS_STDIN)
        sys.exit(1)
    func_type = sys.argv[ArrayIndex.INDEX_FIRST_1]
    param = ParamFileUtil.parse_param_file(JobData.PID)

    CMD_DICT = dict(CheckApplication=DamengApplication,
                    QueryJobPermission=QueryJobPermission,
                    DamengCluster=DamengCluster
                    )
    query_instance = CMD_DICT.get(func_type, None)
    if not query_instance:
        LOGGER.info("Unknow funcType.")
        clear(SysData.SYS_STDIN)
        sys.exit(0)

    try:
        result = query_instance().get_resource(param)
    except Exception as exception_str:
        clear(SysData.SYS_STDIN)
        LOGGER.error(f"Function {func_type} exec fail, exp: {exception_str}")
        sys.exit(1)
    FILE_NAME = "{}{}".format("result", JobData.PID)
    LOGGER.info(f"Result:{result} type: {type(result)}.")
    result_file = os.path.join(ParamConstant.RESULT_PATH, FILE_NAME)
    output_execution_result(result_file, result)

    clear(SysData.SYS_STDIN)
    LOGGER.info(f"Main end...")

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

import sys

from dameng.copy.delete_copies import DeleteCopies

from common.common import output_result_file
from common.logger import Logger


LOGGER = Logger().get_logger("dameng.log")


if __name__ == "__main__":
    job_type = sys.argv[1]
    LOGGER.info(f"Main start. job_type: {job_type}.")
    delete_copies_obj = DeleteCopies(*sys.argv[2:])
    job_func_dict = {
        "DeleteCopy": delete_copies_obj.delete_copies_info,
        "DeleteProgress": delete_copies_obj.delete_copies_progress,
    }
    job_func = job_func_dict.get(job_type)
    if not job_func:
        LOGGER.info(f"This command: {job_type} is not supported.")
        sys.exit(1)

    try:
        result = job_func()
    except Exception as exception_str:
        LOGGER.error(f"Exec fail, exp: {exception_str}")
        sys.exit(1)

    output_result_file(sys.argv[2], result.dict(by_alias=True))
    LOGGER.info(f"Main end.")
    sys.exit(0)
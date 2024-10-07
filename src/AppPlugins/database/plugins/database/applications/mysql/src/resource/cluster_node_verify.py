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

from mysql import log
from common.util.exec_utils import exec_overwrite_file
from mysql.src.common.error_code import MySQLErrorCode, MySQLCode
from mysql.src.common.parse_parafile import ParseParaFile
from mysql.src.common.response_param import get_body_error_param
from mysql.src.resource.ap_cluster_verifiy import ApClusterVerify
from mysql.src.resource.pxc_cluster_verify import PxcClusterVerify
from common.util.check_utils import is_valid_id
from common.common import exter_attack
from common.const import ParamConstant


@exter_attack
def check_is_cluster():
    pid = sys.argv[1]
    if not is_valid_id(pid):
        log.warn(f'req_id is invalid')
        sys.exit(1)
    context = ParseParaFile.parse_para_file(pid)
    log.info(f"Begin to check cluster pid: {pid}.")
    application = context.get("application", {})
    cluster_type = application.get("extendInfo", {}).get("clusterType")
    result_path = os.path.join(ParamConstant.RESULT_PATH, f"result{pid}")
    if cluster_type == "PXC":
        log.info(f"Begin to check pxc cluster pid :{pid}")
        pxc_cluster_verify = PxcClusterVerify(pid, context)
        pxc_cluster_verify.check_is_pxc_cluster()
        return True
    elif cluster_type == "AP":
        log.info(f"Begin to check ap cluster pid :{pid}")
        ap_cluster_verify = ApClusterVerify(pid, context)
        ap_cluster_verify.check_is_cluster()
        return True
    log.error(f"Cluster instance param error! pid: {pid}")
    params = get_body_error_param(MySQLCode.FAILED.value, MySQLErrorCode.CHECK_CLUSTER_FAILED,
                                  f"Cluster instance param error!pid: {pid}")
    return exec_overwrite_file(result_path, params)


if __name__ == '__main__':
    check_is_cluster()

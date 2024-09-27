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
import platform
import stat

from common.const import JobData
from common.file_common import change_path_permission
from common.util.exec_utils import exec_append_file
from openGauss.backup.resource_info import ResourceInfo
from openGauss.common.common import write_progress_file, safe_get_environ
from openGauss.common.const import logger, Env, ProgressInfo, Status
from openGauss.common.error_code import NormalErr, OpenGaussErrorCode
from openGauss.resource.cluster_instance import GaussCluster

if platform.system().lower() == "linux":
    import pwd


class CheckRestore:
    def __init__(self, inputs: {}):
        self.input_info = inputs

    def check_user(self):
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return NormalErr.FALSE
        pre_user_name = self.input_info.get("pre_user")
        if not pre_user_name:
            logger.error("Get previous user name error!")
            return NormalErr.FALSE
        if pre_user_name != user_name:
            logger.error(f'Local user not the same as previous user!')
            return OpenGaussErrorCode.ERROR_DIFFERENT_USER
        return NormalErr.NO_ERR

    def check_node_and_topo(self):
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return NormalErr.FALSE
        role_inst = ResourceInfo(user_name, self.input_info.get("env_path"))
        nodes = role_inst.get_cluster_nodes()
        if len(nodes) != len(self.input_info.get("nodes")):
            return OpenGaussErrorCode.ERROR_DIFFERENT_TOPO
        return NormalErr.NO_ERR

    def check_conf(self):
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return
        conf_path = os.path.join(self.input_info.get("media_path"), "backups/backup_instance/pg_probackup.conf")
        if not os.path.exists(conf_path):
            logger.info(f'Creating conf file :{conf_path}')
            exec_append_file(conf_path, self.input_info.get("conf_file"))
            change_path_permission(conf_path, user_name=user_name)

    def check_version(self):
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return NormalErr.FALSE
        inst = GaussCluster(user_name, self.input_info.get("env_path"))
        present_version = inst.db_version
        if not self.input_info.get("version"):
            logger.error(f'No version info!')
            return NormalErr.FALSE
        if self.input_info.get("version") != present_version:
            logger.error(f'Version not matched! {present_version} not equal to {self.input_info.get("version")}')
            return OpenGaussErrorCode.ERROR_DIFFERENT_VERSION
        return NormalErr.NO_ERR

    def check_status_normal(self):
        user_name = safe_get_environ(f"{Env.OPEN_GAUSS_USER}_{JobData.PID}")
        if not user_name:
            logger.error("Get user name error!")
            return NormalErr.FALSE
        inst = GaussCluster(user_name, self.input_info.get("env_path"))
        cluster_state = inst.cluster_state
        if cluster_state not in (Status.NORMAL, Status.DEGRADED):
            logger.error("Cluster state is not normal!")
            return OpenGaussErrorCode.ERR_DATABASE_STATUS
        return NormalErr.NO_ERR

    def pre_restore_job(self):
        """
        前置任务
        :return:
        """
        logger.info("Start to exec pre task")
        # 还需检查备份文件链是否存在
        progress_file = os.path.join(self.input_info.get("cache_path"), f'{JobData.JOB_ID}pre_job')
        write_progress_file(ProgressInfo.START, progress_file)
        self.check_conf()
        check_result = self.check_user()
        if check_result != NormalErr.NO_ERR:
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return check_result
        check_result = self.check_version()
        if check_result != NormalErr.NO_ERR:
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return check_result
        check_result = self.check_node_and_topo()
        if check_result != NormalErr.NO_ERR:
            write_progress_file(ProgressInfo.FAILED, progress_file)
            return check_result
        write_progress_file(ProgressInfo.SUCCEED, progress_file)
        logger.info("Execute pre task succeed")
        return NormalErr.NO_ERR

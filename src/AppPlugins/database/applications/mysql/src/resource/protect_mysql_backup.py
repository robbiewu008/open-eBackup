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

from common.common import exter_attack
from common.const import BackupTypeEnum, SubJobStatusEnum, JobData
from common.parse_parafile import ParamFileUtil
from common.util.check_utils import is_valid_id
from mysql import log
from mysql.src.common.constant import MySQLProgressFileType, MySQLCmdStr, MySQLClusterType
from mysql.src.common.error_code import BodyErr, MySQLCode
from mysql.src.common.parse_parafile import ParseParaFile
from mysql.src.protect_eapp_mysql_backup_full import EAppMysqlBackupFull
from mysql.src.protect_eapp_mysql_backup_inc import EAppMysqlBackupInc
from mysql.src.protect_mysql_backup_full import MysqlBackupFull
from mysql.src.protect_mysql_backup_inc import MysqlBackupInc
from mysql.src.protect_mysql_backup_log import MysqlBackupLog
from mysql.src.protect_mysql_base import MysqlBase
from mysql.src.protect_mysql_live_mount import MysqlLiveMount
from mysql.src.utils.mysql_utils import MysqlUtils


class MysqlBackupComm(object):

    def __init__(self):
        self._mysql_object = None
        self._backup_type = ""
        self.json_param = None
        pass

    @staticmethod
    def get_code(input_ret):
        if input_ret:
            tmp_code = MySQLCode.SUCCESS.value
        else:
            tmp_code = MySQLCode.FAILED.value
        return tmp_code

    @staticmethod
    def format_body_err(input_ret):
        tmp_body_err = 0
        if not input_ret:
            tmp_body_err = BodyErr.ERR_PLUGIN_CANNOT_BACKUP.value
        return tmp_body_err

    @classmethod
    def get_instance(cls):
        if not hasattr(MysqlBackupComm, '_instance'):
            MysqlBackupComm._instance = MysqlBackupComm()
        return MysqlBackupComm._instance

    @exter_attack
    def create_mysql_backuper_by_backup_type(self, tmp_pid, tmp_job_id, tmp_sub_job_id):
        self.json_param = ParseParaFile.parse_para_file(tmp_pid)
        self._backup_type = ParamFileUtil.parse_backup_type(self.json_param.get("job", {}).get("jobParam", ""))
        cluster_type = MysqlUtils.get_cluster_type(self.json_param)
        if self._backup_type == BackupTypeEnum.FULL_BACKUP.value:
            if cluster_type == MySQLClusterType.EAPP:
                self._mysql_object = EAppMysqlBackupFull(tmp_pid, tmp_job_id, tmp_sub_job_id, self.json_param)
            else:
                self._mysql_object = MysqlBackupFull(tmp_pid, tmp_job_id, tmp_sub_job_id, self.json_param)
        elif self._backup_type == BackupTypeEnum.DIFF_BACKUP.value or \
                self._backup_type == BackupTypeEnum.INCRE_BACKUP.value:
            if cluster_type == MySQLClusterType.EAPP:
                self._mysql_object = EAppMysqlBackupInc(tmp_pid, tmp_job_id, tmp_sub_job_id, self.json_param)
            else:
                self._mysql_object = MysqlBackupInc(tmp_pid, tmp_job_id, tmp_sub_job_id, self.json_param)
        elif self._backup_type == BackupTypeEnum.LOG_BACKUP.value:
            self._mysql_object = MysqlBackupLog(tmp_pid, tmp_job_id, tmp_sub_job_id, self.json_param)
        else:
            return False
        return True

    def create_mysql_backuper_comm(self, tmp_pid, tmp_job_id, tmp_sub_job_id):
        self.json_param = ParseParaFile.parse_para_file(tmp_pid)
        self._mysql_object = MysqlBase(tmp_pid, tmp_job_id, tmp_sub_job_id, self.json_param)
        return True

    def create_mysql_live_mount(self, tmp_pid, tmp_job_id, tmp_sub_job_id):
        self.json_param = ParseParaFile.parse_para_file(tmp_pid)
        self._mysql_object = MysqlLiveMount(tmp_pid, tmp_job_id, tmp_sub_job_id, self.json_param)
        return True

    def write_action_result(self, tmp_code, tmp_body_err, tmp_message):
        self._mysql_object.output_action_result(tmp_code, tmp_body_err, tmp_message)

    def write_action_result_when_failed(self, tmp_code, tmp_body_err, tmp_message):
        if tmp_code == MySQLCode.FAILED.value:
            self._mysql_object.output_action_result(tmp_code, tmp_body_err, tmp_message)

    @exter_attack
    def backup(self):
        tmp_ret = self._mysql_object.exec_sub_job()
        tmp_code = self.get_code(tmp_ret)
        progress_type = self._mysql_object.get_progress_file_type()
        if tmp_ret:
            self._mysql_object.write_progress_file(SubJobStatusEnum.COMPLETED.value,
                                                   100, progress_type,
                                                   self._mysql_object.get_self_average_speed())
        else:
            self._mysql_object.write_progress_file(SubJobStatusEnum.FAILED.value,
                                                   100, progress_type)
        return tmp_code, self.format_body_err(tmp_ret), ""

    @exter_attack
    def live_mount(self):
        tmp_ret = self._mysql_object.live_mount_ex()
        tmp_code = self.get_code(tmp_ret)
        if tmp_ret:
            self._mysql_object.write_progress_file(SubJobStatusEnum.COMPLETED.value,
                                                   100, MySQLProgressFileType.COMMON)
        else:
            self._mysql_object.write_progress_file(SubJobStatusEnum.FAILED.value,
                                                   100, MySQLProgressFileType.COMMON)
        return tmp_code, self.format_body_err(tmp_ret), ""

    @exter_attack
    def cancel_live_mount(self):
        tmp_ret = self._mysql_object.cancel_live_mount()
        tmp_code = self.get_code(tmp_ret)
        if tmp_ret:
            self._mysql_object.write_progress_file(SubJobStatusEnum.COMPLETED.value,
                                                   100, MySQLProgressFileType.COMMON)
        else:
            self._mysql_object.write_progress_file(SubJobStatusEnum.FAILED.value,
                                                   100, MySQLProgressFileType.COMMON)
        return tmp_code, self.format_body_err(tmp_ret), ""

    @exter_attack
    def backup_post_job(self):
        tmp_ret = self._mysql_object.exec_rear_job()
        tmp_code = self.get_code(tmp_ret)
        return tmp_code, self.format_body_err(tmp_ret), ""

    @exter_attack
    def abort_job(self):
        self._mysql_object.abort_job()
        return MySQLCode.SUCCESS.value, self.format_body_err(True), ""

    @exter_attack
    def query_backup_copy(self):
        tmp_ret = self._mysql_object.query_copy_info()
        tmp_code = self.get_code(tmp_ret)
        return tmp_code, self.format_body_err(tmp_ret), ""

    @exter_attack
    def report_progress_live_mount(self):
        tmp_ret = self._mysql_object.report_progress_live_mount()
        tmp_code = self.get_code(tmp_ret)
        return tmp_code, self.format_body_err(tmp_ret), ""

    @exter_attack
    def report_progress_comm(self):
        tmp_ret = self._mysql_object.report_progress_comm()
        tmp_code = self.get_code(tmp_ret)
        return tmp_code, self.format_body_err(tmp_ret), ""

    def execution_exception(self, cmd_param):
        self._mysql_object.cmd_execution_exception(cmd_param)


if __name__ == '__main__':
    args = sys.argv[1:]
    log.info(f"call backup script paramCnt:{len(args)} args:{args}")
    if len(args) < 2:
        log.error(f"Not enough parameters paramCnt:{len(args)} args:{args}")
        sys.exit()
    cmd = args[0]
    p_id = args[1]
    if not is_valid_id(p_id):
        log.warn(f'req_id is invalid')
        sys.exit(1)
    job_id = ""
    sub_job_id = ""

    if len(args) > 2:
        job_id = args[2]
        if not is_valid_id(job_id):
            log.warn(f'job_id is invalid')
            sys.exit(1)
    if len(args) > 3:
        sub_job_id = args[3]
        if not is_valid_id(sub_job_id):
            log.warn(f'sub_id is invalid')
            sys.exit(1)

    JobData.CMD = cmd
    mysql_backup = MysqlBackupComm.get_instance()

    CMD_DICT = {
        MySQLCmdStr.BACKUP: [
            mysql_backup.create_mysql_backuper_by_backup_type,
            mysql_backup.backup,
            mysql_backup.write_action_result
        ],
        MySQLCmdStr.BACKUP_POST: [
            mysql_backup.create_mysql_backuper_by_backup_type,
            mysql_backup.backup_post_job,
            mysql_backup.write_action_result
        ],
        MySQLCmdStr.QUERY_COPY: [
            mysql_backup.create_mysql_backuper_by_backup_type,
            mysql_backup.query_backup_copy,
            mysql_backup.write_action_result_when_failed
        ],
        MySQLCmdStr.PROGRESS_LIVE_MOUNT: [
            mysql_backup.create_mysql_live_mount,
            mysql_backup.report_progress_live_mount,
        ],
        MySQLCmdStr.PROGRESS_COMM: [
            mysql_backup.create_mysql_backuper_by_backup_type,
            mysql_backup.report_progress_comm,
        ],
        MySQLCmdStr.ABORT_JOB: [
            mysql_backup.create_mysql_backuper_comm,
            mysql_backup.abort_job,
            mysql_backup.write_action_result
        ],
        MySQLCmdStr.PAUSE_JOB: [
            mysql_backup.create_mysql_backuper_comm,
            mysql_backup.abort_job,
            mysql_backup.write_action_result
        ],
        MySQLCmdStr.LIVE_MOUNT: [
            mysql_backup.create_mysql_live_mount,
            mysql_backup.live_mount,
            mysql_backup.write_action_result
        ],
        MySQLCmdStr.CANCEL_LIVE_MOUNT: [
            mysql_backup.create_mysql_live_mount,
            mysql_backup.cancel_live_mount,
            mysql_backup.write_action_result
        ]
    }
    func_array = CMD_DICT.get(cmd)

    try:
        if not func_array:
            MysqlBackupFull.output_action_result_ex(p_id, MySQLCode.FAILED.value,
                                                    BodyErr.ERROR_COMMON_INVALID_PARAMETER.value,
                                                    f"Cmd not found. pid:{p_id} jobId:{job_id}")
        elif len(func_array) >= 1:
            ret = func_array[0](p_id, job_id, sub_job_id)
            if ret and len(func_array) >= 2:
                code, body_err, message = func_array[1]()
                if len(func_array) >= 3:
                    func_array[2](code, body_err, message)
            else:
                MysqlBackupFull.output_action_result_ex(p_id, MySQLCode.FAILED.value,
                                                        BodyErr.ERROR_COMMON_INVALID_PARAMETER.value,
                                                        f"Cmd Param error. pid:p_id jobId:{job_id}")
        else:
            MysqlBackupFull.output_action_result_ex(p_id, MySQLCode.FAILED.value,
                                                    BodyErr.ERROR_COMMON_INVALID_PARAMETER.value,
                                                    f"Cmd nothing to do. pid:{p_id} jobId:{job_id}")
    except Exception as exception_str:
        mysql_backup.execution_exception(cmd)
        log.exception(f"Exec cmd failed.errMsg:raise exception  pid:{p_id} jobId:{job_id}")

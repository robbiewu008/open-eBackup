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

from common.const import ReportDBLabel
from mongodb.comm.const import AllStepEnum, EnvName, MongoSubJob, TaskLabel
from mongodb.job_manager import JobManager
from mongodb.param.backup_param import BackupParam
from mongodb.service.backup.backup_service import BackupTask
from mongodb.service.base_service import MetaService, AllowInterfaceMixin, PrerequisiteInterfaceMixin, \
    CheckApplicationInterfaceMixin, GenSubInterfaceMixin, ExecuteInterfaceMixin, PostInterfaceMixin


class BackupService(MetaService):
    def __init__(self, job_manager: JobManager, param_dict):
        super().__init__(job_manager, param_dict)
        self.job_manager = job_manager
        self.param = self.gen_param_obj()
        EnvName.DB_USER_NAME = "job_protectEnv_{}auth_authKey"
        EnvName.DB_PASSWORD = "job_protectEnv_{}auth_authPwd"
        EnvName.DB_AUTH_TYPE = "job_protectEnv_{}auth_authType"
        self.backup_manager = BackupTask(self.job_manager, self.param)

    def gen_param_obj(self):
        param = BackupParam(self.param_dict)
        return param

    def get_steps(self):
        pass


@JobManager.register(AllStepEnum.ALLOW_BACKUP_IN_LOCAL_NODE)
class AllowBackupService(BackupService, AllowInterfaceMixin):

    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)

    def get_steps(self):
        steps = [
            self.backup_manager.get_instance,
            self.backup_manager.check_instance_role,
        ]
        return steps


@JobManager.register(AllStepEnum.PRE_TASK)
class PrerequisiteBackupService(BackupService, PrerequisiteInterfaceMixin):

    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)

    def get_steps(self):
        steps = []
        return steps


@JobManager.register(AllStepEnum.CHECK_BACKUP_JOB_TYPE)
class CheckBackupType(BackupService, CheckApplicationInterfaceMixin):
    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)

    def get_steps(self):
        return [self.backup_manager.check_backup_type]


@JobManager.register(AllStepEnum.BACKUP_GEN_SUB)
class GenSubBackupService(BackupService, GenSubInterfaceMixin):

    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)
        self.log_detail.update({
            "failed": TaskLabel.GENERATE_SUBJOB_FAIL_LABEL,
            "success": TaskLabel.GENERATE_SUBJOB_SUCCESS_LABEL,
            "start": TaskLabel.EXECUTE_GENERATE_SUBJOB_LABEL
        })

    def get_steps(self):
        steps = [
            self.backup_manager.gen_sub_jobs
        ]
        return steps


@JobManager.register(AllStepEnum.BACKUP)
class BackupExecuteService(BackupService, ExecuteInterfaceMixin):

    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)
        self.need_report = True
        self.log_detail.update({
            "failed": ReportDBLabel.BACKUP_SUB_FAILED,
            "start": ReportDBLabel.BACKUP_SUB_START_COPY,
            "success": ReportDBLabel.SUB_JOB_SUCCESS
        })

    def get_steps(self):
        sub_job_name = self.param.get_sub_job_name()
        if sub_job_name == MongoSubJob.REPORT_COPY_INFO:
            return [
                self.backup_manager.report_copy_info
            ]
        if sub_job_name == MongoSubJob.PRE_CHECK:
            return [
                self.backup_manager.check_status,
                self.backup_manager.check_lvm,
                self.backup_manager.check_oplog
            ]
        return [
            self.backup_manager.execute_backup
        ]


@JobManager.register(AllStepEnum.POST_TASK)
class PostBackupService(BackupService, PostInterfaceMixin):

    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)

    def get_steps(self):
        return [self.backup_manager.post_backup]


@JobManager.register(AllStepEnum.STOP_TASK)
class AbortJob(BackupService):
    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)
        self.need_report = False

    def get_steps(self):
        pass


@JobManager.register(AllStepEnum.QUERY_JOB_PERMISSION)
class QueryJobPermission(BackupService):
    def __init__(self, job_manager, param_dict):
        super().__init__(job_manager, param_dict)
        self.need_report = False

    def get_steps(self):
        return []

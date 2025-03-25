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

from common.common import exter_attack
from oceanbase.handle.backup.backup_service import OceanBaseBackupService
from oceanbase.logger import log


class BackupAbility:
    """
    备份相关接口
    """

    @staticmethod
    @exter_attack
    def check_backup_job_type(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.check_backup_job_type(req_id, job_id, sub_id, std_in)

    @staticmethod
    @exter_attack
    def allow_backup_in_local_node(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.allow_backup_in_local_node(req_id, job_id, sub_id)

    @staticmethod
    @exter_attack
    def backup_prerequisite(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.backup_prerequisite_job(req_id, job_id, sub_id, std_in)

    @staticmethod
    @exter_attack
    def backup_prerequisite_progress(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.backup_prerequisite_progress(req_id, job_id, sub_id)

    @staticmethod
    @exter_attack
    def backup_gen_sub_job(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.backup_gen_sub_job(req_id, job_id, sub_id, std_in)

    @staticmethod
    @exter_attack
    def backup(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.exec_backup(req_id, job_id, sub_id, std_in)

    @staticmethod
    @exter_attack
    def query_scan_repositories(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.query_scan_repositories(req_id, job_id, sub_id, std_in)

    @staticmethod
    @exter_attack
    def abort_job(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.abort_job(req_id, job_id)

    @staticmethod
    @exter_attack
    def abort_job_progress(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.query_abort_job_progress(req_id, job_id, sub_id)

    @staticmethod
    @exter_attack
    def backup_post_job(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.backup_post_job(req_id, job_id, sub_id, std_in)

    @staticmethod
    @exter_attack
    def backup_post_job_progress(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.backup_post_job_progress(req_id, job_id, sub_id)

    @staticmethod
    @exter_attack
    def query_backup_copy(req_id, job_id, sub_id, std_in):
        return OceanBaseBackupService.query_backup_copy(req_id, job_id, sub_id, std_in)

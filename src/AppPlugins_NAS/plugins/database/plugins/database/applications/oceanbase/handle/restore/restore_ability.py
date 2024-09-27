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
from oceanbase.handle.restore.restore_service import OceanBaseRestoreService


class RestoreAbility:
    @staticmethod
    @exter_attack
    def allow_restore_in_local_node(req_id, job_id, sub_id, data):
        return OceanBaseRestoreService.allow_restore_in_local_node(req_id, job_id)

    @staticmethod
    @exter_attack
    def restore_prerequisite(req_id, job_id, sub_id, data):
        return OceanBaseRestoreService.restore_prerequisite(req_id, job_id)

    @staticmethod
    @exter_attack
    def restore_prerequisite_progress(req_id, job_id, sub_id, data):
        return OceanBaseRestoreService.restore_prerequisite_progress(req_id, job_id, sub_id)

    @staticmethod
    @exter_attack
    def restore_gen_sub_job(req_id, job_id, sub_id, data):
        return OceanBaseRestoreService.restore_gen_sub_job(req_id, job_id, sub_id)

    @staticmethod
    @exter_attack
    def restore(req_id, job_id, sub_id, data):
        OceanBaseRestoreService.exec_restore(req_id, job_id, sub_id)

    @staticmethod
    @exter_attack
    def restore_post_job(req_id, job_id, sub_id, data):
        return OceanBaseRestoreService.restore_post_job(req_id, job_id)

    @staticmethod
    @exter_attack
    def restore_post_job_progress(req_id, job_id, sub_id, data):
        return OceanBaseRestoreService.restore_post_job_progress(req_id, job_id, sub_id)

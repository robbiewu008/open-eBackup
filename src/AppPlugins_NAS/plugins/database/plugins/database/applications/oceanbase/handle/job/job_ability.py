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


class JobAbility:
    """
    任务相关接口
    """

    @staticmethod
    @exter_attack
    def query_job_permission(req_id, job_id, sub_id, data):
        pass

    @staticmethod
    @exter_attack
    def abort_job(req_id, job_id, sub_id, data):
        pass

    @staticmethod
    @exter_attack
    def pause_job(req_id, job_id, sub_id, data):
        pass

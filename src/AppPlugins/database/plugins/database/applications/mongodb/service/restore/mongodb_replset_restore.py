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

from mongodb import LOGGER
from mongodb.service.restore.mongodb_single_restore import MetaRestore


class ReplSetRestore(MetaRestore):
    def __init__(self, pid, param_obj):
        super().__init__(pid, param_obj)

    def gen_sub_job(self):
        """
        功能描述：拆分子任务
        :return:
        """
        jobs = []
        # 获取所有集群实例中所有node
        nodes = self.param.get_all_instances_dic_info()
        result = self.snapshot_restore.gen_replset_sub_job(nodes, jobs)
        if not result:
            LOGGER.error("Gen replset sub job failed, result is false, job id: %s", self.job_id)
            return []
        return jobs

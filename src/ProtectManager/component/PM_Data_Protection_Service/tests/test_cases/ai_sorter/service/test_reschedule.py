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
import sys
import unittest
from unittest import mock

from tests.test_cases.ai_sorter.mock_db import mock_job_schema_list
from tests.test_cases.ai_sorter.mock_db import mock_vmware_schema_list


def mock_get_vmware_object_from_job(*args, **kwargs):
    job_obj = args[0]
    for mock_vmware_object in mock_vmware_schema_list:
        if job_obj.source_name == mock_vmware_object.name:
            return mock_vmware_object
    return None


def mock_get_prev_jobs(*args, **kwargs):
    vmware_obj = args[0]
    window_len = args[1]
    cnt = 0
    return_job_list = []
    for mock_job_object in mock_job_schema_list:
        if mock_job_object.source_name == vmware_obj.name:
            cnt += 1
            return_job_list.append(mock_job_object)
            if cnt == window_len:
                return return_job_list
    return return_job_list


class TestRescheduleService(unittest.TestCase):
    def setUp(self) -> None:
        super(TestRescheduleService, self).setUp()
        sys.modules["app.common.database"] = mock.Mock()

    def tearDown(self) -> None:
        super(TestRescheduleService, self).tearDown()
        del sys.modules["app.common.database"]

    @mock.patch("app.ai_sorter.utils.job.get_prev_jobs")
    @mock.patch("app.ai_sorter.utils.db.get_vmware_object_from_job")
    def test_reschedule_success(self, _mock_get_vmware_object_from_job,
                                _mock_get_prev_jobs):
        """
        用例名称：测试重新调度成功
        前置条件：数据库中数据正常
        check点：新排序的job长度和之前的job长度一致。
        """
        from app.ai_sorter.service import reschedule_jobs
        _mock_get_vmware_object_from_job.side_effect = mock_get_vmware_object_from_job
        _mock_get_prev_jobs.side_effect = mock_get_prev_jobs
        job_list_ready_to_be_sorted = [mock_job_schema_list[0], mock_job_schema_list[73], mock_job_schema_list[127], mock_job_schema_list[255]]
        new_job_list = reschedule_jobs(job_list_ready_to_be_sorted)
        self.assertEqual(len(new_job_list), len(job_list_ready_to_be_sorted))

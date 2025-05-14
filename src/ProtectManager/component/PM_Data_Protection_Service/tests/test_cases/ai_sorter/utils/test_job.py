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
import json
import sys
import unittest
from unittest import mock

from app.common.schemas.job_schemas import JobSchema


class TestUtilJob(unittest.TestCase):
    def setUp(self) -> None:
        super(TestUtilJob, self).setUp()
        sys.modules["app.common.database"] = mock.Mock()

    def tearDown(self) -> None:
        super(TestUtilJob, self).tearDown()
        del sys.modules["app.common.database"]

    def test_get_job_speed_success(self):
        """
        用例名称：测试获取job速度函数成功
        前置条件：输入正确的JobSchema
        check点：速度符合预期值
        """
        from app.ai_sorter.utils.job import get_job_speed
        job01 = JobSchema()
        job01.extend_str = r'{"slaName":"redis_protection_manual_backup_0010","dataBeforeReduction":"456 KB","dataAfterReduction":"33 KB","taskId":"39627a83-adaa-4001-a735-f58ca217322d","backupType":"full"}'
        job01.start_time = 1663644170
        job01.end_time = 1663647770

        speed = get_job_speed(job01)
        self.assertEqual(
            float(33) / 1024 / ((job01.end_time - job01.start_time) / 1000),
            speed)

        job01.extend_str = r'{"dataAfterReduction":"33 MB"}'
        speed = get_job_speed(job01)
        self.assertEqual(
            float(33) / ((job01.end_time - job01.start_time) / 1000), speed)

        job01.extend_str = r'{"dataAfterReduction":"33 GB"}'
        speed = get_job_speed(job01)
        self.assertEqual(
            float(33) * 1024 / ((job01.end_time - job01.start_time) / 1000),
            speed)

        job01.extend_str = r'{"dataAfterReduction":"33 TB"}'
        speed = get_job_speed(job01)
        self.assertEqual(
            float(33) * 1024 * 1024 / (
                        (job01.end_time - job01.start_time) / 1000), speed)

        job01.extend_str = r'{"dataAfterReduction":"33 B"}'
        speed = get_job_speed(job01)
        self.assertEqual(
            float(33) / (1024 * 1024 * (
                        job01.end_time - job01.start_time) / 1000), speed)

    def test_get_job_speed_fail(self):
        """
        用例名称：测试获取job速度函数成功
        前置条件：输入非法输入
        check点：返回值为-1
        """
        from app.ai_sorter.utils.job import get_job_speed
        job01 = JobSchema()
        job01.extend_str = r'{"slaName":"redis_protection_manual_backup_0010","dataBeforeReduction":"456 KB","taskId":"39627a83-adaa-4001-a735-f58ca217322d","backupType":"full"}'
        job01.start_time = 1663644170
        job01.end_time = 1663647770

        speed = get_job_speed(job01)
        self.assertEqual(-1, speed)

    @mock.patch("app.ai_sorter.utils.job.query_job_list")
    def test_get_backup_jobs_per_vm_success(self, mock_query_job_list):
        """
        用例名称：测试get_backup_jobs_per_vm函数成功
        前置条件：
        check点：
        """
        from app.ai_sorter.utils.job import get_backup_jobs_per_vm
        from app.protection.object.models.projected_object import \
            ProtectedObject

        mock_job_records = [{'jobId': "12345"}, {'jobId': '1234567'}]

        response_str = json.dumps({
            "totalCount": 2,
            "records": mock_job_records
        })

        mock_query_job_list.return_value = response_str

        vm01 = ProtectedObject()
        vm01.name = 'test_VMWare'
        job_schema_list = get_backup_jobs_per_vm(vm01, 1)

        self.assertEqual(job_schema_list[0].job_id, "12345")

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
import asyncio
import sys
import unittest
from unittest import mock

from app.common.schemas.job_schemas import JobSchema


class TestAISorterAPI(unittest.TestCase):
    def setUp(self) -> None:
        super(TestAISorterAPI, self).setUp()
        sys.modules["app.common.database"] = mock.Mock()

    def tearDown(self) -> None:
        super(TestAISorterAPI, self).tearDown()
        del sys.modules["app.common.database"]

    @mock.patch("app.ai_sorter.api.ai_sorter_api.train_start", mock.Mock())
    def test_retrain_success(self):
        """
        用例名称：测试retrain api接口成功
        前置条件：train_start成功
        check点：未抛出异常
        """
        from app.ai_sorter.api.ai_sorter_api import retrain
        asyncio.run(retrain())

    @mock.patch("app.ai_sorter.api.ai_sorter_api.reschedule_jobs")
    def test_reschedule_success(self, mock_reschedule_jobs):
        """
        用例名称：测试reschedule接口成功
        前置条件：底层reschedule_jobs成功
        check点：返回成功，重新训练后的job_list长度和之前一致.
        """
        from app.ai_sorter.api.ai_sorter_api import reschedule
        job01 = JobSchema()
        job01.job_id = '123-456'
        job02 = JobSchema()
        job02.job_id = "456-789"

        job_list = [job01, job02]
        mock_reschedule_jobs.return_value = job_list

        new_job_list = asyncio.run(reschedule(job_list))
        self.assertEqual(len(job_list), len(new_job_list))

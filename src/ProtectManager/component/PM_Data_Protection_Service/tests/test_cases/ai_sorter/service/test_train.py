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
from typing import List
from unittest import mock

from app.common.schemas.job_schemas import JobSchema
from tests.test_cases.ai_sorter.mock_db import mock_vmware_schema_list, mock_job_schema_list


def mock_get_jobs_per_vm(*args, **kwargs) -> \
        List[JobSchema]:
    vm_obj = args[0]
    min_jobs_per_vm = args[1]
    job_list = []
    for mock_job in mock_job_schema_list:
        if mock_job.source_name == vm_obj.name:
            job_list.append(mock_job)
    if len(job_list) < min_jobs_per_vm:
        return []
    return job_list


class TestTrainService(unittest.TestCase):
    @classmethod
    def setUpClass(cls) -> None:
        sys.modules["app.common.database"] = mock.Mock()

    @classmethod
    def tearDownClass(cls) -> None:
        del sys.modules["app.common.database"]

    def setUp(self) -> None:
        super(TestTrainService, self).setUp()

    def tearDown(self) -> None:
        super(TestTrainService, self).tearDown()

    @mock.patch("app.ai_sorter.service.train.threading", mock.Mock())
    def test_train_start_success(self):
        """
        用例名称：测试train_start服务成功
        前置条件：底层成功
        check点：未抛出异常
        """
        from app.ai_sorter.service import train_start
        train_start()

    @unittest.skip
    @mock.patch("app.ai_sorter.service.train.utils.get_vmware_objects")
    @mock.patch("app.ai_sorter.service.train.utils.get_backup_jobs_per_vm")
    @mock.patch("app.ai_sorter.service.train.model.StaticAIModelContainer.training_lock")
    def test_train_model_success(self, _mock_lock, _mock_get_jobs_per_vm,
                                 _mock_get_vmware_objects):
        """
        用例名称：测试train_model服务成功
        前置条件：底层成功
        check点：解锁成功
        """
        from app.ai_sorter.service.train import train_model
        _mock_get_jobs_per_vm.side_effect = mock_get_jobs_per_vm
        _mock_get_vmware_objects.return_value = mock_vmware_schema_list
        train_model()
        _mock_lock.release.assert_called_once()

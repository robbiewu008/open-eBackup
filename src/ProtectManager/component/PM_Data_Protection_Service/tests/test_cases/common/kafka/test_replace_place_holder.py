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
import unittest
from unittest import mock
from tests.test_cases import common_mocker # noqa
from tests.test_cases.tools import functiontools
from tests.test_cases.tools import timezone

mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone", timezone.dmc.query_time_zone).start()


class TestReplacePlaceHolder(unittest.TestCase):
    def test_replace_place_holder_success(self):
        """
        验证场景：成功更换log label占位符
        前置条件：无
        验证点：合法格式的label中的占位符能够被成功替换
        """
        from tests.test_cases.common.events import mock_producer
        from app.common.kafka import replace_place_holder
        label = replace_place_holder(
            "job_status_{payload.job_status|context.job_status|status}_label",
            {
                "payload": {},
                "context": {},
                "status": "SUCCESS"
            }
        )
        self.assertEqual(label, 'job_status_success_label')
        label = replace_place_holder(
            "job_{status}_status_{payload.job_status|context.job_status|status}_label",
            {
                "payload": {},
                "context": {},
                "status": "SUCCESS"
            }
        )
        self.assertEqual(label, 'job_success_status_success_label')

    def test_replace_place_holder_fail(self):
        """
        验证场景：更换log label占位符失败
        前置条件：无
        验证点：不是合法格式的label中的占位符不能够被替换
        """
        from tests.test_cases.common.events import mock_producer
        from app.common.kafka import replace_place_holder
        label = replace_place_holder(
            "job_status_[payload.job_status|context.job_status|status]_label",
            {
                "payload": {},
                "context": {},
                "status": "SUCCESS"
            }
        )
        self.assertEqual(label, 'job_status_[payload.job_status|context.job_status|status]_label')


if __name__ == '__main__':
    unittest.main(verbosity=2)

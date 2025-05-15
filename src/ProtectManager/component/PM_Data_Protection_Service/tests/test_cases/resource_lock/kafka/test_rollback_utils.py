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
from unittest.mock import patch, Mock, MagicMock
from tests.test_cases import common_mocker # noqa
from tests.test_cases.common.mock_settings import fake_settings  # noqa
from app.common.security.kmc_util import Kmc
from app.common.deploy_type import DeployType


class KafkaProducerMock:
    def produce(self, *args, **kwargs):
        pass

    def flush(self, *args, **kwargs):
        pass


producer_patcher = patch("confluent_kafka.Producer", MagicMock(return_value=KafkaProducerMock()))
producer_patcher.start()

sys.modules['app.resource_lock.kafka.messaging_utils'] = Mock()


from app.resource_lock.kafka.rollback_utils import sync_unlock_deduplicate


@sync_unlock_deduplicate()
def func(field: str) -> str:
    return field + "1"


class TestSyncDeduplication(unittest.TestCase):
    @unittest.skip
    @patch.object(Kmc, "decrypt", Mock(return_value=None))
    @mock.patch("app.common.redis_session.redis_session.exists", Mock(return_value=True))
    @mock.patch("app.common.redis_session.redis_session.setex", Mock(return_value=None))
    @patch.object(DeployType, "is_x8000_type", Mock(return_value=False))
    def test_should_exec_once_when_call_test_func(self):
        """
        kafka消息去重注解成功

        期望：
        1.test_func方法不会执行
        :return:
        """
        with patch("tests.test_cases.resource_lock.kafka.test_rollback_utils.func") as mock_func:
            func("1")
            mock_func.assert_not_called()

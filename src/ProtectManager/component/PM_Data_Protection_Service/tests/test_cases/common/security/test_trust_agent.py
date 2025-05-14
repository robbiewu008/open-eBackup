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

from app.common.exception.unified_exception import EmeiStorBizException
from tests.test_cases import common_mocker # noqa
from app.common.security import trust_agent

from tests.test_cases.tools import http
from tests.test_cases.tools.timezone import dmc

mock.patch("requests.get", http.get_request).start()
mock.patch("requests.put", http.get_request).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone", dmc.query_time_zone).start()


def test():
    pass


class TestTrustAgent(unittest.TestCase):

    def setUp(self) -> None:
        self.trust_agent = trust_agent

    def test_trust_agent_when_agent_ip_trust(self):
        self.trust_agent.get_host_trust = mock.Mock(return_value=True)
        self.trust_agent.need_check_trust_deploy_type = mock.Mock(return_value=True)
        try:
            @self.trust_agent.trust_agent
            def test1(forward_ip):
                pass
            test1(forward_ip="127.0.0.1, 127.0.0.2")
        except EmeiStorBizException:
            self.assertTrue(False)
        except Exception:
            self.assertTrue(False)

    def test_trust_agent_when_agent_ip_not_trust(self):
        self.trust_agent.get_host_trust = mock.Mock(return_value=False)
        self.trust_agent.need_check_trust_deploy_type = mock.Mock(return_value=True)
        try:
            @self.trust_agent.trust_agent
            def test1(forward_ip):
                pass
            test1(forward_ip="127.0.0.1, 127.0.0.2")
        except EmeiStorBizException:
            self.assertTrue(True)
        except Exception:
            self.assertTrue(False)

    def test_trust_agent_when_agent_ip_is_none(self):
        self.trust_agent.get_host_trust = mock.Mock(return_value=True)
        self.trust_agent.need_check_trust_deploy_type = mock.Mock(return_value=True)
        try:
            @self.trust_agent.trust_agent
            def test1(forward_ip):
                pass
            test1(forward_ip=None)
        except EmeiStorBizException:
            self.assertTrue(False)
        except Exception:
            self.assertTrue(False)

    def test_trust_agent_when_deploy_type_is_hyper_detect(self):
        """
        用例场景：校验客户顿ip是否授信
        前置条件：部署类型为防勒索
        检查点：防勒索为内置agent不用校验客户端ip是否授信
        """
        self.trust_agent.get_host_trust = mock.Mock(return_value=False)
        self.trust_agent.need_check_trust_deploy_type = mock.Mock(return_value=False)
        try:
            @self.trust_agent.trust_agent
            def test1(forward_ip):
                pass
            test1(forward_ip=None)
        except EmeiStorBizException:
            self.assertTrue(False)
        except Exception:
            self.assertTrue(False)

if __name__ == '__main__':
    unittest.main(verbosity=2)

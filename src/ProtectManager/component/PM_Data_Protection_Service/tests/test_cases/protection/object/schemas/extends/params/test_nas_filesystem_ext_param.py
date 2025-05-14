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
from unittest import TestCase, mock
from unittest.mock import Mock

from app.common.exception.unified_exception import IllegalParamException
from app.protection.object.schemas.extends.params.nas_fileset_ext_system import NasFileSystemExtParam
from app.protection.object.common.protection_enums import NasProtocolType
from tests.test_cases.tools import functiontools, timezone

sys.modules['app.common.database'] = mock.Mock()
sys.modules['app.common.config'] = mock.Mock()
sys.modules['app.common.events.producer'] = mock.Mock()
sys.modules['app.common.events.topics'] = mock.Mock()
mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()


class NasFileSystemExtValuesMock:
    def __init__(self, protocol, agents):
        self.protocol = protocol
        self.agents = agents


class TestNasFileSystemExtParam(TestCase):

    @mock.patch("app.common.deploy_type.DeployType.is_x3000_type", Mock(return_value=True))
    def test_check_protocol_is_nfs_success(self):
        """
        验证场景：测试nas filesystem的协议为nfs时，校验通过
        前置条件：无
        验证点：传入NFS协议
        """
        values = NasFileSystemExtValuesMock(NasProtocolType.NFS.value, "agentid")
        result = NasFileSystemExtParam.check_ext_params(values.__dict__)
        self.assertEqual(result, values.__dict__)

    def test_should_raise_IllegalParamException_if_permissions_is_not_equal_file_protocol(
            self):
        """
        验证场景：测试nas filesystem的协议参数错误，导致抛出异常IllegalParamException
        前置条件：无
        验证点：传入NONE_SHARE参数
        """
        values = NasFileSystemExtValuesMock(NasProtocolType.NONE_SHARE.value, "agentId")
        with self.assertRaises(IllegalParamException):
            NasFileSystemExtParam.check_ext_params(values.__dict__)


if __name__ == '__main__':
    unittest.main(verbosity=2)

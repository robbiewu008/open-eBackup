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
from unittest.mock import Mock

from app.common.exception.unified_exception import EmeiStorBizException
from tests.test_cases.tools import functiontools, timezone
from unittest import TestCase, mock
import uuid

from tests.test_cases import common_mocker # noqa
sys.modules['app.common.events.producer'] = mock.Mock()
sys.modules['app.common.events.topics'] = mock.Mock()
mock.patch("pydantic.validator", functiontools.mock_decorator).start()
mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
           timezone.dmc.query_time_zone).start()

mock.patch("app.common.database.Database.initialize", mock.Mock).start()
sys.modules['app.protection.object.common.db_config'] = Mock()
from app.resource.models.resource_models import ResExtendInfoTable


class TestFusionComputeValidator(TestCase):

    def setUp(self) -> None:
        mock.patch("app.common.database.Database.initialize", mock.Mock).start()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.protection.object.service.validators.fusion_compute_validator import FusionComputeValidator
        self.FusionComputeValidator = FusionComputeValidator

    @mock.patch("app.protection.object.service.validators.fusion_compute_validator.FusionComputeValidator.query_resource_extend_info")
    def test_should_raise_EmeiStorBizException_when_status_is_invalid(self, mock_query_extend_info):
        """
        用例场景：FusionCompute vm状态异常，不能保护
        前置条件：FC vm状态异常
        检查点：状态异常的FC 虚拟机不能保护，抛异常
        """
        resource_id = str(uuid.uuid4())
        resource_type = 'VM'

        extend_info = [ResExtendInfoTable(resource_id=resource_id, key='status', value='hibernated')]
        mock_query_extend_info.return_value = extend_info
        fusion_vm = {
            'uuid': resource_id,
            'type': resource_type
        }
        with self.assertRaises(EmeiStorBizException):
            self.FusionComputeValidator.do_validate(fusion_vm)

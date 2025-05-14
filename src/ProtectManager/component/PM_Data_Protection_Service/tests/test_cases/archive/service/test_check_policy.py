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
from unittest.mock import Mock

from tests.test_cases import common_mocker  # noqa
from tests.test_cases.common.events import mock_producer  # noqa

_mock_common_db_init = mock.patch("app.common.database.Database.initialize", mock.Mock)
_mock_common_db_init.start()
sys.modules['app.common.logger'] = mock.Mock()
mock.patch("app.common.security.kmc_util._build_kmc_handler", Mock(return_value=None)).start()
mock.patch("app.common.clients.client_util._build_ssl_context", Mock(return_value=None)).start()

# 数据准备
copy_1_first = {"uuid": "id1_1", "generated_time": "2023-01-01T08:59:28"}
copy_1_second = {"uuid": "id1_2", "generated_time": "2023-01-02T08:59:28"}
copy_1_three = {"uuid": "id1_3", "generated_time": "2023-01-03T08:59:28"}
copy_2_first = {"uuid": "id2_1", "generated_time": "2023-02-01T08:59:28"}
copy_2_second = {"uuid": "id2_2", "generated_time": "2023-02-02T08:59:28"}
copy_list_archive = [copy_1_first, copy_1_second, copy_1_three, copy_2_first, copy_2_second]

class TestCheckPolicy(unittest.TestCase):
    def setUp(self):
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        sys.modules['app.resource_lock.service'] = Mock()

    @mock.patch("app.copy_catalog.service.curd.copy_query_service.query_first_last_copy_id", Mock(return_value="id1_3"))
    def test_check_month(self):
        """
        验证场景：删选是否正常：每年的一月副本归档、每月的第一个副本归档
        前置条件：无
        验证点：归档回滚是否正常
        """
        from app.archive.service.check_policy import CheckPolicy
        from app.common.enums.sla_enum import CopyTypeEnum, RetentionTimeUnit, TimeRangeYearEnum, TimeRangeMonthEnum

        specified_scope = [{"copy_type": CopyTypeEnum.year, "generate_time_range": TimeRangeYearEnum.jan,
                            "retention_unit": RetentionTimeUnit.years, "retention_duration": 2},
                           {"copy_type": CopyTypeEnum.month, "generate_time_range": TimeRangeMonthEnum.first,
                            "retention_unit": RetentionTimeUnit.months, "retention_duration": 2}]
        check_policy = CheckPolicy(specified_scope)
        copy_filtered = check_policy.filter_copies(copy_list_archive)
        assert len(copy_filtered) == 1
        assert copy_filtered[0]["uuid"] == "id1_3"


class TestImportCopy(unittest.TestCase):
    @mock.patch("app.copy_catalog.service.copy_delete_workflow.request_delete_copy_by_id", Mock(return_value=None))
    def delete_copy(self):
        """
        验证场景：删除copy
        前置条件：无
        验证点：归档回滚是否正常
        """
        from app.archive.service.check_policy import ImportCopy

        ImportCopy(copy_1_first).delete_copy()


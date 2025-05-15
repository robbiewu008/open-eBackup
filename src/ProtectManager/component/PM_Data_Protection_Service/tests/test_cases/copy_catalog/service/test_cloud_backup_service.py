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
import uuid
import datetime
import json
from unittest.mock import Mock

from app.common.exception.unified_exception import EmeiStorBizException
from tests.test_cases import common_mocker # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock

common_mock()

from app.common.enums.sla_enum import BackupTypeEnum
from app.copy_catalog.models.tables_and_sessions import CopyTable


COPY_DATA_DESC = [CopyTable(uuid=str(uuid.uuid4()), resource_id='123',
                            backup_type=BackupTypeEnum.full.value,
                            expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                            properties=json.dumps({"backup_type": "full"})),
                  CopyTable(uuid=str(uuid.uuid4()),  resource_id='123',
                            backup_type=BackupTypeEnum.cumulative_increment.value,
                            expiration_time=datetime.datetime(2021, 4, 21, 2, 0, 0), chain_id=str(uuid.uuid4()),
                            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=2,
                            properties=json.dumps({"backup_type": "cumulative_increment"})),
                  CopyTable(uuid=str(uuid.uuid4()),  resource_id='123',
                            backup_type=BackupTypeEnum.cumulative_increment.value,
                            expiration_time=datetime.datetime(2021, 4, 20, 2, 0, 0), chain_id=str(uuid.uuid4()),
                            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=3,
                            properties=json.dumps({"backup_type": "cumulative_increment"})),
                  CopyTable(uuid=str(uuid.uuid4()),  resource_id='123',
                            backup_type=BackupTypeEnum.full.value,
                            expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=4,
                            properties=json.dumps({"backup_type": "full"}))
                  ]

COPY_DATA_MOCK = [CopyTable(uuid=str(uuid.uuid4()), resource_id='123',
                            backup_type=BackupTypeEnum.cumulative_increment.value,
                            expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=1,
                            properties=json.dumps({"backup_type": "cumulative_increment"})),
                  CopyTable(uuid=str(uuid.uuid4()),  resource_id='123',
                            backup_type=BackupTypeEnum.full.value,
                            expiration_time=datetime.datetime(2021, 4, 21, 2, 0, 0), chain_id=str(uuid.uuid4()),
                            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=2,
                            properties=json.dumps({"backup_type": "full"})),
                  CopyTable(uuid=str(uuid.uuid4()),  resource_id='123',
                            backup_type=BackupTypeEnum.full.value,
                            expiration_time=datetime.datetime(2021, 4, 20, 2, 0, 0), chain_id=str(uuid.uuid4()),
                            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=3,
                            properties=json.dumps({"backup_type": "full"})),
                  CopyTable(uuid=str(uuid.uuid4()),  resource_id='123',
                            backup_type=BackupTypeEnum.cumulative_increment.value,
                            expiration_time=datetime.datetime(2021, 4, 22, 2, 0, 0), chain_id=str(uuid.uuid4()),
                            display_timestamp=datetime.datetime(2021, 4, 15, 2, 0, 0), gn=4,
                            properties=json.dumps({"backup_type": "cumulative_increment"}))
                  ]

class CopyServiceTest(unittest.TestCase):
    def setUp(self):
        super(CopyServiceTest, self).setUp()
        from app.copy_catalog.service import cloud_backup_service

        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        self.cloud_backup_service = cloud_backup_service

    def test_associated_deletion_copies_when_full(self):
        """
        副本为全量副本
        期望：查询结果为全量副本向下的副本依赖链
        return:全量副本向下的副本依赖链
        """
        association_type = "all"
        self.cloud_backup_service.associated_deletion_copies_for_full = Mock(return_value=COPY_DATA_DESC)
        res = self.cloud_backup_service.associated_deletion_copies(COPY_DATA_DESC[0], association_type)
        self.assertEqual(res, COPY_DATA_DESC)

    def test_associated_deletion_copies_when_increase(self):
        """
        副本为增量副本
        期望：查询结果为整个副本依赖链
        return:整个副本依赖链
        """
        association_type = "all"
        self.cloud_backup_service.associated_deletion_copies_for_increase = Mock(return_value=COPY_DATA_DESC)
        res = self.cloud_backup_service.associated_deletion_copies(COPY_DATA_DESC[0], association_type)
        self.assertEqual(res, COPY_DATA_DESC)

    def test_associated_deletion_copies_for_increase_when_first_copy_is_none(self):
        """
        副本为增量副本，副本链关联类型：all
        期望：查询结果为关联副本链路列表
        return:关联副本链路列表
        """
        association_type = "all"
        self.cloud_backup_service.query_first_association_copy = Mock(return_value=None)
        res = self.cloud_backup_service.associated_deletion_copies(COPY_DATA_DESC[1], association_type)
        self.assertEqual(res, [COPY_DATA_DESC[1]])

    def test_associated_deletion_copies_for_increase_when_last_copy_is_none(self):
        """
        副本为增量副本，副本链关联类型：all
        期望：查询结果为空
        return:[]
        """
        association_type = "all"
        self.cloud_backup_service.query_first_association_copy = Mock(return_value=COPY_DATA_DESC[0])
        self.cloud_backup_service.query_last_association_copy = Mock(return_value=None)
        res = self.cloud_backup_service.associated_deletion_copies(COPY_DATA_MOCK[3], association_type)
        self.assertEqual(res, [])

    def test_associated_deletion_copies_for_increase_when_last_copy_is_none_should_throw_exception(self):
        """
        副本为增量副本，副本链关联类型：down
        期望：抛出EmeiStorBizException异常
        return:EmeiStorBizException
        """
        association_type = "dowm"
        self.cloud_backup_service.query_first_association_copy = Mock(return_value=COPY_DATA_DESC[0])
        self.cloud_backup_service.query_last_association_copy = Mock(return_value=None)
        try:
            self.cloud_backup_service.associated_deletion_copies(COPY_DATA_MOCK[3], association_type)
        except:
            self.assertTrue(EmeiStorBizException)
    def test_associated_deletion_copies_for_increase_when_query_copy_is_none(self):
        """
        副本为增量副本，副本链关联类型：down
        期望：查询结果为None
        return:None
        """
        association_type = "dowm"
        self.cloud_backup_service.query_first_association_copy = Mock(return_value=COPY_DATA_DESC[0])
        self.cloud_backup_service.query_last_association_copy = Mock(return_value=COPY_DATA_MOCK[3])
        self.cloud_backup_service.query_association_copies = Mock(return_value=None)
        res = self.cloud_backup_service.associated_deletion_copies(COPY_DATA_MOCK[3], association_type)
        self.assertEqual(res, None)

    def test_associated_deletion_copies_for_full_when_next_copy_is_none(self):
        """
        副本为全量副本，副本链关联类型：all
        期望：查询结果为关联副本链路列表
        return:关联副本链路列表
        """
        association_type = "all"
        self.cloud_backup_service.query_next_association_copy = Mock(return_value=None)
        res = self.cloud_backup_service.associated_deletion_copies(COPY_DATA_MOCK[1], association_type)
        self.assertEqual(res, [COPY_DATA_MOCK[1]])

    def test_associated_deletion_copies_for_full_when_last_copy_is_none(self):
        """
        副本为全量副本，副本链关联类型：all
        期望：查询结果为空
        return:[]
        """
        association_type = "all"
        self.cloud_backup_service.query_next_association_copy = Mock(return_value=COPY_DATA_DESC[0])
        self.cloud_backup_service.query_last_association_copy = Mock(return_value=None)
        res = self.cloud_backup_service.associated_deletion_copies(COPY_DATA_MOCK[1], association_type)
        self.assertEqual(res, [])

    def test_associated_deletion_copies_for_full_when_last_copy_is_none_should_throw_exception(self):
        """
        副本为全量副本，副本链关联类型：down
        期望：抛出EmeiStorBizException异常
        return:EmeiStorBizException
        """
        association_type = "dowm"
        self.cloud_backup_service.query_next_association_copy = Mock(return_value=COPY_DATA_DESC[0])
        self.cloud_backup_service.query_last_association_copy = Mock(return_value=None)
        try:
            self.cloud_backup_service.associated_deletion_copies(COPY_DATA_MOCK[1], association_type)
        except:
            self.assertTrue(EmeiStorBizException)

    def test_associated_deletion_copies_for_full_when_query_copy_is_none(self):
        """
        副本为全量副本，副本链关联类型：down
        期望：查询结果为None
        return:None
        """
        association_type = "dowm"
        self.cloud_backup_service.query_next_association_copy = Mock(return_value=COPY_DATA_DESC[0])
        self.cloud_backup_service.query_last_association_copy = Mock(return_value=COPY_DATA_MOCK[3])
        self.cloud_backup_service.query_association_copies = Mock(return_value=None)
        res = self.cloud_backup_service.associated_deletion_copies(COPY_DATA_MOCK[1], association_type)
        self.assertEqual(res, None)


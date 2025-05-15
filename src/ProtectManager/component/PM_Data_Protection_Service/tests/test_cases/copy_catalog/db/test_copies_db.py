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
import uuid
from unittest import mock

from pydantic import ValidationError

from tests.test_cases import common_mocker  # noqa
from tests.test_cases.copy_catalog.util.mock_util import common_mock

common_mock()


class CopiesDbTest(unittest.TestCase):
    def setUp(self):
        super(CopiesDbTest, self).setUp()

    @mock.patch("app.base.db_base.database.session")
    def test_query_first_association_copy(self, _mock_session):
        """
        用例场景：测试查询第一个关联副本
        前置条件：first copy正常
        检查点：first copy是否为None，因为没有mock CloudBackupCopySchema，是则抛出ValidationError异常
        return:抛出ValidationError异常
        """
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(uuid=str(uuid.uuid4()), resource_id=str(uuid.uuid4()), generated_by="Backup",
                         backup_type=1, resource_sub_type="CloudBackupFileSystem", gn=1)
        from app.copy_catalog.db.copies_db import query_first_association_copy
        self.assertRaises(ValidationError, query_first_association_copy, _mock_session, copy)

    @mock.patch("app.base.db_base.database.session")
    def test_query_first_association_copy_2(self, _mock_session):
        """
        用例场景：测试查询第一个关联副本
        前置条件：first copy为None
        检查点：代码运行成功
        return:None
        """
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(uuid=str(uuid.uuid4()), resource_id=str(uuid.uuid4()), generated_by="Backup",
                         backup_type=1, resource_sub_type="CloudBackupFileSystem", gn=1)
        from app.copy_catalog.db.copies_db import query_first_association_copy
        from app.base.db_base import database
        _mock_session().__enter__().query().filter().order_by().first.return_value = None
        with database.session() as session:
            result = query_first_association_copy(session, copy)
        self.assertIsNone(result)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_query_next_association_copy(self, _mock_session):
        """
        用例场景：测试查询下一个关联副本
        前置条件：next copy正常
        检查点：next copy是否为None，因为没有mock CloudBackupCopySchema，是则抛出ValidationError异常
        return:抛出ValidationError异常
        """
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(resource_id=str(uuid.uuid4()), resource_sub_type="CloudBackupFileSystem", gn=1)
        from app.copy_catalog.db.copies_db import query_next_association_copy
        self.assertRaises(ValidationError, query_next_association_copy, _mock_session, copy)

    @mock.patch("app.base.db_base.database.session")
    def test_query_next_association_copy_2(self, _mock_session):
        """
        用例场景：测试查询下一个关联副本
        前置条件：next copy为None
        检查点：代码运行成功
        return:None
        """
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(uuid=str(uuid.uuid4()), resource_id=str(uuid.uuid4()), generated_by="Backup",
                         backup_type=1, resource_sub_type="CloudBackupFileSystem", gn=1)
        from app.copy_catalog.db.copies_db import query_next_association_copy
        from app.base.db_base import database
        _mock_session().__enter__().query().filter().order_by().first.return_value = None
        with database.session() as session:
            result = query_next_association_copy(session, copy)
        self.assertIsNone(result)

    @mock.patch("app.base.db_base.database.session")
    def test_query_last_association_copy(self, _mock_session):
        """
        用例场景：测试查询最后一个关联副本
        前置条件：last copy正常
        检查点：last copy是否为None，因为没有mock CloudBackupCopySchema，是则抛出ValidationError异常
        return:抛出ValidationError异常
        """
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(uuid=str(uuid.uuid4()), resource_id=str(uuid.uuid4()), generated_by="Backup",
                         backup_type=1, resource_sub_type="CloudBackupFileSystem", gn=1)
        from app.copy_catalog.db.copies_db import query_last_association_copy
        self.assertRaises(ValidationError, query_last_association_copy, _mock_session, copy)

    @mock.patch("app.base.db_base.database.session")
    def test_query_last_association_copy_2(self, _mock_session):
        """
        用例场景：测试查询下一个关联副本
        前置条件：last copy为None
        检查点：代码运行成功
        return:None
        """
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(uuid=str(uuid.uuid4()), resource_id=str(uuid.uuid4()), generated_by="Backup",
                         backup_type=1, resource_sub_type="CloudBackupFileSystem", gn=1)
        from app.copy_catalog.db.copies_db import query_last_association_copy
        from app.base.db_base import database
        _mock_session().__enter__().query().filter().order_by().first.return_value = None
        with database.session() as session:
            result = query_last_association_copy(session, copy)
        self.assertIsNone(result)

    @mock.patch("sqlalchemy.orm.query.Query.first")
    def test_query_association_copies(self, _mock_session):
        """
        用例场景：测试查询关联副本
        前置条件：副本正常
        检查点：代码运行成功
        return:[]
        """
        from app.copy_catalog.models.tables_and_sessions import CopyTable
        copy = CopyTable(resource_id=str(uuid.uuid4()), resource_sub_type="CloudBackupFileSystem", gn=1)
        from app.copy_catalog.db.copies_db import query_association_copies
        result = query_association_copies(_mock_session, copy, copy)
        self.assertEqual([], result)

    @mock.patch("sqlalchemy.orm.query.Query.count")
    def test_count_by_user_id_and_resource_ids(self, _mock_session):
        """
        用例场景：测试根据用户id和资源id查询副本数量
        前置条件：副本正常
        检查点：代码运行成功
        return:返回值不为空
        """
        from app.copy_catalog.db.copies_db import count_by_user_id_and_resource_ids
        result = count_by_user_id_and_resource_ids(_mock_session, "123", ["123", "1234"])
        self.assertIsNotNone(result)

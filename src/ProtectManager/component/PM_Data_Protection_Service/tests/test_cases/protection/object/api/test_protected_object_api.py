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
from unittest.mock import Mock, MagicMock
import asyncio

from sqlalchemy.orm import Session

from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyActionEnum
from app.common.exception.unified_exception import IllegalParamException
from tests.test_cases import common_mocker  # noqa
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam
from tests.test_cases.tools import functiontools

mock.patch("pydantic.validator", functiontools.mock_decorator).start()


class ExtParameters(BaseExtParam):
    @staticmethod
    def support_values() -> List[object]:
        pass


class TestProtectedObjectApi(unittest.TestCase):
    def setUp(self):
        super(TestProtectedObjectApi, self).setUp()
        mock.patch("app.common.database.Database.initialize", mock.Mock).start()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        sys.modules['app.common.events.producer'] = Mock()

    @mock.patch("app.resource.service.common.protect_obj_service.get_protect_obj")
    @mock.patch("sqlalchemy.orm.Query.update")
    def test_synchronize_time_no_object_success(self, mock_get_protect_obj, mock_query_update):
        from app.protection.object.api.protected_object_api import synchronize_time
        resource_id = "8556bb41-abe6-4821-870d-a0252f301dfcmock"
        protect_obj = asyncio.run(synchronize_time(db=Session(), resource_id=resource_id))
        self.assertEqual(protect_obj, None)

    @mock.patch("sqlalchemy.orm.Query.update")
    @mock.patch("sqlalchemy.orm.Query.first")
    def test_update_compliance_success(self, mock_query_update, mock_query_first):
        from app.protection.object.api.protected_object_api import update_compliance
        from app.protection.object.schemas.protected_object import ComplianceUpdate
        resource_id = "8556bb41-abe6-4821-870d-a0252f301dfcmock"
        mock_query_update.return_value = None
        mock_query_first.return_value = None
        update_req = ComplianceUpdate(resource_id=resource_id, compliance=False)
        protect_obj = asyncio.run(update_compliance(database=Session(), update_req=update_req))
        self.assertEqual(protect_obj, None)

    @mock.patch("app.common.security.jwt_utils.get_user_info_from_token")
    def test__resolve_user_info_success(self, mock_token):
        token = "eyJhbGciOiJSUzI1NiJ9.eyJleHBpcmVzX2F0IjoxNjIzODE2NzgwMDAwLCJpc3MiOiJlbW" \
                "Vpc3Rvci5odWF3ZWkuY29tIiwidXNlcl9yb2xlcyI6W10sImV4cCI6MTYyMzgxNjc4MCwidXNl" \
                "ciI6eyJpZCI6Ijg4YTk0YzQ3NmYxMmEyMWUwMTZmMTJhMjQ2ZTUwMDA5IiwibmFtZSI6InN5c2Fkb" \
                "WluIiwicm9sZXMiOlt7Im5hbWUiOiJSb2xlX1NZU19BZG1pbiIsImlkIjoiMSJ9XX0sImlhdCI6MTY" \
                "yMzIxNjc4MCwiaXNzdWVkX2F0IjoxNjIzMjE2NzgwMDAwfQ.mvoRJeiz7f894VBgqLF2vy4dehwHNG7" \
                "WRDcCGULDBGCJW0wMJks3UIPO6Oykd40TEGypBtAD4Q2RrbhjT8Uez41s20VA0SnBM7g2NKvtjsR-tO" \
                "O5AhbniT-JAXpK1d3yT-Ha4QmMe4lDDv7ofK5siXP5Sh9YbENfSWwwoqsuhZoC4JJfPthmbXy6USvFn4" \
                "C2K7yp2VgDPo46YQSdcuJ7dx6R9yD4IthefzdZHdYyE3p5XBRpV0IIdmJOfouiz7RtW9BpQAAjqe9AhRmo" \
                "tEOeQt3d6Nw6xmodSvzfXDHiTj5mZm1nxOWpjEE3SzAvy-bqpbXL-M_d8hcCMKNnhcQSNw"

        user_info = {'exp': 1623816780,
                     'expires_at': 1623816780000,
                     'iat': 1623216780,
                     'iss': 'emeistor.huawei.com',
                     'issued_at': 1623216780000,
                     'user': {'id': '88a94c476f12a21e016f12a246e50009',
                              'name': 'sysadmin',
                              'roles': [{'id': '1',
                                         'name': 'Role_SYS_Admin'}]},
                     'user_roles': ["mmdp"]}

        mock_token.return_value = {
            'user-name': user_info['user']['name'],
            'user-id': user_info['user']['id'],
            'role-list': user_info["user_roles"],
            'es-valid-token': "true",
        }
        from app.protection.object.api.protected_object_api import _resolve_user_info

        result = _resolve_user_info(token)
        self.assertEqual(result, mock_token.return_value)

    @mock.patch("app.protection.object.service.projected_object_service.load_resource")
    @mock.patch("app.common.security.jwt_utils.get_user_info_from_token")
    def test_batch_create(self, mock_load_resource, mock_token):
        from app.protection.object.api.protected_object_api import batch_create
        from app.protection.object.api.protected_object_api import _resolve_user_info
        from app.protection.object.schemas.protected_object import BatchProtectionSubmitReq
        token = "eyJhbGciOiJSUzI1NiJ9.eyJleHBpcmVzX2F0IjoxNjIzODE2NzgwMDAwLCJpc3MiOiJlbW" \
                "Vpc3Rvci5odWF3ZWkuY29tIiwidXNlcl9yb2xlcyI6W10sImV4cCI6MTYyMzgxNjc4MCwidXNl" \
                "ciI6eyJpZCI6Ijg4YTk0YzQ3NmYxMmEyMWUwMTZmMTJhMjQ2ZTUwMDA5IiwibmFtZSI6InN5c2Fkb" \
                "WluIiwicm9sZXMiOlt7Im5hbWUiOiJSb2xlX1NZU19BZG1pbiIsImlkIjoiMSJ9XX0sImlhdCI6MTY" \
                "yMzIxNjc4MCwiaXNzdWVkX2F0IjoxNjIzMjE2NzgwMDAwfQ.mvoRJeiz7f894VBgqLF2vy4dehwHNG7" \
                "WRDcCGULDBGCJW0wMJks3UIPO6Oykd40TEGypBtAD4Q2RrbhjT8Uez41s20VA0SnBM7g2NKvtjsR-tO" \
                "O5AhbniT-JAXpK1d3yT-Ha4QmMe4lDDv7ofK5siXP5Sh9YbENfSWwwoqsuhZoC4JJfPthmbXy6USvFn4" \
                "C2K7yp2VgDPo46YQSdcuJ7dx6R9yD4IthefzdZHdYyE3p5XBRpV0IIdmJOfouiz7RtW9BpQAAjqe9AhRmo" \
                "tEOeQt3d6Nw6xmodSvzfXDHiTj5mZm1nxOWpjEE3SzAvy-bqpbXL-M_d8hcCMKNnhcQSNw"

        user_info = {'exp': 1623816780,
                     'expires_at': 1623816780000,
                     'iat': 1623216780,
                     'iss': 'emeistor.huawei.com',
                     'issued_at': 1623216780000,
                     'user': {'id': '88a94c476f12a21e016f12a246e50009',
                              'name': 'sysadmin',
                              'roles': [{'id': '1',
                                         'name': 'Role_SYS_Admin'}]},
                     'user_roles': ["mmdp"]}

        mock_token.return_value = {
            'user-name': user_info['user']['name'],
            'user-id': user_info['user']['id'],
            'role-list': user_info["user_roles"],
            'es-valid-token': "true",
        }
        result = _resolve_user_info(token)
        batch_create_req = {
            "resources": [
                {"resource_id": "062b4b41-ec5b-3cb8-85b2-06e5251cdb47"}
            ],
            "sla_id": "dccff80c-4c78-4df4-9047-a86d3b66b951",
            "ext_parameters": {}
        }
        async_depend = Mock(return_value=result)
        protect_obj = batch_create(user_info=result, batch_create_req=BatchProtectionSubmitReq(**batch_create_req))
        self.assertIsNotNone(protect_obj)

    @mock.patch("app.protection.object.db.crud_projected_object.CRUDProtectedObject.query_one_by_resource_id")
    @mock.patch("app.protection.object.db.crud_projected_object.CRUDProtectedObject"
                ".update_ext_parameters_by_resource_id")
    def test_internal_update_self_learning_progress(self, mock_query_one_by_resource_id,
                                                    mock_update_ext_parameters_by_resource_id):
        '''
        用例场景：测试开关自学习的内部接口
        前置条件：接口正常
        检查点:无报错
        '''
        from app.protection.object.api.protected_object_api import internal_update_self_learning_progress
        from app.protection.object.schemas.protected_object import UpdateSelfLearningProgressReq
        from app.protection.object.models.projected_object import ProtectedObject
        mock_query_one_by_resource_id.return_value = ProtectedObject(ext_parameters={})
        update_req = UpdateSelfLearningProgressReq(resource_id="mock", progress=20)
        internal_update_self_learning_progress(db=Session(), update_req=update_req)
        self.assertIsNotNone(update_req)

    def test_check_tidb_back_type(self):
        from app.protection.object.api.protected_object_api import check_tidb_back_type
        check_tidb_back_type(BackupReq, Resource)
        self.assertTrue(True)

    def test_check_tidb_back_type_error(self):
        from app.protection.object.api.protected_object_api import check_tidb_back_type
        res = Resource
        res.sub_type = ResourceSubTypeEnum.TiDB_TABLE.value
        with self.assertRaises(IllegalParamException):
            check_tidb_back_type(BackupReq, res)

    def test_check_oceanbase_back_type(self):
        from app.protection.object.api.protected_object_api import check_oceanbase_back_type
        check_oceanbase_back_type(BackupOceanbaseReq, OceanbaseResource)
        self.assertTrue(True)

    def test_check_oceanbase_back_type_error(self):
        from app.protection.object.api.protected_object_api import check_oceanbase_back_type
        res = OceanbaseResource
        res.sub_type = ResourceSubTypeEnum.OCEANBASE_TENANT.value
        with self.assertRaises(IllegalParamException):
            check_oceanbase_back_type(BackupOceanbaseReq, res)

    def test_check_tdsql_back_type(self):
        from app.protection.object.api.protected_object_api import check_tdsql_back_type
        check_tdsql_back_type(BackupTdsqlReq, TdsqlResource)
        self.assertTrue(True)

    def test_check_tdsql_back_type_error(self):
        from app.protection.object.api.protected_object_api import check_tdsql_back_type
        res = TdsqlResource
        res.sub_type = ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP.value
        with self.assertRaises(IllegalParamException):
            check_tdsql_back_type(BackupTdsqlReq, res)


class BackupTdsqlReq:
    action = PolicyActionEnum.difference_increment.value


class TdsqlResource:
    uuid = "123"
    name = "alarm"
    type = "VM"
    user_id = ""
    sub_type = ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE.value
    source_type = "restore"


class BackupOceanbaseReq:
    action = PolicyActionEnum.log.value


class OceanbaseResource:
    uuid = "123"
    name = "alarm"
    type = "VM"
    user_id = ""
    sub_type = ResourceSubTypeEnum.OCEANBASE_CLUSTER.value
    source_type = "restore"


class BackupReq:
    action = PolicyActionEnum.log.value


class Resource:
    uuid = "123"
    name = "alarm"
    type = "VM"
    user_id = ""
    sub_type = ResourceSubTypeEnum.TiDB_CLUSTER.value
    source_type = "restore"


if __name__ == '__main__':
    unittest.main(verbosity=2)

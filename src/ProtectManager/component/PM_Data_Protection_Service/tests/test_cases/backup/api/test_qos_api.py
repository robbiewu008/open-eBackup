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
import asyncio
from tests.test_cases.tools import http, env, redis_mock
from tests.test_cases import common_mocker # noqa
from unittest import mock
mock.patch("requests.get", http.get_request).start()
mock.patch("requests.post", http.post_request).start()
mock.patch("os.getenv", env.get_env).start()
mock.patch("redis.Redis", redis_mock.RedisMock).start()
mock.patch("app.common.database.Database.initialize", mock.Mock).start()
sys.modules['app.backup.common.config.db_config'] = mock.Mock()
from functools import wraps
from unittest.mock import patch, Mock
from sqlalchemy.orm import Session, session
from sqlalchemy.orm import query
from app.backup.schemas import qos
from app.backup.models import qos_table
from app.common.security import jwt_utils


def mock_decorator(*args, **kwargs):
    def decorator(f):
        @wraps(f)
        def decorated_function(*args, **kwargs):
            return f(*args, **kwargs)
        return decorated_function
    return decorator


mock.patch('app.common.security.right_control.right_control', mock_decorator).start()


class QosReqsMock:
    def __init__(self):
        self.name = 'name123'
        self.speed_limit = 1
        self.description = 'description123'
        self.user_id = "user_id123"


qos_object = qos_table.QosTable(uuid="is_delete_qos_uuid", name="name123", speed_limit=1, description="description123",
                                user_id="user_id123")

user_info = {'user-name': "user_name123", 'user-id': "user_id123", 'role-list': "role_list", 'es-valid-token': "true"}


@patch.object(jwt_utils, "get_user_info_from_token", Mock(return_value=user_info))
@patch.object(query.Query, "count", Mock(return_value=0))
class TestQosApi(unittest.TestCase):
    def setUp(self) -> None:
        from tests.test_cases.common.events import mock_producer
        from app.backup.api import qos_api
        self.qos_api = qos_api

    def test_create_qos(self):
        """
        测试创建QOS

        期望：
        1.成功创建QOS，顺利获取user_id
        2.限速策略小于规格64，超过64，不支持新增
        :return:
        """
        token = "eyJhbGciOiJSUzI1NiJ9.eyJleHBpcmVzX2F0IjoxNjE1MTk3Njk2MDAwLCJpc3MiOiJlbWVpc3Rvci5odWF3ZWkuY29tIiwidXNl\
        cl9yb2xlcyI6W10sImV4cCI6MTYxNTE5NzY5NiwidXNlciI6eyJpZCI6Ijg4YTk0YzQ3NmYxMmEyMWUwMTZmMTJhMjQ2ZTUwMDA5IiwibmFtZS\
        I6InN5c2FkbWluIiwicm9sZXMiOlt7Im5hbWUiOiJSb2xlX1NZU19BZG1pbiIsImlkIjoiMSJ9XX0sImlhdCI6MTYxNDU5NzY5NiwiaXNzdWVk\
        X2F0IjoxNjE0NTk3Njk2MDAwfQ.U7nanLo32uT9EuUb_MI_4djNEdVSO8gy8S-fJVZSADDlT3QIqX46x2tsdxqxws0hzTWZeA0qAtZqwZrqQyt\
        PTJCmQZm0j-0LcCOynLrIpmklfH0cD8fmmzLXCZY4D-jBwuQnXXZ3G2e4VMKUNkpr2oEpvE4lTWBY31R3EiGCbfAbayR5bBbldwvRriZ1ZvcT2\
        B97d8xX-Qv3oEXte7jqDKblwiLOinD6KvMCWvmwWb1boxAqfCUWbui8WKxJ0sxmvdh52tKFpp4NJslSFWUClMlBhjt_dz62beXc1APFdWYCIl-\
        4PNPC_7zCcgwV6AKqkSqCZWGnIulZ2xkGaM-wrg"
        qos_req = qos.QosReq(name="full", speed_limit=10, description="description123")
        patch("app.backup.client.rbac_client.RBACClient.add_resource_set_relation", return_value=None).start()
        creat_qos = asyncio.run(self.qos_api.create_qos(qos_req, token, Session()))
        self.assertIsNone(creat_qos)

    def test_delete_qos(self):
        """
        测试删除QOS

        期望：
        1.检查是否能删除限速策略，如果绑定sla不支持删除， can_delete_qos返回True
        :return:
        """

        query.Query.all = Mock(return_value=[qos_object])
        session.Session.delete = Mock(return_value=None)
        patch("app.backup.service.qos_service.can_delete_qos", return_value=True).start()
        patch("app.backup.client.rbac_client.RBACClient.delete_resource_set_relation", return_value=None).start()
        qos_ids = ["qos_id1", "qos_id2"]
        delete_qos = asyncio.run(self.qos_api.delete_qos(qos_ids, Session()))
        self.assertIsNone(delete_qos)

    def test_update_qos(self):
        """
        测试更新QOS

        期望：
        1.检查是否能更新QOS信息
        :return:无返回值
        """
        qos_id = "qos_id123"
        qos_req = QosReqsMock()
        query.Query.update = Mock(return_value=None)
        update_qos = asyncio.run(self.qos_api.update_qos(qos_id, qos_req, Session()))
        self.assertIsNone(update_qos)

    def test_get_qos(self):
        """
        测试get_qos

        期望：
        1.检查是否能获取QOS信息
        :return:无返回值
        """
        qos_id = "qos_id123"
        query.Query.first = Mock(return_value=None)
        get_qos = asyncio.run(self.qos_api.get_qos(qos_id, Session()))
        self.assertIsNone(get_qos)

    def test_internal_get_qos(self):
        """
        测试internal_get_qos

        期望：
        1.检查是否能获取QOS信息
        :return:无返回值
        """
        qos_id = "qos_id123"
        query.Query.first = Mock(return_value=None)
        internal_get_qos = asyncio.run(self.qos_api.internal_get_qos(qos_id, Session()))
        self.assertIsNone(internal_get_qos)

    def test_internal_verify_qos_ownership(self):
        """
        测试内部验证QOS所有权, 分域资源操作权限校验

        期望：
        1.如果没有权限，则抛出异常，如果有权限，正常响应
        :return:无返回值
        """
        user_id= "user_id123"
        qos_uuid_list = ["qos_id123"]
        query.Query.count = Mock(return_value=1)
        qos_ownership = asyncio.run(self.qos_api.internal_verify_qos_ownership(user_id, qos_uuid_list, Session()))
        self.assertIsNone(qos_ownership)

    def test_revoke_qos_user_id(self):
        user_id= "user_id123"
        query.Query.update = Mock(return_value=None)
        revoke_qos_user_id = asyncio.run(self.qos_api.revoke_qos_user_id(user_id, Session()))
        self.assertIsNone(revoke_qos_user_id)


if __name__ == '__main__':
    unittest.main(verbosity=2)

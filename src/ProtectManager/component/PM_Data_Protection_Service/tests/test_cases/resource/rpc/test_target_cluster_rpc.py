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
import json
import sys
import unittest
from unittest import TestCase
from unittest.mock import Mock

import requests

from app.common.exception.unified_exception import EmeiStorBizException
from app.resource.schemas.host_models import HostMigrationSchedule


class TClusterTargetMock:
    def __init__(self):
        self.cluster_id = 1
        self.cluster_ip = "127.1.1.1,127.0.0.0"
        self.username = "123"
        self.password = "sdf"


class MigrationResponseMock:
    def __init__(self):
        self.target_cluster_id = 1
        self.host_migrate_res = [HostMigrationResponseMock()]


class HostMigrationResponseMock:
    def __init__(self):
        self.host_id = "1"
        self.proxy_host_type = "DBBackupAgent"
        self.os_type = "Linux"
        self.ip_type = ""
        self.ip_address = "127.0.0.1"
        self.host_username = "123"
        self.host_password = "123"


class HostMigrationScheduleMock:
    def __init__(self):
        # 后端获取
        self.host_id: str = "b12f4665-8600-4e12-9da3-5c2b130729b5"
        self.host_user_name: str = "name"
        self.host_password: str = "123"
        self.ssh_macs: str = "safe"
        self.proxy_host_type: str = "DBBackupAgent"
        self.os_type: str = "Linux"
        self.ip_type: str = "IPV4"
        self.ip_address: str = "127.0.0.1"
        self.target_cluster_ip: str = "127.0.0.0"
        self.target_cluster_port: int = 99
        self.job_id: str = "1234"
        self.target_cluster_job_id: str = "1234"

    def dict(self):
        return self.__dict__


def json_test():
    return {"job": '123', "errorCode": 16665543665, "parameters": ["11", "22", "33"], "errorMessage": ""}


class TestTargetClusterRpc(TestCase):

    def setUp(self) -> None:
        super(TestTargetClusterRpc, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.resource.rpc import target_cluster_rpc
        self.target_cluster_rpc = target_cluster_rpc
        self.rpc = self.target_cluster_rpc.TargetClusterRpc
        self.decrypt = self.target_cluster_rpc.decrypt
        # self.get_target_cluster_token_by_ip_from_redis = \
        #     self.target_cluster_rpc.get_target_cluster_token_by_ip_from_redis

    def test_is_response_status_ok(self):
        """
        校验更新为迁移中是否存在问题
        """
        req = requests.Response
        req.status_code = 200
        res = self.target_cluster_rpc.is_response_status_ok(req)
        self.assertEqual(res, True)
        req.status_code = 401
        res = self.target_cluster_rpc.is_response_status_ok(req)
        self.assertEqual(res, False)

    def test_get_target_cluster_token(self):
        params = {"userName": "123",
                  "password": "234"}

        self.target_cluster_rpc.decrypt = Mock(return_value="123")
        self.target_cluster_rpc.decrypt = Mock(return_value="123")
        self.target_cluster_rpc.encrypt = Mock(return_value="654321")
        res = requests.Response
        res.status_code = 200
        res.json = json_test
        self.target_cluster_rpc.requests.request = Mock(return_value=res)
        res1 = self.rpc.get_target_cluster_token("127.0.0.0", params)
        self.assertIsNotNone(res1)

    def test_post_target_cluster_migrate_task_rpc(self):
        self.target_cluster_rpc.get_target_cluster_token_by_ip_from_redis = Mock(return_value="123456")
        self.target_cluster_rpc.decrypt = Mock(return_value="654321")
        self.target_cluster_rpc.requests = Mock()
        res = requests.Response
        res.status_code = 200
        res.json = json_test
        self.target_cluster_rpc.requests.request = Mock(return_value=res)
        res1 = self.rpc.post_target_cluster_migrate_task_rpc(
            HostMigrationSchedule(**(HostMigrationScheduleMock().dict())))
        self.assertEqual(res1, json_test())

    def test_post_target_cluster_migrate_task_rpc_exception(self):
        self.target_cluster_rpc.get_target_cluster_token_by_ip_from_redis = Mock(return_value="123456")
        self.target_cluster_rpc.decrypt = Mock(return_value="654321")
        self.target_cluster_rpc.requests = Mock()
        res = requests.Response
        res.status_code = 500
        res.json = json_test
        self.target_cluster_rpc.requests.request = Mock(return_value=res)
        try:
            self.rpc.post_target_cluster_migrate_task_rpc(
                HostMigrationSchedule(**(HostMigrationScheduleMock().dict())))
        except EmeiStorBizException:
            self.assertTrue(True)


if __name__ == '__main__':
    unittest.main(verbosity=2)
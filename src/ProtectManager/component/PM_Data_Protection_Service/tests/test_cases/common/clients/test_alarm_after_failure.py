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
from unittest.mock import Mock
ext_parameters = {
    "alarm_after_failure": True
}
# 1,2
status = 0
ext_parameters_not = {
    "alarm_after_failure": False
}
context = {
    "policy": "",
    "resource_id": "123",
    "resource_name": "alarm",
    "job_type": "BACKUP",
    "resource": {"uuid": "123", "name": "alarm"}
}

resource_obj = {
    "uuid": "123",
    "name": "alarm",

}


class Resource:
    uuid = "123"
    name = "alarm"
    type = "VM"
    user_id = ""
    sub_type = "vim"
    source_type = "restore"

    @classmethod
    def dict(cls):
        return {"name": "alarm"}


class TestSystemBaseClient(unittest.TestCase):
    def setUp(self):
        super(TestSystemBaseClient, self).setUp()
        sys.modules['app.common.database'] = Mock()
        sys.modules['app.common.config'] = Mock()
        sys.modules['app.protection.object.common.db_config'] = Mock()
        from app.common.clients.alarm import alarm_after_failure
        self.alarm_after_failure = alarm_after_failure

    def test_alarm_after_failure_job_type_back_not_alarm_task(self):
        """
        用例场景：sla未配置任务失败发送告警
        前置条件：sla调度任务正常
        检查点：检查sla未配置作业失败时不发送告警
        """
        policy = {
            "ext_parameters": ext_parameters_not
        }
        context['policy'] = policy
        res = self.alarm_after_failure.alarm_after_failure(context, status, resource_obj)
        self.assertIsNone(res)

    def test_alarm_after_failure_job_type_back_send_alarm(self):
        """
        用例场景：sla配置备份任务失败发送告警
        前置条件：sla调度任务正常
        检查点：检查sla配置备份任务作业失败时发送告警
        """
        self.alarm_after_failure.AlarmClient = Mock()
        policy = {
            "ext_parameters": ext_parameters
        }
        context['policy'] = policy
        res = self.alarm_after_failure.alarm_after_failure(context, status, resource_obj)
        self.assertIsNone(res)

    def test_alarm_after_failure_job_type_back_clear_alarm(self):
        """
        用例场景：sla配置备份任务失败，任务成功后清理告警
        前置条件：sla调度任务正常
        检查点：检查sla配置备份任务作业成功时发送告警
        """
        self.alarm_after_failure.AlarmClient = Mock()
        policy = {
            "ext_parameters": ext_parameters
        }
        context['policy'] = policy
        status = 1
        res = self.alarm_after_failure.alarm_after_failure(context, status, resource_obj)
        self.assertIsNone(res)

    def test_alarm_after_failure_job_type_back_no_alarm(self):
        """
        用例场景：sla配置备份任务失败，任务成功后发送告警
        前置条件：sla调度任务正常
        检查点：检查sla配置备份任务作业成功时发送告警
        """
        self.alarm_after_failure.AlarmClient = Mock()
        policy = {
            "ext_parameters": ext_parameters,
            "type": "backup"
        }
        context['policy'] = policy
        context['resource_id'] = None
        context['resource_name'] = None
        context["job_type"]: None
        status = 1
        resource_obj = None
        resource = {"uuid": "123", "name": "alarm"}
        context['resource'] = resource
        self.alarm_after_failure.resource_service.query_resource_by_id = Mock(return_value=Resource)
        res = self.alarm_after_failure.alarm_after_failure(context, status, resource_obj)
        self.assertIsNone(res)
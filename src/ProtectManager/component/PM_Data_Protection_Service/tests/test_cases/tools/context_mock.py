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
from typing import TypeVar

T = TypeVar('T')

schedule = {"window_end": "21:12:10", "window_start": "21:12:10", "interval": 2,
            "interval_unit": "d"}
class ContextMock:
    def __init__(self, request_id):
        self.Context_key_value = {"resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
                                  "first_backup": "1",
                                  "timeout_time": "2020-12-08 10:20:30",
                                  "execute_type": "MANUAL",
                                  "policy": {"action": "full", "schedule": schedule},
                                  "protected_object": {"sub_type": "Oracle"},
                                  "job_type": "backup",
                                  "retry_times": "1",
                                  "wait_minutes": "1",
                                  "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
                                  "auto_retry": "True",
                                  "sla": {"sla_id": "1a881ee6-69f8-4615-9b7c-da599402619d"},
                                  "sla_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
                                  "chain_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
                                  "resource_name": "resource_name",
                                  "time_window_start": "20:20:10",
                                  "time_window_end": "20:10:10",
                                  "resource": {
                                      "name": "60370663-4bf4-4dc4-8439-5dea1b404cd1original",
                                      "path": "8.40.98.222/Datacenter/8.40.98.30",
                                      "root_uuid": "1d0ecd9a-2d74-45b3-911a-0a7efa246df5",
                                      "parent_name": "8.40.98.30",
                                      "parent_uuid": "cdee92aa-f83f-11e5-973c-84ad58e1a25b",
                                      "type": "VM",
                                      "sub_type": "vim.VirtualMachine",
                                      "uuid": "05567304-7a9b-412d-9e9d-8c75c76a2bde",
                                      "created_time": "2021-04-28T11:15:56.829058",
                                      "user_id": "88a94c476f12a21e016f12a246e50010",
                                      "version": "",
                                      "protection_status": 0,
                                      "environment_uuid": "1d0ecd9a-2d74-45b3-911a-0a7efa246df5",
                                      "environment_name": "VC222",
                                      "environment_endpoint": "8.40.98.222",
                                      "environment_type": "vSphere",
                                      "environment_sub_type": "VMware vCenter Server",
                                      "environment_is_cluster": "False",
                                    }
                                  }

    def get(self, key, t: T = str):
        if t is str:
            return self.Context_key_value.get(key)
        elif t is dict:
            value = self.Context_key_value.get(key)
            if isinstance(value, dict):
                return value
            return json.loads(value)

    def set(self, key, value):
        self.Context_key_value.update({key: value})
        return None

    def delete_all(self):
        return

    def exist(self):
        return {"resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
                "first_backup": "1",
                "timeout_time": "2020-12-08 10:20:30",
                "execute_type": "MANUAL",
                "policy": {"action": "full", "schedule": schedule},
                "protected_object": {"sub_type": "Oracle"},
                "job_type": "backup",
                "retry_times": "1",
                "wait_minutes": "1",
                "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
                "auto_retry": "True",
                "sla_id": "1a881ee6-69f8-4615-9b7c-da599402619d"
                }

    def delete(self, key):
        return None


class ContextsMock:
    def __init__(self, request_id):
        self.Context_key_value = {"resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
                                  "first_backup": "1",
                                  "timeout_time": "2020-12-08 10:20:30",
                                  "execute_type": "MANUAL",
                                  "policy": {"action": "full", "schedule": schedule},
                                  "protected_object":{"sub_type":"Oracle"},
                                  "job_type": "backup",
                                  "retry_times":"1",
                                  "wait_minutes": "1",
                                  "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
                                  "auto_retry":"True",
                                  "sla_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
                                  "chain_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
                                  "resource_name":"resource_name",
                                  "time_window_start":"20:20:10",
                                  "time_window_end": "20:10:10"
                                  }

    def get(self, key, t: T = str):
        return self.Context_key_value.get(key)

    def set(self, key, value):
        self.Context_key_value.update({key: value})
        return None

    def delete_all(self):
        return

    def exist(self):
        return False

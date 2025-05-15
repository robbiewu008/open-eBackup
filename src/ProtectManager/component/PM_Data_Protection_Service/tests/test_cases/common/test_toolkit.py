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
# #  Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
# import asyncio
#
# import requests
#
#
# GET_DB_PASSWORD_URL = "http://infrastructure:80/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret"
#
#
# def get_request(url, params=None, **kwargs):
#     response = requests.models.Response()
#     if url in GET_DB_PASSWORD_URL:
#         response.status_code = 400
#     else:
#         response.status_code = 200
#     response._content = b'1'
#     return response
#
#
# import unittest
# import uuid
# from unittest import mock
# from tests.tools import http, functiontools
#
# mock.patch("requests.get", get_request).start()
# mock.patch("requests.post", http.post_request).start()
# mock.patch("requests.put", http.put_request).start()
# from tests.tools.timezone import dmc
#
# mock.patch("pydantic.validator", functiontools.mock_decorator).start()
# mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone", dmc.query_time_zone).start()
# from app.common.concurrency import to_async_method
# from app.common import toolkit
# from app.common.toolkit import json2list, get_obj_val_by_key
#
#
# class Methods:
#     def __init__(self, cargo=None, next=None):
#         self.cargo = cargo
#         self.next = next
#
#
# class TestTookit(unittest.TestCase):
#     def test_count_task_in_status(self):
#         schedule = {"window_end": "21:12:11", "window_start": "21:12:11", "interval": 2,
#                     "interval_unit": "d"}
#         policy = {"action": "full", "schedule": schedule}
#         params = {
#             "policy": policy,
#             "resource_id": "res123",
#             "execute_type": "w",
#             "sla_id": 1,
#             "chain_id": 24,
#             "user_id": "user_id123",
#             "uuid": uuid.uuid1()
#         }
#         count_task_in_status = toolkit.count_task_in_status(params)
#         self.assertEqual(count_task_in_status, 1)
#
#     def test_combine_multiple_method_as_chain_method(self):
#         methods1 = Methods(1)
#         methods2 = Methods(2)
#         methods1.next = methods2
#         methods = Methods
#         result = toolkit.combine_multiple_method_as_chain_method(methods)(1)
#         self.assertIsNotNone(result)
#
#     def test_get_value_by_path(self):
#         obj = "object123"
#         path = "*.*.t=zone"
#         get_value_by_path = toolkit.get_value_by_path(obj, path)
#         self.assertIsNotNone(get_value_by_path)
#
#     def test_json2list(self):
#         test_str: str = '{[{"jobId1":"jobId_str"}]}'
#         self.assertEqual([], json2list(test_str))
#
#         test_str2: str = '[{"jobId2":"jobId_str"}]'
#         self.assertEqual([{"jobId2": "jobId_str"}], json2list(test_str2))
#
#         self.assertEqual([{'jobId': 'jobId_str'}], json2list('{"jobId":"jobId_str"}'))
#
#     def test_json2dict(self):
#         self.assertEqual({'jobId': '3076c3ef789e47b9bff29491e65b87fd'},
#                          toolkit.json2dict('{"jobId":"3076c3ef789e47b9bff29491e65b87fd"}'))
#
#         json2dict = toolkit.json2dict('[{"jobId":"3076c3ef789e47b9bff29491e65b87fd"}')
#         self.assertEqual(json2dict, {})
#
#         test_str: str = '[{"jobId":"3076c3ef789e47b9bff29491e65b87fd"},' \
#                         '{"jobId":"3076c3ef789e47b9bff29491e65b87fd"},' \
#                         '{"jobId":"3076c3ef789e47b9bff29491e65b87fd"}]'
#
#         json2dict = toolkit.json2dict(test_str)
#         self.assertEqual(json2dict, {})
#
#     def test_to_collection(self):
#         obj = ["1"]
#         to_collection = toolkit.to_collection(obj)
#         self.assertEqual(to_collection, ["1"])
#         obj = None
#         to_collection = toolkit.to_collection(obj)
#         self.assertEqual(to_collection, [])
#         obj = False
#         to_collection = toolkit.to_collection(obj)
#         self.assertIsNotNone(to_collection)
#
#     def test_create_job_center_task(self):
#         request_id = "3076c3ef789e47b9bff29491e65b87fd"
#         schedule = {"window_end": "21:12:10", "window_start": "21:12:10", "interval": 2,
#                     "interval_unit": "d"}
#         policy = {"action": "full", "schedule": schedule}
#         params = {
#             "policy": policy,
#             "resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
#             "execute_type": "w",
#             "sla_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
#             "chain_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
#             "user_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
#             "uuid": uuid.uuid1(),
#             "auto_retry": "True",
#             "auto_retry_times": "8",
#             "auto_retry_wait_minutes": 100,
#             "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
#             "message": "it is test"
#         }
#         payload = {
#             "uuid": "'c584d517-f2ef-4512-8fdc-7fdf6b5d0a9a'",
#             "application": "Fileset",
#             "is_global": False,
#             "sla": {
#                 'policy_list': [{"uuid": "0817badf-60b8-4a01-819c-3915d01b5261",
#                                  "type": "replication",
#                                  "schedule": {"trigger": 1,
#                                               "interval": 0,
#                                               "interval_unit": "m",
#                                               "start_time": "2021-03-10T01:26:23"
#                                               }}]
#             }
#         }
#         message = toolkit.JobMessage(topic="topic", payload=payload, traffic={}, abolish=[], context=True)
#         create_job_center_task = toolkit.create_job_center_task(request_id, params, message)
#         self.assertIsNotNone(create_job_center_task)
#
#     def test_complete_job_center_task(self):
#         request_id = "3076c3ef789e47b9bff29491e65b87fd"
#         job_id = "1a881ee6-69f8-4615-9b7c-da5994026e9d"
#         schedule = {"window_end": "21:12:10", "window_start": "21:12:10", "interval": 2,
#                     "interval_unit": "d"}
#         policy = {"action": "full", "schedule": schedule}
#         params = {
#             "policy": policy,
#             "resource_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
#             "execute_type": "w",
#             "sla_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
#             "chain_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
#             "user_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
#             "uuid": uuid.uuid1(),
#             "auto_retry": "True",
#             "auto_retry_times": "8",
#             "auto_retry_wait_minutes": 100,
#             "job_id": "1a881ee6-69f8-4615-9b7c-da599402619d",
#             "message": "it is test",
#             "endTime": "20:12:18",
#         }
#         complete_job_center_task = toolkit.complete_job_center_task(request_id, job_id, params)
#         self.assertIsNone(complete_job_center_task)
#
#     def test_build_update_job_log_request(self):
#         job_id = "1a881ee6-69f8-4615-9b7c-da599402619d"
#         log_label = "log"
#         from app.common.enums.job_enum import JobLogLevel
#         log_info_param = None
#         build_update_job_log_request = toolkit.build_update_job_log_request(job_id, log_label, JobLogLevel("info"),
#                                                                             log_info_param=log_info_param)
#         self.assertIsNotNone(build_update_job_log_request)
#
#     def test_get_obj_val_by_key(self):
#         # None
#         self.assertEqual(None, get_obj_val_by_key(None, 'testKey'))
#
#         # dict
#         test_dict: dict = {'testKey': 'testValue'}
#         self.assertEqual('testValue', get_obj_val_by_key(test_dict, 'testKey'))
#         self.assertEqual(test_dict.values().__str__(), get_obj_val_by_key(test_dict, '*').__str__())
#
#         # list
#         test_list = ['test1', 'test2']
#         self.assertEqual('test2', get_obj_val_by_key(test_list, 1))
#         self.assertEqual(['test1', 'test2'], get_obj_val_by_key(test_list, '*'))
#
#         # class
#         class testClass:
#             def __init__(self, value: str):
#                 self.value = value
#
#         test_class = testClass("testValue")
#         self.assertEqual('testValue', get_obj_val_by_key(test_class, 'value'))
#
#     def test_to_async_method(self):
#         def x():
#             raise Exception('Error')
#
#         x1 = to_async_method(x)
#
#         def y():
#             return 1
#
#         y1 = to_async_method(y)
#
#         async def main():
#             try:
#                 await x1()
#                 self.assertTrue(False)
#             except:
#                 self.assertTrue(True)
#             v = await y1()
#             self.assertEqual(v, 1)
#
#         asyncio.run(main())
#
#
# if __name__ == '__main__':
#     unittest.main(verbosity=2)

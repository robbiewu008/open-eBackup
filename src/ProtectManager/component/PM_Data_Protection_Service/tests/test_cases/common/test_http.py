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
# import unittest
# from unittest import mock
# import requests
#
# from app.common import http
# from app.common.http import LONG_RETRY_POLICY
# from tests.tools import functiontools
# from tests.tools import http as http_mock
# from app.common.exception.unified_exception import EmeiStorBizException
#
# mock.patch("pydantic.validator", functiontools.mock_decorator).start()
# mock.patch("requests.post", http_mock.post_request).start()
#
#
# class TestHttp(unittest.TestCase):
#     def test_adapt(self):
#         name = http.adapt(max_retries=LONG_RETRY_POLICY).get.__name__
#         self.assertTrue(name, "get")
#
#     def test_wrap_post_failed(self):
#         uri = f'http://pm-system-base:30081/v1/schedules'
#         schedule_req = {"schedule_name": "382d5050-b4b6-4adb-afef-9e238169d0a8"}
#         exception_info = f'invoke api to create customize interval schedule, uri={uri}, schedule_req={schedule_req}'
#         warpper = http.wrap(requests.post)
#         self.assertRaises(EmeiStorBizException, warpper, uri, schedule_req, exception_info)

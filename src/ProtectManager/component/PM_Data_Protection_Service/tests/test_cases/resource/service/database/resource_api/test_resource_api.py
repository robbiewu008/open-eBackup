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
# import unittest
# from unittest import TestCase, mock
#
# from tests.test_cases.tools import timezone, functiontools
#
# mock.patch("pydantic.validator", functiontools.mock_decorator).start()
# mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
#            timezone.dmc.query_time_zone).start()
#
# from app.common.exception.unified_exception import EmeiStorBizException
# from app.resource.service.common.resource_service import revoke, authorize_resource
#
#
# class TestResourceService(TestCase):
#     def test_revoke_resource(self):
#         """
#         测试场景：调用接口时校验参数
#         前提条件: 参数user_id 和 resource_uuid_list的各种异常情况
#         检查点: 希望参数不符合时抛出异常
#         """
#         user_id = None
#         user_id1 = "88a94c476f12a21e016f12a246e50010"
#         resource_uuid_list = ["3470387C-564D-48C8-926D-EFB901AD378D"]
#         resource_uuid_list1 = None
#         # 当长度超过512时
#         resource_uuid_list2 = ["dreswerrtwerr4234feferwerewytry5r56756ytyrttertret56767rtrt"
#                                "5r56756ytyrttertret56767rtrtxsd1bc346xsd1bc346xsd1bc346xs11"
#                                "d1bc346xsd1bcxsd1bc346xsd1bc346xsd1bc346xsd1bc3346xsd1bc346"
#                                "46xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd"
#                                "1bc346xd1bc346xsd1bc346xsd1bc346sd1bc346xsd1bc346xsd1bc346xs"
#                                "xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc"
#                                "346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd"
#                                "1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346"
#                                "xsd1bc346xsd1bc346xsd1bc346xsd1bc346x"]
#         self.assertRaises(EmeiStorBizException, revoke, user_id1, resource_uuid_list2)
#         self.assertRaises(EmeiStorBizException, revoke, user_id, resource_uuid_list1)
#         self.assertRaises(EmeiStorBizException, revoke, user_id1, resource_uuid_list1)
#         self.assertRaises(EmeiStorBizException, revoke, user_id, resource_uuid_list2)
#         self.assertRaises(EmeiStorBizException, revoke, user_id, resource_uuid_list)
#
#     def test_authorize_resource(self):
#         """
#         测试场景：调用接口时校验参数
#         前提条件: 参数user_id 和 resource_uuid_list的各种异常情况
#         检查点: 希望参数不符合时抛出异常
#         """
#         user_id = None
#         user_id1 = "88a94c476f12a21e016f12a246e50010"
#         resource_uuid_list = ["3470387C-564D-48C8-926D-EFB901AD378D"]
#         resource_uuid_list1 = None
#         # 当长度超过512时
#         resource_uuid_list2 = ["1bc34sd1bc346xsd1bc1121346xsd1bc346xsd1bc346xsd1bc346xsd1bc"
#                                "346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xs"
#                                "d1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc3"
#                                "46xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd"
#                                "1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346"
#                                "xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc"
#                                "346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd"
#                                "1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346xsd1bc346"
#                                "xsd1bc346xsd1bc346xsd1bc346xsd1bc346x"]
#         self.assertRaises(EmeiStorBizException, authorize_resource, user_id1, resource_uuid_list2)
#         self.assertRaises(EmeiStorBizException, authorize_resource, user_id, resource_uuid_list1)
#         self.assertRaises(EmeiStorBizException, authorize_resource, user_id1, resource_uuid_list1)
#         self.assertRaises(EmeiStorBizException, authorize_resource, user_id, resource_uuid_list2)
#         self.assertRaises(EmeiStorBizException, authorize_resource, user_id, resource_uuid_list)
#
#
# if __name__ == '__main__':
#     unittest.main(verbosity=2)

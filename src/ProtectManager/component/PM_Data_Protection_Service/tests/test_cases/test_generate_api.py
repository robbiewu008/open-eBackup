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
# from unittest import mock
# from tests.test_cases.tools import http, env, timezone
# import sys
# sys.modules['app.common.events.producer'] = mock.Mock()
# sys.modules['app.common.events.topics'] = mock.Mock()
# mock.patch("requests.get", http.get_request).start()
# mock.patch("requests.post", http.post_request).start()
# mock.patch("os.getenv", env.get_env).start()
# mock.patch("app.common.clients.device_manager_client.device_manager_client.init_time_zone",
#            timezone.dmc.query_time_zone).start()
# mock.patch("app.common.database.Database.initialize", mock.Mock).start()
# import yaml
# import os
# from starlette.testclient import TestClient
# from app.routers import api
#
# client = TestClient(api)
#
#
# @mock.patch("threading.Timer.start", mock.Mock())
# def test_generate_open_api_yaml():
#     """
#
#     :param scope: api范围，取值：“all”--全部接口，“public”--不包含internal接口
#     :return:
#     """
#     from app.routers import api
#     client = TestClient(api)
#     assert api is not None
#     response = client.get("/openapi.json")
#     assert response is not None
#     resp_json = response.json()
#     resp_json['info']['title'] = "数据保护接口"
#     resp_json['info']['version'] = "v1"
#     yaml_text = yaml.dump(resp_json)
#     file_path = os.path.join(os.path.dirname(__file__), "../../doc/api/openapi.yaml")
#     # if scope == 'all':
#     #     yaml_text = yaml.dump(resp_json)
#     #     file_path = os.path.join(os.path.dirname(__file__), "../../doc/api/openapi.yaml")
#     # elif scope == 'public':
#     #     # 移除“/v1/internal/”开头的内部接口
#     #     for path in list(resp_json['paths'].keys()):
#     #         if str(path).startswith("/v1/internal/"):
#     #             del resp_json['paths'][path]
#     #     yaml_text = yaml.dump(resp_json)
#     #     file_path = os.path.join(os.path.dirname(__file__), "../../doc/api/openapi_public.yaml")
#     # else:
#     #     return
#     with open(file_path, "w") as out:
#         out.write(yaml_text)
#
#
# if __name__ == '__main__':
#     test_generate_open_api_yaml()
#     # test_generate_open_api_yaml("public")

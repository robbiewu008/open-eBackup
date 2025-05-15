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
import requests

GET_DB_PASSWORD_URL = "http://infrastructure:8088/v1/infra/secret/info?nameSpace=dpa&secretName=common-secret"
DATABASE_URL = "http://pm-protection-service:1/v1/internal/databases"
DELETE_QOS_URL = "http://pm-system-base:30081/v1/internal/slas/policies/ext-parameters?key=qos_id&value=is_delete_qos_uuid"
QUERY_RESOURCE_COUNT = "http://pm-protection-service:30092/v1/internal/protected-objects/count?sla_id=88a94c476f12a21e016f12a246e50009&user_id="
COPIES_ARCHIVE_DONE_HANDLER_URL = "http://pm-protection-service:30092/v1/internal/copies"


def get_request(url, params=None, **kwargs):
    response = requests.models.Response()
    if url in GET_DB_PASSWORD_URL:
        response.status_code = 400
    elif url in QUERY_RESOURCE_COUNT:
        response.status_code = 500
    else:
        response.status_code = 200

    if url in DATABASE_URL:
        response._content = b'{"items":[{"uuid": "123"}]}'
    elif url in DELETE_QOS_URL:
        response._content = b'{}'
    elif url in COPIES_ARCHIVE_DONE_HANDLER_URL:
        response._content = b'{"items": [{"resource_sub_type": "Fileset", ' \
                            b'"resource_id": "2ce0d92d-2841-4552-b272-271887801a5b", ' \
                            b'"uuid": "a979669f-30ed-45c3-abe4-05cc462a1a3a"}]}'
    else:
        response._content = b'{"total":1,"pages":0,"page_size":0,"page_no":0,"items":[],"status":1,' \
                            b'"user_id":"user123","uuid":"uuid123","path":"path123","name":"name123","type":1,' \
                            b'"sub_type":"sub_type123","sla_id":"sla_id123", "earliest_time":"2000:8:01", "data":[]}'
    return response


def post_request(url, data=None, json=None, **kwargs):
    response = requests.models.Response()
    response.status_code = 200
    response._content = b'{}'
    return response


def put_request(url, data=None, **kwargs):
    response = requests.models.Response()
    response.status_code = 200
    response._content = b'{"total":1,"pages":0,"page_size":0,"page_no":0,"items":[],"status":1}'
    return response


def sessions_request(*args, **kwargs):
    response = requests.models.Response()
    response.status_code = 200
    return response
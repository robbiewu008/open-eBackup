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
from http import HTTPStatus

from fastapi import HTTPException

from app.common.clients.client_util import SystemBaseHttpsClient, is_response_status_ok, parse_response_data
from app.common.exter_attack import exter_attack
from app.common.logger import get_logger

log = get_logger(__name__)


@exter_attack
def get_file_list(params: dict):
    try:
        response = SystemBaseHttpsClient().request(
            "GET", f"/v1/adapter/browse_vm", fields=params)
    except Exception as e:
        raise HTTPException(HTTPStatus.INTERNAL_SERVER_ERROR, str(e)) from e
    finally:
        pass

    if not is_response_status_ok(response):
        raise HTTPException(response.status)
    return parse_response_data(response.data)


@exter_attack
def get_instance_list(params: dict):
    try:
        response = SystemBaseHttpsClient().request(
            "GET", f"/v1/adapter/oracle/instances", fields=params)
    except Exception as e:
        raise HTTPException(HTTPStatus.INTERNAL_SERVER_ERROR, str(e)) from e
    finally:
        pass

    if not is_response_status_ok(response):
        raise HTTPException(response.status)

    res = parse_response_data(response.data)

    if not res.get('status') == 'success':
        raise HTTPException(HTTPStatus.INTERNAL_SERVER_ERROR, res.get('error', ''))

    dbs = res.get('responseData', [])

    return {
        'databases': dbs,
        'startIndex': params['index'],
        'currentCount': len(dbs),
        'total': res.get('totalNum', ''),
    }


def add_db_cluster(data):
    try:
        response = SystemBaseHttpsClient().request(
            "POST", f"/v1/adapter/oracle/rac", body=json.dumps(data))
    except HTTPException:
        raise
    except Exception as e:
        raise HTTPException(HTTPStatus.INTERNAL_SERVER_ERROR, str(e)) from e
    finally:
        pass

    if not is_response_status_ok(response):
        raise HTTPException(response.status)


def authorize_db(body: dict, container: dict):
    url = f"/v1/adapter/oracle"

    if container.get('host_id'):
        url += '/authorization'
    elif container.get('cluster_id'):
        url += '/rac/authorization'

    try:
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(body))
    except Exception as e:
        raise HTTPException(HTTPStatus.INTERNAL_SERVER_ERROR, str(e)) from e
    finally:
        pass

    if not is_response_status_ok(response):
        raise HTTPException(response.status)


def rename_cluster(old_name, new_name):
    try:
        data = {'clusterName': new_name}
        response = SystemBaseHttpsClient().request(
            "PUT", f"/v1/adapter/oracle/rac/{old_name}",
            body=json.dumps(data))
    except Exception as e:
        raise HTTPException(HTTPStatus.INTERNAL_SERVER_ERROR, str(e)) from e
    finally:
        pass

    if not is_response_status_ok(response):
        raise HTTPException(response.status)


def delete_cluster(name: str):
    try:
        data = {'clusterName': name}
        response = SystemBaseHttpsClient().request(
            "DELETE", f"/v1/adapter/oracle/rac", body=json.dumps(data))
    except Exception as e:
        raise HTTPException(HTTPStatus.INTERNAL_SERVER_ERROR, str(e)) from e
    finally:
        pass

    if not is_response_status_ok(response):
        raise HTTPException(response.status)


def config_oracle_asm(data: dict):
    try:
        response = SystemBaseHttpsClient().request(
            "POST", f"/v1/adapter/oracle/asm", body=json.dumps(data))
    except Exception as e:
        raise HTTPException(HTTPStatus.INTERNAL_SERVER_ERROR, str(e)) from e
    finally:
        pass

    if not is_response_status_ok(response):
        raise HTTPException(response.status)

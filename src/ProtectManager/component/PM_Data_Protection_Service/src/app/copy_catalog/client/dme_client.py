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

from app.common.logger import get_logger
from app.common.clients.client_util import DataEnableEngineHttpsClient, is_response_status_ok, parse_response_data
from app.common.exception.unified_exception import EmeiStorBizException
from app.copy_catalog.common.constant import ExtendRetentionConstant
from app.copy_catalog.service.curd.copy_query_service import query_copy_by_id

log = get_logger(__name__)


def update_dm_copy_retention(update_dm_copy_retention_params):
    copy_id, retention_duration, duration_unit, retention_type, extend_retention, copy_generate_type \
        = update_dm_copy_retention_params
    esn = query_copy_by_id(copy_id).device_esn
    params = {
        "copyId": copy_id,
        "expiredTime": retention_duration,
        "expiredTimeUnit": duration_unit,
        "retentionType": retention_type,
        "extendDays": extend_retention / ExtendRetentionConstant.ONE_DAY if extend_retention else
        ExtendRetentionConstant.FOREVER,
        "deviceEsn": esn
    }
    if copy_generate_type == "IO_DETECT":
        url = '/v1/internal/anti/ransomware/io-detect/copy-expire-policy'
    else:
        url = '/v1/internal/anti/ransomware/detect/copy-expire-policy'
    log.info(f"update retention duration copy_id:{copy_id} report start, url: {url}, esn:{esn}")
    try:
        response = DataEnableEngineHttpsClient().request("PUT", url, body=json.dumps(params))
    except Exception as e:
        log.exception(f"update retention duration fail:{e}")
    finally:
        pass
    resp_data = parse_response_data(response.data)
    if not is_response_status_ok(response):
        log.error(f"update copy expire policy error,response resp_data:{resp_data}")
        resp_data['parameters'] = []
        raise EmeiStorBizException.build_from_error(resp_data)

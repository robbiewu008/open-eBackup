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
from http import HTTPStatus

from app.common.clients.client_util import SystemBaseHttpsClient, parse_response_data
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException


def check_is_need_to_forward(ens: str):
    need_forward_res = SystemBaseHttpsClient().request(
        "GET", f"/v1/internal/clusters/need-forward/{ens}")
    if need_forward_res.status == HTTPStatus.OK:
        return parse_response_data(need_forward_res.data)
    else:
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke check need for ward api failed or timeout")


def send_create_index_forward_request(copy_id: str, esn: str):
    response = SystemBaseHttpsClient().request(
        "POST", f"/v1/internal/copies/forward/{copy_id}/action/create-index/{esn}")
    if not response.status == HTTPStatus.OK:
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                   error_message="invoke pm create index api failed or timeout")

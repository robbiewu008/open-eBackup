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

from app.common.clients.client_util import parse_response_data
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException


def propagate_http_error(
    response, expected_status, additional_info=None, log=None
):
    if additional_info is None:
        additional_info = {}

    def _get_info_from_response(response) -> dict:
        if response.data is None or len(response.data) == 0:
            return {"status": "No response content"}
        try:
            return dict(**parse_response_data(response.data))
        except:
            return {"content": response.data}
    expected = expected_status
    if isinstance(expected_status, int):
        expected = [expected_status]
    if response.status not in expected:
        info = {}
        info.update(additional_info)
        info.update(_get_info_from_response(response))
        if log:
            log.exception('Propagating error from underlying servicexx', extra=info)
        raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, message=json.dumps(info))

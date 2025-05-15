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
import re
import time

from fastapi import Request

from app.common.clients.client_util import ProtectionServiceHttpsClient, is_response_status_ok, parse_response_data, \
    SystemBaseHttpsClient
from app.common.constraints import thread_local
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException, IllegalParamException
from app.common.exter_attack import exter_attack
from app.common.log.event_client import EventClient
from app.common.log.event_schemas import SendEventReq, BatchOperationResult, \
    OperationConfig, LogRank
from app.common.logger import get_logger
from app.common.security.jwt_utils import get_username, get_user_id
from app.common.toolkit import get_value_by_path, listify

log = get_logger(__name__)


def page_query_data_by_uuid_list(url, uuids, key="uuid"):
    results = []
    page_no = 0
    page_size = 100
    while page_no >= 0:
        params = {
            "page_no": page_no,
            "page_size": page_size,
            "conditions": json.dumps({
                key: uuids
            })
        }
        response = ProtectionServiceHttpsClient().request("GET", url, fields=params)
        if not is_response_status_ok(response):
            raise EmeiStorBizException(
                CommonErrorCodes.SYSTEM_ERROR,
                message=f"prepare operation log info failed"
            )
        data = parse_response_data(response.data)
        items: list = data.get('items')
        results.extend(items)
        if len(items) < page_size:
            page_no = -1
        else:
            page_no = page_no + 1
    return results


def page_query_data_by_uuid_list_in_system_base(url, uuids, key="uuid"):
    results = []
    page_no = 0
    page_size = 100
    while page_no >= 0:
        params = {
            "pageNo": page_no,
            "pageSize": page_size,
            "conditions": json.dumps({
                key: uuids
            })
        }
        response = SystemBaseHttpsClient().request("GET", url, fields=params)
        if not is_response_status_ok(response):
            raise EmeiStorBizException(
                CommonErrorCodes.SYSTEM_ERROR,
                message=f"prepare operation log info failed"
            )
        data = parse_response_data(response.data)
        items: list = data.get('records')
        results.extend(items)
        if len(items) < page_size:
            page_no = -1
        else:
            page_no = page_no + 1
    return results


def query_data_by_uuid_list(url):
    response = ProtectionServiceHttpsClient().request("GET", url)
    if not is_response_status_ok(response):
        raise EmeiStorBizException(
            CommonErrorCodes.SYSTEM_ERROR,
            message="prepare operation log info failed"
        )
    return parse_response_data(response.data)


@exter_attack
def query_sla_by_uuid_list(url):
    response = SystemBaseHttpsClient().request("GET", url)
    if not is_response_status_ok(response):
        raise EmeiStorBizException(
            CommonErrorCodes.SYSTEM_ERROR,
            message="prepare operation log info failed"
        )
    return parse_response_data(response.data)


def load_sla(uuids):
    results = []
    for uuid in uuids:
        data = query_sla_by_uuid_list(
            f"/v1/internal/slas/{uuid}")
        results.append(data)
    return results


def load_resource(uuids):
    results = []
    for uuid in uuids:
        data = query_data_by_uuid_list(
            f"/v1/internal/resource/{uuid}")
        results.append(data)
    return results


def load_copy(uuids):
    return page_query_data_by_uuid_list(
        f"/v1/internal/copies", uuids)


def load_qos(uuids):
    return page_query_data_by_uuid_list(
        f"/v1/internal/qos", uuids)


def load_copy_resource(uuids):
    return page_query_data_by_uuid_list_in_system_base(
        f"/v2/internal/copies/summary/resources", uuids,
        "resourceIds")


def timestamp(date):
    if isinstance(date, (list, set, tuple)):
        return [timestamp(item) for item in date]
    return re.sub(r'\.\d+$', '', str(date).replace('T', ' '))


def cast_as_string(data):
    if isinstance(data, (tuple, list, set)):
        return " ".join([str(item) for item in data])
    elif data is None:
        return '--'
    else:
        return str(data)


data_loaders = {
    "copy": load_copy,
    "resource": load_resource,
    "sla": load_sla,
    "qos": load_qos,
    "copyResource": load_copy_resource,
    "timestamp": timestamp,
    "string": lambda data: [cast_as_string(item) for item in data],
}


def resolve_expr_with_loader(argument_list, expr: str):
    count = 0
    value = argument_list
    skip = False
    for e in expr.split("!"):
        if value is None:
            break
        if e.strip() == '':
            skip = True
        else:
            if count > 0:
                value = evaluate_with_loader(e, value, skip)
            else:
                value = get_value_by_path(value, e)
            skip = False
        count = count + 1
    return value


def evaluate_with_loader(e, value, original):
    offset = e.find('.')
    if offset == -1:
        loader_type = e
        suffix = None
    else:
        loader_type = e[:offset]
        suffix = e[offset + 1:]
    if loader_type not in data_loaders:
        raise EmeiStorBizException(
            CommonErrorCodes.SYSTEM_ERROR,
            message=f"not supported source loader type: {loader_type}"
        )
    loader = data_loaders[loader_type]
    if original:
        argument_list = (value,)
    else:
        argument_list = listify(value)
    items = listify(loader(argument_list))
    results = [get_value_by_path(item, suffix) for item in
               items] if suffix else items
    value = results if isinstance(value, (tuple, list, set)) else results[
        0] if results else None
    return value


def resolve_expr_value(arguments, expr):
    if callable(expr):
        param = expr(arguments)
    elif isinstance(expr, str):
        if expr[0] == '@':
            param = expr[1:]
        else:
            param = resolve_expr_with_loader(arguments, expr)
    else:
        param = str(expr)
    return listify(param)


def get_detail_params(detail_config, arguments):
    if callable(detail_config):
        result = listify(detail_config(arguments))
        return [listify(item) for item in result]
    return [resolve_expr_value(arguments, config) for config in detail_config]


def record_log(
        operation_config: OperationConfig,
        log_target_list,
        log_params,
        result,
        success: bool,
        argument_dict: dict,
        token: str,
        error_code: str
):
    username: str = get_username(token)
    user_id: str = get_user_id(token)
    fast_api_request: Request = argument_dict.get('fast_api_request')
    try:
        client = fast_api_request.headers.get('X-Forwarded-For').split(',')[0]
    except:
        client = fast_api_request.client.host
    for i in range(len(log_target_list)):
        items = [username, client]
        for j in range(len(log_params)):
            param = log_params[j]
            item = param[i] if len(param) > i else None
            item = item if item else '--'
            items.append(item)
        log_target: str = log_target_list[i].lower()
        operation_req = SendEventReq(
            userId=user_id,
            eventId=operation_config.name,
            sourceId=operation_config.name,
            sourceType=f"operation_target_{log_target}_label",
            eventLevel=LogRank.INFO.value,
            moName=operation_config.name,
            eventParam=items,
            moIP=client,
            eventTime=int(time.time()),
            eventSequence=time.time_ns(),
            dmeToken=fast_api_request.headers.get('Dme-X-Auth-Token'),
            hcsToken=fast_api_request.headers.get('HCS-X-AUTH-TOKEN')
        )
        if isinstance(result, BatchOperationResult):
            operation_result = [result for result in result.results if result.id == log_target]
            if operation_result:
                operation_req.isSuccess = operation_result[0].errorCode != 0
            else:
                operation_req.isSuccess = success
        else:
            operation_req.isSuccess = success
        if error_code:
            operation_req.legoErrorCode = error_code
        EventClient.send_event(operation_req)


# 存储类型map key设备类型关键词(resource.extend_info.sub_type)， value规范存储设备类型输出
storage_type = {"Pacific": "OceanStor Pacific", "Dorado": "OceanStor Dorado", "OceanProtect": "OceanProtect"}


def convert_storage_type(original: str):
    for key, val in storage_type.items():
        if key in original:
            return val
    return original


def trans_array_params(log_params: list):
    res = []
    for item_list in log_params:
        if isinstance(item_list, list) and len(item_list) > 0:
            val = item_list[len(item_list) - 1]
            if isinstance(val, str) and val == "cyber-array-true":
                list_array = [listify(item) for item in item_list]
                list_array.pop()
                res.extend(list_array)
            else:
                res.append(item_list)
        else:
            res.append(item_list)
    return res


def call_origin_method_with_record_log(func, token_data,
                                       operation_config: OperationConfig, *args,
                                       **kwargs):
    right_control = getattr(thread_local, 'right_control', False)
    if not right_control and not token_data:
        thread_local.operation_config = operation_config
        try:
            return func(*args, **kwargs)
        finally:
            del thread_local.operation_config

    arguments = {**kwargs}
    for i in args:
        arguments[i] = args[i]
    log_targets = listify(
        resolve_expr_value(arguments, operation_config.target))
    log_params = get_detail_params(operation_config.detail, arguments)
    # 处理operation_config.detail中callable返回数组，数组最后标记位cyber-array-true,则返回的数组拆分为多个数组
    log_params = trans_array_params(log_params)
    token_data = token_data or right_control
    try:
        result = func(*args, **kwargs)
        add_result_param(log_params, operation_config, result)
        record_log(operation_config, log_targets, log_params, result, True,
                   kwargs, token_data, None)
        return result
    except EmeiStorBizException as e:
        record_log(operation_config, log_targets, log_params, None, False,
                   kwargs, token_data, e.error_code)
        raise
    except IllegalParamException as e:
        record_log(operation_config, log_targets, log_params, None, False,
                   kwargs, token_data, e.error_code)
        raise
    except:
        record_log(operation_config, log_targets, log_params, None, False,
                   kwargs, token_data, CommonErrorCodes.SYSTEM_ERROR['code'])
        raise


def add_result_param(log_params, operation_config, result):
    for param in listify(operation_config.detail):
        if callable(param):
            pass
        elif param.startswith('#result'):
            if isinstance(result, list):
                res_param = str(result[0]) if len(result) > 0 else ""
            else:
                res_param = str(result)
            log_params.pop()
            log_params.append([res_param])

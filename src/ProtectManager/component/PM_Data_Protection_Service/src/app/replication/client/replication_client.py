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
from collections import defaultdict
from datetime import datetime
from http import HTTPStatus
from typing import List

from app.common import logger
from app.common.clients.client_util import ProtectionServiceHttpsClient, is_response_status_ok, parse_response_data, \
    SystemBaseHttpsClient
from app.common.enums.copy_enum import GenerationType
from app.common.enums.sla_enum import ReplicationTypeEnum, ReplicationModeEnum, PolicyActionEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.replication.schemas.replication_request import ReplicationRequest
from app.replication.service.check_policy import CheckPolicy

LOGGER = logger.get_logger(__name__)


class ReplicationClient(object):
    SUPPORT_REPLICATION_TYPE = [
        GenerationType.BY_BACKUP.value,
        GenerationType.BY_REPLICATED.value,
        GenerationType.BY_CASCADED_REPLICATION.value,
        GenerationType.BY_REVERSE_REPLICATION.value
    ]

    @staticmethod
    def query_copies_by_resource_id(resource_id: str, sla_target_mode: int = ReplicationModeEnum.EXTRA,
                                    action: str = PolicyActionEnum.replication):
        copies = []
        url = f"/v1/internal/copies"
        # source_copy_type： 1：完全备份 2：增量备份 3：差异备份 4：日志备份 5:永久增量备份 6:快照备份
        if action == PolicyActionEnum.replication:
            source_copy_type = [1, 2, 3, 5, 6]
        else:
            source_copy_type = [4]
        extra_conditions = {"resource_id": resource_id, "source_copy_type": source_copy_type}
        if sla_target_mode == ReplicationModeEnum.INTRA:
            extra_conditions.update({"generated_by": "Backup"})
        else:
            extra_conditions.update({"generated_by": ReplicationClient.SUPPORT_REPLICATION_TYPE})
        params = {
            "page_no": 0,
            "page_size": 200,
            "conditions": json.dumps(extra_conditions),
            "orders": "-display_timestamp"
        }
        while True:
            response = ProtectionServiceHttpsClient().request("GET", url, fields=params)
            LOGGER.info(f'invoke api to query backup copies number, request url: {url}, params:{params}')
            if not is_response_status_ok(response):
                raise EmeiStorBizException(CommonErrorCodes.SYSTEM_ERROR, message="query backup copies failed")
            data = parse_response_data(response.data)
            copies.extend(data.get("items"))
            total: int = data.get("total")
            if len(copies) == total:
                break
            params.update({"page_no": params["page_no"] + 1})
        return copies

    @staticmethod
    def filter_copies_generate_time(copies: List, start_replicate_time: datetime):
        filters_copies = []
        LOGGER.debug(f'Filter replicas based on replication start time, replication start time: {start_replicate_time},'
                     f'copy length:{len(copies)}')
        for copy in copies:
            generated_time: datetime = datetime.fromisoformat(copy.get("generated_time"))
            if generated_time >= start_replicate_time:
                filters_copies.append(copy)
            else:
                return filters_copies
        LOGGER.info(f'Filter replicas based on replication start time, filters_copies: {filters_copies}')
        return filters_copies

    @staticmethod
    def query_copy_statistic(resource_id: str, policy: dict):
        ext_parameters = policy.get("ext_parameters")
        action = policy.get("action", "replication")
        replication_target_type = ext_parameters.get("replication_target_type", ReplicationTypeEnum.ALL_COPY)
        replication_target_mode = ext_parameters.get("replication_target_mode", ReplicationModeEnum.EXTRA)
        LOGGER.info(f"[REPLICATION_TASK]replication_target_type:{replication_target_type}, "
                    f"replication_target_mode:{replication_target_mode}, resource_id:{resource_id}")
        copies = ReplicationClient.query_copies_by_resource_id(resource_id, replication_target_mode, action)
        LOGGER.info(f"[REPLICATION_TASK] query {len(copies)} copies by resource_id({resource_id})")
        copy_id_esn_map = defaultdict(list)
        copy_format_map = defaultdict(list)
        device_esn = set()
        unit_id = set()
        if ReplicationTypeEnum.SPECIFIED_COPY == replication_target_type:
            LOGGER.debug(f"[REPLICATION_TASK]Copy specified time copy")
            specified_scope = ext_parameters.get("specified_scope", [])
            check_policy = CheckPolicy(specified_scope)
            filter_copies = check_policy.filter_copies(copies)
        else:
            start_replicate_time = ext_parameters.get("start_replicate_time")
            start_replicate_time: datetime = datetime.fromisoformat(start_replicate_time)
            filter_copies = ReplicationClient.filter_copies_generate_time(copies, start_replicate_time)
        LOGGER.info(f"[REPLICATION_TASK] filters the number of copies that can be copied is {len(filter_copies)} ")
        if filter_copies:
            for copy in filter_copies:
                # 获取当前副本的设备 esn 和存储 unit_id
                device_esn.add(copy["device_esn"])
                unit_id.add(copy["storage_unit_id"])

                # 处理 copy_id_esn_map 映射
                copy_id_esn_map[copy["device_esn"]].append(copy["uuid"])

                # 提取并处理 copy_format_map 映射
                copy_format = json.loads(copy["properties"]).get("format", "0")
                copy_format_map[copy_format].append(copy["uuid"])
            return {
                'copy_format_map': dict(copy_format_map),
                'device_esn': list(device_esn),
                'unit_id': list(unit_id),
                'copy_id_esn_map': dict(copy_id_esn_map)
            }
        else:
            return {}

    @staticmethod
    def is_copy_replicated(copy_id, target_id):
        """
        查询指定副本是否已复制
        :param copy_id: 副本ID
        :param target_id: 复制集群
        :return: 是否已复制
        """
        url = f'/v1/internal/replication/check'
        params = {"copyId": copy_id, "targetId": target_id}
        LOGGER.info(f'Invoke api to check copy replication, request url:{url}, params:{params}')
        response = SystemBaseHttpsClient().request("GET", url, fields=params)
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="Invoke external system api failed or timeout.")
        return json.loads(response.data.decode('utf-8'))

    @staticmethod
    def notify_reverse_replication(msg):
        """
        给java端发送反向复制消息
        :param msg:反向复制消息
        :return:
        """
        url = f'/v1/internal/clusters/replication/reverse'
        LOGGER.info(f'Invoke api to dispatch reverse replication, request url:{url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(msg, default=lambda obj: obj.__dict__))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="Invoke external system api failed or timeout.")

    @staticmethod
    def notify_manual_replication(msg):
        """
        给java端发送手动复制消息
        :param msg:手动复制消息
        :return:
        """
        url = f'/v1/internal/clusters/replication/manual'
        LOGGER.info(f'Invoke api to dispatch manual replication, request url:{url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(msg, default=lambda obj: obj.__dict__))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="Invoke external system api failed or timeout.")

    @staticmethod
    def dispatch_schedule_replication(msg):
        """
        给java端发送周期复制消息
        :param msg:周期复制消息
        :return:
        """
        url = f'/v1/internal/clusters/replication/schedule'
        LOGGER.info(f'Invoke api to dispatch schedule replication, request url:{url}')
        response = SystemBaseHttpsClient().request("POST", url, body=json.dumps(msg, default=lambda obj: obj.__dict__))
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="Invoke external system api failed or timeout.")

    @staticmethod
    def check_before_manual_replicate(replication_req: ReplicationRequest, resource_id: str) -> any:
        url = f'/v1/internal/replication/check-manual-replication'
        target_req = {
            "storageType": replication_req.storage_type,
            "clusterId": replication_req.external_system_id,
            "storageId": replication_req.storage_id,
            "resourceId": resource_id
        }
        LOGGER.info(f'invoke api to check before manual replication, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url, fields=target_req)
        if response.status != HTTPStatus.OK:
            response_data = json.loads(response.data.decode('utf-8'))
            error_code = response_data.get("errorCode")
            error_message = response_data.get("errorMessage")
            error_parameters = response_data.get("parameters")
            error_dict = {
                "code": error_code,
                "message": error_message
            }
            LOGGER.error(
                f"invoke api to check before manual replication. Status: {response.status}, Data: {response.data}")
            raise EmeiStorBizException(error_dict,
                                       error_parameters,
                                       message="invoke policy api to check before manual replication failed")

    @staticmethod
    def is_hcs_service():
        url = f'/v1/internal/replication/is-op-service'
        LOGGER.info(f'invoke api to get is hcs service, request url is {url}')
        response = SystemBaseHttpsClient().request("GET", url)
        if response.status != HTTPStatus.OK:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR,
                                       error_message="Invoke external system api failed or timeout.")
        return json.loads(response.data.decode('utf-8'))

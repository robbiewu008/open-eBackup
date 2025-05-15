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
import time
from typing import List

from app.common import logger
from app.common.clients.alarm.alarm_client import AlarmClient
from app.common.clients.alarm.alarm_schemas import SendAlarmReq
from app.common.deploy_type import DeployType
from app.common.enums.alarm_enum import AlarmSourceType
from app.common.enums.resource_enum import ResourceTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.log.event_client import EventClient
from app.common.log.event_schemas import LogRank, SendEventReq
from app.copy_catalog.common.common import IndexStatus, BrowseMountStatus, CommonOperationID, \
    ResourceStatus, CopyFeatureEnum
from app.copy_catalog.common.copy_status import CopyStatus
from app.copy_catalog.models.tables_and_sessions import database, CopyTable, CopyProtectionTable
from app.copy_catalog.schemas import CopyStatusUpdate
from app.copy_catalog.service.anti_ransomware_service import refresh_resource_detect_table_with_lock
from app.copy_catalog.service.curd.copy_query_service import get_copy
from app.copy_catalog.util.copy_util import check_status
from app.copy_catalog.common.copy_status import CopyWormStatus

log = logger.get_logger(__name__)


def update_copy_index_status(copy_id: str, index_status: str, error_code: str):
    log.info(f"update copy indexed: copy id is {copy_id}, index_status is {index_status}，alarm_desc={error_code}.")
    if index_status not in list(e.value for e in IndexStatus):
        log.error(f"update copy indexed: unsupported index_status parameter value {index_status}.")
        error_message = f'unsupported index_status parameter value: {index_status}'
        raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, error_message=error_message)
    # 更新副本索引状态
    with database.session() as session:
        copy_obj = session.query(CopyTable).filter(CopyTable.uuid == copy_id).first()
        if not copy_obj:
            log.error(f"update copy indexed: copy with id {copy_id} does not exist.")
            error_message = f'copy with id {copy_id} does not exist'
            raise EmeiStorBizException(CommonErrorCodes.OBJ_NOT_EXIST, error_message=error_message)
        copy_obj.indexed = index_status
        session.merge(copy_obj)
    # 索引失败的副本发送运行事件
    try:
        if (index_status == IndexStatus.UNINDEXED.value and copy_obj.resource_type == ResourceTypeEnum.VM.value) \
                or index_status == IndexStatus.INDEX_FAIL.value:
            log.info(f"send copy unindex event req,error_code={error_code}.")
            resourece_name = copy_obj.resource_name
            generated_time = copy_obj.generated_time
            timestamp = int(time.time())
            if not copy_obj.user_id:
                copy_obj.user_id = ''
            if copy_obj.is_archived:
                AlarmClient.send_alarm(SendAlarmReq(
                    alarmId=CommonOperationID.ARCHIVE_COPIES_INDEX_FAILED.value,
                    userId=copy_obj.user_id,
                    params=resourece_name + "," + str(generated_time) + "," + error_code,
                    severity=LogRank.MAJOR.value,
                    createTime=timestamp,
                    sequence=timestamp,
                    sourceType=AlarmSourceType.COPY_CATALOG.value,
                ))
            else:
                EventClient.send_running_event(SendEventReq(
                    userId=copy_obj.user_id,
                    eventId=CommonOperationID.EVENT_CREATE_COPIES_INEDEX_FAILED.value,
                    eventParam=[resourece_name, str(generated_time), error_code],
                    eventTime=timestamp,
                    eventLevel=LogRank.MAJOR.value,
                    sourceId=CommonOperationID.EVENT_CREATE_COPIES_INEDEX_FAILED.value,
                    sourceType=AlarmSourceType.COPY_CATALOG.value,
                    eventSequence=timestamp,
                    isSuccess=False
                ))
    except Exception as _:
        log.exception(f"index failed: failed.copy_id={copy_id},error_code={error_code}")
    finally:
        pass
    log.info(f"update copy indexed: success.copy_id={copy_id},index_status={index_status}")


def update_resource_copy_index_status(resource_id: str, index_status: str, error_code: str):
    log.info(f"update resource copy: resource id {resource_id}, index_status {index_status}，alarm_desc={error_code}.")
    if index_status not in list(e.value for e in IndexStatus):
        log.error(f"update resource copy indexed: unsupported index_status parameter value {index_status}.")
        error_message = f'unsupported index_status parameter value: {index_status}'
        raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, error_message=error_message)
    # 更新资源副本索引状态
    with database.session() as session:
        session.query(CopyTable).filter(CopyTable.resource_id == resource_id).update({CopyTable.indexed: index_status})
    log.info(f"update resource copy index status success. resource_id={resource_id},index_status={index_status}")


def update_copy_list_index_status(copy_id_list: List[str], index_status: str, error_code: str):
    log.info(f"update resource copy: copy id list {copy_id_list}, index_status {index_status}，alarm_desc={error_code}.")
    if not copy_id_list:
        return
    if index_status not in list(e.value for e in IndexStatus):
        log.error(f"update resource copy indexed: unsupported index_status parameter value {index_status}.")
        error_message = f'unsupported index_status parameter value: {index_status}'
        raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, error_message=error_message)
    # 更新资源副本索引状态
    with database.session() as session:
        session.query(CopyTable).filter(CopyTable.uuid.in_(copy_id_list)).update({CopyTable.indexed: index_status})
    log.info(f"update resource copy index status success. copy_list={copy_id_list},index_status={index_status}")


def update_copy_browse_mount_status(copy_id: str, browse_mount_status: str, error_code: str):
    log.info(f"update copy browse mounted: copy_id={copy_id}, status={browse_mount_status}, alarm_desc={error_code}.")
    if browse_mount_status not in list(e.value for e in BrowseMountStatus):
        log.error(f"update copy browse mounted: unsupported browse_mount_status parameter value {browse_mount_status}.")
        error_message = f'unsupported browse_mount_status parameter value: {browse_mount_status}'
        raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, error_message=error_message)
    # 更新副本浏览挂载状态
    with database.session() as session:
        copy_obj = session.query(CopyTable).filter(CopyTable.uuid == copy_id).first()
        if not copy_obj:
            log.error(f"update copy browse mounted: copy with id {copy_id} does not exist.")
            error_message = f'copy with id {copy_id} does not exist'
            raise EmeiStorBizException(CommonErrorCodes.OBJ_NOT_EXIST, error_message=error_message)
        copy_obj.browse_mounted = browse_mount_status
        session.merge(copy_obj)
    log.info(f"update copy  browse mounted: success.copy_id={copy_id}, browse_mount_status={browse_mount_status}")


def update_copy_status_by_id(copy_id: str, param: CopyStatusUpdate):
    copy = get_copy(copy_id, True)
    if copy is None:
        return
    copy_status = param.status
    data = {CopyTable.status: copy_status.value}
    if param.deletable is not None:
        data[CopyTable.deletable] = param.deletable
    if param.display_timestamp is not None:
        data[CopyTable.display_timestamp] = param.display_timestamp
    if param.timestamp is not None:
        data[CopyTable.timestamp] = param.timestamp
    if param.is_archived is not None:
        data[CopyTable.is_archived] = param.is_archived
    if param.is_replicated is not None:
        data[CopyTable.is_replicated] = param.is_replicated
    if param.expiration_time is not None:
        data[CopyTable.expiration_time] = param.expiration_time
    if param.generated_time is not None:
        data[CopyTable.generated_time] = param.generated_time
    with database.session() as session:
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).update(data)


def update_copy_worm_status_by_id(copy_id: str, worm_status: int):
    log.info(f"Update copy: {copy_id} worm status to : {worm_status}")
    copy = get_copy(copy_id, True)
    if copy is None:
        log.info(f"Updated copy: {copy_id} is null.")
        return
    data = {CopyTable.worm_status: worm_status}
    # 如果worm未设置，那么就清除worm过期时间
    if worm_status == CopyWormStatus.UNSET.value:
        data[CopyTable.worm_expiration_time] = None
    with database.session() as session:
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).update(data)


def update_copy_status(copy_id: str, status: CopyStatus):
    with database.session() as session:
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).update({CopyTable.status: status.value})


def handle_resource_added(request_id: str, resource_id: str):
    log.info(f"handle resource added. request: {request_id}")
    with database.session() as session:
        session.query(CopyTable).filter(CopyTable.resource_id == resource_id).update({
            CopyTable.resource_status: ResourceStatus.EXIST.value
        })
        # 安全一体机部署场景 考虑到资源原本存在于环境上, 并且有副本存在的场景, 需要初始化资源防勒索检测表
        if DeployType().is_cyber_engine_deploy_type():
            refresh_resource_detect_table_with_lock(resource_id, True)


def deactivate_copy_protection(resource_ids: List[str]):
    with database.session() as session:
        session.query(CopyProtectionTable).filter(CopyProtectionTable.protected_resource_id.in_(resource_ids)) \
            .update({CopyProtectionTable.protected_status: False}, synchronize_session=False)


def update_copy_detail(copy_id: str, detail: str):
    with database.session() as session:
        copy = session.query(CopyTable).filter(CopyTable.uuid == copy_id).first()
        exists = bool(copy)
        check_status(exists, True, CommonErrorCodes.OBJ_NOT_EXIST, f"The copy '{copy_id}' is not exist")
        data = {CopyTable.detail: detail}
        session.query(CopyTable).filter(CopyTable.uuid == copy_id).update(data)


def resource_sla_name_of_copy_update_handle(request_id: str, old_sla, new_sla):
    """同步修改副本资源绑定的sla名称
    @param request_id:
    @param old_sla:原sla
    @param new_sla:修改后的sla
    @return:
    """
    with database.session() as session:
        old_sla = json.loads(old_sla)
        new_sla = json.loads(new_sla)
        old_sla_name = old_sla.get("name")
        new_sla_name = new_sla.get("name")
        log.info(f"resource_sla_name_of_copy_update_handle request_id={request_id} "
                 f"old_sla_name={old_sla_name}, new_sla_name={new_sla_name}")
        if old_sla_name == new_sla_name:
            return
        copies = session.query(CopyTable).filter(
            CopyTable.sla_properties.like(f'%"uuid": "{old_sla.get("uuid")}"%')).all()
        if len(copies) == 0:
            return
        for copy in copies:
            resource_properties = json.loads(copy.resource_properties)
            resource_properties["sla_name"] = new_sla_name
            copy.resource_properties = json.dumps(resource_properties)
            session.merge(copy)
        log.info(f"already change copies sla info request_id={request_id},sla_id={old_sla.get('uuid')}")


def revoke_copy_user_id(user_id: str):
    with database.session() as session:
        session.query(CopyTable).filter(
            CopyTable.user_id == user_id
        ).update({CopyTable.user_id: None})


def activated_copy_protection(resource_ids: List[str]):
    with database.session() as session:
        session.query(CopyProtectionTable).filter(CopyProtectionTable.protected_resource_id.in_(resource_ids)) \
            .update({CopyProtectionTable.protected_status: True}, synchronize_session=False)


def check_value_is_changed(key, value, extend_info):
    """
    判断扩展参数中key对应的value是否变化

    @param key 副本扩展参数中的key
    @param value 副本扩展参数对应的value
    @param extend_info 副本扩展参数
    """
    current_value = extend_info.get(key)
    if not current_value:
        return True
    return current_value != value


def update_copy_properties_by_key(copy_id: str, key: str, value):
    """
    根据副本中的key，更新副本扩展参数中的
    @param copy_id 副本id
    @param key 副本扩展参数中的key
    @param value 副本扩展参数对应的value
    """
    with database.session() as session:
        copy_info = session.query(CopyTable).filter(CopyTable.uuid == copy_id).first()
        if not copy_info:
            error_message = f'copy with id {copy_id} does not exist'
            log.error(error_message)
            raise EmeiStorBizException(CommonErrorCodes.OBJ_NOT_EXIST, error_message=error_message)
        extend_info = json.loads(copy_info.properties)
        if not check_value_is_changed(key, value, extend_info):
            # 值没有变化，不做修改，减少调用并支持幂等
            return
        extend_info[key] = value
        copy_info.properties = json.dumps(extend_info)
        session.merge(copy_info)


def update_copy_protection_sla_name_by_sla_id(request_id: str, old_sla, new_sla):
    """
    根据sla_id更新副本保护对象sla_name
    :param request_id: 请求id
    :param old_sla: 老的sla对象
    :param new_sla: 新的sla对象
    """
    old_sla = json.loads(old_sla)
    new_sla = json.loads(new_sla)
    old_sla_name = old_sla.get("name")
    new_sla_name = new_sla.get("name")
    sla_id = old_sla.get("uuid")
    log.info(
        f"update copy protection sla name:{old_sla_name} to: {new_sla_name} "
        f"by sla id:{sla_id}, request_id:{request_id}")
    with database.session() as session:
        session.query(CopyProtectionTable).filter(CopyProtectionTable.protected_sla_id == sla_id) \
            .update({CopyProtectionTable.protected_sla_name: new_sla_name}, synchronize_session=False)


def update_status_by_device_esn(device_esn: str, status: CopyStatus):
    """
    根据集群esn，更新集群中所有副本状态为无效
    @param device_esn: 副本所在集群esn
    @param status: 修改后副本状态
    """
    if device_esn is not None and status is not None:
        with database.session() as session:
            session.query(CopyTable).filter(CopyTable.device_esn == device_esn) \
                .update({CopyTable.status: status.value}, synchronize_session=False)


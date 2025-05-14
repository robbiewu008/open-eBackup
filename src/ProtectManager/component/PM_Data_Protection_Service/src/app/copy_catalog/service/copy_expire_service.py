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
import datetime
import json
import threading
import time

import app.copy_catalog.client.dme_client as dme_client
import app.copy_catalog.client.copy_client as copy_client

from app.common import logger
from app.common.clients.client_util import SystemBaseHttpsClient, is_response_status_ok
from app.common.deploy_type import DeployType
from app.common.enums.copy_enum import GenerationType
from app.common.enums.sla_enum import BackupTypeEnum, WormValidityTypeEnum
from app.common.enums.sla_enum import RetentionTypeEnum, time_interval_switcher
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.lock.lock import Lock
from app.common.lock.lock_manager import lock_manager
from app.common.redis.redis_lock import redis_lock
from app.common.redis_session import redis_session
from app.copy_catalog.common.constant import RedisKey, ExtendRetentionConstant
from app.copy_catalog.common.copy_status import CopyStatus, CopyWormStatus
from app.copy_catalog.common.copy_topics import CopyTopic
from app.copy_catalog.copy_error_code import CopyErrorCode
from app.copy_catalog.models.tables_and_sessions import CopyTable, CopyAntiRansomwareTable, database
from app.copy_catalog.schemas import CopyRetentionPolicySchema, CopyWormRetentionSettingSchema
from app.copy_catalog.service.anti_ransomware_service import is_generate_type_io_detect
from app.copy_catalog.service.cloud_backup_service import associated_deletion_copies
from app.copy_catalog.service.copy_delete_workflow import request_delete_copy, CopyDeleteParam
from app.copy_catalog.service.curd.copy_delete_service import resource_has_copy_link, \
    check_expire_copy_reach_to_delete_time, check_copy_anti_ransomware
from app.copy_catalog.service.curd.copy_query_service import get_deleting_copy
from app.copy_catalog.service.redis_service import COPY_CACHE_LOCK, cache_recent_expiring_copy, delete_cached_copy, \
    update_copy_cache
from app.copy_catalog.util.copy_util import get_future_time, get_valid_retention_info, check_status, is_worm_copy, \
    get_boolean_value_from_config_by_key
from app.resource.common.constants import CopyExpireConstants

log = logger.get_logger(__name__)


def is_expired_switch_open():
    return get_boolean_value_from_config_by_key('copy_expire_switch')


def handle_expired_copy():
    log.info("Before cache_recent_expiring_copies.")
    if is_expired_switch_open():
        log.info("Expire copy switch open, skip copy expire.")
        return
    try:
        cache_recent_expiring_copies()
    except BaseException:
        log.exception("Cache_recent_expiring_copies fail.")
    log.info("Before init_schedule.")
    init_schedule()
    while True:
        try:
            time_zone = None
            now = datetime.datetime.now(tz=time_zone).timestamp()
            items = redis_session.zrange(RedisKey.COPY_CACHE_EXPIRE, 0, 0, withscores=True)
            if not items or items[0][1] > now:
                time.sleep(.01)
                continue
            item = items[0]
            copy_id = item[0]
            delete_cached_expired_copy(copy_id)
        except BaseException as _:
            log.exception(f"auto clean copy failed.")
            time.sleep(1)
        finally:
            pass


def init_schedule():
    schedule_name = "copy-retention-message-dispatch-schedule"
    while True:
        try:
            data = {
                "schedule_type": "interval",
                "schedule_name": schedule_name,
                "action": CopyTopic.COPY_RETENTION_TOPIC,
                "replace_existing": False,
                "interval": "1h"
            }
            response = SystemBaseHttpsClient().request(
                "POST", f"/v1/schedules", body=json.dumps(data))
            if is_response_status_ok(response):
                log.info(f"initialize schedule '{schedule_name}' success.")
                break
            log.error(f"init_schedule failed. response: {response}")
            time.sleep(3)
        except BaseException:
            log.exception(f"create schedule '{schedule_name}' failed")
            time.sleep(1)
        finally:
            pass


@redis_lock(COPY_CACHE_LOCK)
def cache_recent_expiring_copies(*conditions):
    log.info(f"Caching recent expiring copies ...")
    copies = query_recent_expiring_copies(*conditions)
    for copy in copies:
        cache_recent_expiring_copy(copy[0], copy[1].timestamp())
    log.info(f"Cache recent expiring copies finished, copies size: {len(copies)}.")
    del copies


def query_recent_expiring_copies(*conditions):
    time_limit = get_future_time(hours=3)
    conditions = conditions if conditions else []
    temporary = RetentionTypeEnum.temporary.value
    conditions.extend([CopyTable.expiration_time < time_limit,
                       CopyTable.retention_type == temporary,
                       CopyTable.status != CopyStatus.DELETING,
                       CopyTable.generated_by != GenerationType.BY_TAPE_ARCHIVE.value])
    with database.session() as session:
        rets = session.query(CopyTable.uuid, CopyTable.expiration_time).filter(*conditions).all()
        return rets


def delete_cached_expired_copy(copy_id):
    lock: Lock = lock_manager.get_lock(key=COPY_CACHE_LOCK)
    if lock.lock(timeout=CopyExpireConstants.LOCK_TIME_OUT,
                 blocking_timeout=CopyExpireConstants.LOCK_WAIT_TIME_OUT):
        try:
            _delete_cached_expired_copy(copy_id)
        finally:
            lock.unlock()


def _delete_cached_expired_copy(copy_id):
    copy = get_deleting_copy(copy_id, False)
    if copy is None:
        log.info(f"copy_id :{copy_id} copy is none, no need expire.")
        redis_session.zrem(RedisKey.COPY_CACHE_EXPIRE, copy_id)
        return
    time_zone = None
    now = datetime.datetime.now(tz=time_zone)
    if copy.expiration_time > now:
        log.info(f"copy_id :{copy_id} copy is not expire, skip. expire time is {copy.expiration_time}.")
        return
    copy_delete_param = CopyDeleteParam(user_id=copy.user_id, job_type="COPY_EXPIRE")
    if resource_has_copy_link(copy):
        associated_copies = associated_deletion_copies(copy)
        if not associated_copies:
            # 防止卡住
            log.info("associated copies is empty, remove redis")
            redis_session.zrem(RedisKey.COPY_CACHE_EXPIRE, copy_id)
            return
        for associated_copy in associated_copies:
            if associated_copy.expiration_time > now:
                # 关联副本如果存在大于当前时间则退出，直到最大的gn值副本时间过期才过期删除
                log.error(f"copy_id :{copy_id} copy is not expire, delete expire fail.")
                return
            # 副本链路中最大GN值的副本过期才删除整个副本链路
            del_cache_ret = delete_cached_copy(copy.uuid)
            if del_cache_ret:
                log.info(f"delete expire copy, copy_id: {copy_id}.")
                delete_expire_copy(copy, copy_delete_param)
    else:
        del_cache_ret = delete_cached_copy(copy_id)
        if del_cache_ret:
            log.info(f"delete expire copy, copy_id: {copy_id}.")
            delete_expire_copy(copy, copy_delete_param)


def handle_copy_check(copy_id: str):
    cache_recent_expiring_copies(CopyTable.uuid == copy_id)


def update_copy_retention(copy_id: str, retention_policy: CopyRetentionPolicySchema):
    with database.session() as session:
        query = session.query(CopyTable).filter(CopyTable.uuid == copy_id)
        copy: CopyTable = query.one_or_none()
        exists = bool(copy)
        check_status(exists, True, CommonErrorCodes.OBJ_NOT_EXIST, f"The copy '{copy_id}' is not exist")
        timestamp = copy.display_timestamp
        generated_time = copy.generated_time
        if copy.status == CopyStatus.DELETING.value:
            raise EmeiStorBizException(
                CopyErrorCode.ALREADY_IN_DELETING_OCEAN_CYBER if DeployType().is_cyber_engine_deploy_type() else \
                CopyErrorCode.ALREADY_IN_DELETING,
                message=f"copy(copy_id={copy_id},timestamp={timestamp},generate time={generated_time}) is deleting"
            )
        retention_info = get_valid_retention_info(retention_policy, generated_time if generated_time else timestamp,
                                                  copy.generated_by)
        duration_unit, expiration_time, retention_duration, temporary, worm_validity_type = retention_info
        # 以前该副本的保留类型
        raw_retention_policy = CopyRetentionPolicySchema(**{"retention_type": copy.retention_type,
                                                            "retention_duration": copy.retention_duration,
                                                            "duration_unit": copy.duration_unit})
        query.update({
            CopyTable.retention_type: retention_policy.retention_type.value,
            CopyTable.duration_unit: duration_unit,
            CopyTable.retention_duration: retention_duration,
            CopyTable.expiration_time: expiration_time
        }, synchronize_session=False)
        session.commit()
        session.close()
        # 防勒索部署形态下修改DM安全快照保留时间
        if copy.backup_type == BackupTypeEnum.snapshot or is_worm_copy(copy.as_dict()):
            # worm保留和副本保留需求后，只有安全一体机和防勒索场景才会在在更新副本保留时间时走进更新worm保留时间的逻辑
            # 其他部署形态修改到worm设置功能中去更新worm保留时间
            if DeployType().is_cyber_engine_deploy_type() and DeployType().is_hyper_detect_deploy_type():
                extend_retention = check_worm_copy_retention(raw_retention_policy, copy, retention_info)
                log.info(f"begin to update retention duration, copy_id:{copy_id}")
                retention_policy.retention_duration = retention_duration
                retention_policy.duration_unit = duration_unit
                update_dm_worm_copy_retention(session, copy_id, retention_policy, extend_retention)
        update_copy_cache(expiration_time, copy_id, temporary)


def update_dm_worm_copy_retention(session, copy_id, retention_policy: CopyRetentionPolicySchema, extend_retention):
    query = session.query(CopyAntiRansomwareTable).filter(CopyAntiRansomwareTable.copy_id == copy_id)
    copy_anti_ransomware: CopyAntiRansomwareTable = query.one_or_none()
    generate_type = "COPY_DETECT" if copy_anti_ransomware is None else copy_anti_ransomware.generate_type
    update_dm_copy_retention_params = [
        copy_id, retention_policy.retention_duration, retention_policy.duration_unit,
        retention_policy.retention_type, extend_retention,
        generate_type
    ]
    dme_client.update_dm_copy_retention(update_dm_copy_retention_params)


def check_worm_retention_with_copy_retention(copy: CopyTable, worm_retention_policy: CopyRetentionPolicySchema):
    # 永久保留
    if copy.retention_type == RetentionTypeEnum.permanent:
        return
    worm_duration = time_interval_switcher.get(worm_retention_policy.duration_unit, "d")(
        worm_retention_policy.retention_duration)
    copy_duration = time_interval_switcher.get(copy.duration_unit, "d")(copy.retention_duration)
    if copy_duration < worm_duration:
        log.error(f"Copy:{copy.uuid} worm expiration time can not greater than copy expiration time.")
        raise EmeiStorBizException(
            error=CommonErrorCodes.MODIFY_WORM_VALIDITY_TIME_EXCEEDS_COPY_RETENTION_TIME_ERROR,
            parameters=[],
            error_message="worm expiration time can not greater than copy expiration time."
        )


def update_worm_copy_setting(copy_id, worm_setting: CopyWormRetentionSettingSchema):
    with database.session() as session:
        query = session.query(CopyTable).filter(CopyTable.uuid == copy_id)
        copy: CopyTable = query.one_or_none()
        exists = bool(copy)
        check_status(exists, True, CommonErrorCodes.OBJ_NOT_EXIST, f"The copy '{copy_id}' is not exist")
        worm_retention_policy = build_worm_retention_policy(copy, worm_setting)
        timestamp = copy.display_timestamp
        generated_time = copy.generated_time
        worm_retention_info = get_valid_retention_info(worm_retention_policy,
                                                  generated_time if generated_time else timestamp, copy.generated_by)
        duration_unit, expiration_time, retention_duration, temporary, worm_validity_type = worm_retention_info

        if is_worm_copy(copy.as_dict()):
            # 以前该副本的worm保留类型
            raw_retention_policy = CopyRetentionPolicySchema(**{"retention_type": RetentionTypeEnum.temporary,
                                                                "retention_duration": copy.worm_retention_duration,
                                                                "duration_unit": copy.worm_duration_unit,
                                                                "worm_validity_type": worm_validity_type})
            # 校验最新worm保留是否大于以前worm保留时间
            check_worm_copy_retention(raw_retention_policy, copy, worm_retention_info)
            # 校验worm保留时间是否小于等于副本过期保留时间
            check_worm_retention_with_copy_retention(copy, worm_retention_policy)
            update_worm_copy_retention(session, copy, worm_retention_policy, worm_setting, expiration_time)
            # 修改前后时间一样时不调用dee更新worm
            if need_update_dm_worm_setting(raw_retention_policy, copy, worm_retention_info):
                update_dm_worm_copy_retention(session, copy.uuid, worm_retention_policy, "")
        else:
            if not worm_setting.convert_worm_switch:
                raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                           message="convert worm retention should true")
            # 校验worm保留时间是否小于等于副本过期保留时间
            check_worm_retention_with_copy_retention(copy, worm_retention_policy)
            update_worm_copy_retention(session, copy, worm_retention_policy, worm_setting, expiration_time)
            copy_client.create_copy_worm_task(copy_id, worm_retention_policy)


def build_worm_retention_policy(copy: CopyTable, worm_setting: CopyWormRetentionSettingSchema):
    if worm_setting.worm_validity_type.value == WormValidityTypeEnum.copy_retention_time_consistent.value:
        retention_policy = dict(retention_type=copy.retention_type, retention_duration=copy.retention_duration,
                                duration_unit=copy.duration_unit,
                                worm_validity_type=WormValidityTypeEnum.copy_retention_time_consistent.value)
        return CopyRetentionPolicySchema(**retention_policy)

    retention_policy = dict(retention_type=RetentionTypeEnum.temporary.value,
                            retention_duration=worm_setting.retention_duration,
                            duration_unit=worm_setting.duration_unit,
                            worm_validity_type=WormValidityTypeEnum.custom_retention_time.value)
    return CopyRetentionPolicySchema(**retention_policy)


def update_worm_copy_retention(session, copy, retention_policy: CopyRetentionPolicySchema,
                               worm_setting: CopyWormRetentionSettingSchema, expiration_time):
    log.info(f"update worm copy setting: {retention_policy}", )
    duration_unit = None
    if not (copy.retention_type == RetentionTypeEnum.permanent
            and worm_setting.worm_validity_type.value == WormValidityTypeEnum.copy_retention_time_consistent.value):
        duration_unit = retention_policy.duration_unit.value
    session.query(CopyTable).filter(CopyTable.uuid == copy.uuid).update({
        CopyTable.worm_validity_type: worm_setting.worm_validity_type.value,
        CopyTable.worm_retention_duration: retention_policy.retention_duration,
        CopyTable.worm_duration_unit: duration_unit,
        CopyTable.worm_expiration_time: expiration_time
    }, synchronize_session=False)
    session.commit()


def check_worm_copy_retention(raw_retention_policy: CopyRetentionPolicySchema, copy: CopyTable, retention_info):
    if not can_modify_retention(copy.as_dict()):
        raise EmeiStorBizException.build_from_error(CopyErrorCode.CAN_NOT_MODIFY_COPY_RETENTION)
    if not is_worm_copy(copy.as_dict()):
        log.info(f"Only worm copy need to check retention value. copy id: {copy.uuid}")
        return ExtendRetentionConstant.FOREVER
    duration_unit, expiration_time, retention_duration, temporary, worm_validity_type = retention_info
    # 如果需要把副本改为永久保留，由于修改后时间必然大于等于原保留时间，直接跳过检测，
    if not temporary:
        log.info(f"Permanent retention is illegal. copy id: {copy.uuid}")
        return ExtendRetentionConstant.FOREVER
    if copy.retention_type == RetentionTypeEnum.permanent.value and copy.worm_duration_unit is None:
        raise_worm_copy_or_security_snapshot_retention_fail_exception()
    new_duration = time_interval_switcher[duration_unit](retention_duration)
    if raw_retention_policy.duration_unit is not None:
        raw_duration = time_interval_switcher[raw_retention_policy.duration_unit](\
            raw_retention_policy.retention_duration)
    else:
        # 修改前worm保留策略是永久保留时，duration_unit为None
        raw_duration = None
    if raw_duration is None or new_duration < raw_duration:
        log.info(f"The retention of worm copy:{copy.uuid}  only can be increased.")
        # 只能增加防篡改副本的保留时间
        raise_worm_copy_or_security_snapshot_retention_fail_exception()
    return new_duration - raw_duration


def need_update_dm_worm_setting(raw_retention_policy: CopyRetentionPolicySchema, copy: CopyTable, retention_info):
    # 非worm副本不调dee
    if not is_worm_copy(copy.as_dict()):
        return False
    duration_unit, expiration_time, retention_duration, temporary, worm_validity_type = retention_info
    if not temporary:
        return True
    new_duration = time_interval_switcher[duration_unit](retention_duration)
    if raw_retention_policy.duration_unit is not None:
        raw_duration = time_interval_switcher[raw_retention_policy.duration_unit](
            raw_retention_policy.retention_duration)
    else:
        # 修改前后时间一致时，不调用dee接口更新worm
        raw_duration = None
    if new_duration == raw_duration:
        return False
    else:
        return True


def can_modify_retention(copy: dict):
    return copy.get("worm_status") != CopyWormStatus.SETTING.value


def delete_expire_copy(copy: CopyTable, copy_delete_param: CopyDeleteParam):
    if not check_expire_copy_reach_to_delete_time(copy):
        log.info(f"copy_id:{copy.uuid} not reach execute expire job time after lengthen.")
        return
    if check_copy_anti_ransomware(copy):
        log.info(f"copy_id:{copy.uuid} only one uninfected.")
        return
    # 如果是事中快照则不删除
    if is_generate_type_io_detect(copy.uuid):
        log.info(f"copy_id:{copy.uuid} generate type is IO_DETECT.")
        return
    request_delete_copy(copy, copy_delete_param)


def raise_worm_copy_or_security_snapshot_retention_fail_exception():
    if DeployType().is_cyber_engine_deploy_type():
        raise EmeiStorBizException.build_from_error(CopyErrorCode.MODIFY_SECURITY_SNAP_RETENTION_FAIL)
    else:
        raise EmeiStorBizException.build_from_error(CopyErrorCode.MODIFY_WORM_COPY_RETENTION_FAIL)


class CopyExpireThreadWrap:
    def __init__(self):
        self.copy_expire_thread = None

    def start_copy_expire_thread(self):
        log.info("Start copy expire thread.")
        self.copy_expire_thread = threading.Thread(target=handle_expired_copy, name=f"expired-copy-handle-thread")
        self.copy_expire_thread.start()

    def check_copy_expire_thread(self):
        if not self.copy_expire_thread.is_alive():
            log.info("Copy expire thread is not alive.")
            self.start_copy_expire_thread()


copy_expire_thread = CopyExpireThreadWrap()



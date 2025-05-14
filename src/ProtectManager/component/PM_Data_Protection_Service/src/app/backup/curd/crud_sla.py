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
import math
import uuid
from datetime import datetime, date
from itertools import groupby
from typing import List, Optional

from sqlalchemy import desc
from sqlalchemy import or_
from sqlalchemy.orm import Session

from app.backup.curd.base import CRUDBase
from app.backup.models.sla_table import SlaModel, PolicyModel, RetentionModel, ScheduleModel
from app.backup.schemas.policy import Policy
from app.backup.schemas.sla import SlaCreate, SlaUpdate, SlaPageRequest
from app.common import logger
from app.common.constants.constant import DBRetryConstants
from app.common.enums.sla_enum import PolicyTypeEnum, TriggerEnum, TriggerActionEnum
from app.common.exception.unified_exception import DBRetryException
from app.common.util.retry.retryer import retry

logger = logger.get_logger(__name__)

__all__ = ["sla"]


def modify_schedule_start_time(policy: Policy):
    # 根据类型和前端传入时间，修正首次执行时间
    start_time = policy.schedule.start_time
    if policy.schedule.trigger == TriggerEnum.interval.value and policy.schedule.window_start:
        start_time = start_time if isinstance(start_time, date) else start_time.date()
        start_time = datetime.combine(start_time, policy.schedule.window_start).isoformat()
    if policy.schedule.trigger == TriggerEnum.customize_interval.value and \
            policy.schedule.trigger_action == TriggerActionEnum.year.value:
        start_time = datetime.combine(policy.schedule.days_of_year,
                                      policy.schedule.window_start).isoformat()
    return start_time


def build_policy_model(sla_id, policy, sla_model) -> PolicyModel:
    if not policy.uuid:
        policy.uuid = uuid.uuid4()
    policy_model = PolicyModel(uuid=str(policy.uuid), sla_id=sla_id, name=policy.name,
                               type=policy.type,
                               action=policy.action, ext_parameters=policy.ext_parameters.json())
    retention_model = RetentionModel(
        policy_id=str(policy.uuid),
        retention_type=policy.retention.retention_type,
        retention_duration=policy.retention.retention_duration,
        duration_unit=policy.retention.duration_unit
    )
    policy_model.retention = retention_model
    start_time = modify_schedule_start_time(policy)
    schedule_model = ScheduleModel(
        policy_id=str(policy.uuid),
        trigger=policy.schedule.trigger,
        interval=policy.schedule.interval,
        interval_unit=policy.schedule.interval_unit,
        start_time=start_time,
        window_start=policy.schedule.window_start,
        window_end=policy.schedule.window_end,
        days_of_week=policy.schedule.days_of_week,
        days_of_month=policy.schedule.days_of_month,
        days_of_year=policy.schedule.days_of_year,
        trigger_action=policy.schedule.trigger_action
    )
    policy_model.schedule = schedule_model
    return policy_model


def search_rank(items: List, name: str) -> Optional[SlaModel]:
    for item in items:
        if item.name == name:
            items.remove(item)
            return item
    return None


class CrudSla(CRUDBase[SlaModel, SlaCreate, SlaUpdate]):

    @retry(exceptions=DBRetryException, tries=DBRetryConstants.RETRY_TIMES, wait=DBRetryConstants.WAIT_TIME,
           backoff=DBRetryConstants.BACKOFF, logger=logger)
    def check_sla_name(self, db: Session, sla_name: str, sla_id: uuid.UUID) -> bool:
        if sla_id is None:
            return db.query(self.model).filter(SlaModel.name == sla_name).count()
        else:
            return db.query(self.model).filter(SlaModel.name == sla_name).filter(SlaModel.uuid != str(sla_id)).count()

    @retry(exceptions=DBRetryException, tries=DBRetryConstants.RETRY_TIMES, wait=DBRetryConstants.WAIT_TIME,
           backoff=DBRetryConstants.BACKOFF, logger=logger)
    def query_sla_count(self, db: Session) -> int:
        return db.query(self.model).count()

    @retry(exceptions=DBRetryException, tries=DBRetryConstants.RETRY_TIMES, wait=DBRetryConstants.WAIT_TIME,
           backoff=DBRetryConstants.BACKOFF, logger=logger)
    def create(self, db: Session, obj_in: SlaCreate) -> uuid.UUID:
        """
        创建SLA
        :param db: 数据库连接会话
        :param obj_in: 待创建SLA
        :return: 创建成功的SLA编号
        """
        sla_id = uuid.uuid4()
        sla_model = SlaModel(uuid=str(sla_id), name=obj_in.name, type=obj_in.type, application=obj_in.application.value,
                             user_id=obj_in.user_id, is_global=False)
        db.add(sla_model)
        policy_list = obj_in.policy_list
        for policy in policy_list:
            policy_id = uuid.uuid4()
            policy_model = PolicyModel(uuid=str(policy_id), sla_id=str(sla_id), name=policy.name, type=policy.type,
                                       action=policy.action, ext_parameters=policy.ext_parameters.json())
            db.add(policy_model)
            retention_model = RetentionModel(
                policy_id=str(policy_id),
                retention_type=policy.retention.retention_type,
                retention_duration=policy.retention.retention_duration,
                duration_unit=policy.retention.duration_unit
            )
            db.add(retention_model)
            start_time = modify_schedule_start_time(policy)
            schedule_model = ScheduleModel(
                policy_id=str(policy_id),
                trigger=policy.schedule.trigger,
                interval=policy.schedule.interval,
                interval_unit=policy.schedule.interval_unit,
                start_time=start_time,
                window_start=policy.schedule.window_start,
                window_end=policy.schedule.window_end,
                days_of_week=policy.schedule.days_of_week,
                days_of_month=policy.schedule.days_of_month,
                days_of_year=policy.schedule.days_of_year,
                trigger_action=policy.schedule.trigger_action
            )
            db.add(schedule_model)
        return sla_id

    @retry(exceptions=DBRetryException, tries=DBRetryConstants.RETRY_TIMES, wait=DBRetryConstants.WAIT_TIME,
           backoff=DBRetryConstants.BACKOFF, logger=logger)
    def update(self, db: Session, obj_in: SlaUpdate) -> uuid.UUID:
        logger.info(f'update sla, sla_id:{str(obj_in.uuid)}')
        obj = db.query(self.model).get(str(obj_in.uuid))
        if obj is not None:
            db.delete(obj)
        sla_model = SlaModel(uuid=str(obj_in.uuid), name=obj_in.name, type=obj_in.type, application=obj_in.application,
                             user_id=obj_in.user_id, is_global=False)
        policy_list = [build_policy_model(
            str(obj_in.uuid), policy, sla_model) for policy in obj_in.policy_list]
        sla_model.policy_list = policy_list
        db.add(sla_model)
        return obj_in.uuid

    @retry(exceptions=DBRetryException, tries=DBRetryConstants.RETRY_TIMES, wait=DBRetryConstants.WAIT_TIME,
           backoff=DBRetryConstants.BACKOFF, logger=logger)
    def get(self, db: Session, sla_id: str, user_id=None) -> SlaModel:
        query = db.query(self.model).distinct(self.model.uuid)
        if sla_id:
            query = query.filter(self.model.uuid == sla_id)
        if user_id:
            query = query.filter(self.model.user_id == user_id)
        return query.first()

    @retry(exceptions=DBRetryException, tries=DBRetryConstants.RETRY_TIMES, wait=DBRetryConstants.WAIT_TIME,
           backoff=DBRetryConstants.BACKOFF, logger=logger)
    def get_slas(self, db: Session, user_id: str, sla_id_list: List[str]) -> bool:
        global_default = True
        query = db.query(self.model.uuid).filter(
            self.model.uuid.in_(sla_id_list))
        query = query.filter(or_(self.model.user_id == user_id, self.model.is_global == global_default))
        count = query.count()
        return count == len(sla_id_list)

    @retry(exceptions=DBRetryException, tries=DBRetryConstants.RETRY_TIMES, wait=DBRetryConstants.WAIT_TIME,
           backoff=DBRetryConstants.BACKOFF, logger=logger)
    def page_query(
            self, db: Session, *, page_req: SlaPageRequest
    ) -> dict:
        from app.protection.object.service.projected_object_service import ProtectedObjectService
        query = db.query(self.model)
        global_default = True
        if page_req.user_id:
            query = query.filter(
                or_(self.model.user_id == page_req.user_id, self.model.is_global == global_default))
        if page_req.name:
            query = query.filter(self.model.name.ilike(
                f'%{page_req.name.strip()}%'))
        if page_req.types:
            type_list = [sla_type.value for sla_type in page_req.types]
            query = query.filter(self.model.type.in_(type_list))
        if page_req.applications:
            applications_list = [
                application.value for application in page_req.applications]
            query = query.filter(self.model.application.in_(applications_list))
        if page_req.actions:
            action_list = [action.value for action in page_req.actions]
            query = query.join(PolicyModel, self.model.uuid == PolicyModel.sla_id) \
                .filter(PolicyModel.action.in_(action_list))
        count = query.count()
        items = query.order_by(desc(self.model.created_time)).offset(page_req.page_no * page_req.page_size).limit(
            page_req.page_size).all()
        for item in items:
            item.__setattr__("archival_count", 0)
            item.__setattr__("replication_count", 0)
            item.__setattr__("resource_count", 0)
            item.policy_list.sort(key=lambda x: x.type)
            for policy_type, policies in groupby(item.policy_list, key=lambda x: x.type):
                if PolicyTypeEnum[policy_type] == PolicyTypeEnum.archiving:
                    item.__setattr__("archival_count", len(list(policies)))
                if PolicyTypeEnum[policy_type] == PolicyTypeEnum.replication:
                    item.__setattr__("replication_count", len(list(policies)))
            item.__setattr__(
                "resource_count", ProtectedObjectService.count_by_sla_id(db, str(item.uuid), page_req.user_id))
        return {
            "total": count,
            "pages": math.ceil(count / page_req.page_size),
            "page_size": page_req.page_size,
            "page_no": page_req.page_no,
            "items": items
        }

    @retry(exceptions=DBRetryException, tries=DBRetryConstants.RETRY_TIMES, wait=DBRetryConstants.WAIT_TIME,
           backoff=DBRetryConstants.BACKOFF, logger=logger)
    def get_policy(self, db: Session, policy_id: str) -> PolicyModel:
        query = db.query(PolicyModel).distinct(PolicyModel.uuid)
        if policy_id:
            query = query.filter(PolicyModel.uuid == policy_id)
        return query.first()


sla = CrudSla(SlaModel)

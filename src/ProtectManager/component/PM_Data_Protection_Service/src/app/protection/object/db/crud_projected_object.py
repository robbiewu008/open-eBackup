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
from typing import Type, List

import sqlalchemy_utils
from sqlalchemy import func
from sqlalchemy.orm import Session
from sqlalchemy.orm.util import aliased

from app.base.db_base import database
from app.common.enums.rbac_enum import ResourceSetTypeEnum
from app.common.logger import get_logger
from app.protection.object.models.projected_object import ProtectedObject
from app.protection.object.schemas.protected_copy_object import SlaResourceQuantityRelationship
from app.protection.object.schemas.protected_object import ProtectedObjectCreate, ModifyProtectionSubmitReq, \
    ProtectedObjectQueryRequest
from app.resource.models.rbac_models import DomainResourceObjectTable
from app.resource.models.resource_models import EnvironmentTable, ResourceTable

log = get_logger(__name__)


class CRUDProtectedObject(object):

    def __init__(self, model: Type[ProtectedObject]):
        self.model = model

    def batch_delete(self, db: Session, obj_id_list: List):
        obj_list = db.query(self.model).filter(
            self.model.uuid.in_(obj_id_list)).all()
        for obj in obj_list:
            db.delete(obj)
        db.flush()

    def delete_by_condition(self, db: Session, conditions: list):
        obj_list = db.query(self.model).filter(*conditions).all()
        for obj in obj_list:
            db.delete(obj)
        db.flush()

    def create(self, db: Session, obj_in: ProtectedObjectCreate) -> ProtectedObject:
        db_obj = self.model(
            **obj_in.dict(), uuid=str(uuid.uuid4()), status=1)
        db.add(db_obj)
        return db_obj

    def page_query(
            self, db: Session, page_req: ProtectedObjectQueryRequest
    ) -> dict:
        environment = aliased(EnvironmentTable)
        query = db.query(*sqlalchemy_utils.get_columns(self.model).values(), ResourceTable, environment) \
            .join((ResourceTable, self.model.resource_id == ResourceTable.uuid),
                  (environment, ResourceTable.root_uuid == environment.uuid))
        if page_req.domain_id:
            sub_query = db.query(DomainResourceObjectTable.resource_object_id).filter(
                DomainResourceObjectTable.domain_id == page_req.domain_id).filter(
                DomainResourceObjectTable.type == ResourceSetTypeEnum.RESOURCE.value).subquery()
            query.filter(environment.uuid.in_(sub_query))
        if page_req.sla_id:
            query = query.filter(self.model.sla_id == str(page_req.sla_id))
        if page_req.name:
            query = query.filter(self.model.name.ilike(
                f"%{page_req.name.strip()}%"))
        if page_req.sub_type:
            type_list = list(sla_type.value for sla_type in page_req.sub_type)
            query = query.filter(self.model.sub_type.in_(type_list))
        if page_req.path:
            query = query.filter(ResourceTable.path.ilike(
                f"%{page_req.path.strip()}%"))
        if page_req.sla_compliance:
            query = query.filter(
                self.model.sla_compliance.in_(page_req.sla_compliance))
        count = query.count()

        if count == 0 or page_req.page_size <= 0 or page_req.page_no < 0:
            return {
                "total": count,
                "pages": 0,
                "page_size": page_req.page_size,
                "page_no": page_req.page_no,
                "items": []
            }
        results = query.offset(page_req.page_no * page_req.page_size).limit(
            page_req.page_size).all()
        items = []
        for result in results:
            resource_id = result.resource_id
            resource_query = db.query(ResourceTable.name).filter(ResourceTable.uuid == str(resource_id)).first()
            result = result._asdict()
            result["name"] = str(resource_query.name)
            items.append(result)
        return {
            "total": count,
            "pages": math.ceil(count / page_req.page_size),
            "page_size": page_req.page_size,
            "page_no": page_req.page_no,
            "items": items
        }

    def update_ext_parameters(self, db: Session,
                              update_req: ModifyProtectionSubmitReq):
        return db.query(self.model).filter(ProtectedObject.resource_id == str(update_req.resource_id)).update(
            {ProtectedObject.ext_parameters: update_req.ext_parameters}
        )

    def update_ext_parameters_by_resource_id(self, db: Session,
                                             resource_id: str,
                                             ext_parameters: str):
        return db.query(self.model).filter(ProtectedObject.resource_id == resource_id).update(
            {ProtectedObject.ext_parameters: ext_parameters}
        )

    def query_by_resource_ids(self, db: Session, resource_ids: List[str]) -> List[ProtectedObject]:
        return db.query(self.model).filter(ProtectedObject.resource_id.in_(resource_ids)).all()

    def query_one_by_resource_id(self, db: Session, resource_id: str) -> ProtectedObject:
        return db.query(self.model).filter(ProtectedObject.resource_id == resource_id).first()

    def query_by_env_id(self, db: Session, env_id: str) -> ProtectedObject:
        return db.query(self.model).filter(ProtectedObject.resource_id == str(env_id)).first()

    def query_by_sla_id(self, db: Session, sla_id: str, domain_id: str = None) -> List[ProtectedObject]:
        environment = aliased(EnvironmentTable)
        query = db.query(self.model, environment).join((ResourceTable, self.model.resource_id == ResourceTable.uuid),
                                                       (environment, ResourceTable.root_uuid == environment.uuid))
        if domain_id:
            sub_query = db.query(DomainResourceObjectTable.resource_object_id).filter(
                DomainResourceObjectTable.domain_id == domain_id).filter(
                DomainResourceObjectTable.type.in_([ResourceSetTypeEnum.RESOURCE.value,
                                                    ResourceSetTypeEnum.AGENT.value])).subquery()
            query = query.filter(environment.uuid.in_(sub_query))
        if sla_id:
            query = query.filter(self.model.sla_id == str(sla_id))
        return query.all()

    def query_obj_by_sla_id(self, db: Session, sla_id: str) -> List[ProtectedObject]:
        return db.query(self.model).filter(ProtectedObject.sla_id == str(sla_id)).all()

    def update_status(self, db: Session, resource_ids: List[str], status: int):
        query = db.query(self.model).filter(
            ProtectedObject.resource_id.in_(resource_ids))
        query.update({ProtectedObject.status: status},
                     synchronize_session='fetch')

    def check_status(self, db: Session, resource_ids: List[str], status: int) -> bool:
        status_count = db.query(self.model).filter(ProtectedObject.resource_id.in_(resource_ids)).filter(
            ProtectedObject.status == status).count()
        return len(resource_ids) == status_count

    def update_by_params(self, db: Session, resource_id: str, update_conditions: dict) -> ProtectedObject:
        query = db.query(self.model).filter(
            ProtectedObject.resource_id == resource_id)
        query.update(update_conditions)
        return query.first()

    def update_multi_by_params(self, db: Session, resource_ids: List,
                               update_conditions: dict) -> List[ProtectedObject]:
        query = db.query(self.model).filter(
            ProtectedObject.resource_id.in_(resource_ids))
        query.update(update_conditions, synchronize_session='fetch')
        return query.all()

    def query_multi_by_params(self, db: Session, conditions: list) -> List[ProtectedObject]:
        return db.query(self.model).filter(*conditions).all()

    def query_first_by_env_id(self, env_id: str) -> ProtectedObject:
        """
        根据受保护环境的uuid查询该环境下的受保护对象
        :param db: 数据库
        :param env_id: 受保护环境的uuid
        :return: 返回第一个
        """
        with database.session() as db:
            return db.query(self.model).filter(ProtectedObject.env_id == str(env_id)).first()

    def query_projected_object_by_user_id(self, db: Session,
                                          domain_id: str = None) -> List[SlaResourceQuantityRelationship]:
        environment = aliased(EnvironmentTable)
        query = db.query(ProtectedObject.sla_id, func.count('*')).join(
            (ResourceTable, self.model.resource_id == ResourceTable.uuid),
            (environment, ResourceTable.root_uuid == environment.uuid))
        sub_query = db.query(DomainResourceObjectTable.resource_object_id).filter(
            DomainResourceObjectTable.domain_id == domain_id).filter(
            DomainResourceObjectTable.type.in_([ResourceSetTypeEnum.RESOURCE.value,
                                                ResourceSetTypeEnum.AGENT.value])).subquery()
        query = query.filter(environment.uuid.in_(sub_query))
        projected_object_info = query.group_by(ProtectedObject.sla_id).all()
        sla_resource_quantity_relationship_list = \
            [SlaResourceQuantityRelationship(sla_id=item[0], resource_count=item[1]) for item in projected_object_info]
        return sla_resource_quantity_relationship_list


projected_object = CRUDProtectedObject(ProtectedObject)

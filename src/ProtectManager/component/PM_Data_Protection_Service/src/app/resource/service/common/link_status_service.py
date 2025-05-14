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
import uuid
from app.base.db_base import database
from app.common.enums.resource_enum import ResourceSubTypeEnum, LinkStatusEnum
from app.resource.schemas.link_status import LinkStateUpdate, LinkStateCreate
from app.resource.models.resource_models import LinkStatusTable
from app.resource.service.common import resource_service
from app.common import logger
log = logger.get_logger(__name__)

QUERY_LINK_STATUS_LIMIT_SECONDS = 600


def query_list(source_role: str,
               source_addr: str,
               destination_role: str,
               destination_addr: str,
               is_internal: bool):
    # 查询链路状态信息操作，支持参数过滤
    now_time = int(time.time())
    with database.session() as session:
        query = session.query(LinkStatusTable)
        if source_role:
            query = query.filter(LinkStatusTable.source_role == source_role)
        if source_addr:
            query = query.filter(LinkStatusTable.source_addr == source_addr)
        if destination_role:
            query = query.filter(LinkStatusTable.dest_role == destination_role)
        if destination_addr:
            query = query.filter(LinkStatusTable.dest_addr == destination_addr)
        if not is_internal:
            query = query.filter(LinkStatusTable.update_time >=
                                 now_time-QUERY_LINK_STATUS_LIMIT_SECONDS)
        items = query.all()
        return items


def get_link_state(source_role: str,
                   source_addr: str,
                   destination_role: str,
                   destination_addr: str):
    # 获取链路状态信息，并对字段进行转化
    items = query_list(source_role=source_role,
                       source_addr=source_addr,
                       destination_role=destination_role,
                       destination_addr=destination_addr,
                       is_internal=False)
    # 过滤离线vm agent
    if source_role == ResourceSubTypeEnum.VMBackupAgent.value:
        items = is_vm_offline_agent(items)
    return list(link_table_to_link_state(item) for item in items)


def is_vm_offline_agent(items):
    query_expression = {
        'sub_type': ResourceSubTypeEnum.VMBackupAgent.value,
        'link_status': LinkStatusEnum.Online.value
    }
    vm_agents = resource_service.query_environment(query_expression)
    online_agent_ips = [i.get('endpoint') for i in vm_agents]
    return [item for item in items if item.source_addr in online_agent_ips]


def link_table_to_link_state(links):
    # 将数据库表字段名进行一个修改（有更加简便的方法待定）
    return {
        "sourceRole": links.source_role,
        "sourceAddr": links.source_addr,
        "destRole": links.dest_role,
        "destAddr": links.dest_addr,
        "state": links.state,
        "updateTime": links.update_time
    }


def modify_link_state(update_req: LinkStateUpdate):
    # 查询链路状态信息
    item = query_list(source_role=update_req.source_role,
                      source_addr=update_req.source_addr,
                      destination_role=update_req.dest_role,
                      destination_addr=update_req.dest_addr,
                      is_internal=True)
    req = LinkStateCreate(
        source_role=update_req.source_role,
        source_addr=update_req.source_addr,
        dest_role=update_req.dest_role,
        dest_addr=update_req.dest_addr,
        state=update_req.state,
        update_time=int(time.time())
    )
    with database.session() as session:
        if item:
            # 存在 且state不同 可被更新
            query = session.query(LinkStatusTable).filter(
                LinkStatusTable.uuid == item[0].uuid)
            query.update({LinkStatusTable.state: req.state,
                          LinkStatusTable.update_time: req.update_time})
            return item[0].uuid
        else:
            # 对象不存在 重新创建
            db_obj = LinkStatusTable(uuid=str(uuid.uuid4()), **req.dict())
            session.add(db_obj)
            return db_obj.uuid

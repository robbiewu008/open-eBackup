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
import functools
import json
import threading
import time

from app.common.clients.dme_ubc_client import remove_log_repo_whitelist_of_resource
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.logger import get_logger
from app.protection.object.models.projected_object import ProtectedObject
from app.protection.object.schemas.agent_req_schema import CheckAppReq, Application, AppEnv
from app.resource.client.agent_client import remove_protect_unmount_repo
from app.resource.schemas.env_schemas import EnvironmentSchema
from app.resource.schemas.resource import EnvironmentResourceSchema
from app.resource.service.common import resource_service
from app.resource.service.common.resource_service import get_extend_info_by_resource_id

log = get_logger(__name__)


def not_handle_exception_decorator(func):
    @functools.wraps(func)
    def inner(*args, **kwargs):
        try:
            func(*args, **kwargs)
        except Exception:
            log.exception(f"Execute func: {func} exception.")

    return inner


@not_handle_exception_decorator
def handle_unmount_repo_for_sap_hana_db(projected_object: ProtectedObject):
    """
    对于SAP HANA数据库资源，如果实例开启了日志备份，移除保护时需卸载日志文件系统
    """
    if projected_object.sub_type != ResourceSubTypeEnum.SAPHANA_DATABASE.value:
        return
    resource_id = projected_object.resource_id
    db_resource: EnvironmentResourceSchema = resource_service.query_resource_by_id(resource_id)
    if not db_resource:
        log.warning("The sap hana database resource(uuid=%s) does not exist.", resource_id)
        return
    inst_env: EnvironmentSchema = resource_service.query_resource_by_id(db_resource.parent_uuid)
    if not inst_env:
        log.warning("The sap hana instance resource(uuid=%s) does not exist.", db_resource.parent_uuid)
        return
    inst_ext_info_dict = get_extend_info_by_resource_id(inst_env.uuid)
    enable_log_bak = inst_ext_info_dict.get("enableLogBackup")
    if str(enable_log_bak).lower() != "true":
        log.info(f"The instance of sap hana database(uuid=%s) disabled log backup, no need to unmount repository.",
                 resource_id)
        return
    log.info(f"The instance of sap hana database(uuid=%s) enabled log backup, need to unmount repository.",
             resource_id)
    db_ext_info_dict = get_extend_info_by_resource_id(db_resource.uuid)
    try:
        thread = threading.Thread(
            target=execute_unmount_repo_task,
            name=f"unmount_repo_{resource_id}_{int(time.time())}",
            args=(db_resource, db_ext_info_dict, inst_env, inst_ext_info_dict, projected_object.sub_type)
        )
        thread.setDaemon(False)
        thread.start()
    except threading.ThreadError:
        log.exception(f"Start unmount repository thread failed, resource id: %s.", resource_id)


def execute_unmount_repo_task(resource: EnvironmentResourceSchema, res_ext_info: dict, env: EnvironmentSchema,
                              env_ext_info: dict, app_type: str):
    """
    执行解挂载存储仓任务
    """
    log.info("Start executing umount repo task, resource id: %s.", resource.uuid)
    nodes_ext_info = res_ext_info.get("nodes", "[]")
    node_list = json.loads(nodes_ext_info)
    log.info("Need unmount repository nodes: %s.", [i.get("endpoint") for i in node_list])
    req_body = build_check_app_req(resource, res_ext_info, env, env_ext_info)
    # 遍历资源的节点（需要查询最新节点信息）
    for node in node_list:
        cur_node: EnvironmentSchema = resource_service.query_resource_by_id(node.get("uuid"))
        if not cur_node:
            log.warning("The host(uuid=%s) does not exist.", node.get("uuid"))
            continue
        remove_protect_unmount_repo(cur_node.endpoint, cur_node.port, app_type, body=req_body.json(by_alias=True))
    # 删除持久仓白名单
    remove_log_repo_whitelist_of_resource(resource.uuid)
    log.info("Execute umount repo task completed, resource id: %s.", resource.uuid)


def build_check_app_req(resource: EnvironmentResourceSchema, res_ext_info: dict, env: EnvironmentSchema,
                        env_ext_info: dict):
    req_body = CheckAppReq()
    app = Application(
        uuid=resource.uuid, name=resource.name, parentUuid=resource.parent_uuid, parentName=resource.parent_name,
        type=resource.type, subType=resource.sub_type, extendInfo=res_ext_info)
    req_body.application = app
    app_env = AppEnv(
        uuid=env.uuid, name=env.name, type=env.type, subType=env.sub_type, endpoint=env.endpoint, port=int(env.port),
        extendInfo=env_ext_info
    )
    req_body.app_env = app_env
    return req_body

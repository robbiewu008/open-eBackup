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
import math
import asyncio
import re
import time
import uuid
from uuid import uuid4

from sqlalchemy import and_, true, false
from sqlalchemy.orm import Session, Query, aliased

import app.resource.service.host.host_service
from app.backup.client.rbac_client import RBACClient
from app.base.db_base import database
from app.base.resource_consts import ResourceConstant
from app.common.constants.constant import CommonConstants, AgentConstants
from app.common.enums.rbac_enum import ResourceSetTypeEnum
from app.common.event_messages.event import CommonEvent, EventBase
from app.common.events.topics import RESOURCE_DELETED_TOPIC
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.logger import get_logger
from app.common.security.role_dict import RoleEnum
from app.protection.object.db import projected_object
from app.resource.client.agent_client import query_can_update_agent_versions
from app.resource.client.system_base import get_snmp_trap_config, get_snmp_trap_addresses

from app.resource.discovery.res_discovery_plugin import delete_schedule_task
from app.resource.service.common.resource_service import comment_event_message
from app.resource.discovery.res_discovery_plugin import DiscoveryManager
from app.resource.models.database_models import DatabaseTable, AsmInfoTable, ClusterNodeTable
from app.resource.models.resource_models import EnvironmentTable, ResExtendInfoTable
from app.resource.models.resource_models import ResourceTable
from app.protection.object.models.projected_object import ProtectedObject, ProtectedTask
from app.resource.rpc import hw_agent_rpc
from app.resource.service.common import resource_service
from app.resource.schemas.env_schemas import ScanEnvSchema
from app.resource.schemas.host_models import AsmAuthRequest, AsmInfo, AsmAuthResponse, HostCreateInfo
from app.common.events.producer import produce
from app.common.enums.resource_enum import ResourceSubTypeEnum, ResourceTypeEnum
from app.common.event_messages.Discovery.discovery_rest import AuthType, ResourceType, HostOnlineStatus
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.rpc import system_base_rpc
from app.common.concurrency import run_in_pool
from app.common.events import producer


class ScanEnvironmentRequest(EventBase):
    uuid: str

    def __init__(self, resource_id: str):
        super().__init__(None, "Sanning_environment_v2")
        self.uuid = resource_id


log = get_logger(__name__)
NAME = "host_manager"
ONE_HOUR = 3600
THREE_MINUTES = 180
version_dict = {
    "1.2.1RC1": ["1.2.1RC2", "1.2.1"],
    "1.2.1RC2": ["1.2.1"],
    "1.2.1": ["1.3.RC1", "1.3.0", "1.3.0.SPC", "1.5.RC1", "1.5.0", "1.5.0.SPC"],
    "1.2.1.SPC": ["1.3.RC1", "1.3.0", "1.3.0.SPC", "1.5.0", "1.5.RC1", "1.5.0.SPC"],
    "1.3.RC1": ["1.3.0"],
    "1.3.0": ["1.3.0.SPC", "1.5.RC1", "1.5.0", "1.5.0.SPC", "1.6.0", "1.6.RC1", "1.6.0.SPC", "1.6.RC2", "1.6.RC1.SPC",
              "1.6.RC2.SPC"],
    "1.3.0.SPC": ["1.5.0", "1.5.RC1", "1.5.0.SPC", "1.6.0", "1.6.RC1", "1.6.0.SPC", "1.6.RC2", "1.6.RC1.SPC",
                  "1.6.RC2.SPC"],
    "1.5.RC1": ["1.5.0", "1.6.RC2.SPC"],
    "1.5.0": ["1.5.0.SPC", "1.6.0", "1.6.RC1", "1.6.0.SPC", "1.6.RC2", "1.6.RC1.SPC", "1.6.RC2.SPC"],
    "1.5.0.SPC": ["1.6.0", "1.6.RC1", "1.6.0.SPC", "1.6.RC2", "1.6.RC1.SPC", "1.6.RC2.SPC"],
    "1.6.RC1": ["1.6.RC2", "1.6.0", "1.6.RC1.SPC", "1.6.RC2.SPC"],
    "1.6.RC1.SPC": ["1.6.RC2", "1.6.0", "1.6.RC2.SPC"],
    "1.6.RC2": ["1.6.0", "1.6.RC2.SPC", "1.6.RC1.SPC"],
    "1.6.RC2.SPC": ["1.6.0"],
    "1.6.0": ["1.6.0.SPC"],
}


def get_environment_type_by_proxy_type(proxy_type: int):
    proxy_map = {
        0: ResourceSubTypeEnum.DBBackupAgent,
        2: ResourceSubTypeEnum.VMBackupAgent,
        3: ResourceSubTypeEnum.DWSBackupAgent
    }
    return proxy_map.get(proxy_type, "")


def add_host(host: HostCreateInfo):
    default_host_info = None
    env_type = get_environment_type_by_proxy_type(host.proxy_type)
    log.info(f"add a new host env_type: {env_type} host: {host}")
    if env_type in [ResourceSubTypeEnum.DBBackupAgent,
                    ResourceSubTypeEnum.VMBackupAgent,
                    ResourceSubTypeEnum.DWSBackupAgent]:
        env_param = host_to_env_param(host)
        existed_host = get_host_by_id(host.host_id)
        dm = DiscoveryManager(env_type)
        if existed_host:
            # 二次注册更新信息
            log.info(f"update env_type: {env_type} host information")
            env_param.extend_context.get('host')['user_id'] = existed_host.get('user_id')
            # 主动卸载场景 加入标识 健康检查使用
            if host.link_status == '0':
                env_param.extend_context.get('extend_info').update({'manualUninstallation': '1'})
            dm.modify_env(env_param)
            # 通用代理更新为vmware代理,触发一次插件扫描
            if existed_host.get('sub_type') == ResourceSubTypeEnum.UBackupAgent.value:
                message = ScanEnvironmentRequest(host.host_id)
                producer.produce(message)
            delete_extend_info(host.host_id, "use_old_private")
            # 非卸载场景 删除标识
            if host.link_status != '0':
                delete_extend_info(host.host_id, "logLeve")
                delete_extend_info(host.host_id, "manualUninstallation")
            return env_param.dict()

        if host.link_status == '0':
            # 卸载时会上传一次主机离线信息，重复ip时用户会删除pm数据
            return default_host_info

        existed_ip_host = get_host_by_ip(host.ip)
        if existed_ip_host:
            # 二次注册IP相同
            env_param_dict = env_param.dict()
            env_param_dict['error_duplicate_ip'] = existed_ip_host.endpoint
            env_param_dict['host_id'] = existed_ip_host.uuid
            log.error(f"has existed ip host, information: {env_param_dict['host_id']}")
            return env_param_dict

        dm.scan_env(env_param)
        return get_host_by_id(host.host_id)
    return default_host_info


def delete_extend_info(resource_id: str, key: str):
    with database.session() as session:
        extend_info = session.query(ResExtendInfoTable).filter(and_(
            ResExtendInfoTable.key == key,
            ResExtendInfoTable.resource_id == resource_id)).first()
        if extend_info is not None:
            session.delete(extend_info)


def page_query_host(page_no: int, page_size: int, type_of_app: str, host_uuid: str,
                    current_user_info: dict, conditions: str):
    log.info(f"page_query_hosts start, filter is:{conditions}", )

    filters = (
        "name", 'endpoint', "os_type", "host_id", "is_cluster",
        "is_database", 'link_status', 'authorized_user', 'sub_type',
        'orderType', 'asm_info'
    )
    condition = {k: v
                 for k, v in json.loads(conditions).items()
                 if k in filters} if conditions else {}
    for k, v in condition.items():
        if str(v).lower() == "true":
            condition[k] = True
        if str(v).lower() == "false":
            condition[k] = False
    # 默认不排序，
    num_reverse = 'null'
    if condition.get('orderType') == 'asc':
        num_reverse = False
        del condition['orderType']
    if condition.get('orderType') == 'desc':
        num_reverse = True
        del condition['orderType']
    # asm信息筛选
    is_asm_list = []
    if 'asm_info' in condition:
        is_asm_list = list(condition.get("asm_info"))
        del condition["asm_info"]

    with database.session() as session:
        query = page_query_host_conditioned(session, type_of_app, host_uuid, condition, current_user_info)
        items = query.limit(page_size).offset(page_no * page_size).all()
        count = query.count()

    items = list(environment_to_host(item) for item in items if item is not None)

    if is_asm_list:
        items = list(item
                     for item in items
                     if item.get('asm_info', {}).get('asm_type', 0) in is_asm_list)
        count = len(items)

    if num_reverse != 'null':
        items = sorted(items, key=lambda x: x.get(
            'extend_db', {}).get('num', 0), reverse=num_reverse)

    return {
        "total": count,
        "pages": math.ceil(count / page_size),
        "page_size": page_size,
        "page_no": page_no,
        "items": items
    }


def page_query_host_conditioned(session, type_of_app: str, host_uuid: str, condition: dict, current_user_info: dict):
    query: Query = session.query(EnvironmentTable)
    # user role is only ROLE_DP_ADMIN
    if (RoleEnum.ROLE_SYS_ADMIN.value not in current_user_info["role-list"]) and \
            (RoleEnum.ROLE_DP_ADMIN.value in current_user_info["role-list"]):
        current_user_host_has_resource = session.query(ResourceTable.uuid). \
            filter(ResourceTable.user_id == current_user_info["user-id"]).all()
        query = query.filter(EnvironmentTable.uuid.in_(current_user_host_has_resource))
    filters = build_query_filters(condition, session)
    if filters:
        query = query.filter(*filters)

    if host_uuid:
        query = query.filter(EnvironmentTable.uuid == host_uuid) \
            .filter(EnvironmentTable.sub_type != ResourceSubTypeEnum.ABBackupClient.value)

    if type_of_app:
        query = query.filter(EnvironmentTable.sub_type == type_of_app)

    return query


def build_query_filters(condition, session):
    filters = set()
    for k, v in condition.items():
        if k not in ["name", 'endpoint', 'authorized_user']:
            continue
        for token in '#%?*_':
            v = v.replace(token, f"#{token}")
            filters.add(getattr(EnvironmentTable, k).like(f'%{v}%', escape='#'))

    if condition.__contains__("host_id"):
        filters.add(EnvironmentTable.uuid == condition.get("host_id"))

    if condition.__contains__('sub_type'):
        filters.add(EnvironmentTable.sub_type.in_(list(condition['sub_type'])))

    if len(condition.get("link_status", [])) == 1:
        # host_link_satus: 0离线 1在线
        filters.add(EnvironmentTable.link_status == condition.get("link_status", [])[0])

    if bool(condition.get("is_database", False)):
        host_has_db = session.query(DatabaseTable.root_uuid).filter(
            DatabaseTable.root_uuid == EnvironmentTable.uuid).all()
        filters.add(EnvironmentTable.uuid.in_(list(i[0] for i in host_has_db)))

    if len(list(condition.get("is_cluster", []))) == 1:
        if condition.get("is_cluster", [])[0] == 'true':
            filters.add(EnvironmentTable.is_cluster == true())
        if condition.get("is_cluster", [])[0] == 'false':
            filters.add(EnvironmentTable.is_cluster == false())
    return filters


def environment_to_host(env):
    with database.session() as session:
        query: Query = session.query(DatabaseTable)
        filters = [DatabaseTable.root_uuid ==
                   env.uuid, DatabaseTable.valid == true()]
        db_info = query.filter(*filters).first()
        db_num = query.filter(*filters).count()
        db_type = getattr(db_info, 'sub_type', '')
        return {
            'host_id': env.uuid,
            'name': env.name,
            'endpoint': env.endpoint,
            'port': env.port,
            'link_status': env.link_status,
            'os_type': env.os_type,
            'type': env.type,
            'sub_type': env.sub_type,
            'is_cluster': env.is_cluster,
            'cluster_info': env.children_uuids,
            'user_id': env.user_id,
            'authorized_user': env.authorized_user,
            'asm_info': get_is_need_asm(env.uuid),
            'app_type': env.type,
            'extend_db': {'num': db_num,
                          'type': db_type}
        }


def handle_os_type(os_type):
    if os_type in ResourceConstant.LINUX_HOST_OS_TYPE_LIST:
        return ResourceConstant.HOST_OS_TYPE_LINUX
    elif os_type in ResourceConstant.AGENT_UNIX_OS_TYPE_MAP.keys():
        return ResourceConstant.AGENT_UNIX_OS_TYPE_MAP.get(os_type, "")
    else:
        return os_type


def host_to_env_param(host: HostCreateInfo):
    env_type = get_environment_type_by_proxy_type(host.proxy_type)
    if not host.host_id:
        host.host_id = str(uuid4())
    env = {
        'uuid': host.host_id,
        'name': host.name,
        'type': ResourceTypeEnum.Host,
        'sub_type': env_type,
        'endpoint': host.ip,
        'root_uuid': host.host_id,
        'port': host.port,
        'link_status': host.link_status,
        'os_type': handle_os_type(host.os_type),
        'os_name': host.os_type,
        'path': host.ip,
        'user_id': host.userid,
        'version': host.agent_version,
        'created_time': change_timestamp_to_date(host.agent_timestamp)
    }
    env_param = {
        "uuid": host.host_id,
        'name': host.name,
        'type': ResourceTypeEnum.Host,
        'sub_type': env_type,
        'user_name': "",
        'password': "",
        'endpoint': host.ip,
        'port': host.port,
        'extend_context': {
            'host': env,
            'extend_info': {
                'src_deduption': host.src_deduption,
                'install_path': host.install_path,
                'is_auto_synchronize_host_name': host.is_auto_synchronize_host_name
            }
        },
        'rescan_interval_in_sec': THREE_MINUTES
    }
    log.info(f"register host id: {host.host_id}")
    return ScanEnvSchema(**env_param)


def compare_regexps(rex, compare_str) -> bool:
    """
    检查字符串str是否符合正则表达式re_exp
    :param rex: 正则表达式
    :param compare_str: 校验字符串
    :return: 布尔值
    """
    if re.match(rex, compare_str):
        return True
    else:
        return False


def get_can_update_agent_host(session: Session, uuids):
    """
    :param session: 数据库session
    :return:
    """
    # 查接口获取最新的版本号,只有一个dict元素
    agents = query_can_update_agent_versions()  # list(dict)
    agent_version = agents[CommonConstants.NEW_AGENT_PACKAGE_INFO_LENTH]  # dict
    # 将upgradeVersions字段转为json
    upgrade_versions = json.loads(agent_version.get("upgradeVersions"))
    agent_version.update(upgradeVersions=upgrade_versions)
    log.debug(f"new agent info is: {agent_version}", )
    # 没有可更新agent版本返回空
    if agent_version is None or len(agent_version) == 0:
        log.debug(f"query newest agent_version info error, agent_versions data is: {agent_version}")
        return []
    can_update_clients_data = regex_filter(session, uuids, agent_version)
    # upgradeableVersion字段展示最新的版本号,不写入数据库
    return can_update_clients_data


def change_timestamp_to_date(timestamp):
    """
    将时间戳转为date格式
    :param timestamp: 1342044850000
    :return: 2012-07-12 06:14:10
    """
    date_time = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime(int(timestamp) / 1000))
    return date_time


def change_date_to_timestamp(date):
    """
    将date格式转为时间戳
    :param date: 2012-07-12 06:14:10
    :return:  1342044850000
    """
    timestamp = int(time.mktime(time.strptime(str(date).split(".")[0], '%Y-%m-%d %H:%M:%S'))) * 1000
    return timestamp


def regex_filter(session, uuids, new_agent_versions: dict):
    """
    过滤主机
    :param session: 数据库连接
    :param new_agent_versions: 推送的版本信息
    :return: 可更新的主机信息
    """
    # 过滤掉内置代理主机(ResExtendInfoTable资源扩展表，KEY字段为scenario，并且值value为1表示为内置代理主机)
    query_res_extend_datas = session.query(ResExtendInfoTable).filter(and_(
        ResExtendInfoTable.key == "scenario", ResExtendInfoTable.value == "1")).all()
    res_extend_datas = list(i.__dict__ for i in query_res_extend_datas)

    # 内置主机的uuid列表
    inner_host_uuids = []
    for data in res_extend_datas:
        #  ResExtendInfoTablebiao表中的resource_id对应Resource表中的uuid
        inner_host_uuids.append(data.get("resource_id"))
    log.debug(f"inner host uuids is: {inner_host_uuids}")
    environment = aliased(EnvironmentTable)
    query_datas = session.query(environment).filter(environment.uuid.notin_(inner_host_uuids)).filter(
        environment.uuid.in_(uuids)).all()
    query_dict_datas = list(i.__dict__ for i in query_datas)

    # 只过滤出在线的主机提示可升级
    can_updates = can_update(query_dict_datas, new_agent_versions)
    return can_updates


def modifiy_res_extend_info(host_uuid, upgradeable, upgradeable_version, agent_id):
    """
    更新数据库中主机的agent扩展字段
    """
    res_extend_infos = [
        {"resource_id": host_uuid, "key": "agentUpgradeable", "value": upgradeable},
        {"resource_id": host_uuid, "key": "agentUpgradeableVersion",
         "value": upgradeable_version},
        {"resource_id": host_uuid, "key": "agentId", "value": agent_id}]
    with database.session() as session:
        # 更新ResExtendInfoTable表的upgradeable和upgradeableVersion 和agentId三个字段
        for res_extend_info in res_extend_infos:
            query = session.query(ResExtendInfoTable).filter(*{
                ResExtendInfoTable.resource_id == host_uuid,
                ResExtendInfoTable.key == res_extend_info.get('key')})
            query.update(res_extend_info)


def merge_res_extend_info(host_uuid, upgradeable, upgradeable_version, agent_id):
    """
    插入数据库中主机的agent扩展字段
    """

    res_extend_infos = [
        {"uuid": str(uuid.uuid4()), "resource_id": host_uuid, "key": "agentUpgradeable", "value": upgradeable},
        {"uuid": str(uuid.uuid4()), "resource_id": host_uuid, "key": "agentUpgradeableVersion",
         "value": upgradeable_version},
        {"uuid": str(uuid.uuid4()), "resource_id": host_uuid, "key": "agentId", "value": agent_id}]
    with database.session() as session:
        for res_extend_info in res_extend_infos:
            query = session.query(ResExtendInfoTable).filter(*{
                ResExtendInfoTable.resource_id == host_uuid,
                ResExtendInfoTable.key == res_extend_info.get('key')}).first()
            resource = session.query(ResourceTable).filter(ResourceTable.uuid == host_uuid).first()
            if not query and resource:
                session.add(ResExtendInfoTable(**res_extend_info))
            elif query and resource:
                query = session.query(ResExtendInfoTable).filter(*{
                    ResExtendInfoTable.resource_id == host_uuid,
                    ResExtendInfoTable.key == res_extend_info.get('key')})
                res_extend_info.pop("uuid")
                query.update(res_extend_info)


def compare_agent(env_info, new_agent_version):
    """
    获取可更新版本列表
    :param env_info: 查询到的环境信息
    :param new_agent_version: 新推送主机的版本信息:
    :return: 主机agent升级信息
    """
    # 当前发行版本: 1.1.RC2.015
    release_version = str(new_agent_version.get("releaseVersion"))
    # 获取当前版本
    if (env_info.get("agent_version") or env_info.get("version")) is None:
        return False
    original_agent_version = env_info.get("version") if env_info.get(
        "version") is not None else env_info.get("agent_version")
    log.debug(f"original_agent_version: {original_agent_version}, release_version: {release_version}")
    # 防止数据库中的agent没有版本信息时候报错
    if (original_agent_version or release_version) is None:
        return False
    # 如果数据库中的agent version 版本号不满足新包里的版本号或补丁版本号
    if len(original_agent_version.split(".")) != CommonConstants.AGENT_VERSION_LENTH and len(
            original_agent_version.split(".")) != CommonConstants.AGENT_SPC_VERSION_LENTH:
        return False
    agent_id = new_agent_version.get("agentId")
    # 初始主机设置为不可升级upgradeable字段(0:不可升级, 1:可升级)
    if release_version == original_agent_version:
        # 版本一致时不做任何处理
        modifiy_res_extend_info(env_info.get("uuid"), AgentConstants.AGENT_NOT_UPGRADEABLE, release_version, agent_id)
        return False
    merge_res_extend_info(env_info.get("uuid"), AgentConstants.AGENT_NOT_UPGRADEABLE, release_version, agent_id)
    list_agents_total = get_upgradable_agents(env_info, original_agent_version, release_version, new_agent_version)
    if list_agents_total:
        return env_info
    return False


class AgentUpgradeInfo:
    def __init__(self, list_agents, env_info, host_uuid, original_agent_version_b_version,
                 release_b_version, max_b_version, original_sub_version, release_sub_version,
                 release_version, agent_id):
        self.list_agents = list_agents
        self.env_info = env_info
        self.host_uuid = host_uuid
        self.original_agent_version_b_version = original_agent_version_b_version
        self.release_b_version = release_b_version
        self.max_b_version = max_b_version
        self.original_sub_version = original_sub_version
        self.release_sub_version = release_sub_version
        self.release_version = release_version
        self.agent_id = agent_id


def check_and_add_upgradable_agent(agent_data: AgentUpgradeInfo):
    # 检查主版本号的升级
    if agent_data.original_sub_version < agent_data.release_sub_version:
        agent_data.list_agents.append(agent_data.env_info.get("uuid"))
        modifiy_res_extend_info(agent_data.host_uuid, AgentConstants.AGENT_UPGRADEABLE, agent_data.release_version,
                                agent_data.agent_id)
        # 如果主版本号无法判断 则在主版本号一致时比较sub_b_version
    elif agent_data.original_sub_version == agent_data.release_sub_version:
        # 版本更新则允许升级
        if agent_data.original_agent_version_b_version < agent_data.release_b_version:
            agent_data.list_agents.append(agent_data.env_info.get("uuid"))
            modifiy_res_extend_info(agent_data.host_uuid, AgentConstants.AGENT_UPGRADEABLE, agent_data.release_version,
                                    agent_data.agent_id)


def get_upgradable_agents(env_info, original_agent_version, release_version, new_agent_version):
    host_uuid = env_info.get("uuid")
    agent_id = new_agent_version.get("agentId")
    # 升级包信息
    upgrade_versions = new_agent_version.get("upgradeVersions")
    # 获取当前版本前后缀
    original_agent_version_b_version, original_agent_version_prefix, original_sub_version = (
        get_prefix_and_version(original_agent_version))
    # 获取推送新版本前后缀
    release_b_version, release_version_prefix, release_sub_version = get_prefix_and_version(release_version)
    list_agents = []
    log.info(f"agent can be upgraded,origin version:{original_agent_version_prefix},"
             f"original_sub_version:{original_sub_version},"
             f"original_agent_version_b_version:{original_agent_version_b_version}"
             f"release version:{release_version_prefix},"
             f"release_sub_version:{release_sub_version},"
             f"release_b_version:{release_b_version}")
    for new_version in upgrade_versions:
        # 版本号与当前版本一致时，判断B版本大于等于minBversion并且小于maxBversion
        min_b_version = int(str(new_version.get("minBVesion")).strip())
        max_b_version = int(str(new_version.get("maxBVesion")).strip())
        # 防呆(自己的版本和当前推送的版本一样的时候不提示更新)
        if original_agent_version_prefix == release_version_prefix:
            agent_data = AgentUpgradeInfo(
                list_agents=list_agents,
                env_info=env_info,
                host_uuid=host_uuid,
                original_agent_version_b_version=original_agent_version_b_version,
                release_b_version=release_b_version,
                max_b_version=max_b_version,
                original_sub_version=original_sub_version,
                release_sub_version=release_sub_version,
                release_version=release_version,
                agent_id=agent_id
            )
            check_and_add_upgradable_agent(agent_data)
        elif check_agent_version(original_agent_version_prefix, release_version_prefix):
            list_agents.append(env_info.get("uuid"))
            modifiy_res_extend_info(host_uuid, AgentConstants.AGENT_UPGRADEABLE, release_version, agent_id)
    return tuple(set(list_agents))


def get_prefix_and_version(original_agent_version):
    pre_and_version_arr = original_agent_version.rsplit(".", 1)
    if CommonConstants.SPC_PRE_FIX in pre_and_version_arr[0]:
        # 前缀应包含整个 SPC 后面的数字部分 (如 SPC2)
        spc_last_index = pre_and_version_arr[0].rindex(CommonConstants.SPC_PRE_FIX)
        # 版本后缀的版本号(如1.1.RC1.SPC1.010，返回1.1.RC1.SPC)
        agent_version_prefix = pre_and_version_arr[0][:spc_last_index + len(CommonConstants.SPC_PRE_FIX)]
        # 获取 SPC 后的版本信息 (如 SPC2.2 中的 2)
        agent_version_b_version = int(pre_and_version_arr[1])  # 获取最后的版本号
        try:
            sub_version_str = pre_and_version_arr[0][spc_last_index + len(CommonConstants.SPC_PRE_FIX):].split(".")[-1]
            sub_version = int(sub_version_str) if sub_version_str.isdigit() else 0
        except (IndexError, ValueError):
            sub_version = 0  # 当解析失败时，默认子版本号为0
    else:
        # 补丁版本前缀
        agent_version_prefix = pre_and_version_arr[0]
        # 补丁版本后缀
        agent_version_b_version = int(pre_and_version_arr[1])
        sub_version = 0  # 如果没有子版本号则设置为0
    return agent_version_b_version, agent_version_prefix, sub_version


def check_agent_version(original_agent_version_prefix, release_version_prefix):
    log.info(f"Check whether the agent version can be upgraded: origin version：{original_agent_version_prefix},"
             f"release version:{release_version_prefix}")
    if original_agent_version_prefix in version_dict.keys():
        can_update_version = version_dict.get(original_agent_version_prefix)
        return release_version_prefix in can_update_version
    else:
        log.info(f"agent can not be upgraded,origin version:{original_agent_version_prefix},"
                 f"release version:{release_version_prefix}")
        return False


def can_update(env_infos, new_agent_versions):
    # 可升级的版本号列表
    can_update_list = []
    for env_info in env_infos:
        # 比较版本是否可以升级
        data = compare_agent(env_info, new_agent_versions)
        if data:
            can_update_list.append(data)
    return can_update_list


def compare_versions(versions):
    """
    获取推送的agent中最大版本号
    :param versions: ['1.2.0.065', '1.0.0.035', '1.1.0.035']
    :return:
    """
    version_list = []
    for i in versions:
        version = i.replace('.', "")
        version_list.append({i: version})
    upgrading = max(list(dic.keys()) for dic in version_list)[0]
    return upgrading


def get_host_by_id(host_id: str):
    default_host_info = None
    # 根据id获取主机信息
    host = app.resource.service.host.host_service.get_environment(host_id)
    if host:
        return environment_to_host(host)
    return default_host_info


def get_resource_by_id(host_id: str):
    """
    去ResourceTable查询代理主机信息
    :param host_id: 传入的代理主机id
    :return: host 代理主机信息
    """
    log.info(f"Start to execute get resource by id.")
    default_host_info = None
    # 根据id获取resource表主机信息
    host = app.resource.service.host.host_service.get_resource(host_id)
    if host:
        return host
    return default_host_info


def get_lan_free_by_id(host_id: str):
    """
    去ResExtendInfoTable查询代理主机扩展信息
    :param host_id: 传入的代理主机id
    :return: extend_info 代理主机扩展信息
    """
    log.info(f"Start to execute get lan free by id.")
    # 根据id获取ResExtendInfoTable表主机信息
    extend_info = app.resource.service.host.host_service.get_lan_free_extend_info(host_id)
    log.info(f"finish execute get lan free by id:{extend_info}")
    return extend_info


def get_host_by_ip(host_ip: str):
    with database.session() as session:
        env = session.query(EnvironmentTable).filter(EnvironmentTable.endpoint == host_ip).first()
        return env


def get_vm_host_by_ip(host_ip: str, host_port: str):
    default_host_info = None
    host = app.resource.service.host.host_service.get_environment_by_ip_port(
        host_ip, host_port)
    if host:
        return environment_to_host(host)
    return default_host_info


def get_is_need_asm(host_id: str) -> dict:
    with database.session() as db:
        filters = [AsmInfoTable.root_uuid == host_id]
        asm_db = db.query(AsmInfoTable).filter(*filters).all()
        if not asm_db:
            asm_info_dict = {'asm_type': 0, 'asm_instances': None}
        elif not asm_db[0].verify_status:
            asm_info_dict = {'asm_type': 1, 'asm_instances': asm_db[0].asm_instances}
        else:
            asm_info_dict = {'asm_type': 2, 'asm_instances': asm_db[0].asm_instances}
        return asm_info_dict


def check_resource_has_sla(session: Session, host_id: str):
    # 查询主机下资源,除去主机本身
    resource_ids = session.query(ResourceTable.uuid).filter(and_(
        ResourceTable.root_uuid == host_id, ResourceTable.uuid != host_id)).all()
    if not resource_ids:
        return
    # 查询多个资源是否被保护
    resource_list = projected_object.query_by_resource_ids(session, list(i[0] for i in resource_ids))
    if bool(resource_list):
        name = {resource_sla.name for resource_sla in resource_list}
        raise EmeiStorBizException(
            ResourceErrorCodes.DELETE_RESOURCE_HAS_SLD, name)


def delete_res_extend_info_by_host_uuid(host_uuid):
    """
    删除ResExtendInfoTable表中主机的agent信息
    删除ResExtendInfoTable表中主机的源端重删信息
    """
    with database.session() as session:
        host_res_extend_info_list = ["agentUpgradeable", "agentUpgradeableVersion", "agentId", "src_deduption"]
        for i in host_res_extend_info_list:
            session.query(ResExtendInfoTable).filter(ResExtendInfoTable.resource_id == host_uuid).filter(
                ResExtendInfoTable.key == i).delete()


def delete_host(host, session: Session):
    log.debug(f'host_id: {host["host_id"]}')
    env_host = resource_service.query_resource_by_id(host["host_id"])
    if env_host.sub_type == ResourceSubTypeEnum.VMBackupAgent:
        dm = DiscoveryManager(ResourceSubTypeEnum.VMBackupAgent)
        dm.delete_env(host["host_id"])
        return True
    if env_host.sub_type == ResourceSubTypeEnum.UBackupAgent:
        log.error(f"It's not allowed to delete General Backup Agent.")
        raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR,
                                   message=f"host is General Backup Agent.")
    if str(host["link_status"]) != str(HostOnlineStatus.OFF_LINE.value):
        # 前端已限制
        log.error(f"delete host error: host is not online")
        raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR,
                                   message=f"Cannot delete because host is not online.")
    if projected_object.query_by_env_id(db=session, env_id=host["host_id"]):
        # 前端已限制
        log.error(f"delete host error: host has sla")
        raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR,
                                   message=f"Cannot delete because host is sla.")

    check_resource_has_sla(session=session, host_id=host["host_id"])
    delete_host_or_cluster(session, host["host_id"])

    # 删除res_extend_info表中的agent主机信息
    delete_res_extend_info_by_host_uuid(host.get("host_id"))
    return True


def get_cluster_by_node(host_id: str):
    default_cluster_info = None
    with database.session() as session:
        db_clusters_hosts = session.query(ResourceTable).filter(
            and_(host_id in ResourceTable.children_uuids)).all()
        for db_clusters_host in db_clusters_hosts:
            if host_id in db_clusters_host.children_uuids:
                return db_clusters_host.uuid
    return default_cluster_info


def delete_host_or_cluster(session, host_id: str, has_only_delete_cluster=True):
    log.info(f"begin to delete host or cluster id: {host_id}")
    # 查询主机，先删除子节点表
    host = session.query(EnvironmentTable).filter(EnvironmentTable.uuid == host_id).one()
    if host.is_cluster:
        db_nodes = session.query(EnvironmentTable).filter(
            EnvironmentTable.uuid.in_(host.children_uuids),
            EnvironmentTable.type == ResourceTypeEnum.Host.value).all()
        for db_node in db_nodes:
            if not has_only_delete_cluster:
                continue
                # 检查集群下主机和资源是否被保护
            check_resource_has_sla(session=session, host_id=db_node.uuid)

        if has_only_delete_cluster:
            # 只删除一个离线节点，has_only_delete_cluster==False
            for db_node in db_nodes:
                session.query(ClusterNodeTable).filter(ClusterNodeTable.host_uuid == db_node.uuid).delete()
                RBACClient.delete_resource_set_relation([db_node.uuid],
                                                        ResourceSetTypeEnum.RESOURCE.value)
                session.commit()
                delete_host_or_cluster(session, db_node.uuid)
    else:
        # 删除调度任务
        delete_schedule_task(host.uuid)
    node_host = session.query(ClusterNodeTable).filter(
        ClusterNodeTable.host_uuid == host_id).one_or_none()
    # 删除集群节点判断
    if node_host is not None:
        cluster_ip = json.loads(node_host.cluster_info)['ClusterIP']
        clusters_host = session.query(EnvironmentTable).filter(EnvironmentTable.endpoint == cluster_ip).one_or_none()
        session.delete(node_host)
        session.commit()
        if clusters_host is not None:
            # 如果集群存在，只删除集群，不删除其他节点
            delete_host_or_cluster(session, clusters_host.uuid, has_only_delete_cluster=False)
    session.delete(host)
    RBACClient.delete_resource_set_relation([host.uuid], ResourceSetTypeEnum.AGENT.value)
    # 删除asm信息
    session.query(ResourceTable).filter(
        and_(ResourceTable.parent_uuid == host_id, ResourceTable.type == 'asm')).delete()
    # 删除主机下应用信息
    session.query(ResourceTable).filter(and_(
        ResourceTable.root_uuid == host_id, ResourceTable.type == ResourceTypeEnum.Application.value)).delete()
    session.commit()
    # 删除当前主机所有数据库
    db_resources = session.query(ResourceTable).filter(and_(
        ResourceTable.root_uuid == host_id, ResourceTable.sub_type == ResourceSubTypeEnum.Oracle.value)).all()
    if db_resources:
        for db_resource in db_resources:
            # 发送删除数据库消息
            comment_event_message(topic=RESOURCE_DELETED_TOPIC, request_id=str(
                uuid4()), resource_id=db_resource.uuid)
            session.delete(db_resource)
            RBACClient.delete_resource_set_relation([db_resource.uuid],
                                                    ResourceSetTypeEnum.RESOURCE.value)
        session.commit()
    session.commit()
    # 发送删除主机消息
    message = CommonEvent(RESOURCE_DELETED_TOPIC, resource_id=host_id)
    produce(message)


def refresh_host_list():
    log.info('start refresh_host_list.')
    with database.session() as db:
        is_cluster = False
        hosts = db.query(EnvironmentTable).filter(
            EnvironmentTable.is_cluster == is_cluster).all()
        refresh_host_list_one_by_one(hosts, db)

    log.info('end refresh_host_list.')


def refresh_host_list_one_by_one(hosts, db):
    for host in hosts:
        if str(host.sub_type) == str(ResourceSubTypeEnum.DBBackupAgent.value):
            refresh_dme_host(db, host)


def refresh_dme_host(db, host: EnvironmentTable):
    try:
        dm = DiscoveryManager(ResourceSubTypeEnum.DBBackupAgent)
        param = {
            'name': host.name,
            'type': ResourceTypeEnum.Host.value,
            'user_name': "",
            'password': "",
            'endpoint': host.endpoint,
            'port': host.port,
            'extend_context': {'host': host.as_dict()},
            'sub_type': ResourceSubTypeEnum.DBBackupAgent.value
        }
        env_param = ScanEnvSchema(**param)
        dm.refresh_env(env_param)
    except Exception:
        log.exception(msg="wrong with refresh dme host")
        host.link_status = str(HostOnlineStatus.OFF_LINE.value)
        db.merge(host)
        log.error(f'update host ip: {host.endpoint} to offline.')
    finally:
        pass


def refresh_cluster_host_info(host_id, db, db_name, is_valid, is_clear_protection_object=None):
    if is_clear_protection_object:
        # 清除主机下数据库名为xx的sla保护关系
        protection_objects = db.query(ProtectedObject).filter(
            and_(ProtectedObject.env_id == host_id, ProtectedObject.name == db_name)).all()

        for protection_object in protection_objects:
            protection_task = db.query(ProtectedTask).filter(
                and_(ProtectedTask.protected_object_id == protection_object.uuid))
            if len(protection_task.all()) > 0:
                protection_task.delete()
                db.delete(protection_object)
            else:
                db.delete(protection_object)
        db.commit()

    host = db.query(EnvironmentTable). \
        filter(EnvironmentTable.uuid == host_id).one()

    if host.is_cluster:
        asyncio.run(task_run_refresh_cluster_dme_host(db, host.children_uuids))
        return get_list_db_uuid(host, db, db_name, is_valid)
    else:
        refresh_dme_host(db, host)
        return get_list_db_uuid(host, db, db_name, is_valid)


def task_run_refresh_cluster_dme_host_node(db, node):
    child_env = db.query(EnvironmentTable).filter(EnvironmentTable.uuid == node).one()
    return refresh_dme_host(db, child_env)


async def task_run_refresh_cluster_dme_host(db, host_children_uuids):
    tasks = []
    for node in host_children_uuids:
        tasks.append(run_in_pool(method=task_run_refresh_cluster_dme_host_node, args=[db, node]))
    await asyncio.gather(*tasks, return_exceptions=True)


def get_list_db_uuid(host, db, db_name, is_valid):
    db_uuid_list = []
    # 数据库名存在且是集群
    if host.is_cluster and db_name is not None:
        filters = {DatabaseTable.root_uuid == host.uuid, DatabaseTable.valid == is_valid,
                   DatabaseTable.name == db_name}
        database_refreshed = db.query(DatabaseTable).filter(*filters).all()
        if database_refreshed is not None:
            for databases in database_refreshed:
                db_uuid_list.append(databases.uuid)
        return db_uuid_list
    elif host.is_cluster and db_name is None:
        filters = {DatabaseTable.root_uuid ==
                   host.uuid, DatabaseTable.valid == is_valid}
        database_refreshed = db.query(DatabaseTable).filter(*filters).all()
        if database_refreshed is not None:
            for databases in database_refreshed:
                db_uuid_list.append(databases.uuid)
        return db_uuid_list

    # 数据库名不存在
    if db_name is not None:
        filters = {DatabaseTable.root_uuid == host.uuid,
                   DatabaseTable.name == db_name,
                   DatabaseTable.valid == is_valid}
        database_refreshed = db.query(DatabaseTable).filter(*filters).all()
        if database_refreshed is not None:
            for databases in database_refreshed:
                db_uuid_list.append(databases.uuid)
        return db_uuid_list
    else:
        filters = {DatabaseTable.root_uuid == host.uuid}
        database_refreshed = db.query(DatabaseTable).filter(*filters).all()
        if database_refreshed is not None:
            for databases in database_refreshed:
                db_uuid_list.append(databases.uuid)
        return db_uuid_list


def asm_info(host_id: str):
    with database.session() as database_session:
        filters = [EnvironmentTable.uuid == host_id]
        host = database_session.query(EnvironmentTable).filter(*filters).one()
        if host.sub_type != str(ResourceSubTypeEnum.DBBackupAgent.value):
            return []
        if not host.is_cluster:
            return auto_storage_management_info_from_cluster(host)
        else:
            return auto_storage_management_info_from_common(host, database_session)


def auto_storage_management_info_from_cluster(host):
    """
    Get ASM infos by host which is Cluster
    :param host: host
    :return: ASM Infos(list)
    """
    asm_json_list = hw_agent_rpc.query_asm_instance(host.endpoint)
    rtn_list = []
    if asm_json_list is not None:
        for asm_json in asm_json_list:
            rtn_list.append(AsmInfo(auth_type=asm_json['authType'], inst_name=asm_json['instName'],
                                    is_cluster=asm_json['isCluster']))
    return rtn_list


def auto_storage_management_info_from_common(host, database_session):
    """
    Get ASM infos by host which is not cluster
    :param host: host
    :param database_session: session of database
    :return: ASM infos(list)
    """
    nodes = host.children_uuids
    filters = {EnvironmentTable.uuid.in_(nodes)}
    cluster_hosts = database_session.query(
        EnvironmentTable).filter(*filters).all()
    rtn_list = []
    if cluster_hosts is not None:
        for cluster_host in cluster_hosts:
            rtn_list.extend(
                auto_storage_management_info_from_cluster(cluster_host))
    return rtn_list


# 筛选 主机的集群信息
def get_host_cluster_info(host_id, db):
    cluster_filters = [ResourceTable.uuid == host_id]
    host_list = db.query(ResourceTable).filter(*cluster_filters).all()
    list_host = []
    if host_list is not None:
        for node in host_list[0].children_uuids:
            filters = [EnvironmentTable.uuid == node]
            host = db.query(EnvironmentTable).filter(*filters).first()
            cluster_host = {
                "host_id": host.uuid,
                "name": host.name,
                "endpoint": host.endpoint,
                "os_type": host.os_type
            }
            list_host.append(cluster_host)
    return list_host


# 手工通过PM界面修改主机名称
def manual_modify_host_name(host_id, host_name):
    host = get_host_by_id(host_id)
    if host is None:
        raise EmeiStorBizException(ResourceErrorCodes.HOST_NOT_EXISTS,
                                   message=f"Environment {host_id} is not exists.")
    if host.get("sub_type", "") not in (ResourceSubTypeEnum.DBBackupAgent.value,
                                        ResourceSubTypeEnum.ABBackupClient.value,
                                        ResourceSubTypeEnum.VMBackupAgent.value):
        raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS)
    param = {"uuid": host_id, "name": host_name}
    with database.session() as session:
        # 文件集path字段存的主机名，同步修改
        if host.get("sub_type", "") == ResourceSubTypeEnum.ABBackupClient.value:
            session.query(ResourceTable).filter(
                ResourceTable.root_uuid == host_id,
                ResourceTable.sub_type == ResourceSubTypeEnum.Fileset.value
            ).update({"path": host_name})

        session.merge(EnvironmentTable(**param))
        log.info(f"Modify host name success")


# 手动同步snmp到agent主机
def get_snmp_trap_info():
    snmp_conf = {}
    error_info = set()
    trap_config = get_snmp_trap_config()
    trap_addresses = get_snmp_trap_addresses()

    if trap_config.get('version', ''):
        if trap_config.get('version', '') == 'V2C':
            trap_config['securityNameV2C'] = system_base_rpc.decrypt(trap_config.get('securityNameV2C', ''))

        if trap_config.get('version', '') == 'V3':
            if trap_config.get('authProtocol', ''):
                trap_config['authPwd'] = system_base_rpc.decrypt(trap_config.get('authPwd', ''))
            if trap_config.get('encryptProtocol', ''):
                trap_config['encryptPwd'] = system_base_rpc.decrypt(trap_config.get('encryptPwd', ''))

        snmp_conf['trap_config'] = trap_config
    else:
        error_info.add('trap_config')
    if len(trap_addresses) > 0:
        snmp_conf['trap_addresses'] = trap_addresses
    else:
        error_info.add('trap_addresses')

    if error_info:
        log.error('failed to get snmp: {}'.format(error_info))
        raise EmeiStorBizException(CommonErrorCodes.OPERATION_FAILED,
                                   message=f"Get snmp failed")

    return snmp_conf


def manual_synchronize_snmp_to_agent(host_id):
    host = get_host_by_id(host_id)
    if host is None:
        raise EmeiStorBizException(ResourceErrorCodes.HOST_NOT_EXISTS,
                                   message=f"Environment {host_id} is not exists.")
    return host.get('endpoint', ''), get_snmp_trap_info()


def check_host_resource_if_san_client(host_id):
    """
    校验是否是SanClient代理主机
    :param host_id: 传入的代理主机id
    """
    log.info(f"Start to check host resource if is san client.")
    host = get_resource_by_id(host_id)
    if host is None:
        raise EmeiStorBizException(ResourceErrorCodes.HOST_NOT_EXISTS,
                                   message=f"Resource {host_id} is not exists.")
    sub_type = host.sub_type
    if sub_type == ResourceSubTypeEnum.S_BACKUP_AGENT.value:
        raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                   message=f"subType {sub_type} is not support setting snmp.")


def check_host_resource_if_configured_lan_free(host_id):
    """
    校验是否已配置LAN-Free
    :param host_id: 传入的代理主机id
    """
    log.info(f"Start to check host resource if configured LAN-Free.")
    extend_info = get_lan_free_by_id(host_id)
    log.info(f"finish get_lan_free_by_id:{extend_info}")
    if len(extend_info) == 0:
        return
    if extend_info.get('value') == '1':
        raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                   message=f"The host is configured LAN-Free, can not migrate.")

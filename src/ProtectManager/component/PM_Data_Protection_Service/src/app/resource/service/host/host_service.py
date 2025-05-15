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
import os
import uuid
from uuid import uuid4, UUID

from sqlalchemy import true, or_, and_
from sqlalchemy.orm.exc import NoResultFound

from app.backup.client.rbac_client import RBACClient
from app.base.clean_except import clean_except
from app.base.db_base import database
from app.common.enums.rbac_enum import ResourceSetTypeEnum, ResourceSetScopeModuleEnum
from app.common.events.topics import RESOURCE_DELETED_TOPIC
from app.resource.models.database_models import DatabaseTable, ClusterNodeTable, AsmInfoTable
from app.resource.models.resource_models import log, EnvironmentTable, ResourceTable, ResExtendInfoTable
from app.resource.service.common.resource_service import comment_event_message
from app.resource.client.system_base import get_user_info_by_user_id
from app.common.enums.resource_enum import ResourceSubTypeEnum, ResourceTypeEnum
from app.common.event_messages.Discovery.discovery_rest import AuthType, ResourceType, \
    ResourceStatus, HostOnlineStatus
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.event_messages.Resource.resource import ResourceAddedRequest
from app.common.sensitive.sensitive_word_filter_util import sensitive_word_filter
from app.resource.service.common.user_domain_service import get_domain_id_by_user_id
from app.common.schemas.resource_set_relation_schemas import ResourceSetRelationInfo

DEFAULT_ENVIRONMENT_RES = None
MAX_RESOURCE_NUM = int(os.getenv("MAX_RESOURCE_NUM", "20000"))


@clean_except()
def query_environment(query_expression: dict = None) -> list:
    log.debug(f'query_expression={query_expression}')
    with database.session() as session:
        if not query_expression:
            envs = session.query(EnvironmentTable).all()
        else:
            envs = session.query(EnvironmentTable).filter_by(
                **query_expression).all()

        return list(item.as_dict for item in envs)


def get_domain_id_list(domain_id):
    if domain_id:
        return [domain_id]
    else:
        return []


def get_environment(env_uuid):
    log.debug(f'uuid={env_uuid}')
    with database.session() as session:
        env = session.query(EnvironmentTable).filter(
            EnvironmentTable.uuid == env_uuid
        ).first()
        if env:
            return env
    return DEFAULT_ENVIRONMENT_RES


def get_resource(env_uuid):
    log.debug(f'uuid={env_uuid}')
    with database.session() as session:
        env = session.query(ResourceTable).filter(
            ResourceTable.uuid == env_uuid
        ).first()
        if env:
            return env
    return DEFAULT_ENVIRONMENT_RES


def get_environment_by_ip_port(end_point: str, port: str):
    log.debug(f'vm_agent_ip={end_point}')
    log.debug(f'vm_agent_port={port}')
    with database.session() as session:
        query = session.query(EnvironmentTable)
        if end_point:
            query = query.filter(
                EnvironmentTable.endpoint == end_point
            )
        if port:
            query = query.filter(EnvironmentTable.port == port)
        env = query.first()
        if env:
            return env
    return DEFAULT_ENVIRONMENT_RES


def automatic_authorization_by_agent_userid(env: dict):
    if env['user_id'] is not None:
        user_info = get_user_info_by_user_id(env['user_id'])
        # 如果查询不到用户返回的空字典
        if user_info:
            # 非管理员才进行授权
            if user_info.get('sysAdmin', "") is False:
                env['authorized_user'] = user_info.get('userName', "")
            else:
                env['user_id'] = None
                env['authorized_user'] = None
        else:
            # 未查询到agent带的user_id
            env['user_id'] = None
    else:
        # agent不传userid则归属于管理员
        env['user_id'] = None
    return env


def get_relative_extend_info(env_uuid, extend_info_key):
    log.debug(f'env_uuid={env_uuid}')
    with database.session() as session:
        extend_info = session.query(ResExtendInfoTable).filter(and_(
            ResExtendInfoTable.key == extend_info_key,
            ResExtendInfoTable.resource_id == env_uuid)).first()
        if extend_info:
            return extend_info.__dict__
    return dict()


def get_lan_free_extend_info(env_uuid):
    log.debug(f'env_uuid={env_uuid}')
    with database.session() as session:
        extend_info = session.query(ResExtendInfoTable).filter(and_(
            ResExtendInfoTable.key == "isAddLanFree",
            ResExtendInfoTable.resource_id == env_uuid)).first()
        if extend_info:
            return extend_info.__dict__
    return dict()


# upsert扩展信息表中的extend_info_key字段
def upsert_extend_info(env_uuid, extend_info):
    log.info(f'upsert_extend_info env={env_uuid}')
    log.info(f'extend_info env={extend_info}')
    for key, _value in extend_info.items():
        log.info(f'extend_info extend_info_key={key}')
        old_extend = get_relative_extend_info(env_uuid, key)
        log.info(f'oldExtend env={old_extend}')
        env_extend_info = {}
        if old_extend:
            log.info(f'if oldExtend')
            # 有，更新
            env_extend_info["uuid"] = str(old_extend.get('uuid'))
            env_extend_info["value"] = extend_info.get(key)
        else:
            log.info(f'else oldExtend')
            # 没有关联的, 新增
            env_extend_info["uuid"] = str(uuid.uuid4())
            env_extend_info["resource_id"] = env_uuid
            env_extend_info["key"] = key
            env_extend_info["value"] = extend_info.get(key)
        log.info(f'indb env_extend_info={env_extend_info}')
        with database.session() as session:
            session.merge(ResExtendInfoTable(**env_extend_info))


# 检查新增后 资源规格是否超出规格上限
def check_resource_exceeds_limit(reource_uuid):
    if MAX_RESOURCE_NUM == -1:
        return
    res_uuid = str(reource_uuid)
    with database.session() as session:
        resource_obj = session.query(ResourceTable).filter(ResourceTable.uuid == res_uuid).first()
        if resource_obj is not None:
            log.info(f'update, do not check. resource_id={res_uuid}')
            return

        db_count = session.query(ResourceTable).filter(ResourceTable.type != ResourceTypeEnum.PLUGIN.value).count()
        log.info(f'Number of resources in the database={db_count}')
        if db_count is not None and db_count + 1 > MAX_RESOURCE_NUM:
            log.info(f'do not add resource={res_uuid}, casue will exceeds the upper limit.')
            raise EmeiStorBizException(error=CommonErrorCodes.RESOURCE_NUM_EXCEED_LIMIT, parameters=[MAX_RESOURCE_NUM],
                                       message="The total number of resources exceeds the upper limit.")


def check_and_limit_resources(resources):
    """
    检查添加目标数量资源后是否超过最大数量限制，是则限制其数量
    :param resources: 要添加的资源列表
    :return: 限制数量后的资源列表
    """
    if MAX_RESOURCE_NUM == -1:
        return resources
    with database.session() as session:
        db_count = session.query(ResourceTable).filter(ResourceTable.type != ResourceTypeEnum.PLUGIN.value).count()
        log.info(f'Number of resources in the database={db_count}, add number:{len(resources)}')
        if db_count is not None and len(resources) > MAX_RESOURCE_NUM - db_count:
            reserve_count = MAX_RESOURCE_NUM - db_count
            log.warn(f"Added VM resources exceeded limit, reserved resources: {reserve_count}")
            return resources[:reserve_count] if reserve_count > 0 else []
        return resources


@clean_except()
def upsert_environment(env: dict):
    log.debug(f'env={sensitive_word_filter(env)}')
    # 高级备份 is_rescan == True是轮询刷新机制，==False是第一次注册上来
    domain_id = ""
    if env['user_id']:
        domain_id = get_domain_id_by_user_id(env['user_id'])
    # 检查资源规格是否超出规格上限
    check_resource_exceeds_limit(env.get('uuid', ""))
    with database.session() as session:
        session.merge(EnvironmentTable(**env))
    resource_set_relation_info = ResourceSetRelationInfo(resource_object_id=env.get('uuid'),
                                                         resource_set_type=ResourceSetTypeEnum.AGENT.value,
                                                         scope_module=ResourceSetScopeModuleEnum.AGENT.value,
                                                         domain_id_list=get_domain_id_list(domain_id))
    RBACClient.add_resource_set_relation(resource_set_relation_info)


@clean_except()
def delete_environment(env_uuid: str):
    log.debug(f'uuid={env_uuid}')

    with database.session() as session:
        env_obj = session.query(EnvironmentTable).filter_by(uuid=env_uuid).delete()
        if not env_obj:
            raise NoResultFound


@clean_except()
def delete_environments(query_expression: dict = None) -> list:
    log.debug(f'query_expression={query_expression}')
    with database.session() as session:
        if not query_expression:
            delete_count = session.query(EnvironmentTable).delete()
        else:
            delete_count = session.query(EnvironmentTable).filter_by(
                **query_expression).delete()
        return delete_count


@clean_except()
def batch_environments(updates: dict):
    log.debug(f'Batch update')
    for env in updates.get('insert', []) + updates.get('update', []):
        upsert_environment(env)
    for env in updates.get('delete', []):
        delete_environment(env['uuid'])


@clean_except()
def update_application(host_uuid, params, applications=None, cluster_info=None):
    # 默认只支持一个应用（agent）,也就是一台主机只有一条记录，
    if cluster_info is not None:
        applications = cluster_info['applications']

    time_zone = applications['timezone']
    if time_zone is None:
        time_zone = ''

    del applications['timezone']
    with database.session() as db:

        if cluster_info is not None:
            # 更新application集群信息
            for _, application in applications.items():
                application['oracleHome'] = None
            params.name = cluster_info['name']
            host_uuid = cluster_info['uuid']

        # 更新主机时区信息
        if len(time_zone) > 0:
            filters = {EnvironmentTable.uuid == host_uuid}
            host = db.query(EnvironmentTable).filter(*filters).one_or_none()
            if host is not None:
                log.info('current time zone:{}'.format(time_zone))
                host.time_zone = time_zone
                db.merge(host)

        filters = {ResourceTable.type == ResourceTypeEnum.Application.value,
                   ResourceTable.sub_type == ResourceSubTypeEnum.OracleApp.value,
                   ResourceTable.root_uuid == host_uuid}
        exist_application = db.query(ResourceTable).filter(*filters).one_or_none()

        if exist_application:
            if applications is None:
                db.delete(exist_application)
                db.commit()
                return DEFAULT_ENVIRONMENT_RES
            for _, application in applications.items():
                # 更新
                update_new_application(exist_application, application, params, host_uuid)
                db.merge(exist_application)
            db.commit()
        else:
            for _, application in applications.items():
                database_model = {
                    'name': ResourceSubTypeEnum.OracleApp.value,
                    'type': ResourceTypeEnum.Application.value,
                    'application_type': ResourceSubTypeEnum.OracleApp.value,
                    'version': application['version'],
                    'oracleHome': application['oracleHome']
                }
                new_item = create_new_application(database_model, host_uuid, params)
                new_item = ResourceTable(**new_item)
                db.add(new_item)
            db.commit()
    # 非集群更新才返回
    if cluster_info is None:
        applications['timezone'] = time_zone
        return applications
    return DEFAULT_ENVIRONMENT_RES


def update_new_application(exist_application, application, params, host_uuid):
    exist_application.name = ResourceSubTypeEnum.OracleApp.value
    exist_application.path = application['oracleHome']
    exist_application.version = application['version']
    exist_application.parent_name = params.name
    exist_application.parent_uuid = host_uuid
    exist_application.root_uuid = host_uuid
    exist_application.type = ResourceTypeEnum.Application.value
    exist_application.sub_type = ResourceSubTypeEnum.OracleApp.value
    return exist_application


def create_new_application(database_model: dict, host_uuid, params):
    return {'uuid': str(uuid4()),
            'name': database_model['name'],
            'path': str(database_model['oracleHome']),
            'type': database_model['type'],
            'parent_uuid': host_uuid,
            'root_uuid': host_uuid,
            'parent_name': params.name,
            'version': database_model['version'],
            'sub_type': database_model['application_type']
            }


@clean_except()
def update_database(env, params, database_agent, asm_info):
    filters = {DatabaseTable.sub_type.in_([ResourceSubTypeEnum.Oracle.value]),
               DatabaseTable.root_uuid == str(env['uuid'])}
    with database.session() as db:
        exist_dbs = db.query(DatabaseTable).filter(*filters).all()
        exist_dbs_dict = {i.name: i.valid for i in exist_dbs}
        exist_dbs_instance = {i.name: i.inst_name for i in exist_dbs}
        exist_dbs_verify_status = {i.name: i.verify_status for i in exist_dbs}

        # 更新主机asm信息
        asm = create_host_auth(db=db, env=env, params=params, asm_info=asm_info)

        for db_name, db_instance_info_list in database_agent.items():
            if db_name is None:
                continue
            if db_name not in exist_dbs_dict.keys():
                # 第一次注册
                auth_status = False
                auth_instance = None
            else:
                auth_status = exist_dbs_verify_status[db_name]
                auth_instance = exist_dbs_instance[db_name]

            db_info = choose_available_databases_instance(db_instance_info_list, auth_status, auth_instance)
            create_dme_database(env, params, asm, db_info)

        for exist_db in exist_dbs:
            if exist_db.name not in database_agent.keys():
                exist_db.valid = False
                exist_db.link_status = ResourceStatus.OFF_LINE.value
                db.merge(exist_db)
                # 删除的资源发送kafka消息
                request_id = str(uuid4())
                comment_event_message(topic=RESOURCE_DELETED_TOPIC, request_id=str(request_id),
                                      resource_id=exist_db.uuid)
                log.info(f'send topic: resource.deleted request_id:{request_id} resource_id:{exist_db.uuid}')

        # 再查一次判断是否新增进去
        new_exist_dbs = db.query(DatabaseTable).filter(*filters).all()
        for new_exist_db in new_exist_dbs:
            has_add_database_resource_topic(new_exist_db, exist_dbs_dict)
            if new_exist_db.name not in exist_dbs_dict.keys():
                # 新增资源发送 kafa
                request_id = str(uuid4())
                comment_event_message(topic=ResourceAddedRequest.default_topic, request_id=request_id,
                                      resource_id=new_exist_db.uuid)
                log.info(f'send topic: resource.added request_id:{request_id} resource_id:{new_exist_db.uuid}')


def update_database_when_agent_is_none(env):
    log.info(f"agent info is none , update all {env['uuid']} oracle offline and valid is false")
    with database.session() as db:
        filters = {DatabaseTable.sub_type.in_([ResourceSubTypeEnum.Oracle.value]),
                   DatabaseTable.root_uuid == str(env['uuid'])}
        exist_dbs = db.query(DatabaseTable).filter(*filters).all()
        for database_need_valid in exist_dbs:
            database_need_valid.valid = False
            database_need_valid.link_status = ResourceStatus.OFF_LINE.value
            db.merge(database_need_valid)
            # 删除的资源发送kafka消息
            comment_event_message(topic=RESOURCE_DELETED_TOPIC, request_id=str(uuid.uuid4()),
                                  resource_id=database_need_valid.uuid)


def choose_available_databases_instance(instance_info_list, auth_status: bool, auth_instance):
    # auth_status: ture 已认证  false 未认证 ，第一次注册auth_status默认false
    if len(instance_info_list) == 1:
        return instance_info_list[0]

    instance_info_list_uuid = list(instance_info for instance_info in instance_info_list if instance_info['dbUUID'])

    if len(instance_info_list_uuid) == 1:
        return instance_info_list_uuid[0]

    if not auth_status:
        # 未认证
        instance_auth_type_list = list(instance_info['authType'] for instance_info in instance_info_list)
        if AuthType.OS_OFF.value in instance_auth_type_list:
            # 取是否在线，如果一个在线，则取在线一个，否则取离线
            if ResourceStatus.ON_LINE.value in list(instance_info['state'] for instance_info in instance_info_list):
                instance_info_list[0]['state'] = ResourceStatus.ON_LINE.value
            else:
                instance_info_list[0]['state'] = ResourceStatus.OFF_LINE.value

            instance_info_list[0]['instName'] = ",".join(
                list(instance_info['instName'] for instance_info in instance_info_list))
            log.info('auth auth_status=false')
            return instance_info_list[0]
        log.error(f'auth_status=false,db_info:{instance_info_list}')
    else:
        # 已认证
        instance_info_list_exit = list(
            instance_info for instance_info in instance_info_list if instance_info['instName'] == auth_instance)
        if len(instance_info_list_exit) == 1:
            if ResourceStatus.ON_LINE.value in list(
                    instance_info['state'] for instance_info in instance_info_list_exit):
                instance_info_list_exit[0]['state'] = ResourceStatus.ON_LINE.value
            else:
                instance_info_list_exit[0]['state'] = ResourceStatus.OFF_LINE.value
            log.info('auth auth_status=true')
            return instance_info_list_exit[0]
        else:
            # 多个实例，数据库认证后更改实例名称，更改为离线，下次轮询修正
            log.info(f'the instance name has modifies {auth_instance} to {str(instance_info_list)}')
            instance_info_list[0]['state'] = ResourceStatus.ON_LINE.value
            return instance_info_list[0]
    return DEFAULT_ENVIRONMENT_RES


def has_add_database_resource_topic(new_exist_db, exist_dbs_dict):
    # 不可用变为可用
    for exist_db_name in exist_dbs_dict:
        if new_exist_db.name == exist_db_name and \
                new_exist_db.valid is True and \
                exist_dbs_dict[exist_db_name] is False:
            request_id = str(uuid4())
            comment_event_message(topic=ResourceAddedRequest.default_topic, request_id=request_id,
                                  resource_id=new_exist_db.uuid)
            log.info(f'send topic: resource.added request_id：{request_id} resource_id：{new_exist_db.uuid}')


@clean_except()
def create_dme_database(env, params, asm, database_model: dict):
    # 是否使用ASM存储 0-否 1-是，兼容传统容灾接口，备份场景需要通过tcp的接口查询数据库是否是ASM存储
    with database.session() as db:
        filters = {DatabaseTable.type.in_([ResourceTypeEnum.Database.value]),
                   DatabaseTable.root_uuid == str(env['uuid']),
                   DatabaseTable.name == database_model['dbName']}
        exist_dbs = db.query(DatabaseTable).filter(*filters).all()
        if exist_dbs:
            # 更新
            exist_db = exist_dbs[0]
            verify_status = exist_db.verify_status
            # 数据库认证，且上一次认证为os认证
            if database_model['authType'] == AuthType.OS_OFF.value and \
                    exist_db.auth_type != database_model['authType']:
                verify_status = False
            # 数据库离线，数据库未认证
            if database_model['state'] == ResourceStatus.OFF_LINE.value:
                verify_status = False
            if database_model['authType'] == AuthType.OS_ON.value:
                verify_status = True

            # 查看集群主机是否生成
            filters = {EnvironmentTable.children_uuids.any(env['uuid']),
                       EnvironmentTable.type == ResourceTypeEnum.Host.value}
            clusters_host = db.query(EnvironmentTable).filter(*filters).all()

            valid = True
            if len(clusters_host) > 0:
                valid = False

            exist_db.path = env['endpoint']
            exist_db.link_status = database_model['state']
            exist_db.parent_name = params.name
            exist_db.verify_status = verify_status
            exist_db.auth_type = database_model['authType']
            exist_db.db_role = database_model['dbRole']
            exist_db.inst_name = database_model['instName']
            exist_db.is_asminst = database_model['isAsmInst']
            exist_db.version = database_model['version']
            exist_db.valid = valid
            exist_db.asm = asm
            exist_db.type = ResourceTypeEnum.Database.value
            exist_db.sub_type = ResourceSubTypeEnum.Oracle.value
            db.merge(exist_db)
            db.commit()
        else:
            # 新增
            verify_status = False
            if database_model['authType'] == AuthType.OS_ON.value:
                verify_status = True
            new_item = create_new_database_table_oracle(
                database_model, verify_status, env, params, asm)
            new_item = DatabaseTable(**new_item)

            # 检查资源规格是否超出规格上限
            check_resource_exceeds_limit(new_item.uuid)

            db.add(new_item)
            db.commit()


def create_host_auth(db, env, params, asm_info):
    asm_name = None
    if asm_info is not None and len(asm_info) > 0:
        asm_name = asm_info[0].get('instName')
        asm = json.dumps(asm_info)
    else:
        asm = None

    host_asm_type = None
    # 保存主机ASM信息
    filters = {AsmInfoTable.type.in_([ResourceType.Asm.value]),
               AsmInfoTable.root_uuid == str(env['uuid'])}
    exist_asms = db.query(AsmInfoTable).filter(*filters).all()
    if asm_name:
        # 主机鉴权信息
        if asm_info[0].get('authType') == "0":
            host_asm_type = 0
        if asm_info[0].get('authType') == "1":
            host_asm_type = 1
        if len(exist_asms) > 0:
            # 更新asm
            exist_asm = exist_asms[0]
            host_verify_status = exist_asm.verify_status
            if host_asm_type == 0 and exist_asm.auth_type == 1:
                host_verify_status = False
            if host_asm_type == 1:
                host_verify_status = True
            exist_asm.name = ResourceType.Asm.value
            exist_asm.asm_instances = asm_name
            exist_asm.asm = asm
            exist_asm.auth_type = host_asm_type
            exist_asm.verify_status = host_verify_status
            db.merge(exist_asm)
        else:
            # 新增asm
            host_verify_status = False
            if host_asm_type == 1:
                host_verify_status = True
            new_item = create_new_auto_storage_management_table(
                asm_name, host_verify_status, env, params, asm, host_asm_type)
            new_item = AsmInfoTable(**new_item)
            db.add(new_item)

    if asm is None and len(exist_asms) > 0:
        exist_asm = exist_asms[0]
        exist_asm.verify_status = False
        db.merge(exist_asm)
    db.commit()

    return asm


def create_new_database_table_oracle(database_model: dict, verify_status: bool, env, params, asm):
    uuid_params = env['uuid'] + database_model['dbName']
    database_uuid = get_id_by_condition(uuid_params)

    return {'uuid': database_uuid,
            'name': str(database_model['dbName']),
            'path': str(env['endpoint']),
            'type': ResourceTypeEnum.Database.value,
            'link_status': database_model['state'],
            'verify_status': verify_status,
            'parent_uuid': str(env['uuid']),
            'root_uuid': str(env['uuid']),
            'parent_name': params.name,
            'auth_type': database_model['authType'],
            'db_role': database_model['dbRole'],
            'inst_name': database_model['instName'],
            'is_asminst': database_model['isAsmInst'],
            'version': database_model['version'],
            'asm': asm,
            'sub_type': ResourceSubTypeEnum.Oracle.value}


def create_new_auto_storage_management_table(asm_name, host_verify_status, env, params, asm, host_asm_type):
    return {'uuid': str(uuid4()),
            'name': ResourceType.Asm.value,
            'asm_instances': str(asm_name),
            'type': ResourceType.Asm.value,
            'auth_type': host_asm_type,
            'verify_status': host_verify_status,
            'parent_uuid': str(env['uuid']),
            'root_uuid': str(env['uuid']),
            'parent_name': params.name,
            'asm': asm}


def check_nodes(cluster_hosts: EnvironmentTable):
    # 构建集群成功后清除主机的授权
    with database.session() as db:
        for cluster_host in cluster_hosts:
            cluster_host.authorized_user = None
            cluster_host.user_id = None
            db.merge(cluster_host)


def get_id_by_condition(params: str):
    uuid.NAMESPACE_DNS = UUID('6ba7b810-9dad-11d1-80b4-00c04fd430c8')
    uuid_str = uuid.uuid3(uuid.NAMESPACE_DNS, params)
    return str(uuid_str)


def update_environment_oracle_cluster(db, exist_cluster, host_nodes, link_status, cluster_ip):
    cluster_id = exist_cluster.uuid
    exist_cluster.children_uuids = host_nodes
    exist_cluster.link_status = link_status
    exist_cluster.endpoint = cluster_ip
    exist_cluster.path = cluster_ip
    db.merge(exist_cluster)
    db.commit()
    return exist_cluster, cluster_id


def update_cluster_resource(db, exist_cluster, real_db):
    filters = {DatabaseTable.sub_type.in_([ResourceSubTypeEnum.Oracle.value]),
               DatabaseTable.root_uuid == str(exist_cluster.uuid)}
    exist_dbs = db.query(DatabaseTable).filter(*filters).all()
    exist_db_name = []
    exist_dbs_dict = {}
    for exist_db in exist_dbs:
        exist_db_name.append(exist_db.name)
        exist_dbs_dict[exist_db.name] = exist_db.valid

    deal_cluster_resource(exist_cluster, real_db)

    # 再查一次判断是否新增进去
    new_exist_dbs = db.query(DatabaseTable).filter(*filters).all()
    for new_exist_db in new_exist_dbs:
        has_add_database_resource_topic(new_exist_db, exist_dbs_dict)
        if new_exist_db.name not in exist_db_name:
            # 新增资源发送 kafa
            request_id = str(uuid4())
            comment_event_message(topic=ResourceAddedRequest.default_topic, request_id=request_id,
                                  resource_id=new_exist_db.uuid)
            log.info(f'send topic: resource.added request_id：{request_id} resource_id：{new_exist_db.uuid}')


def create_cluster_info(host, oracle_clusterinfo):
    cluster_type = ResourceTypeEnum.Host.value
    host_uuid = host['uuid']
    cluster_name = oracle_clusterinfo['ClusterName']
    nodes = oracle_clusterinfo['Nodes']
    cluster_ip = oracle_clusterinfo['ClusterIP']
    cluster_info_json_str = json.dumps(oracle_clusterinfo)
    return cluster_type, host_uuid, cluster_name, nodes, cluster_ip, cluster_info_json_str


@clean_except()
def refresh_database_cluster(host, params, applications, oracle_clusterinfo, real_db):
    if str(params.sub_type.value) != str(ResourceSubTypeEnum.DBBackupAgent.value):
        return

    cluster_type, host_uuid, cluster_name, nodes, cluster_ip, cluster_info_json_str = \
        create_cluster_info(host, oracle_clusterinfo)

    with database.session() as db:
        oracle_clusterinfo = create_new_cluster_node_table(host_uuid, cluster_name, params, cluster_type,
                                                           cluster_info_json_str)
        oracle_clusterinfo = ClusterNodeTable(**oracle_clusterinfo)
        db.merge(oracle_clusterinfo)
        db.commit()

    with database.session() as db:
        exist_cluster_infos = db.query(ClusterNodeTable). \
            filter(ClusterNodeTable.cluster_info == cluster_info_json_str).all()

        if len(exist_cluster_infos) >= len(nodes):
            host_nodes = []
            for exist_cluster_info in exist_cluster_infos:
                host_nodes.append(exist_cluster_info.host_uuid)

            cluster_hosts = db.query(EnvironmentTable). \
                filter(EnvironmentTable.uuid.in_(host_nodes)).order_by(EnvironmentTable.uuid.desc()).all()
            if len(cluster_hosts) != len(nodes):
                log.error(f'cluster_hosts={cluster_hosts} not equals nodes={nodes}')
                return

            # 集群主机有一个在线，则集群在线
            link_status = str(HostOnlineStatus.OFF_LINE.value)
            cluster_host_node_uuids = ''
            link_status_nodes = []
            for cluster_host in cluster_hosts:
                cluster_host_node_uuids += cluster_host.uuid
                link_status_nodes.append(cluster_host.link_status)

            if str(HostOnlineStatus.ON_LINE.value) in link_status_nodes:
                link_status = str(HostOnlineStatus.ON_LINE.value)

            exist_cluster = check_exited_cluster(cluster_info_json_str)

            if not exist_cluster:
                # 固定集群uuid,根据两个主机id生成
                cluster_id = get_id_by_condition(cluster_host_node_uuids)
                # 检查资源规格是否超出规格上限
                check_resource_exceeds_limit(cluster_id)
                exist_cluster = create_new_environment_table_item(cluster_id, cluster_name, params, link_status, host,
                                                                  cluster_type, cluster_ip, host_nodes)
                db.add(exist_cluster)
                db.commit()
                # 检查节点信息，1.将已授权的节点回收给管理员
                check_nodes(cluster_hosts)

            else:
                exist_cluster, cluster_id = update_environment_oracle_cluster(
                    db, exist_cluster, host_nodes, link_status, cluster_ip)

            cluster_info = {
                'name': cluster_name,
                'uuid': cluster_id,
                'applications': applications
            }
            # 更新集群应用信息
            update_application(host_uuid, params, cluster_info=cluster_info)

            # 更新集群数据库信息
            update_cluster_resource(db, exist_cluster, real_db)


def check_exited_cluster(cluster_info_json_str):
    # 集群下节点如果已经形成了在pm组成了集群则更改已组成的集群，否则新增。
    with database.session() as db:
        node_ip_list = list(node_info['NodeIP'] for node_info in json.loads(cluster_info_json_str)['Nodes'])
        rule1_list = list(ClusterNodeTable.cluster_info.like(f'%"{node_ip}"%') for node_ip in node_ip_list)
        rule2_list = list(ClusterNodeTable.cluster_info.like(f"%'{node_ip}'%") for node_ip in node_ip_list)
        rule = rule1_list + rule2_list
        node_envs = db.query(ClusterNodeTable).filter(or_(*rule)).all()
        for node_env in node_envs:
            filters = {
                EnvironmentTable.is_cluster == true(),
                EnvironmentTable.sub_type == ResourceSubTypeEnum.DBBackupAgent,
                EnvironmentTable.children_uuids.any(node_env.host_uuid)
            }
            exited_cluster = db.query(EnvironmentTable).filter(*filters).one_or_none()
            if exited_cluster:
                return exited_cluster
    return DEFAULT_ENVIRONMENT_RES


def create_new_cluster_node_table(host_uuid, cluster_name, params, cluster_type, cluster_info_json_str):
    return {
        'host_uuid': host_uuid,
        'cluster_name': cluster_name,
        'env_type': params.sub_type.value,
        'type': cluster_type,
        'cluster_info': cluster_info_json_str
    }


def create_new_environment_table_item(cluster_id, cluster_name, params, link_status, host, cluster_type,
                                      cluster_ip, host_nodes):
    return EnvironmentTable(uuid=cluster_id, name=cluster_name, sub_type=params.sub_type.value,
                            link_status=link_status, os_type=host['os_type'],
                            type=cluster_type, endpoint=cluster_ip,
                            is_cluster=True, children_uuids=host_nodes, parent_uuid=cluster_id,
                            root_uuid=cluster_id, path=cluster_ip)


def deal_cluster_resource(cluster: EnvironmentTable, real_db):
    with database.session() as db:
        filters = {DatabaseTable.root_uuid.in_(cluster.children_uuids)}
        exist_dbs = db.query(DatabaseTable).filter(*filters).all()
        db_map = {}
        for exist_db in exist_dbs:
            # 主机的数据库全都设置无效，不要在界面展示
            exist_db.valid = False
            db.merge(exist_db)
            db_list = db_map.get(exist_db.name)
            if not db_list:
                db_list = []
                db_map[exist_db.name] = db_list
            db_list.append(exist_db)

        # 集群数据库不在当前列表中，则设置离线
        filters = {DatabaseTable.root_uuid == cluster.uuid}
        tmp_dbs = db.query(DatabaseTable).filter(*filters).all()
        if len(tmp_dbs) > 0:
            for db_tmp in tmp_dbs:
                if db_tmp.name not in db_map:
                    db_tmp.link_status = ResourceStatus.OFF_LINE.value
                    db.merge(db_tmp)
        # 如果agent上报信息不存在数据了，则致离线数据库为不可现。
        if len(tmp_dbs) > 0:
            for db_tmp in tmp_dbs[:]:
                if db_tmp.name not in real_db:
                    db_tmp.link_status = ResourceStatus.OFF_LINE.value
                    db_tmp.valid = False
                    db.merge(db_tmp)
                    # 删除的资源发送kafka消息
                    comment_event_message(topic=RESOURCE_DELETED_TOPIC, request_id=str(uuid.uuid4()),
                                          resource_id=db_tmp.uuid)

        # 节点存在数据库认证，以数据库认证为准，存在数据库未认证以未认证为准。
        for db_name in db_map:
            db_list = db_map.get(db_name)
            refresh_cluster_database(cluster, db, db_list, db_name, real_db)

        refresh_cluster_automatic_storage_management(cluster, db)


def refresh_cluster_database(cluster, db, db_list, db_name, real_db):
    verify_status = False
    link_status = ResourceStatus.OFF_LINE.value
    asm = []
    auth_type = AuthType.OS_ON.value
    db_auth_verified = False

    inst_name_str = ','.join(list(node.inst_name for node in db_list if node.inst_name))
    cluster_inst_name = ','.join(set(inst_name_str.split(',')))
    auth_type_nodes = list(node.auth_type for node in db_list)
    verify_status_nodes = list(node.verify_status for node in db_list)
    for db_tmp in db_list:
        if db_tmp.asm is not None:
            asm_temps = json.loads(db_tmp.asm)
            for asm_temp in asm_temps:
                asm.append(asm_temp)

        if db_tmp.link_status == ResourceStatus.OFF_LINE.value:
            continue
        link_status = ResourceStatus.ON_LINE.value
        if db_auth_verified:
            continue
        auth_type, db_auth_verified, verify_status = cluster_auth_is_os_off(auth_type, auth_type_nodes,
                                                                            db_auth_verified, db_tmp, verify_status,
                                                                            verify_status_nodes)
    if len(asm) <= 0:
        asm = None
    else:
        asm = json.dumps(asm)
    tmp_dbs = db.query(DatabaseTable).filter(DatabaseTable.root_uuid == cluster.uuid,
                                             DatabaseTable.name == db_name).all()
    if len(tmp_dbs) > 0:
        update_cluster_database(asm, auth_type, cluster_inst_name, db, link_status, real_db, tmp_dbs, verify_status)
    else:
        create_new_cluster_database(asm, cluster, cluster_inst_name, db, db_list, db_name, link_status, verify_status)


def cluster_auth_is_os_off(auth_type, auth_type_nodes, db_auth_verified, db_tmp, verify_status, verify_status_nodes):
    if db_tmp.auth_type == AuthType.OS_OFF.value:
        auth_type = AuthType.OS_OFF.value
        # 存在数据库已认证则集群认证成功
        if True in verify_status_nodes and AuthType.OS_ON.value not in auth_type_nodes:
            verify_status = True
        else:
            verify_status = db_tmp.verify_status
        db_auth_verified = verify_status
    else:
        verify_status = True
    return auth_type, db_auth_verified, verify_status


def update_cluster_database(asm, auth_type, cluster_inst_name, db, link_status, real_db, tmp_dbs, verify_status):
    cluster_db = tmp_dbs[0]
    # 只要agent上报节点的数据库，则可显示对应集群数据库
    valid = False
    if cluster_db.name in real_db:
        valid = True
    cluster_db.link_status = link_status
    cluster_db.asm = asm
    cluster_db.verify_status = verify_status
    cluster_db.auth_type = auth_type
    cluster_db.valid = valid
    cluster_db.inst_name = cluster_inst_name,
    db.merge(cluster_db)


def create_new_cluster_database(asm, cluster, cluster_inst_name, db, db_list, db_name, link_status, verify_status):
    uuid_params = cluster.uuid + db_name
    cluster_db_id = get_id_by_condition(uuid_params)

    # 检查资源规格是否超出规格上限
    check_resource_exceeds_limit(str(cluster_db_id))

    cluster_db = DatabaseTable(uuid=str(cluster_db_id),
                               name=db_name,
                               verify_status=verify_status,
                               parent_name=cluster.name,
                               root_uuid=cluster.uuid,
                               parent_uuid=cluster.uuid,
                               link_status=link_status,
                               type=db_list[0].type,
                               auth_type=db_list[0].auth_type,
                               db_role=db_list[0].db_role,
                               inst_name=cluster_inst_name,
                               is_asminst=db_list[0].is_asminst,
                               sub_type=db_list[0].sub_type,
                               asm=asm,
                               version=db_list[0].version,
                               path=cluster.endpoint)
    db.add(cluster_db)


def refresh_cluster_automatic_storage_management(cluster, db):
    # 集群ASM, 根据节点asm信息更新集群
    filters = {AsmInfoTable.root_uuid.in_(cluster.children_uuids)}
    exist_asms = db.query(AsmInfoTable).filter(*filters).all()
    if len(exist_asms) > 0:
        asm_names = set()
        asm_list = []
        auth_type_nodes = 1
        auth_status_nodes = True
        for exist_asm in exist_asms:
            # 节点数据库存在未认证则集群未认证
            if exist_asm.auth_type == AuthType.OS_OFF.value and exist_asm.verify_status is False:
                auth_status_nodes = False
            if exist_asm.auth_type == AuthType.OS_OFF.value:
                # 存在数据库认证则取数据认证
                auth_type_nodes = AuthType.OS_OFF.value
            asm_names.add(exist_asm.asm_instances)
            asm_list.extend(json.loads(exist_asm.asm))

        asm_names = ','.join(asm_names)
        asm_list_str = json.dumps(asm_list)
        filters = {AsmInfoTable.type.in_([ResourceType.Asm.value]),
                   AsmInfoTable.root_uuid == cluster.uuid}
        exist_asms = db.query(AsmInfoTable).filter(*filters).all()

        if len(exist_asms) > 0:
            exist_asm = exist_asms[0]

            exist_asm.name = ResourceType.Asm.value
            exist_asm.asm_instances = asm_names
            exist_asm.asm = asm_list_str
            exist_asm.auth_type = auth_type_nodes
            exist_asm.verify_status = auth_status_nodes
            db.merge(exist_asm)
        else:
            new_item = {'uuid': str(uuid4()),
                        'name': ResourceType.Asm.value,
                        'asm_instances': str(asm_names),
                        'type': ResourceType.Asm.value,
                        'verify_status': auth_status_nodes,
                        'parent_uuid': str(cluster.uuid),
                        'root_uuid': str(cluster.uuid),
                        'parent_name': cluster.name,
                        'asm': asm_list_str,
                        'auth_type': auth_type_nodes}

            new_item = AsmInfoTable(**new_item)
            db.add(new_item)
    # 提交
    db.commit()


def update_databases_hosts_all_offline(host_id: str):
    try:
        with database.session() as db:
            # 置主机状态为离线
            host = db.query(EnvironmentTable).filter(
                EnvironmentTable.uuid == host_id).one()
            host.link_status = HostOnlineStatus.OFF_LINE.value
            db.merge(host)
            # 设置sam离线
            filters = {AsmInfoTable.type.in_([ResourceType.Asm.value]), AsmInfoTable.root_uuid == host_id}
            exist_asm = db.query(AsmInfoTable).filter(*filters).one_or_none()
            if exist_asm:
                exist_asm.verify_status = False
                db.merge(exist_asm)
            db.commit()
        with database.session() as db:
            # 置数据库状态为离线
            databases = db.query(DatabaseTable).filter(
                DatabaseTable.root_uuid == host_id).all()
            update_databases_offline(databases, db)
            db.commit()
        with database.session() as db:
            # 更新集群状态为离线
            cluster_node = db.query(ClusterNodeTable).filter(
                ClusterNodeTable.host_uuid == host_id).one_or_none()
            if cluster_node is None:
                return
            update_cluster_offline(cluster_node.cluster_info)
    except Exception as ex:
        log.error("refresh host status error with{" + host_id + "}.")
        raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR) from ex
    finally:
        pass


def update_host_online(host_id: str):
    try:
        with database.session() as db:
            # 置主机状态为在线
            host = db.query(EnvironmentTable).filter(
                EnvironmentTable.uuid == host_id).one()
            host.link_status = HostOnlineStatus.ON_LINE.value
            db.merge(host)
            db.commit()
    except Exception as ex:
        log.error("refresh host status error with{" + host_id + "}.")
        raise EmeiStorBizException(CommonErrorCodes.STATUS_ERROR) from ex
    finally:
        pass


def update_databases_offline(databases, db):
    for database_need_offline in databases:
        database_need_offline.link_status = ResourceStatus.OFF_LINE.value
        database_need_offline.verify_status = False
        db.merge(database_need_offline)


def update_cluster_offline(cluster_info):
    with database.session() as db:
        cluster_info_dict = json.loads(cluster_info)
        cluster_ip = cluster_info_dict['ClusterIP']
        for cluster_node in cluster_info_dict['Nodes']:
            host_info = db.query(EnvironmentTable).filter(
                EnvironmentTable.endpoint == cluster_node['NodeIP']).one_or_none()
            if host_info is None:
                return
            if host_info.link_status == str(HostOnlineStatus.ON_LINE.value):
                return
        # 更新集群为离线
        cluster = db.query(EnvironmentTable).filter(
            EnvironmentTable.endpoint == cluster_ip).one_or_none()
        if cluster is None:
            return
        cluster.link_status = str(HostOnlineStatus.OFF_LINE.value)
        # 更新集群数据库离线
        databases = db.query(DatabaseTable).filter(DatabaseTable.root_uuid == cluster.uuid).all()
        for database_need_offline in databases:
            database_need_offline.link_status = ResourceStatus.OFF_LINE.value
            database_need_offline.verify_status = False
            db.merge(database_need_offline)
        # 更新集群asm为离线
        filters = {AsmInfoTable.type.in_(
            [ResourceType.Asm.value]), AsmInfoTable.root_uuid == cluster.uuid}
        exist_asm = db.query(AsmInfoTable).filter(*filters).one_or_none()
        if exist_asm:
            exist_asm.verify_status = False
            db.merge(exist_asm)
        db.merge(cluster)
        db.commit()

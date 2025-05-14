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
import urllib.parse
import uuid
from ipaddress import ip_address
from itertools import groupby
from typing import List

from redis.exceptions import LockError

import app.protection.object.db as curd
from app.backup.client.rbac_client import RBACClient
from app.backup.client.resource_client import ResourceClient
from app.base.consts import WORKING_STATUS_LIST, CAN_NOT_REMOVE_PROTECT_TYPES
from app.base.db_base import database
from app.common import logger
from app.common.clients.alarm.alarm_client import AlarmClient
from app.common.clients.alarm.alarm_schemas import SendAlarmReq
from app.common.clients.system_base_client import SystemBaseClient
from app.common.concurrency import DEFAULT_ASYNC_POOL
from app.common.enums.alarm_enum import AlarmSourceType
from app.common.enums.rbac_enum import ResourceSetTypeEnum, ResourceSetScopeModuleEnum
from app.common.enums.resource_enum import ResourceSubTypeEnum, ProtectionStatusEnum
from app.common.event_messages.Resource.resource import ResourceAddedRequest
from app.common.events import producer
from app.common.events.topics import RESOURCE_DELETED_TOPIC
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException, IllegalParamException
from app.common.lock.lock import Lock
from app.common.lock.lock_manager import lock_manager
from app.common.log.event_client import EventClient
from app.common.log.event_schemas import SendEventReq, LogRank
from app.common.rpc.system_base_rpc import encrypt
from app.common.schemas.resource_set_relation_schemas import ResourceSetRelationInfo
from app.common.util.cleaner import clear
from app.protection.object.client.job_client import JobClient
from app.protection.object.common.constants import CommonOperationID
from app.protection.object.common.protection_enums import FilterType, FilterColumn, FilterMode, ResourceFilter, \
    FilterRule, SlaApplyType
from app.protection.object.schemas.protected_object import BatchProtectionSubmitReq, ModifyProtectionSubmitReq, \
    ProtectedObjectId
from app.protection.object.service.batch_protection_service import BatchProtectionService
from app.resource.common.constants import EnvironmentRemoveConstants, VMWareCertConstants, VMWareScanConstants, \
    ResourceConstants
from app.resource.discovery.discovery_plugin import DiscoveryPlugin
from app.resource.discovery.res_discovery_plugin import delete_schedule_task, add_schedule_task
from app.resource.models.resource_group_models import ResourceGroup, ResourceGroupMember
from app.resource.models.resource_models import EnvironmentTable, ResExtendInfoTable
from app.resource.models.resource_models import ResourceTable
from app.resource.models.virtual_res_models import VirtualResourceTable
from app.resource.rpc import hw_agent_rpc
from app.resource.schemas.env_schemas import ScanEnvSchema, UpdateEnvSchema
from app.resource.service.common import resource_service
from app.resource.service.common.resource_service import comment_event_message, query_environment
from app.resource.service.host.host_service import check_and_limit_resources
from app.resource.service.vmware.service_instance_manager import service_instance_manager
from app.resource.service.vmware.storage_cache_manager import build_storage_service
from app.resource.service.vmware.storage_netapp_service import StorageNetAppService
from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService, get_vir_domain_id_list

SERVICES = [
    ResourceSubTypeEnum.VMware, ResourceSubTypeEnum.vCenter,
    ResourceSubTypeEnum.ESX, ResourceSubTypeEnum.ESXi
]
log = logger.get_logger(__name__)


def _check_env_params_endpoint(params: ScanEnvSchema):
    # 参数校验
    if not params.endpoint:
        raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                    ["scan_env", "params", "end_point"])
    else:
        try:
            ip_address(params.endpoint)
        except Exception as ex:
            log.exception(f"error : {ex}")
            raise IllegalParamException(CommonErrorCodes.ILLEGAL_PARAMS,
                                        ["scan_env", "params", "end_point"]) from ex
        finally:
            pass


def _check_resource_duplicate(resource_list: List[str], check_env_id: str = None):
    # 两条分支 一 注册而来 二 重新扫描而来
    if not resource_list:
        return
    with database.session() as session:
        check_result = session.query(ResourceTable).filter(ResourceTable.uuid.in_(resource_list)).all()
        if check_env_id is None:
            message = "Cannot register environment because there has registed host info."
            if check_result and len(check_result) > 0:
                raise EmeiStorBizException(ResourceErrorCodes.VSPHERE_HOST_DUPLICATE,
                                           check_result[0].name, check_result[0].env_ip, message=message)
        else:
            check_env = resource_service.query_resource_by_id(check_env_id)
            message = "Cannot refresh environment because there has registed host info."
            if check_result and len(check_result) > 0:
                exist_env = resource_service.query_resource_by_id(
                    check_result[0].root_uuid)
                raise EmeiStorBizException(ResourceErrorCodes.VSPHERE_HOST_DUPLICATE,
                                           check_result[0].parent_name,
                                           (check_env.endpoint, exist_env.endpoint), message=message)


def _build_vm_ware_environment(env_uuid, params: ScanEnvSchema, content):
    # vmware注册信息构造
    user_info = SystemBaseClient.query_user(params.user_id)
    authorized_user = None
    if user_info is not None:
        authorized_user = user_info.get("userName")
    env = {
        "uuid": env_uuid,
        "user_id": params.user_id,
        "type": params.type.value,
        "name": params.name,
        "sub_type": content.about.name,
        "path": params.endpoint,
        "endpoint": params.endpoint,
        "port": params.port,
        "user_name": params.user_name,
        "password": params.password,
        "link_status": 1,
        "version": content.about.version,
        "authorized_user": authorized_user
    }
    if "cert_name" in params.extend_context:
        env["cert_name"] = params.extend_context["cert_name"]
    return env


# 检验str是否满足正则表达式 name_pattern
def match_pattern(name_pattern, name_str):
    res = re.search(name_pattern, name_str)
    if res:
        return True
    else:
        return False


def add_rbac_relation(table, domain_id: str):
    resource_set_relation_info = ResourceSetRelationInfo(resource_object_id=table.uuid,
                                                         resource_set_type=ResourceSetTypeEnum.RESOURCE.value,
                                                         scope_module=ResourceSetScopeModuleEnum.VMWARE.value,
                                                         domain_id_list=get_vir_domain_id_list(table.parent_uuid,
                                                                                               domain_id=domain_id),
                                                         parent_uuid=table.parent_uuid,
                                                         sub_type=ResourceSubTypeEnum.VirtualMachine.value)
    RBACClient.add_resource_set_relation(resource_set_relation_info)


def _delete_resource_group(db, resource_group_id: str):
    db.query(ResourceGroupMember).filter(ResourceGroupMember.resource_group_id == resource_group_id).delete(
        synchronize_session=False)
    db.query(ResourceGroup).filter(ResourceGroup.uuid == resource_group_id).delete(synchronize_session=False)
    log.info(f"Finished deleting resource group with id {resource_group_id} and its members.")


def _delete_resource_groups_of_env(db, env_id: str):
    resource_groups = db.query(ResourceGroup).filter(ResourceGroup.scope_resource_id == env_id).all()
    if resource_groups is None:
        return
    for resource_group in resource_groups:
        _delete_resource_group(db, resource_group.uuid)
    log.info(f"Finished deleting all resource groups of environment {env_id} and their members.")


class VMwareDiscoveryPlugin(DiscoveryPlugin):
    service_type = ResourceSubTypeEnum.VMware

    @staticmethod
    def delete_vmware_check(db, vm_ids):
        # 校验是否存在vmware中是否存在保护对象 如果存在则不能删除
        protected_vmware = curd.projected_object.query_by_resource_ids(
            db=db, resource_ids=vm_ids)
        if len(protected_vmware) > 0:
            message = f"Cannot delete environment because there is vmware has protected."
            raise EmeiStorBizException(
                ResourceErrorCodes.VCENTER_RESOURCE_IS_PROTECTED, message=message)

    @staticmethod
    def remove_protect_and_send_running_event(env_name, vm_resource, parent_protect_obj, tags, tag_filter_str):
        overwrite = parent_protect_obj.get("ext_parameters", {}).get("overwrite")
        if not overwrite:
            return
        sla_name = parent_protect_obj.get("sla_name")
        uuid_list = [vm_resource.uuid]
        count = JobClient.count_job(uuid_list, WORKING_STATUS_LIST, CAN_NOT_REMOVE_PROTECT_TYPES)
        # 如果存在则不让移除保护
        if count and count > 0:
            log.error(f"Can not remove protect, because backup task is working. resource id: {vm_resource.uuid}")
        else:
            try:
                with database.session() as session:
                    BatchProtectionService.batch_remove_protection(session=session, resource_ids=[vm_resource.uuid])
                timestamp = int(time.time())
                vm_uuid = vm_resource.uuid
                vm_name = vm_resource.name
                params = [env_name, vm_uuid, vm_name, urllib.parse.unquote(tags), sla_name, tag_filter_str]
                event_id = CommonOperationID.AUTO_SCAN_AND_REMOVE_PROTECT
                EventClient.send_running_event(SendEventReq(
                    userId=vm_resource.user_id,
                    eventId=event_id,
                    eventParam=params,
                    eventTime=timestamp,
                    eventLevel=LogRank.INFO.value,
                    sourceId=event_id,
                    resourceId=vm_resource.uuid,
                    sourceType=AlarmSourceType.PROTECTION,
                    eventSequence=timestamp,
                    isSuccess=False
                ))
            except Exception:
                log.error(f"Remove protect failed, resource id: {vm_resource.uuid}")

    @staticmethod
    def is_vm_type(tag_filter):
        return tag_filter.get("type") == FilterType.VM

    @staticmethod
    def is_name_filter(tag_filter):
        return tag_filter.get("filter_by") == FilterColumn.NAME

    @staticmethod
    def is_tag_filter(tag_filter):
        return tag_filter.get("filter_by") == FilterColumn.TAG

    @staticmethod
    def build_create_protect_request(sla_id, vm_resource, parent_protect_obj):
        filter_data = ResourceFilter(
            filter_by=FilterColumn.ID,
            type=FilterType.DISK,
            rule=FilterRule.ALL,
            mode=FilterMode.INCLUDE,
            values=["*"]
        )
        resource_data = {
            "resource_id": vm_resource.uuid,
            "filters": [filter_data]
        }
        ext_parameters = parent_protect_obj.get("ext_parameters", {})
        ext_param_data = {
            "pre_script": ext_parameters.get("pre_script"),
            "post_script": ext_parameters.get("post_script"),
            "backup_res_auto_index": ext_parameters.get("backup_res_auto_index"),
            "enable_security_archive": ext_parameters.get("enable_security_archive"),
            "host_list": ext_parameters.get("host_list"),
            "worm_switch": ext_parameters.get("worm_switch"),
            "failed_node_esn": None
        }
        batch_create_req = BatchProtectionSubmitReq(
            sla_id=sla_id,
            resources=[resource_data],
            ext_parameters=ext_param_data,
            name=vm_resource.name
        )
        return batch_create_req

    @staticmethod
    def build_create_group_protect_request(sla_id, resource_group_id, vm_resource, parent_protect_obj):
        filter_data = ResourceFilter(
            filter_by=FilterColumn.ID,
            type=FilterType.DISK,
            rule=FilterRule.ALL,
            mode=FilterMode.INCLUDE,
            values=["*"]
        )
        resource_data = {
            "resource_id": vm_resource.uuid,
            "filters": [filter_data]
        }
        ext_parameters = parent_protect_obj.get("ext_parameters", {})
        ext_param_data = {
            "pre_script": ext_parameters.get("pre_script"),
            "post_script": ext_parameters.get("post_script"),
            "backup_res_auto_index": ext_parameters.get("backup_res_auto_index"),
            "enable_security_archive": ext_parameters.get("enable_security_archive"),
            "host_list": ext_parameters.get("host_list"),
            "worm_switch": ext_parameters.get("worm_switch"),
            "failed_node_esn": None
        }
        batch_create_req = BatchProtectionSubmitReq(
            sla_id=sla_id,
            resource_group_id=resource_group_id,
            resources=[resource_data],
            ext_parameters=ext_param_data,
            name=vm_resource.name
        )
        return batch_create_req

    @staticmethod
    def build_modify_protect_request(new_sla_id, vm_resource, parent_protect_obj):
        parent_ext_parameters = parent_protect_obj.get("ext_parameters", {})
        self_protect_obj = ResourceClient.query_protected_object(resource_id=vm_resource.uuid)
        self_ext_parameters = self_protect_obj.get("ext_parameters", {})
        ext_param_data = {
            # 以下字段都使用父资源SLA相同的扩展参数信息
            "pre_script": parent_ext_parameters.get("pre_script"),
            "post_script": parent_ext_parameters.get("post_script"),
            "host_list": parent_ext_parameters.get("host_list"),
            "backup_res_auto_index": parent_ext_parameters.get("backup_res_auto_index"),
            "enable_security_archive": parent_ext_parameters.get("enable_security_archive"),
            "worm_switch": parent_ext_parameters.get("worm_switch"),
            # 以下字段使用虚拟机自己之前保护的扩展参数中相同的扩展参数信息
            "first_backup_esn": self_ext_parameters.get("first_backup_esn"),
            "first_backup_target": self_ext_parameters.get("first_backup_target"),
            "last_backup_esn": self_ext_parameters.get("last_backup_esn"),
            "last_backup_target": self_ext_parameters.get("last_backup_target"),
            "priority_backup_esn": self_ext_parameters.get("priority_backup_esn"),
            "priority_backup_target": self_ext_parameters.get("priority_backup_target"),
            "failed_node_esn": self_ext_parameters.get("failed_node_esn"),
            "all_disk": self_ext_parameters.get("all_disk"),
            "disk_info": self_ext_parameters.get("disk_info")
        }
        submit_req = ModifyProtectionSubmitReq(
            sla_id=new_sla_id,
            resource_id=vm_resource.uuid,
            ext_parameters=ext_param_data
        )
        return submit_req

    @staticmethod
    def process_tag_filter(resource_tag_filters):
        if resource_tag_filters is None:
            return ""
        result = []
        exclude_present = False
        for tag_filter_item in resource_tag_filters:
            rule = tag_filter_item.get('rule')
            values = tag_filter_item.get('values', [])
            processed_values = []
            for value in values:
                if rule == FilterRule.ALL:
                    processed_values.append(value)
                elif rule == FilterRule.START_WITH:
                    processed_values.append('*' + value)
                elif rule == FilterRule.END_WITH:
                    processed_values.append(value + '*')
                elif rule == FilterRule.FUZZY:
                    processed_values.append('*' + value + '*')
            processed_values_str = ', '.join(processed_values)
            mode_str = FilterMode.INCLUDE if tag_filter_item.get('mode') == FilterMode.INCLUDE else FilterMode.EXCLUDE
            if mode_str == FilterMode.EXCLUDE:
                exclude_present = True
                result.append(f"{mode_str}: {processed_values_str}")
            else:
                result.append(f"{mode_str}: {processed_values_str}")
        if exclude_present:
            return '，'.join(result)
        else:
            return ''.join(result)

    @staticmethod
    def filter_by_rule(value, filters):
        """
        根据规则判断当前过滤值是否符合条件
        :param value: 需要过滤的值
        :param filters: 过滤器列表
        :return True/False: 是否符合
        """
        if not filters or len(filters) <= 0:
            return True
        for rule_filter in filters:
            if rule_filter.get("rule") == FilterRule.ALL and '*' in rule_filter.get("values") and value != '':
                return True
            elif rule_filter.get("rule") == FilterRule.ALL and value in rule_filter.get("values"):
                return True
            elif rule_filter.get("rule") == FilterRule.START_WITH and \
                    value.startswith(tuple(rule_filter.get("values"))):
                return True
            elif rule_filter.get("rule") == FilterRule.END_WITH and value.endswith(tuple(rule_filter.get("values"))):
                return True
            elif rule_filter.get("rule") == FilterRule.FUZZY and \
                    any(temp.lower() in value.lower() for temp in rule_filter.get("values")):
                return True
        return False

    @staticmethod
    def release_lock(lock: Lock):
        try:
            if lock.is_locked():
                lock.unlock()
            else:
                log.warn("lock is not owned.")
        except LockError as error:
            log.error(f"lock release failed. {error.__cause__}")

    @staticmethod
    def sync_deleted_resources(deleted_res_id_list):
        for deleted_id in deleted_res_id_list:
            # 删除的资源发送kafka消息
            comment_event_message(topic=RESOURCE_DELETED_TOPIC, request_id=str(
                uuid.uuid4()), resource_id=deleted_id)

    @staticmethod
    def sync_new_resources(added_res_id_list):
        for add_id in added_res_id_list:
            # 新增的资源发送kafka消息
            add_msg = ResourceAddedRequest(
                request_id=str(uuid.uuid4()), resource_id=add_id)
            producer.produce(add_msg)

    @staticmethod
    def notice_agent_vcenter_info_updated():
        query_expression = {'sub_type': ResourceSubTypeEnum.VMBackupAgent}
        agents = query_environment(query_expression)
        if not agents:
            return
        for agent in agents:
            time.sleep(5)
            try:
                hw_agent_rpc.notice_vm_agent_update_service_links(
                    agent["endpoint"])
            except Exception as ex:
                log.exception(
                    f"when notice_vm_agent_update_service_links get exception:{ex}")
            finally:
                pass

    @staticmethod
    def update_env_info(old_uuid, env):
        table = EnvironmentTable(root_uuid=env["uuid"], **env)
        table.password = SystemBaseClient.encrypt(
            table.password).get("ciphertext")
        with database.session() as db:
            db.query(ResourceTable).filter(ResourceTable.uuid ==
                                           old_uuid).delete(synchronize_session=False)

            db.add(table)

    @staticmethod
    def save_cert_infos(env_uuid, params: ScanEnvSchema):
        """
        保存证书内容到扩展信息表
        """
        cert_infos = {
            VMWareCertConstants.CERT_NAME: "", VMWareCertConstants.CERT_SIZE: "",
            VMWareCertConstants.CERTIFICATION: "", VMWareCertConstants.CRL_NAME: "",
            VMWareCertConstants.CRL_SIZE: "", VMWareCertConstants.REVOCATION_LIST: ""
        }
        extend_context = params.extend_context
        if extend_context.get(VMWareCertConstants.CERTIFICATION):
            cert_infos[VMWareCertConstants.CERT_NAME] = extend_context.get(VMWareCertConstants.CERT_NAME)
            cert_infos[VMWareCertConstants.CERT_SIZE] = extend_context.get(VMWareCertConstants.CERT_SIZE)
            cert_infos[VMWareCertConstants.CERTIFICATION] = SystemBaseClient.encrypt(
                extend_context.get(VMWareCertConstants.CERTIFICATION)).get("ciphertext")
        if extend_context.get(VMWareCertConstants.REVOCATION_LIST):
            cert_infos[VMWareCertConstants.CRL_NAME] = extend_context.get(VMWareCertConstants.CRL_NAME)
            cert_infos[VMWareCertConstants.CRL_SIZE] = extend_context.get(VMWareCertConstants.CRL_SIZE)
            cert_infos[VMWareCertConstants.REVOCATION_LIST] = SystemBaseClient.encrypt(
                extend_context.get(VMWareCertConstants.REVOCATION_LIST)).get("ciphertext")
        # TLS兼容性开关
        cert_infos[VMWareCertConstants.TLS_COMPATIBLE] = extend_context.get(VMWareCertConstants.TLS_COMPATIBLE, 'False')
        VMwareDiscoveryPlugin.upsert_res_extend_info(cert_infos, env_uuid)

    @staticmethod
    def save_storage_info(env_uuid, params: ScanEnvSchema):
        # 1.将参数中的存储信息使用map封装，key为storages，value为json
        extend_context = params.extend_context
        storages = extend_context.get('storages', [])
        # 检验存储信息，登录存储是否报错。参数校验，参考fc的代码
        for storage in storages:
            ip_list = storage.get('ip', '')
            username = storage.get('username', '')
            password = storage.get('password', '')
            storage_type = storage.get('type', '')
            for ip in ip_list:
                if ResourceSubTypeEnum.NET_APP.value == storage_type:
                    port = storage.get('port', 443)
                    StorageNetAppService(ip, port, username, password)
                else:
                    port = storage.get('port', 8088)
                    build_storage_service(ip, username, password, port)
            storage.update({'password': encrypt(storage.get('password', ''))})
        storages = json.dumps(storages)
        storages_dict = {'storages': storages}
        VMwareDiscoveryPlugin.upsert_res_extend_info(storages_dict, env_uuid)

    @staticmethod
    def upsert_res_extend_info(extend_infos, res_id):
        for key, value in extend_infos.items():
            res_extend_info = ResExtendInfoTable(uuid=str(uuid.uuid4()), resource_id=res_id, key=key, value=value)
            with database.session() as session:
                exist_extend_info = session.query(ResExtendInfoTable).filter(*{
                    ResExtendInfoTable.resource_id == res_id,
                    ResExtendInfoTable.key == key}).first()
                if not exist_extend_info:
                    session.add(res_extend_info)
                    continue
                if exist_extend_info.value != value:
                    session.query(ResExtendInfoTable) \
                        .filter(*{ResExtendInfoTable.resource_id == res_id, ResExtendInfoTable.key == key}) \
                        .update({"value": value})

    @classmethod
    def do_modify_env(cls, params: UpdateEnvSchema):
        if params.name and not match_pattern(ResourceConstants.RESOURCE_NAME_PATTERN, params.name):
            raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                       message="The resource name does not meet the rule.")
        scan_params = ScanEnvSchema(**params.dict())
        _check_env_params_endpoint(scan_params)
        cls.check_scan_interval(scan_params)
        if params.verify_cert:
            cls.fill_env_cert_info(scan_params)
        service_instance, cert_name = VMwareDiscoveryService.login(scan_params)
        with database.session() as session:
            env = session.query(EnvironmentTable).filter_by(
                uuid=params.uuid).first()
            env.name = params.name
            env.endpoint = params.endpoint
            env.port = params.port
            env.user_name = params.user_name
            env.password = SystemBaseClient.encrypt(
                params.password).get("ciphertext")
            scan_params.sub_type = env.sub_type
            scan_params.type = env.type
            env.cert_name = cert_name
            env.scan_interval = scan_params.rescan_interval_in_sec
            session.merge(env)
        # 保存证书信息并清理告警
        cls.save_cert_infos(scan_params.uuid, scan_params)
        cls.clear_cert_alarm(scan_params)
        scan_params.password = SystemBaseClient.encrypt(
            params.password).get("ciphertext")
        # 保存存储信息
        cls.save_storage_info(params.uuid, scan_params)
        # 断开连接
        VMwareDiscoveryService.disconnect(service_instance)

        return scan_params

    @classmethod
    def check_scan_interval(cls, scan_params):
        interval_in_sec = scan_params.rescan_interval_in_sec
        if not interval_in_sec or interval_in_sec < VMWareScanConstants.MIN_SCAN_INTERVAL:
            log.warn(f"vsphere scan interval {interval_in_sec} is lower than min interval.")
            scan_params.rescan_interval_in_sec = VMWareScanConstants.MIN_SCAN_INTERVAL
        elif interval_in_sec > VMWareScanConstants.MAX_SCAN_INTERVAL:
            log.warn(f"vsphere scan interval {interval_in_sec} is more than max interval.")
            scan_params.rescan_interval_in_sec = VMWareScanConstants.MAX_SCAN_INTERVAL

    @classmethod
    def fill_env_cert_info(cls, scan_params: ScanEnvSchema):
        res_id = scan_params.uuid
        extend_context = scan_params.extend_context
        cert_name = extend_context.get(VMWareCertConstants.CERT_NAME)
        crl_name = extend_context.get(VMWareCertConstants.CRL_NAME)
        cert_data = extend_context.get(VMWareCertConstants.CERTIFICATION)
        crl_data = extend_context.get(VMWareCertConstants.REVOCATION_LIST)
        if not cert_name:
            return
        exist_cert_name = None
        exist_cert_data = None
        exist_crl_name = None
        exist_crl_data = None
        with database.session() as session:
            exist_extend_infos = session.query(ResExtendInfoTable).filter(*{
                ResExtendInfoTable.resource_id == res_id}).all()
        for extend_info in exist_extend_infos:
            if extend_info.key == VMWareCertConstants.CERT_NAME:
                exist_cert_name = extend_info.value
            if extend_info.key == VMWareCertConstants.CERTIFICATION:
                exist_cert_data = extend_info.value
            if extend_info.key == VMWareCertConstants.CRL_NAME:
                exist_crl_name = extend_info.value
            if extend_info.key == VMWareCertConstants.REVOCATION_LIST:
                exist_crl_data = extend_info.value
        # 如果名称相同但是内容为空则需要把数据库的证书回填到参数用于登陆
        if cert_name == exist_cert_name and not cert_data:
            scan_params.extend_context[VMWareCertConstants.CERTIFICATION] = SystemBaseClient.decrypt(
                exist_cert_data).get("plaintext")
        if crl_name and crl_name == exist_crl_name and not crl_data:
            scan_params.extend_context[VMWareCertConstants.REVOCATION_LIST] = SystemBaseClient.decrypt(
                exist_crl_data).get("plaintext")
        log.info("Modify env, fill cert data finished.")

    @classmethod
    def pre_check(cls, params: ScanEnvSchema):
        if not match_pattern(ResourceConstants.RESOURCE_NAME_PATTERN, params.name):
            raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM,
                                       message="The resource name does not meet the rule.")
        _check_env_params_endpoint(params)
        # 1 检查IP对应的环境是否存在
        query_expression = {
            'type': params.type,
            'endpoint': params.endpoint
        }
        env = resource_service.query_environment(query_expression)
        if len(env) > 0:
            raise EmeiStorBizException(ResourceErrorCodes.RESOURCE_IS_REGISTERED,
                                       message="The resource has been registered and cannot be registered again.")
        # 对vcenter名称重复进行限制
        name_query_expression = {
            'type': params.type,
            'name': params.name
        }
        name_envs = resource_service.query_resource(name_query_expression)
        if len(name_envs) > 0:
            raise EmeiStorBizException(CommonErrorCodes.DUPLICATE_NAME,
                                       message="The resource name is duplicate and cannot be registered again.")

        # 2 检查用户名和密码是否正确
        service_instance, cert_name = VMwareDiscoveryService.login(params)
        params.password = SystemBaseClient.encrypt(params.password).get("ciphertext")
        # 3 检查vCenter或ESX是否已经发现
        vsphere_info = VMwareDiscoveryService.get_vsphere_info(
            service_instance, params)
        vsphere_id = vsphere_info.get('uuid')
        params.uuid = vsphere_id
        res = resource_service.query_resource_by_id(vsphere_id)

        if res:
            raise EmeiStorBizException(ResourceErrorCodes.RESOURCE_IS_REGISTERED,
                                       message="The resource has been registered and cannot be registered again.")

        # 4 检查service_instance是否已经被vCenter管理，如果被管理则抛出异常
        if vsphere_info.get("sub_type") == ResourceSubTypeEnum.ESXi \
                or vsphere_info.get("sub_type") == ResourceSubTypeEnum.ESX:
            is_managed_by_vcenter, vcenter_ip = VMwareDiscoveryService.is_managed_by_vcenter(
                service_instance)
            if is_managed_by_vcenter:
                raise EmeiStorBizException(ResourceErrorCodes.ESX_IS_MANAGED_BY_VCENTER, *[vcenter_ip],
                                           message="The ESX/ESXi has been managed by the vCenter.")
        # 5 检查是否存在重复的主机
        hosts = VMwareDiscoveryService.get_host_info(
            service_instance=service_instance, env_uuid=vsphere_id)
        host_id_list = list(host.get("uuid") for host in hosts)
        _check_resource_duplicate(host_id_list)

        # 6 检查外部存储信息

    @classmethod
    def clear_cert_alarm(cls, params):
        alarm_req = SendAlarmReq(alarmId=VMWareCertConstants.CERT_EXPIRED_ID,
                                 params=params.sub_type + "," + params.name,
                                 alarmSource="Resource", createTime=int(time.time()), sequence=1,
                                 sourceType=AlarmSourceType.CERTIFICATE)
        AlarmClient.clear_entity_alarm(alarm_req)
        alarm_req.alarm_id = VMWareCertConstants.CRL_EXPIRED_ID
        AlarmClient.clear_entity_alarm(alarm_req)
        log.info(f"clear cert alarm success.")

    def do_delete_env(self, params: str):
        resource = resource_service.query_resource_by_id(params)
        vm_ids = []
        with database.session() as db:
            filters = {VirtualResourceTable.root_uuid == params}
            exist_vms = db.query(VirtualResourceTable).filter(*filters).all()
            if exist_vms is not None:
                for virtualresourcetable in exist_vms:
                    vm_ids.append(virtualresourcetable.uuid)
            self.delete_vmware_check(db, vm_ids)
            # 删除vcenter下所有虚拟机
            db.query(VirtualResourceTable).filter(VirtualResourceTable.uuid.in_(vm_ids)).delete(
                synchronize_session=False)
            db.query(ResourceTable).filter(*filters).delete(synchronize_session=False)
            db.query(ResourceTable).filter(ResourceTable.uuid == params).delete(synchronize_session=False)
            vm_ids.append(params)
            # 删除环境所有的 resource group
            _delete_resource_groups_of_env(db, params)
        RBACClient.delete_resource_set_relation(vm_ids, ResourceSetTypeEnum.RESOURCE.value)
        self.clear_cert_alarm(resource)
        DEFAULT_ASYNC_POOL.submit(self.sync_deleted_resources, vm_ids)
        DEFAULT_ASYNC_POOL.submit(self.notice_agent_vcenter_info_updated)

    def do_scan_env(self, params: ScanEnvSchema, is_rescan=False, is_session_connect=False):
        log.info(f"start do scan environment: {params.uuid}, is rescan :{is_rescan}")
        env_uuid = params.uuid
        self.check_scan_interval(params)
        # 密码解密
        params.password = SystemBaseClient.decrypt(
            params.password).get("plaintext")
        if is_session_connect:
            service_instance = service_instance_manager.get_service_instance(
                env_uuid)
        else:
            service_instance, cert_name = service_instance_manager._do_login(
                params)
        content = service_instance.RetrieveContent()

        log.info(f"[VMWare Scan] Register env, ip={params.endpoint} build env info")
        # vSphere环境信息
        env = _build_vm_ware_environment(env_uuid, params, content)
        service = VMwareDiscoveryService(
            service_instance, ScanEnvSchema(**env))
        log.info(f"[VMWare Scan] Register env ip={params.endpoint}, scan start")
        resources = service.scan_vsphere(env, params)
        log.info(f"[VMWare Scan] Register env ip={params.endpoint}, scan end")
        # 密码加密还原
        params.password = SystemBaseClient.encrypt(params.password).get("ciphertext")
        result = {}
        # 如果不是重新扫描，即第一次发现，则提交定时扫描任务
        if not is_rescan:
            table = EnvironmentTable(root_uuid=env_uuid, **env)
            table.scan_interval = params.rescan_interval_in_sec
            table.password = params.password
            first_add_id_list = []
            # 检查资源规格是否超出规格上限
            limited_resources = check_and_limit_resources(resources)
            result["is_exceed_limit"] = len(limited_resources) < len(resources)
            result["added_resource_size"] = len(limited_resources)
            with database.session() as db:
                db.add(table)
                add_rbac_relation(table, params.domain_id)
                for res in limited_resources:
                    vir_res = VirtualResourceTable(**res.dict())
                    db.add(vir_res)
                    first_add_id_list.append(vir_res.uuid)
                    add_rbac_relation(vir_res, params.domain_id)
            log.info(f"[VMWare Scan] Register env, save resource complete, scan [{len(resources)}] resources")
            # 保存证书信息和tls开关
            if params.verify_cert:
                self.save_cert_infos(env_uuid, params)
            # 保存存储信息
            self.save_storage_info(env_uuid, params)
            DEFAULT_ASYNC_POOL.submit(self.sync_new_resources, first_add_id_list)
            DEFAULT_ASYNC_POOL.submit(self.notice_agent_vcenter_info_updated)
        else:
            env_name = params.name
            env_param = [env_uuid, env_name]
            self.do_rescan(env, params, resources, result, env_param)
        clear(env.get("password"))
        return result

    def do_rescan(self, env, params, resources, result, env_param):
        # 加锁防止环境被删除后重新扫描更新数据库
        env_uuid = env_param[0]
        env_name = env_param[1]
        lock: Lock = lock_manager.get_lock(key=EnvironmentRemoveConstants.DELETE_ENV_KEY_PREFIX + env_uuid)
        if lock.lock(timeout=EnvironmentRemoveConstants.LOCK_TIME_OUT,
                     blocking_timeout=EnvironmentRemoveConstants.LOCK_WAIT_TIME_OUT):
            try:
                if not resource_service.query_environment({'uuid': env_uuid}):
                    log.info(f'[Environment not exist]: no need rescan. endpoint: {params.endpoint},\
                            params.sub_type: {params.sub_type}')
                    return
                if env_uuid != env.get("uuid"):
                    # 此时更新env_uuid 更新定时任务同时更新当前vcenter信息
                    self.update_env_info(env_uuid, env)
                    delete_schedule_task(env_uuid)
                    params.uuid = env.get("uuid")
                self.re_scan_env(resources, env_uuid, result, params.domain_id, env_name)
                add_schedule_task(params)
            finally:
                self.release_lock(lock)
        else:
            log.info(f"[VMWare Rescan] env_uuid: {env_uuid}, get lock failed.")

    def re_scan_env(self, resources, env_uuid, result, domain_id, env_name):
        new_res_id_list = list(res.uuid for res in resources)
        with database.session() as db:
            filters = {VirtualResourceTable.root_uuid == env_uuid}
            exist_res_list = db.query(VirtualResourceTable).filter(*filters).all()
            exist_res_id_list = list(res.uuid for res in exist_res_list)
            # 数据库存在，而新扫描上的不存在，则表示资源已经被删除
            deleted_res_id_list = list(
                res.uuid for res in exist_res_list if res.uuid not in new_res_id_list)
            delete_filters = {
                ResourceTable.uuid.in_(deleted_res_id_list)
            }
            db.query(ResourceTable).filter(
                *delete_filters).delete(synchronize_session=False)
            if deleted_res_id_list:
                RBACClient.delete_resource_set_relation(deleted_res_id_list, ResourceSetTypeEnum.RESOURCE.value)
            log.info(f"[VMWare Rescan] [{len(deleted_res_id_list)} resources deleted].")
            # 数据库中不存在，但是新扫描上来的资源存在，则表示资源时新增加的
            added_res_id_list = list(
                res_id for res_id in new_res_id_list if res_id not in exist_res_id_list)
            # 校验新增资源是否重复
            _check_resource_duplicate(added_res_id_list, env_uuid)
            limited_res_id_list = check_and_limit_resources(added_res_id_list)
            result["is_exceed_limit"] = len(limited_res_id_list) < len(added_res_id_list)
            result["added_resource_size"] = len(limited_res_id_list)
            added_res = list(VirtualResourceTable(
                **res.dict()) for res in resources if res.uuid in limited_res_id_list)
            # 针对资源从数据库中其他vcenter移动过来的状况，uuid不变使用merge
            for obj in added_res:
                db.merge(obj)
                domain_id_list = get_vir_domain_id_list(obj.parent_uuid, domain_id=domain_id)
                log.info(f"[VMWare Rescan] [res {obj.uuid} add to domain:{domain_id_list}].")
                resource_set_relation_info = ResourceSetRelationInfo(
                    resource_object_id=obj.uuid,
                    resource_set_type=ResourceSetTypeEnum.RESOURCE.value,
                    scope_module=ResourceSetScopeModuleEnum.VMWARE.value,
                    domain_id_list=domain_id_list,
                    parent_uuid=obj.parent_uuid,
                    sub_type=ResourceSubTypeEnum.VirtualMachine.value)
                RBACClient.add_resource_set_relation(resource_set_relation_info)
            log.info(f"[VMWare Rescan] [{len(limited_res_id_list)} resources created].")
            # 数据库中存在，且新扫描上的也存在，表示需要更新
            updated_res_id_list = list(
                res_id for res_id in new_res_id_list if res_id in exist_res_id_list)
            updated_res = list(
                res for res in resources if res.uuid in updated_res_id_list)
            db.bulk_update_mappings(VirtualResourceTable, list(res.dict() for res in updated_res))
            log.info(f"[VMWare Rescan] [{len(updated_res_id_list)} resources updated].")
            db.flush()
        self.sync_deleted_resources(deleted_res_id_list)
        self.sync_new_resources(added_res_id_list)
        self.update_protect_status_after_tags_check(updated_res, env_uuid, env_name)
        log.info(f"[VMWare Rescan] Finished Rescan Env: {env_uuid}")

    def do_fetch_resource(self, params: ScanEnvSchema):
        pass

    # 资源扫描时静态组保护无变化，仅处理 虚拟机动态组保护 和 集群/主机保护，优先级：动态组保护 > 集群/主机保护
    def update_protect_status_after_tags_check(self, updated_res_list, env_id, env_name):
        log.info(f"Start to check vm tags and update protect status, env id: {env_id}, env name: {env_name}")
        # 按照创建时间从早到晚查询所有已保护的动态虚拟机组
        resource_groups = resource_service.get_protected_rule_resource_groups(
            env_id, ResourceSubTypeEnum.VirtualMachine)

        for updated_res in updated_res_list:
            if updated_res.sub_type != "vim.VirtualMachine":
                continue
            parent_uuid = updated_res.parent_uuid
            parent_resource = resource_service.query_resource_by_id(parent_uuid)
            vm = resource_service.query_resource_by_id(updated_res.uuid)

            if self.match_manual_group(updated_res.uuid):
                # 静态组————已经绑定静态组时，不管是否保护，不做处理
                continue

            # 动态组
            resource_group = self.get_match_rule_group(updated_res, resource_groups, vm)
            if resource_group is not None:
                log.debug(f'vm: {vm.uuid} match group: {resource_group.uuid}')
                self.update_rule_group_protect(updated_res, resource_group, vm, env_name)
            else:
                # 无匹配的静态组/动态组时，根据集群/主机的tag规则匹配虚拟机
                if parent_resource.protection_status == ProtectionStatusEnum.protected:
                    self.update_resource_protect_status_for_batch_protection(
                        updated_res.tags, parent_resource, updated_res, env_name)

    def update_rule_group_protect(self, updated_res, resource_group, vm, env_name):
        # 动态组
        if vm.protection_status == ProtectionStatusEnum.unprotected:
            self.update_unprotected_vm_protection(updated_res, resource_group, env_name, vm)
        elif vm.protection_status == ProtectionStatusEnum.protected:
            self.update_protected_vm_protection(updated_res, resource_group, env_name, vm)

    def match_manual_group(self, resource_id):
        group = resource_service.get_resource_group_by_member(resource_id)
        return group and group.group_type == 'manual'

    def get_match_rule_group(self, updated_res, resource_groups, vm):
        if vm.protection_status == ProtectionStatusEnum.unprotected:
            # 未保护————直接查询是否有匹配的动态组
            resource_group = self.get_matched_resource_group(resource_groups, updated_res)
            if resource_group is not None:
                return resource_group

        elif vm.protection_status == ProtectionStatusEnum.protected:
            # 已保护————先判断原有动态组规则是否匹配，如不匹配，则查询是否有匹配的动态组
            return self.get_match_rule_group_when_protected(updated_res, resource_groups)
        return None

    def get_match_rule_group_when_protected(self, updated_res, resource_groups):
        path = updated_res.path
        name = updated_res.name
        tags = updated_res.tags
        protected_resource = resource_service.query_protected_resource_by_id(updated_res.uuid)
        if protected_resource and protected_resource.resource_group_id:
            resource_group_id = protected_resource.resource_group_id
        else:
            resource_group_id = ''
        if resource_group_id != '':
            current_resource_group = resource_service.get_resource_group_by_group_id(resource_group_id)
            if self.is_matched_vm_for_group(current_resource_group, path, name, tags):
                return current_resource_group
            else:
                resource_group = self.get_matched_resource_group(resource_groups, updated_res)
                if resource_group is not None:
                    return resource_group
        return None

    def update_unprotected_vm_protection(self, updated_res, resource_group, env_name, vm):
        tags = updated_res.tags
        group_protect_obj = ResourceClient.query_protected_object(resource_id=resource_group.uuid)
        extend_str = json.loads(resource_group.extend_str)
        name_filter_str, tag_filter_str = self.get_filter_str_from_protect_obj(extend_str)
        self.create_group_protect_and_send_running_event(env_name, vm, group_protect_obj,
                                                         resource_group.uuid, tags, tag_filter_str)

    def update_protected_vm_protection(self, updated_res, new_resource_group, env_name, vm):
        protect_obj = ResourceClient.query_protected_object(resource_id=updated_res.uuid)
        protected_resource = resource_service.query_protected_resource_by_id(updated_res.uuid)
        current_resource_group_id = protected_resource.resource_group_id
        log.debug(f'vm: {vm.uuid} match rule group, old: {current_resource_group_id}, new: {new_resource_group.uuid}')
        if current_resource_group_id != new_resource_group.uuid:
            # 先移除原有保护
            current_resource_group = resource_service.get_resource_group_by_group_id(current_resource_group_id)
            current_extend_str = json.loads(current_resource_group.extend_str)
            current_name_filter_str, current_tag_filter_str = (
                self.get_filter_str_from_protect_obj(current_extend_str))
            self.remove_protect_and_send_running_event(env_name, vm, protect_obj, updated_res.tags,
                                                       current_tag_filter_str)
            # 新增新虚拟机组的保护
            group_protect_obj = ResourceClient.query_protected_object(resource_id=new_resource_group.uuid)
            new_extend_str = json.loads(new_resource_group.extend_str)
            new_name_filter_str, new_tag_filter_str = self.get_filter_str_from_protect_obj(new_extend_str)
            self.create_group_protect_and_send_running_event(env_name, vm, group_protect_obj, new_resource_group.uuid,
                                                             updated_res.tags, new_tag_filter_str)

    # 获取资源组保护对象扩展参数中的过滤条件字符串
    def get_filter_str_from_protect_obj(self, extend_str):
        resource_name_filters = extend_str.get("resource_filters", [])
        name_filter_str = self.process_tag_filter(resource_name_filters)
        resource_tag_filters = extend_str.get("resource_tag_filters", [])
        tag_filter_str = self.process_tag_filter(resource_tag_filters)
        return name_filter_str, tag_filter_str

    # 获取匹配的创建时间最早的虚拟机组
    def get_matched_resource_group(self, resource_groups, updated_res):
        for resource_group in resource_groups:
            if self.is_matched_vm_for_group(resource_group, updated_res.path, updated_res.name, updated_res.tags):
                return resource_group
        return None

    def is_matched_vm_for_group(self, resource_group, path, name, tags):
        # 规则过滤包括：计算位置模糊匹配、虚拟机名称过滤、虚拟机标记过滤
        extend_str = json.loads(resource_group.extend_str)
        return (self.is_path_match(extend_str, path)
                and self.is_name_match(extend_str, name)
                and self.is_tag_match(extend_str, tags))

    # 虚拟机计算位置过滤
    def is_path_match(self, extend_str, path):
        path_filter = extend_str.get("resource_path_filter", "")
        return path_filter in path

    # 虚拟机名称过滤
    def is_name_match(self, extend_str, name):
        resource_filters = extend_str.get("resource_filters", [])
        if resource_filters is None or len(resource_filters) <= 0:
            return True
        else:
            batch_filters = [
                name_filter
                for name_filter in resource_filters
                if self.is_vm_type(name_filter) and self.is_name_filter(name_filter)
            ]
            batch_filters.sort(key=lambda x: x.get('mode', None))
            batch_filters_group = groupby(batch_filters, key=lambda x: x.get('mode', None))
            # 匹配条件：规则满足（包含模式 + 包含name） 或者 （排除模式 + 不包含name）
            is_include_match, is_exclude_match = None, None
            for name_filter_mode, name_filters in batch_filters_group:
                filter_list = list(name_filters)
                is_match_one = self.filter_by_rule(urllib.parse.unquote(name), filter_list)
                is_include_match, is_exclude_match = self.mode_filter_match(name_filter_mode, is_match_one,
                                                                            is_include_match, is_exclude_match)
            is_include_match = is_include_match if is_include_match is not None else True
            is_exclude_match = is_exclude_match if is_exclude_match is not None else False
            return bool(is_include_match and not is_exclude_match)

    # 虚拟机标记过滤
    def is_tag_match(self, extend_str, tags):
        resource_filters = extend_str.get("resource_tag_filters", [])
        if resource_filters is None or len(resource_filters) <= 0:
            return True
        else:
            batch_filters = [
                tag_filter
                for tag_filter in resource_filters
                if self.is_vm_type(tag_filter) and self.is_tag_filter(tag_filter)
            ]
            batch_filters.sort(key=lambda x: x.get('mode', None))
            batch_filters_group = groupby(batch_filters, key=lambda x: x.get('mode', None))
            # 匹配条件：满足 任意一个（包含模式 + 包含tag） 且 所有（排除模式 + 不包含tag）
            is_include_match, is_exclude_match = None, None
            for tag_filter_mode, tag_filters in batch_filters_group:
                filter_list = list(tag_filters)
                is_match_one = any(self.filter_by_rule(urllib.parse.unquote(tag), filter_list)
                                   for tag in tags.split(","))
                is_include_match, is_exclude_match = self.mode_filter_match(tag_filter_mode, is_match_one,
                                                                            is_include_match, is_exclude_match)
            is_include_match = is_include_match if is_include_match is not None else True
            is_exclude_match = is_exclude_match if is_exclude_match is not None else False
            return bool(is_include_match and not is_exclude_match)

    def mode_filter_match(self, mode, is_match_one, is_include_match, is_exclude_match):
        if mode == FilterMode.INCLUDE:
            # 满足任意一条包含规则
            is_include_match = is_include_match if is_include_match is not None else is_match_one
            is_include_match = is_include_match or is_match_one
        elif mode == FilterMode.EXCLUDE:
            # 所有排除规则都不能满足
            is_exclude_match = is_exclude_match if is_exclude_match is not None else is_match_one
            is_exclude_match = is_exclude_match or is_match_one
        return is_include_match, is_exclude_match

    def update_resource_protect_status_for_batch_protection(self, tags, parent_resource, updated_res, env_name):
        log.info(f"Start to update resource protection status for batch protection, "
                 f"resource name: {updated_res.name},"
                 f"resource id {updated_res.uuid}")
        parent_protect_obj = ResourceClient.query_protected_object(resource_id=parent_resource.uuid)
        parent_ext_str = parent_protect_obj.get("ext_parameters", {})
        should_protect = (self.is_name_match(parent_ext_str, updated_res.name)
                          and self.is_tag_match(parent_ext_str, tags))
        self.update_vm_protect_status(should_protect, parent_protect_obj, updated_res, env_name, tags)

    def update_vm_protect_status(self, should_protect, parent_protect_obj, updated_res, env_name, tags):
        log.info(f"Update vm protect status for resource {updated_res.uuid}, should_protect {should_protect}")
        parent_ext_str = parent_protect_obj.get("ext_parameters", {})
        tag_filter_str = self.process_tag_filter(parent_ext_str.get("resource_tag_filters", []))
        binding_policy = parent_ext_str.get("binding_policy", [])

        vm = resource_service.query_resource_by_id(updated_res.uuid)
        if should_protect:
            if vm.protection_status == ProtectionStatusEnum.protected:
                # 如果被保护，且父资源的保护的高级配置中设置了覆盖虚拟机已有的SLA，则要修改SLA为父资源同一个SLA
                # 是否修改 SLA 判断逻辑在 modify_protect_and_send_running_event 中实现
                self.modify_protect_and_send_running_event(vm, parent_protect_obj, env_name, tags, tag_filter_str)
            elif (vm.protection_status == ProtectionStatusEnum.unprotected
                  and SlaApplyType.APPLY_TO_ALL in binding_policy):
                # 如果没有被保护, 且勾选了将主机SLA应用到所有未关联SLA的虚拟机上，则需要添加父资源同一个保护
                self.create_protect_and_send_running_event(env_name, vm, parent_protect_obj, tags, tag_filter_str)
        elif vm.protection_status == ProtectionStatusEnum.protected:
            # 如果不应该保护，则移除保护
            self.remove_protect_and_send_running_event(env_name, vm, parent_protect_obj, tags, tag_filter_str)

    def create_protect_and_send_running_event(self, env_name, vm_resource, parent_protect_obj, tags, tag_filter_str):
        sla_id = parent_protect_obj.get("sla_id")
        sla_name = parent_protect_obj.get("sla_name")
        batch_create_req = self.build_create_protect_request(sla_id, vm_resource, parent_protect_obj)
        # 使用系统管理员sysadmin账户创建保护
        user_id = "88a94c476f12a21e016f12a246e50009"
        try:
            BatchProtectionService.submit_batch_protection_task(user_id, batch_create_req)
            timestamp = int(time.time())
            vm_uuid = vm_resource.uuid
            vm_name = vm_resource.name
            params = [env_name, vm_uuid, vm_name, urllib.parse.unquote(tags), sla_name, tag_filter_str]
            event_id = CommonOperationID.AUTO_SCAN_AND_CREATE_PROTECT
            EventClient.send_running_event(SendEventReq(
                userId=vm_resource.user_id,
                eventId=event_id,
                eventParam=params,
                eventTime=timestamp,
                eventLevel=LogRank.INFO.value,
                sourceId=event_id,
                resourceId=vm_resource.uuid,
                sourceType=AlarmSourceType.PROTECTION,
                eventSequence=timestamp,
                isSuccess=False
            ))
        except Exception:
            log.error(f"Create protect failed, resource id: {vm_resource.uuid}")

    def create_group_protect_and_send_running_event(self, env_name, vm_resource, parent_protect_obj,
                                                    resource_group_id, tags, tag_filter_str):
        sla_id = parent_protect_obj.get("sla_id")
        sla_name = parent_protect_obj.get("sla_name")
        batch_create_req = self.build_create_group_protect_request(sla_id, resource_group_id, vm_resource,
                                                                   parent_protect_obj)
        # 使用系统管理员sysadmin账户创建保护
        user_id = "88a94c476f12a21e016f12a246e50009"
        try:
            BatchProtectionService.submit_batch_protection_task(user_id, batch_create_req)
            timestamp = int(time.time())
            vm_uuid = vm_resource.uuid
            vm_name = vm_resource.name
            params = [env_name, vm_uuid, vm_name, urllib.parse.unquote(tags), sla_name, tag_filter_str]
            event_id = CommonOperationID.AUTO_SCAN_AND_CREATE_PROTECT
            EventClient.send_running_event(SendEventReq(
                userId=vm_resource.user_id,
                eventId=event_id,
                eventParam=params,
                eventTime=timestamp,
                eventLevel=LogRank.INFO.value,
                sourceId=event_id,
                resourceId=vm_resource.uuid,
                sourceType=AlarmSourceType.PROTECTION,
                eventSequence=timestamp,
                isSuccess=False
            ))
        except Exception:
            log.error(f"Create protect failed, resource id: {vm_resource.uuid}")

    def modify_protect_and_send_running_event(self, vm_res, parent_protect_obj, env_name, tags, tag_filter_str):
        new_sla_id = parent_protect_obj.get("sla_id")
        new_sla_name = parent_protect_obj.get("sla_name")
        overwrite = parent_protect_obj.get("ext_parameters", {}).get("overwrite")
        if not overwrite:
            return
        # 需要修改sla，改为父资源的sla
        ori_sla = vm_res.sla_name
        ori_sla_id = vm_res.sla_id
        if ori_sla_id == new_sla_id:
            log.info(f"Vm's({vm_res.uuid}) sla id:{new_sla_id} is same as vm's parent's sla:{new_sla_id}")
            return
        submit_req = self.build_modify_protect_request(new_sla_id, vm_res, parent_protect_obj)
        # 使用系统管理员sysadmin账户修改保护
        user_id = "88a94c476f12a21e016f12a246e50009"
        try:
            obj_id = BatchProtectionService.modify_protection_task_submit(user_id, submit_req)
            object_id = ProtectedObjectId(uuid=obj_id)
            timestamp = int(time.time())
            vm_uuid = vm_res.uuid
            vm_name = vm_res.name
            params = [env_name, vm_uuid, vm_name, urllib.parse.unquote(tags), ori_sla, new_sla_name, tag_filter_str]
            event_id = CommonOperationID.AUTO_SCAN_AND_MODIFY_PROTECT
            EventClient.send_running_event(SendEventReq(
                userId=vm_res.user_id,
                eventId=event_id,
                eventParam=params,
                eventTime=timestamp,
                eventLevel=LogRank.INFO.value,
                sourceId=event_id,
                resourceId=vm_res.uuid,
                sourceType=AlarmSourceType.PROTECTION,
                eventSequence=timestamp,
                isSuccess=False
            ))
        except Exception:
            log.error(f"Modify protect failed, resource id: {vm_res.uuid}")


def create():
    return VMwareDiscoveryPlugin()

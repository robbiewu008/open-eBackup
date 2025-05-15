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
import itertools
import json
import os
import re
import urllib.parse
import uuid
from itertools import groupby
from typing import List, Union

from sqlalchemy import and_

from app.base.consts import WORKING_STATUS_LIST, CAN_NOT_REMOVE_PROTECT_TYPES
from app.base.db_base import database
from app.common import toolkit
from app.common.clients.anti_ransomware_client import AntiRansomwareClient
from app.common.clients.protection_client import ProtectionClient
from app.common.clients.resource_client import ResourceClient
from app.common.clients.scheduler_client import SchedulerClient
from app.common.clients.system_base_client import SystemBaseClient
from app.common.context.db_session import Session
from app.common.deploy_type import DeployType
from app.common.enums.job_enum import JobType, JobLogLevel, JobStatus
from app.common.enums.protected_object_enum import Status
from app.common.enums.rbac_enum import AuthOperationEnum
from app.common.enums.resource_enum import (ResourceSubTypeEnum, ResourceTypeEnum, ProtectionStatusEnum, \
                                            LinkStatusEnum, GeneralDbSubTypeMappingEnum, DeployTypeEnum)
from app.common.enums.schedule_enum import ExecuteType
from app.common.enums.sla_enum import PolicyActionEnum, PolicyTypeEnum
from app.common.event_messages.Eam.eam import ProtectionRemoveEvent
from app.common.events import producer
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.protection_error_codes import ProtectionErrorCodes
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exception.user_error_codes import UserErrorCodes
from app.common.logger import get_logger
from app.common.schemas.common_schemas import BatchOperationResult
from app.common.toolkit import JobMessage
from app.common.util.cleaner import clear
from app.copy_catalog.models.tables_and_sessions import StorageUnitTable
from app.protection.object import db
from app.protection.object.client.dee_client import update_self_learning_config
from app.protection.object.client.job_client import JobClient
from app.protection.object.common.constants import ResourceProtectionModifyJobSteps, ResourceProtectionJobSteps, \
    ProtectVolumeServiceEnum, LocalDiskSupportSubType, WormValidityType
from app.protection.object.common.protection_enums import FilterRule, FilterMode, FilterType, FilterColumn, \
    ProtectPostAction, \
    SlaApplyType, \
    ResourceFilter, HypervFormatEnum
from app.protection.object.models.projected_object import ProtectedObject
from app.protection.object.schemas.extends.params.cloud_backup_ext_param import CloudBackupExtParam
from app.protection.object.schemas.extends.params.vmware_ext_param import ProtectResource, VirtualResourceExtParam, \
    VmExtParam
from app.protection.object.schemas.protected_object import \
    BatchProtectionSubmitReq, CurrentManualBackupRequest, \
    ModifyProtectionSubmitReq, \
    BatchProtectionExecuteReq, ModifyProtectionExecuteReq
from app.protection.object.service import projected_object_service
from app.protection.object.service.agent_remove_protect import handle_unmount_repo_for_sap_hana_db
from app.protection.object.service.projected_object_service import ProtectedObjectService, build_protection_object, \
    build_protection_task, build_schedule_params, create_job, filter_sla_policy, \
    check_protected_obj_script, is_need_generate_new_chain_id, check_action_and_small_file, check_is_need_build_task, \
    build_protection_object_without_task, fill_protection_ext_parameters, build_sub_vm_protection_object
from app.protection.object.service.protection_plugin_manager import ProtectionPluginManager
from app.protection.object.service.validator_manager import ValidatorManager
from app.resource.db.db_desestitation_db_api import query_resource_info
from app.resource.kafka import topics
from app.resource.models.resource_models import ResourceTable
from app.resource.schemas.env_schemas import ScanEnvSchema
from app.resource.service.common import domain_resource_object_service
from app.resource.service.common import resource_service
from app.resource.service.common.role_auth_service import get_default_role_auth_list
from app.resource.service.vmware.service_instance_manager import service_instance_manager

log = get_logger(__name__)
GROUP_BACKUP_TOPIC = "schedule.group_backup"


def filter_by_rule(value: str, filters: List[ResourceFilter]):
    """
    根据规则判断当前过滤值是否符合条件

    :param value:  需要过滤的值
    :param filters: 过滤器列表
    :return:
    """
    if not filters or len(filters) <= 0:
        return True
    for rule_filter in filters:
        if rule_filter.rule == FilterRule.ALL and value in rule_filter.values:
            return True
        elif rule_filter.rule == FilterRule.START_WITH and value.startswith(tuple(rule_filter.values)):
            return True
        elif rule_filter.rule == FilterRule.END_WITH and value.endswith(tuple(rule_filter.values)):
            return True
        elif rule_filter.rule == FilterRule.FUZZY and \
                any(temp.lower() in value.lower() for temp in rule_filter.values):
            return True
    return False


def tag_filter_by_rule(value: str, filters: List[ResourceFilter]):
    """
    根据规则判断当前tag过滤值是否符合条件

    :param value:  需要过滤的值
    :param filters: 过滤器列表
    :return:
    """
    if not filters or len(filters) <= 0:
        return True
    for rule_filter in filters:
        if rule_filter.rule == FilterRule.ALL and '*' in rule_filter.values and value != '':
            return True
        elif rule_filter.rule == FilterRule.ALL and value in rule_filter.values:
            return True
        elif rule_filter.rule == FilterRule.START_WITH and value.startswith(tuple(rule_filter.values)):
            return True
        elif rule_filter.rule == FilterRule.END_WITH and value.endswith(tuple(rule_filter.values)):
            return True
        elif rule_filter.rule == FilterRule.FUZZY and \
                any(temp.lower() in value.lower() for temp in rule_filter.values):
            return True
    return False


def always_not_filter_resources(resource_list):
    # CNware, Nutanix, FusionCompute, Fusion One在对集群添加保护时需要对集群下的主机同步添加保护
    not_filter_resources = []
    for resource in resource_list:
        if (resource.get("sub_type") == ResourceSubTypeEnum.FusionCompute
            or resource.get("sub_type") == ResourceSubTypeEnum.FUSION_ONE_COMPUTE) \
                and resource.get("type") == ResourceTypeEnum.Host.value:
            not_filter_resources.append(resource)
        if resource.get("sub_type") in [ResourceSubTypeEnum.CNWARE_HOST, ResourceSubTypeEnum.NUTANIX_HOST]:
            not_filter_resources.append(resource)
    return not_filter_resources


def filter_resources(resource_list, extended_parameters):
    """
    过滤资源
    1.筛选过滤类型为ResourceType过滤数据
    2.匹配ResourceType枚举的value与resource中的type
    3.根据rule、mode、value进行数据过滤
    4.返回匹配之后的数据
    :param extended_parameters: 包含资源过滤器信息的拓展信息
    :return id_list_after_tag_filter: 过滤后的资源uuid列表
    :return:
    """
    not_filter_resources = always_not_filter_resources(resource_list)
    not_filter_resource_ids = list(item.get("uuid") for item in not_filter_resources)
    resource_list_ids = list(item.get("uuid") for item in resource_list)
    resource_list_ids = list(set(resource_list_ids).difference(not_filter_resource_ids))
    resource_list = list(item for item in resource_list if item.get("uuid") in resource_list_ids)
    log.debug(f"not_filter_resource len:{len(not_filter_resources)}, resource_list len:{len(resource_list)} ")
    if hasattr(extended_parameters, 'resource_filters'):
        resource_name_filters = extended_parameters.resource_filters
        id_list_after_name_filter = name_filter_resources(not_filter_resource_ids, resource_list, resource_name_filters)
    else:
        id_list_after_name_filter = list(item.get("uuid") for item in resource_list)
    if hasattr(extended_parameters, 'resource_tag_filters'):
        resource_tag_filters = extended_parameters.resource_tag_filters
        id_list_after_tag_filter = tag_filter_resources(id_list_after_name_filter, resource_tag_filters)
    else:
        id_list_after_tag_filter = id_list_after_name_filter
    # 这里需要把FusionCompute或FusionOne的主机id再加回去，not_filter_resource_ids中存的即为主机的id列表
    id_list_after_tag_filter.extend(not_filter_resource_ids)
    return id_list_after_tag_filter


def name_filter_resources(not_filter_resource_ids, resource_list, resource_name_filters):
    """
    过滤资源
    :param not_filter_resource_ids: 最后返回的资源列表
    :param resource_list: 资源列表
    :param resource_name_filters: 资源名称过滤器
    :return not_filter_resource_ids: 名称过滤后的资源uuid列表
    """
    if resource_name_filters is None or len(resource_name_filters) <= 0:
        return list(item.get("uuid") for item in resource_list)
    batch_filters = list(filter(lambda x: x.type == FilterType.VM and x.filter_by == FilterColumn.NAME,
                                resource_name_filters))
    filtered_resources = list(item.get("uuid") for item in resource_list)

    include_resources = []
    exclude_resources = []
    include_filter = False
    exclude_filter = False
    batch_filters.sort(key=lambda x: x.mode)
    batch_filters_group = groupby(batch_filters, key=lambda x: x.mode)
    for filter_mode, resource_name_filters in batch_filters_group:
        filter_list = list(resource_name_filters)
        if filter_mode == FilterMode.INCLUDE:
            include_filter = True
            include_resources = list(
                item.get("uuid") for item in resource_list if filter_by_rule(item.get("name"), filter_list))
        elif filter_mode == FilterMode.EXCLUDE:
            exclude_filter = True
            exclude_resources = list(
                item.get("uuid") for item in resource_list if filter_by_rule(item.get("name"), filter_list))
    # 取过滤之后的差集
    if include_filter:
        filtered_resources = list(set(filtered_resources).intersection(include_resources))
    if exclude_filter:
        filtered_resources = list(set(filtered_resources).difference(exclude_resources))
    return filtered_resources


def tag_filter_resources(not_filter_resource_ids, resource_tag_filters):
    """
    过滤资源
    1.筛选过滤类型为ResourceType过滤数据
    2.匹配ResourceType枚举的value与resource中的type
    3.根据rule、mode、value进行数据过滤
    4.返回匹配之后的数据
    :param not_filter_resource_ids: 名称过滤后的资源uuid列表
    :param resource_tag_filters: 资源tag过滤器
    :return not_filter_resource_ids: tag过滤后的资源uuid列表
    """
    if resource_tag_filters is None or len(resource_tag_filters) <= 0:
        return not_filter_resource_ids
    resource_ids_after_tag_filter = []
    resource_list = []
    for resource_id in not_filter_resource_ids:
        resource = resource_service.query_resource_by_id(resource_id)
        resource_list.append(resource)
    tag_filtered_resources = [item.uuid for item in resource_list]
    batch_tag_filters = list(filter(lambda x: x.type == FilterType.VM and x.filter_by == FilterColumn.TAG,
                                    resource_tag_filters))
    include_resources = []
    exclude_resources = []
    include_tag_filter = False
    exclude_tag_filter = False
    batch_tag_filters.sort(key=lambda x: x.mode)
    batch_tag_filters_group = groupby(batch_tag_filters, key=lambda x: x.mode)
    for tag_filter_mode, tag_filters in batch_tag_filters_group:
        filter_list = list(tag_filters)
        if tag_filter_mode == FilterMode.INCLUDE:
            include_tag_filter = True
            include_resources = [
                item.uuid
                for item in resource_list
                if check_tag_filters(item.tags, filter_list)
            ]
        elif tag_filter_mode == FilterMode.EXCLUDE:
            exclude_tag_filter = True
            exclude_resources = [
                item.uuid
                for item in resource_list
                if check_tag_filters(item.tags, filter_list)
            ]
    # 取过滤之后的差集
    if include_tag_filter:
        tag_filtered_resources = list(set(tag_filtered_resources).intersection(include_resources))
    if exclude_tag_filter:
        tag_filtered_resources = list(set(tag_filtered_resources).difference(exclude_resources))
    resource_ids_after_tag_filter.extend(tag_filtered_resources)
    return resource_ids_after_tag_filter


def check_tag_filters(tags, filter_list):
    return any(tag_filter_by_rule(urllib.parse.unquote(tag), filter_list) for tag in tags.split(","))


def filter_disks_with_uuid(disk, filters: List[ResourceFilter]) -> bool:
    return filter_disks(disk, filters) and disk.get('uuid')


def filter_disks(disk, filters: List[ResourceFilter]) -> bool:
    """
    过滤VM真实的磁盘信息，过滤值*表示匹配全部
    :param disk:
    :param filters:
    :return:
    """
    if filters is None or len(filters) <= 0:
        return disk
    for current_filter in list(filter(
            lambda x: x.type == FilterType.DISK and x.filter_by == FilterColumn.SLOT and x.rule == FilterRule.ALL,
            filters)):
        if '*' in current_filter.values:
            return True
        if current_filter.mode == FilterMode.INCLUDE and disk.get("slot") in current_filter.values:
            return True
        if current_filter.mode == FilterMode.EXCLUDE and disk.get("slot") not in current_filter.values:
            return True
    return False


def covert_to_vm_ext_parameters(disk_filters: List[ResourceFilter], vm_resource,
                                ext_parameters: VirtualResourceExtParam) -> Union[None, VmExtParam]:
    """
    容器保护的高级参数转化为虚拟机保护的高级参数
        1. 查询当前虚拟机当前的磁盘
        2. 与过滤条件中的磁盘信息进行对比，匹配到符合条件的磁盘信息
        3. 构造vm参数对象
    :param disk_filters: 磁盘信息过滤器
    :param vm_resource: vm虚拟机资源
    :param ext_parameters: 容器保护的高级参数
    :return:
    """
    disks = ResourceClient.query_vm_disk(vm_resource.get("uuid"))
    if disks is None or len(disks) <= 0:
        return None
    disk_info = list(disk.get("uuid") for disk in disks if filter_disks_with_uuid(disk, disk_filters))

    return VmExtParam(**{
        "pre_script": ext_parameters.pre_script,
        "post_script": ext_parameters.post_script,
        "all_disk": len(disks) == len(disk_info),
        "disk_info": disk_info
    })


def protect_common_resource(request_id, resource_list, sla,
                            batch_create_req: BatchProtectionExecuteReq) -> BatchOperationResult:
    """
    处理非容器类资源批量保护
        1.根据资源和SLA构造保护对象
        2.构造保护对象task并调用schedule接口创建调度任务
        3.批量保存保护对象及task
    :param request_id: 请求id
    :param resource_list: 资源列表
    :param sla: 绑定的sla信息
    :param batch_create_req: 批量保护请求
    :return:
    """
    batch_result = BatchOperationResult()
    job_id = batch_create_req.job_id
    ext_parameters = batch_create_req.ext_parameters
    with database.session() as session:
        for resource in resource_list:
            resource_name = None
            sla_name = None
            resource_id = resource.get("uuid")
            try:
                protected_obj = build_protection_object(resource, sla, ext_parameters)
                session.add(protected_obj)
                batch_result.append_success_id(resource_id)
                resource_name = resource.get("name")
                sla_name = protected_obj.sla_name
                handle_protect_success(job_id, request_id, resource_id, resource_name, session, sla_name)
            except Exception as es:
                log.info(f'common EmeiStorBizException={es}')
                log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
                log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
                log.exception(f"resource[{resource_id}] execute protection failed")
                batch_result.append_failed_id(resource_id)
                handle_protect_failed(job_id, request_id, resource_name, sla_name, session, resource_id, log_detail,
                                      log_detail_param)
        return batch_result


def protect_current_resource(session, request_id, resource: ProtectResource, sla,
                             batch_create_req: BatchProtectionExecuteReq) -> BatchOperationResult:
    """
    保护主虚拟化容器：集群、主机
        1. 判断主机是否绑定SLA，如果绑定，直接忽略
        2. 创建保护对象，根据SLA创建保护调度任务，创建保护对象
            2.1 调度任务实际执行扫描受保护环境
    :param request_id:
    :param session:
    :param resource:
    :param sla:
    :param batch_create_req:
    :return:
    """
    batch_result = BatchOperationResult()
    job_id = batch_create_req.job_id
    resource_id = resource.resource_id
    resource_name = None
    sla_name = None
    try:
        ext_params = batch_create_req.ext_parameters
        ext_params.disk_filters = resource.filters
        resource_info = ResourceClient.query_resource(resource_id)
        plm = ProtectionPluginManager(ResourceSubTypeEnum(resource_info.get("sub_type")))
        protected_obj = plm.build_protection_object(resource_info, sla, ext_params)
        session.add(protected_obj)
        batch_result.append_success_id(resource_id)
        resource_name = protected_obj.name
        sla_name = protected_obj.sla_name
        handle_protect_success(job_id, request_id, resource_id, resource_name, session, sla_name)
    except Exception as es:
        log.error(f'common EmeiStorBizException={es}')
        log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
        log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
        log.exception(f"resource[{resource_id}] execute protection failed")
        batch_result.append_failed_id(resource_id)
        handle_protect_failed(job_id, request_id, resource_name, sla_name, session, resource_id, log_detail,
                              log_detail_param)
    return batch_result


def protect_sub_resources(session, request_id, resource: ProtectResource, sub_resource_list: List[dict], sla,
                          batch_create_req: BatchProtectionExecuteReq) -> BatchOperationResult:
    """
    保护虚拟机数据
        1. 根据名称过滤虚拟机，规则冲突的情况下，包含的优先级更高
        2. 查询哪些资源已经绑定了SLA
        3. 根据资源覆盖策略
            3.1 如果强制覆盖并且绑定的SLA不同，则先移除保护
            3.2 如果不覆盖，则忽略这些资源
        4. 批量保护的磁盘信息，与虚拟机真实的磁盘信息匹配过滤
        4. 符合条件的数据，根据SLA创建保护调度任务，创建保护对象
    :param request_id:
    :param session:
    :param resource:
    :param sub_resource_list:
    :param sla:
    :param batch_create_req:
    :return:
    """
    batch_result = BatchOperationResult()
    job_id = batch_create_req.job_id
    ext_parameters = batch_create_req.ext_parameters
    # 过滤符合条件的虚拟机
    all_resource_ids = filter_resources(sub_resource_list, batch_create_req.ext_parameters)
    if len(all_resource_ids) <= 0:
        # 如果过滤之后无没有匹配的资源
        return batch_result
    # 查询已经绑定不相同SLA的保护对象
    protected_obj_list = db.projected_object.query_multi_by_params(db=session, conditions=[
        ProtectedObject.resource_id.in_(all_resource_ids)])
    resource_map = dict()
    for sub_resource in sub_resource_list:
        resource_map[sub_resource.get("uuid")] = sub_resource
    # 获取已经被保护的资源id列表
    protected_resource_ids = list(temp.resource_id for temp in protected_obj_list)
    need_protected_resource_ids = []
    if SlaApplyType.APPLY_TO_ALL in ext_parameters.binding_policy:
        # 应用到所有未保护的虚拟机会将未保护的id加入需要重新保护的list
        need_protected_resource_ids.extend(list(
            set(all_resource_ids).difference(protected_resource_ids)))
    if ext_parameters.overwrite and len(protected_resource_ids) > 0:
        # 如果覆盖SLA,已经绑定SLA的虚拟机，需要先移除保护，再添加保护
        # 并将已经保护的虚拟机id加入需要重新保护的list
        _batch_remove_protection(protected_obj_list, session, job_id, request_id, batch_result,
                                 need_protected_resource_ids)
    for resource_id in need_protected_resource_ids:
        resource_info = resource_map.get(resource_id)
        resource_name = resource_info.get("name")
        sla_name = sla.get("name")
        try:

            ext_params = build_extend_parameter(ext_parameters, resource, resource_info)
            # 无法构造高级参数，直接忽略这个资源
            if ext_params is None:
                continue
            protected_object = None
            if check_is_need_build_task(resource_info.get("type"), resource_info.get("sub_type")):
                protected_object = build_protection_object(resource_info, sla, ext_params)
            else:
                protected_object = build_protection_object_without_task(resource_info, sla, ext_params)
            session.add(protected_object)
            batch_result.append_success_id(resource_id)
            handle_protect_success(job_id, request_id, resource_id, resource_name, session, sla_name)
        except Exception as es:
            log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
            log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
            log.exception(f"resource[{resource_id}] execute protection failed")
            batch_result.append_failed_id(resource_id)
            handle_protect_failed(job_id, request_id, resource_name, sla_name, session, resource_id, log_detail,
                                  log_detail_param)
    return batch_result


def _batch_remove_protection(protected_obj_list, session, job_id, request_id, batch_result,
                             need_protected_resource_ids):
    for protected_obj in protected_obj_list:
        job_step = ResourceProtectionModifyJobSteps.PROTECTION_REMOVE_SUCCESS
        log_level = JobLogLevel.INFO
        resource_name = protected_obj.name
        log_detail = None

        try:
            ProtectedObjectService.batch_remove_protection(session, [protected_obj])
            need_protected_resource_ids.append(protected_obj.resource_id)
        except EmeiStorBizException as es:
            job_step = ResourceProtectionModifyJobSteps.PROTECTION_REMOVE_FAILED
            log_level = JobLogLevel.ERROR
            log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
            batch_result.append_failed_id(protected_obj.resource_id)
            log.exception(f"resource({resource_name}) remove protection failed")
        BatchProtectionService.record_job_step(job_id, request_id, job_step, log_level, [resource_name],
                                               log_detail=log_detail)


def handle_protect_failed(job_id, request_id, resource_name, sla_name, session, resource_id,
                          log_detail=None, log_detail_param=None):
    # 将资源状态更新为未保护
    resource_service.update_protection_status(session, [resource_id], ProtectionStatusEnum.unprotected)
    BatchProtectionService.record_job_step(job_id, request_id,
                                           ResourceProtectionJobSteps.PROTECTION_EXECUTING_FAILED,
                                           JobLogLevel.ERROR, [resource_name, sla_name], log_detail,
                                           log_detail_param)


def handle_protect_success(job_id, request_id, resource_id, resource_name, session, sla_name):
    # 将资源状态更新为已保护
    resource_service.update_protection_status(session, [resource_id], ProtectionStatusEnum.protected)
    BatchProtectionService.record_job_step(job_id, request_id,
                                           ResourceProtectionJobSteps.PROTECTION_EXECUTING_SUCCESS,
                                           JobLogLevel.INFO, [resource_name, sla_name])


def build_extend_parameter(ext_parameters, resource, resource_info):
    plm = ProtectionPluginManager(ResourceSubTypeEnum(resource_info.get("sub_type")))
    ext_params = plm.convert_extend_parameter(
        resource.filters, resource_info, ext_parameters)
    return ext_params


def protect_resources(request_id, sla, batch_create_req: BatchProtectionExecuteReq,
                      resource_sub_type) -> BatchOperationResult:
    """
    批量保护资源及其子资源
    1.查询所有资源
    :param resource_sub_type: 资源子类型
    :param request_id:
    :param session: 资源列表
    :param sla: 绑定的sla信息
    :param batch_create_req: 批量保护请求
    :return:
    """
    batch_result = BatchOperationResult()
    for resource in batch_create_req.resources:
        with database.session() as session:
            # 保护当前资源本身
            protect_result = protect_current_resource(session, request_id, resource, sla, batch_create_req)
            batch_result.merge_result(protect_result)
            if not protect_result.is_success():
                continue
            # 保护子资源
            current_res = resource_service.query_resource_by_id(resource.resource_id)
            plm = ProtectionPluginManager(ResourceSubTypeEnum(resource_sub_type))
            sub_resources = plm.query_sub_resources(current_res)
            log.info(f"protect sub resources num is: {len(sub_resources)}")
            if not sub_resources or len(sub_resources) <= 0:
                continue
            protect_sub_resources(session, request_id, resource, sub_resources, sla, batch_create_req)
    return batch_result


def protect_vmware(request_id, resource_list, sla,
                   batch_create_req: BatchProtectionExecuteReq) -> BatchOperationResult:
    """
    保护VMware虚拟机
    :param request_id:
    :param resource_list: 批量保护资源信息列表
    :param sla: 本次绑定的sla
    :param batch_create_req: 批量保护请求
    :return:
    """
    batch_result = BatchOperationResult()
    job_id = batch_create_req.job_id
    protected_resources = batch_create_req.resources
    ext_parameters = batch_create_req.ext_parameters
    if not protected_resources or len(protected_resources) <= 0:
        return batch_result
    resource_map = {resource.get("uuid"): resource for resource in resource_list}

    for protected_resource in protected_resources:
        resource_id = protected_resource.resource_id
        resource_name = None
        sla_name = None
        try:
            if not protected_resource.filters:
                return batch_result
            filters = list(filter(
                lambda x: (x.type == FilterType.DISK and
                           x.filter_by == FilterColumn.ID and
                           x.rule == FilterRule.ALL and
                           x.mode == FilterMode.INCLUDE),
                protected_resource.filters))
            disk_values = list(disks.values for disks in filters)
            disk_list = list(itertools.chain(*disk_values))
            vm_ext = VmExtParam(**ext_parameters.dict(exclude_unset=True, exclude_none=True),
                                all_disk=True, disk_info=[]) if "*" in disk_list else VmExtParam(
                **ext_parameters.dict(exclude_unset=True, exclude_none=True), all_disk=False, disk_info=disk_list)
            resource_obj = resource_map.get(resource_id)
            if batch_create_req.resource_group_id != '':
                protected_obj = build_sub_vm_protection_object(resource_obj, sla, vm_ext,
                                                               batch_create_req.resource_group_id)
            else:
                protected_obj = build_protection_object(resource_obj, sla, vm_ext)
            with database.session() as session:
                session.add(protected_obj)
                resource_name = resource_obj.get("name")
                sla_name = protected_obj.sla_name
                handle_protect_success(job_id, request_id, resource_id, resource_name, session, sla_name)
            batch_result.append_success_id(resource_id)
        except Exception as es:
            log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
            log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
            log.exception(f"resource[{resource_id}] execute protection failed")
            batch_result.append_failed_id(resource_id)
            handle_protect_failed(job_id, request_id, resource_name, sla_name, session, resource_id, log_detail,
                                  log_detail_param)
    return batch_result


def is_need_protect_children_resource(resource_type, resource_sub_type) -> bool:
    if resource_sub_type in [ResourceSubTypeEnum.ClusterComputeResource, ResourceSubTypeEnum.HostSystem,
                             ResourceSubTypeEnum.KubernetesNamespace, ResourceSubTypeEnum.HCSProject,
                             ResourceSubTypeEnum.OPENSTACK_PROJECT, ResourceSubTypeEnum.APSARA_STACK_ZONE,
                             ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET, ResourceSubTypeEnum.CNWARE_CLUSTER,
                             ResourceSubTypeEnum.CNWARE_HOST, ResourceSubTypeEnum.CNWARE_HOST_POOL,
                             ResourceSubTypeEnum.NUTANIX_CLUSTER, ResourceSubTypeEnum.NUTANIX_HOST,
                             ResourceSubTypeEnum.HYPER_V_HOST]:
        return True
    if resource_sub_type in [ResourceSubTypeEnum.FusionCompute, ResourceSubTypeEnum.FUSION_ONE_COMPUTE] \
            and resource_type in [ResourceTypeEnum.Cluster, ResourceTypeEnum.Host]:
        return True
    return False


def protect_by_resource_type(request_id, resource_list, sla,
                             batch_create_req: BatchProtectionExecuteReq) -> BatchOperationResult:
    """
    根据资源大类，执行不同的保护逻辑
    :param request_id:
    :param resource_list: 批量保护资源信息列表
    :param sla: 本次绑定的sla
    :param batch_create_req: 批量保护请求
    :return:
    """
    if is_need_protect_children_resource(ResourceTypeEnum(resource_list[0].get("type")),
                                         ResourceSubTypeEnum(resource_list[0].get("sub_type"))):
        return protect_resources(request_id, sla, batch_create_req, resource_list[0].get("sub_type"))
    elif ResourceSubTypeEnum(resource_list[0].get("sub_type")) in [ResourceSubTypeEnum.VirtualMachine]:
        return protect_vmware(request_id, resource_list, sla, batch_create_req)
    else:
        return protect_common_resource(request_id, resource_list, sla, batch_create_req)


def get_resource_obj_list(session, projected_object_list: List[ProtectedObject]) -> List[ProtectedObject]:
    """
    虚拟化数据移除保护
    :param session:
    :param projected_object_list: 保护对象列表
    :return:
    """
    all_object_list = []
    all_object_list.extend(projected_object_list)
    for projected_object in projected_object_list:
        plm = ProtectionPluginManager(ResourceSubTypeEnum(projected_object.sub_type))
        sub_resources = plm.query_sub_resources_by_obj(projected_object)
        sub_resource_ids = list(resource.get("uuid") for resource in sub_resources)
        sub_object_list = db.projected_object.query_multi_by_params(db=session, conditions=[
            ProtectedObject.resource_id.in_(sub_resource_ids),
            ProtectedObject.sla_id == projected_object.sla_id
        ])
        all_object_list.extend(sub_object_list)
    return all_object_list


def get_stateful_resource_obj(session, projected_object_list: List[ProtectedObject]) -> List[ProtectedObject]:
    """
    虚拟化数据移除保护
    :param session:
    :param projected_object_list: 保护对象列表
    :return:
    """
    all_object_list = []
    all_object_list.extend(projected_object_list)
    for projected_object in projected_object_list:
        sub_resources = ResourceClient.query_resource_list(
            {"path": projected_object.path + os.sep, "type": ResourceTypeEnum.StatefulSet})
        sub_resource_ids = list(resource.get("uuid") for resource in sub_resources)
        sub_object_list = db.projected_object.query_multi_by_params(db=session, conditions=[
            ProtectedObject.resource_id.in_(sub_resource_ids),
            ProtectedObject.sla_id == projected_object.sla_id
        ])
        all_object_list.extend(sub_object_list)
    return all_object_list


def get_common_resource_obj(projected_object_list: List[ProtectedObject]) -> List[ProtectedObject]:
    """
    通用资源移除保护逻辑
    :param projected_object_list: 保护对象列表
    :return:
    """
    return projected_object_list


def get_protect_obj_by_resource_type(session, projected_object_list: List[ProtectedObject]) -> List[ProtectedObject]:
    """
    批量移除保护
    根据资源大类，执行不同的保护逻辑
    :param session:
    :param projected_object_list: 保护对象列表
    :return:
    """
    protect_obj_list = []
    if projected_object_list is None or len(projected_object_list) == 0:
        return protect_obj_list
    if ResourceSubTypeEnum(projected_object_list[0].sub_type) in [ResourceSubTypeEnum.ClusterComputeResource,
                                                                  ResourceSubTypeEnum.HostSystem,
                                                                  ResourceSubTypeEnum.KubernetesNamespace,
                                                                  ResourceSubTypeEnum.HCSProject,
                                                                  ResourceSubTypeEnum.FusionCompute,
                                                                  ResourceSubTypeEnum.FUSION_ONE_COMPUTE,
                                                                  ResourceSubTypeEnum.OPENSTACK_PROJECT,
                                                                  ResourceSubTypeEnum.CNWARE_CLUSTER,
                                                                  ResourceSubTypeEnum.CNWARE_HOST,
                                                                  ResourceSubTypeEnum.NUTANIX_CLUSTER,
                                                                  ResourceSubTypeEnum.NUTANIX_HOST,
                                                                  ResourceSubTypeEnum.HYPER_V_HOST,
                                                                  ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET,
                                                                  ResourceSubTypeEnum.APSARA_STACK_ZONE
                                                                  ]:
        protect_obj_list.extend(get_resource_obj_list(session, projected_object_list))
    else:
        protect_obj_list.extend(get_common_resource_obj(projected_object_list))
    return protect_obj_list


def remove_protection(session, projected_object_list: List[ProtectedObject]):
    resource_ids = list(projected_object.resource_id for projected_object in projected_object_list)
    tasks = list(itertools.chain.from_iterable(list(obj.task_list for obj in projected_object_list)))
    schedule_ids = list(task.uuid for task in tasks)
    SchedulerClient.batch_delete_schedules(schedule_ids)
    # 删除条件数据库记录
    db.projected_object.delete_by_condition(db=session, conditions=[
        ProtectedObject.resource_id.in_(resource_ids)
    ])
    for projected_object in projected_object_list:
        # 移除保护时发送消息到副本管理微服务，处理副本清理
        resource_id = projected_object.resource_id
        msg = ProtectionRemoveEvent(
            request_id=str(uuid.uuid4()), resource_id=resource_id, sla_id=projected_object.sla_id)
        producer.produce(msg)


def container_protection_modify(sla, execute_req: ModifyProtectionExecuteReq) -> BatchOperationResult:
    """
    容器类资源修改保护
    :param projected_object:
    :param sla:
    :param execute_req: 修改保护执行请求
    :return:
    """
    job_id = execute_req.job_id
    request_id = execute_req.request_id
    resource_id = execute_req.resource_id
    resource_name = None
    result = BatchOperationResult()
    try:
        with database.session() as session:
            projected_object = db.projected_object.query_one_by_resource_id(
                db=session, resource_id=execute_req.resource_id)
            projected_object.ext_parameters = execute_req.ext_parameters.json()
            if execute_req.is_sla_modify:
                schedule_ids = list(task.uuid for task in projected_object.task_list)
                SchedulerClient.batch_delete_schedules(schedule_ids)
                plm = ProtectionPluginManager(
                    ResourceSubTypeEnum(projected_object.sub_type))
                task_list = plm.build_task_list(sla, resource_id, projected_object, execute_req)
                projected_object = update_protected_object_fields(projected_object, sla, task_list)
                projected_object.sla_compliance = None
            resource_name = projected_object.name
            session.add(projected_object)
            protection_status = ProtectionStatusEnum.protected \
                if projected_object.status == Status.Active.value else ProtectionStatusEnum.unprotected
            resource_service.update_protection_status(session, resource_id_list=[resource_id],
                                                      protection_status=protection_status)
        result.append_success_id(resource_id)
        BatchProtectionService.record_job_step(job_id, request_id,
                                               ResourceProtectionModifyJobSteps.PROTECTION_MODIFY_EXECUTING_SUCCESS,
                                               JobLogLevel.INFO, [resource_name])
        log.debug(f"Resource[{resource_id}] modify protection success.")
    except Exception as es:
        log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
        log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
        log.exception(f"Resource[{resource_id}] modify protection failed.")
        BatchProtectionService.record_job_step(job_id, request_id,
                                               ResourceProtectionModifyJobSteps.PROTECTION_MODIFY_EXECUTING_FAILED,
                                               JobLogLevel.ERROR, [resource_name], log_detail, log_detail_param)
        result.append_failed_id(resource_id)
    return result


def sub_resources_protection_modify(sla, execute_req: ModifyProtectionExecuteReq) -> BatchOperationResult:
    """
    修改子资源保护
    :param protected_object:
    :param original_sla_id: 修改前的sla_id
    :param sla:
    :param execute_req: 修改保护请求
    :return:
    """
    extended_parameters = execute_req.ext_parameters
    result = BatchOperationResult()
    resource_info = resource_service.query_resource_by_id(execute_req.resource_id)
    plm = ProtectionPluginManager(
        ResourceSubTypeEnum(execute_req.resource_sub_type))
    sub_resources = plm.query_sub_resources(resource_info)
    if sub_resources is None or len(sub_resources) <= 0:
        return result
    sub_resource_ids = list(sub_resource.get("uuid")
                            for sub_resource in sub_resources)
    resource_map = dict()
    for sub_resource in sub_resources:
        resource_map[sub_resource.get("uuid")] = sub_resource
    # 过滤符合条件的虚拟机uuid
    protected_resource_ids = filter_resources(sub_resources, extended_parameters)
    if len(protected_resource_ids) <= 0:
        # 如果过滤之后无没有匹配的资源,删除所有绑定了相同SLA的保护对象及对应的调度任务
        with database.session() as session:
            protected_objects = db.projected_object.query_multi_by_params(db=session, conditions=[
                ProtectedObject.resource_id.in_(sub_resource_ids),
                ProtectedObject.sla_id == sla.get("uuid")
            ])
            remove_protection(session, protected_objects)
            remove_resource_ids = []
            for obj in protected_objects:
                remove_resource_ids.append(obj.resource_id)
            resource_service.update_protection_status(session, remove_resource_ids, ProtectionStatusEnum.unprotected)
        return result
    with database.session() as session:
        existed_objects = db.projected_object.query_multi_by_params(db=session, conditions=[
            ProtectedObject.resource_id.in_(protected_resource_ids)
        ])
        # 处理需要修改资源
        modify_result = handle_need_modify(session, sla, existed_objects, resource_map, execute_req)
        result.merge_result(modify_result)
    # 处理需要删除的资源
    delete_result = handle_need_delete(sla, protected_resource_ids, sub_resource_ids, execute_req)
    result.merge_result(delete_result)
    # 处理新增资源
    create_result = handle_need_create(sla, existed_objects, protected_resource_ids, resource_map, execute_req)
    result.merge_result(create_result)

    return result


def handle_need_delete(sla, protected_resource_ids, sub_resource_ids,
                       execute_req: ModifyProtectionExecuteReq) -> BatchOperationResult:
    """
    处理需要被删除的资源
    :param execute_req:
    :param sla:
    :param protected_resource_ids:
    :param sub_resource_ids:
    :return:
    """
    result = BatchOperationResult()
    # 取被过滤的资源
    delete_object_ids = list(set(sub_resource_ids).difference(protected_resource_ids))
    if len(delete_object_ids) <= 0:
        return result
    for delete_id in delete_object_ids:
        delete_result = do_remove_protection(execute_req.request_id, execute_req.job_id, delete_id, sla)
        result.merge_result(delete_result)
    return result


def do_remove_protection(request_id: str, job_id: str, unprotect_id: str, sla):
    result = BatchOperationResult()
    resource_name = None
    try:
        with database.session() as session:
            # 过滤掉的资源中，已经绑定了相同SLA的保护对象需要删除
            deleted_objects = db.projected_object.query_multi_by_params(db=session, conditions=[
                ProtectedObject.resource_id == unprotect_id,
                ProtectedObject.sla_id == sla.get("uuid")
            ])
            if len(deleted_objects) <= 0:
                return result
            resource_name = deleted_objects[0].name
            remove_protection(session, deleted_objects)
            resource_service.update_protection_status(session, [deleted_objects[0].resource_id],
                                                      ProtectionStatusEnum.unprotected)
    except Exception as es:
        log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
        log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
        result.append_failed_id(unprotect_id)
        BatchProtectionService.record_job_step(job_id, request_id,
                                               ResourceProtectionModifyJobSteps.PROTECTION_REMOVE_FAILED,
                                               JobLogLevel.ERROR, log_detail, log_detail_param, [resource_name])
        log.exception(f"Resource[{unprotect_id}] remove protection failed.")
    result.append_success_id(unprotect_id)
    BatchProtectionService.record_job_step(job_id, request_id,
                                           ResourceProtectionModifyJobSteps.PROTECTION_REMOVE_SUCCESS,
                                           JobLogLevel.INFO, [resource_name])
    log.debug(f"Resource[{unprotect_id}] remove protection success.")
    return result


def handle_need_create(sla, existed_objects, protected_resource_ids, resource_map,
                       execute_req: ModifyProtectionExecuteReq) -> BatchOperationResult:
    """
    处理需要新创建的资源
    :param sla:
    :param existed_objects:
    :param protected_resource_ids:
    :param resource_map:
    :param execute_req:
    :return:
    """
    request_id = execute_req.request_id
    job_id = execute_req.job_id
    ext_parameters = execute_req.ext_parameters
    result = BatchOperationResult()
    existed_object_resource_ids = list(protected_obj.resource_id for protected_obj in existed_objects)
    create_object_ids = list(set(protected_resource_ids).difference(existed_object_resource_ids))
    if len(create_object_ids) <= 0:
        return result
    # 根据高级参数字段对新增资源创建保护对象及调度任务
    if SlaApplyType.APPLY_TO_ALL not in ext_parameters.binding_policy:
        return result
    for resource_id in create_object_ids:
        resource_info = resource_map.get(resource_id)
        try:
            plm = ProtectionPluginManager(ResourceSubTypeEnum(resource_info.get("sub_type")))
            ext_params = plm.convert_extend_parameter(ext_parameters.disk_filters, resource_info, ext_parameters)
            if ext_params is None:
                continue
            protection_obj = None
            if check_is_need_build_task(resource_info.get("type"), resource_info.get("sub_type")):
                protection_obj = build_protection_object(resource_info, sla, ext_params)
            else:
                protection_obj = build_protection_object_without_task(resource_info, sla, ext_params)
            with database.session() as session:
                session.add(protection_obj)
            result.append_success_id(resource_id)
            resource_service.update_protection_status(session, [resource_id], ProtectionStatusEnum.protected)
            BatchProtectionService.record_job_step(job_id, request_id,
                                                   ResourceProtectionModifyJobSteps.PROTECTION_CREATE_SUCCESS,
                                                   JobLogLevel.INFO, [resource_info.get("name"), sla.get("name")])
            log.debug(f"Resource[{resource_id}] create protection success.")
        except Exception as es:
            log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
            log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
            result.append_failed_id(resource_id)
            BatchProtectionService.record_job_step(job_id, request_id,
                                                   ResourceProtectionModifyJobSteps.PROTECTION_CREATE_FAILED,
                                                   JobLogLevel.ERROR, log_detail, log_detail_param,
                                                   [resource_info.get("name"), sla.get("name")])
            log.exception(f"Resource[{resource_id}] create protection failed.")
    return result


def handle_need_modify(session, sla, existing_object_list,
                       resource_map, execute_req: ModifyProtectionExecuteReq):
    """
    处理需要修改的资源
    :param session:
    :param sla:
    :param existing_object_list:
    :param resource_map:
    :param execute_req:
    :return:
    """
    request_id = execute_req.request_id
    job_id = execute_req.job_id
    ext_parameters = execute_req.ext_parameters
    result = BatchOperationResult()
    for projected_object in existing_object_list:
        resource_id = projected_object.resource_id
        resource_name = projected_object.name
        try:
            resource_info = resource_map.get(resource_id)
            plm = ProtectionPluginManager(ResourceSubTypeEnum(resource_info.get("sub_type")))
            ext_params = plm.convert_extend_parameter(ext_parameters.disk_filters, resource_info, ext_parameters)
            if ext_params is None:
                remove_protection(session, [projected_object])
                continue
            else:
                _fill_base_esn_and_target_ext_parameters(ext_params, projected_object)
            # 为绑定相同SLA的资源不处理
            if projected_object.sla_id == sla.get('uuid') and \
                    ext_params.json() == projected_object.ext_parameters:
                continue
            # 不覆盖虚拟机已有的SLA策略 && 虚拟机已有的SLA策略和主机（或集群）修改前的SLA策略不同
            if not ext_parameters.overwrite and projected_object.sla_id != execute_req.origin_sla_id:
                continue
            update_protection_object(execute_req, ext_params, projected_object, resource_id, session)
            result.append_success_id(resource_id)
            handle_modify_success(job_id, request_id, resource_name)
        except Exception as es:
            log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
            log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
            log.exception(f"Resource[{resource_id}] modify protection failed.")
            result.append_failed_id(resource_id)
            handle_modify_failed(job_id, request_id, resource_name, log_detail, log_detail_param)
    return result


def _fill_base_esn_and_target_ext_parameters(ext_params, projected_object):
    if not projected_object.ext_parameters:
        return
    if ext_params is None:
        return
    if isinstance(projected_object.ext_parameters, str):
        ob_ext_parameters = json.loads(projected_object.ext_parameters)
    else:
        ob_ext_parameters = projected_object.ext_parameters
    if not ob_ext_parameters:
        return
    ext_params.first_backup_esn = ob_ext_parameters.get("first_backup_esn", None)
    ext_params.last_backup_esn = ob_ext_parameters.get("last_backup_esn", None)
    ext_params.priority_backup_esn = ob_ext_parameters.get("priority_backup_esn", None)
    ext_params.first_backup_target = ob_ext_parameters.get("first_backup_target", None)
    ext_params.last_backup_target = ob_ext_parameters.get("last_backup_target", None)
    ext_params.priority_backup_target = ob_ext_parameters.get("priority_backup_target", None)
    ext_params.failed_node_esn = ob_ext_parameters.get("failed_node_esn", None)


def handle_modify_failed(job_id, request_id, resource_name, log_detail, log_detail_param):
    BatchProtectionService.record_job_step(
        job_id=job_id, request_id=request_id,
        job_step_label=ResourceProtectionModifyJobSteps.PROTECTION_MODIFY_EXECUTING_FAILED,
        log_level=JobLogLevel.ERROR, log_info_param=[resource_name],
        log_detail=log_detail, log_detail_param=log_detail_param)


def handle_modify_success(job_id, request_id, resource_name):
    BatchProtectionService.record_job_step(job_id, request_id,
                                           ResourceProtectionModifyJobSteps.PROTECTION_MODIFY_EXECUTING_SUCCESS,
                                           JobLogLevel.INFO, [resource_name])


def update_protection_object(execute_req, ext_params, projected_object, resource_id, session):
    projected_object.ext_parameters = ext_params.json()
    if execute_req.sla_id != projected_object.sla_id:
        schedule_ids = list(task.uuid for task in projected_object.task_list)
        SchedulerClient.batch_delete_schedules(schedule_ids)
        sla = ProtectionClient.query_sla(execute_req.sla_id)
        task_list = []
        if check_is_need_build_task(projected_object.type, projected_object.sub_type):
            task_list = list(build_protection_task(
                projected_object.uuid,
                policy,
                "schedule." + policy.get("type"),
                build_schedule_params(policy.get("type"), resource_id, sla.get("uuid"),
                                      projected_object.chain_id, policy,
                                      ExecuteType.AUTOMATIC.value)
            ) for policy in sla.get("policy_list") if filter_sla_policy(sla, policy))
        projected_object = update_protected_object_fields(projected_object, sla, task_list)
    session.add(projected_object)


def container_resource_protection_modify(execute_req: ModifyProtectionExecuteReq) -> BatchOperationResult:
    """
    容器类虚拟化资源修改保护
    1.修改容器类资源保护
    2.修改子资源保护
    :param protected_object: 保护对象
    :param execute_req: 保护修改执行请求
    :return:
    """
    # 记录修改前的sla_id
    current_sla = ProtectionClient.query_sla(execute_req.sla_id)
    # 容器修改保护
    result = container_protection_modify(current_sla, execute_req)
    if not result.is_success():
        return result
    # 子资源修改保护
    sub_resources_result = sub_resources_protection_modify(current_sla, execute_req)
    result.merge_result(sub_resources_result)
    return result


def common_resource_protection_modify(execute_req: ModifyProtectionExecuteReq) -> BatchOperationResult:
    """
    非容器类资源修改保护
    :param execute_req:
    :return:
    """
    result = BatchOperationResult()
    job_id = execute_req.job_id
    request_id = execute_req.request_id
    resource_id = execute_req.resource_id
    resource_name = None
    is_resource_group = execute_req.is_resource_group
    is_group_sub_resource = execute_req.is_group_sub_resource
    topic = GROUP_BACKUP_TOPIC if is_resource_group else ""
    try:
        resource_obj = get_resource_obj(is_resource_group, resource_id)
        with database.session() as session:
            projected_object = db.projected_object.query_one_by_resource_id(db=session,
                                                                            resource_id=execute_req.resource_id)
            projected_object.ext_parameters = execute_req.ext_parameters.json()
            if execute_req.is_sla_modify:
                schedule_ids = list(task.uuid for task in projected_object.task_list)
                SchedulerClient.batch_delete_schedules(schedule_ids)
                sla = ProtectionClient.query_sla(execute_req.sla_id)
                task_list = list(build_protection_task(projected_object.uuid, policy,
                                                       topic or "schedule." + policy.get("type"),
                                                       build_schedule_params(topic or policy.get("type"),
                                                                             resource_id, sla.get("uuid"),
                                                                             projected_object.chain_id, policy,
                                                                             ExecuteType.AUTOMATIC.value)
                                                       ) for policy in sla.get("policy_list") if
                                 filter_sla_policy(sla, policy, is_resource_group, is_group_sub_resource))
                projected_object = update_protected_object_fields(projected_object, sla, task_list)
            session.add(projected_object)
            protection_status = ProtectionStatusEnum.protected \
                if projected_object.status == Status.Active.value else ProtectionStatusEnum.unprotected
            resource_service.update_protection_status(session, resource_id_list=[resource_id],
                                                      protection_status=protection_status,
                                                      is_resource_group=is_resource_group)
        result.append_success_id(resource_id)
        resource_name = resource_obj.get("name")
        BatchProtectionService.record_job_step(job_id, request_id,
                                               ResourceProtectionModifyJobSteps.PROTECTION_MODIFY_EXECUTING_SUCCESS,
                                               JobLogLevel.INFO, [resource_name])
    except Exception as es:
        log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
        log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
        log.exception(f"Resource[{resource_id}] modify protection failed.")
        BatchProtectionService.record_job_step(
            job_id=job_id, request_id=request_id,
            job_step_label=ResourceProtectionModifyJobSteps.PROTECTION_MODIFY_EXECUTING_FAILED,
            log_level=JobLogLevel.ERROR, log_info_param=[resource_name],
            log_detail=log_detail, log_detail_param=log_detail_param)
    return result


def get_resource_obj(is_resource_group, resource_id):
    if is_resource_group:
        resource_obj = resource_service.query_resource_group_by_id(resource_id).as_dict()
        resource_obj["type"] = resource_obj.get("source_type", "")
        resource_obj["sub_type"] = resource_obj.get("source_sub_type", "")
    else:
        resource_obj = ResourceClient.query_resource(resource_id)
    return resource_obj


def update_protected_object_fields(projected_object, sla, task_list):
    projected_object.task_list = task_list
    projected_object.sla_id = sla.get("uuid")
    projected_object.sla_name = sla.get("name")
    if is_need_generate_new_chain_id(ResourceSubTypeEnum(projected_object.sub_type)):
        projected_object.chain_id = str(uuid.uuid4())
    return projected_object


def modify_protection_by_resource_type(execute_req: ModifyProtectionExecuteReq) -> BatchOperationResult:
    """
    根据资源大类，执行不同的保护逻辑
    :param execute_req:
    :return:
    """
    if is_need_protect_children_resource(execute_req.resource_type, execute_req.resource_sub_type):
        return container_resource_protection_modify(execute_req)
    elif execute_req.is_resource_group:
        return group_resource_protection_modify(execute_req)
    else:
        return common_resource_protection_modify(execute_req)


def group_resource_protection_modify(execute_req: ModifyProtectionExecuteReq) -> BatchOperationResult:
    """
    资源组修改保护
    1.修改资源组保护
    2.修改子资源保护
    :param protected_object: 保护对象
    :param execute_req: 保护修改执行请求
    :return:
    """
    # 资源组修改改保护
    result = common_resource_protection_modify(execute_req)
    if not result.is_success():
        return result
    # 子资源修改保护 查询出所有子资源
    sub_resources = resource_service.query_protected_resource_by_group_id(execute_req.resource_id)
    if not sub_resources or len(sub_resources) <= 0:
        return result
    for sub_resource in sub_resources:
        # 不处理资源组本身
        if sub_resource.uuid == execute_req.resource_id:
            continue
        sub_execute_req = ModifyProtectionExecuteReq(
            sla_id=str(execute_req.sla_id), resource_id=sub_resource.uuid,
            ext_parameters=execute_req.ext_parameters,
            job_id=execute_req.job_id, is_sla_modify=execute_req.is_sla_modify,
            resource_type=sub_resource.type, resource_sub_type=sub_resource.sub_type,
            origin_sla_id=execute_req.origin_sla_id, is_resource_group=False,
            is_group_sub_resource=True
        )
        sub_resources_result = common_resource_protection_modify(sub_execute_req)
        result.merge_result(sub_resources_result)
    return result


def sync_sub_resource_add(session: Session, resource):
    """
    判断虚拟机的父资源是否已经存在保护对象
    1.已经存在，说明主机或集群已经保护，查询对应保护对象的保护策略，根据策略中的内容,
      判断当前对象是否需要保护
        1.1如果需要保护，则绑定与主机或集群相同的SLA
        1.2如果不需要保护，则直接结束
    2.如果不存在，直接结束
    """
    parent_uuid = resource.get("parent_uuid")
    resource_uuid = resource.get("uuid")
    if resource.get("sub_type") == ResourceSubTypeEnum.VirtualMachine.value:
        # VMware主机集群保护勾选“将集群SLA应用到新创建的虚拟机上”，则集群和主机下的其他层级中新增的虚拟机同步添加保护
        parent_resource = get_vm_parent_resource_info_only_cluster_or_host(resource)
        if parent_resource:
            parent_uuid = parent_resource.get("uuid")
    projected_object = db.projected_object.query_one_by_resource_id(db=session, resource_id=parent_uuid)
    if not projected_object:
        log.info(f"No protected object found, resource id: {resource_uuid}")
        return
    plm = ProtectionPluginManager(ResourceSubTypeEnum(resource.get("sub_type")))
    ext_parameters = plm.build_ext_parameters(projected_object.ext_parameters)
    if SlaApplyType.APPLY_TO_NEW not in ext_parameters.binding_policy:
        log.info(f"APPLY_TO_NEW is not in binding_policy, no need to add sla, resource id: {resource_uuid}")
        return
    need_protect_resource = filter_resources([resource], ext_parameters)
    if len(need_protect_resource) <= 0:
        log.info(f"Need protect resource size:{len(need_protect_resource)}, resource id: {resource_uuid}")
        return
    sla = ProtectionClient.query_sla(projected_object.sla_id)
    ext_params = plm.convert_extend_parameter(ext_parameters.disk_filters, resource, ext_parameters)
    # 无法构造高级参数，直接忽略这个子资源
    if ext_params is None or sla is None:
        log.info(f"Ext_params is None or sla is None, resource id: {resource_uuid}")
        return
    protected_object = build_protection_object(resource, sla, ext_params)
    session.add(protected_object)
    # 更新资源表中的保护状态为已保护
    resource_service.update_protection_status(session=session, resource_id_list=[resource.get("uuid")],
                                              protection_status=ProtectionStatusEnum.protected)


def get_vm_parent_resource_info_only_cluster_or_host(resource):
    parent_uuid = resource.get("parent_uuid")
    resource_uuid = resource.get("uuid")
    if not parent_uuid:
        log.info(f"No parent uuid found, resource uuid: {resource_uuid}.")
        return None
    parent_resource = ResourceClient.query_resource(parent_uuid)
    if not parent_resource:
        log.info(f"No parent resource found, resource uuid: {resource_uuid}.")
        return None
    sub_type = parent_resource.get("sub_type")
    if sub_type in [ResourceSubTypeEnum.ClusterComputeResource.value, ResourceSubTypeEnum.HostSystem.value]\
            or parent_uuid == resource.get("root_uuid"):
        log.info(f"Found resource(uuid: {resource_uuid})'s parent resource(uuid: {parent_uuid}, type:{sub_type}).")
        return parent_resource
    else:
        return get_vm_parent_resource_info_only_cluster_or_host(parent_resource)


def batch_protect_host_pre_check(resource_obj, first_os_type, ext_params):
    """
    主机校验 是否离线、操作类型是否相同
    :param resource_obj:  资源对象
    :param first_os_type: first_os_type
    :return:
    """
    if ResourceSubTypeEnum(resource_obj.get("sub_type")) in [ResourceSubTypeEnum.DBBackupAgent,
                                                             ResourceSubTypeEnum.EXCHANGE_GROUP,
                                                             ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE,
                                                             ResourceSubTypeEnum.VMBackupAgent]:
        if resource_obj.get("link_status") == 0:
            raise EmeiStorBizException(error=CommonErrorCodes.STATUS_ERROR,
                                       message=f"resource [{resource_obj.get('root_uuid')}] is not linked")
        if first_os_type != resource_obj.get('os_type'):
            raise EmeiStorBizException(error=CommonErrorCodes.STATUS_ERROR,
                                       message=f"resources have different os types")
    elif ResourceSubTypeEnum(resource_obj.get("sub_type")) in [ResourceSubTypeEnum.EXCHANGE_DATABASE,
                                                               ResourceSubTypeEnum.EXCHANGE_MAILBOX]:
        root_res = ResourceClient.query_v2_resource(resource_obj.get('root_uuid'))
        if root_res.get("linkStatus") == str(0):
            raise EmeiStorBizException(error=CommonErrorCodes.STATUS_ERROR,
                                       message=f"root of resource [{resource_obj.get('root_uuid')}] is not linked")
    else:
        if ext_params is None:
            return
        if hasattr(ext_params, "agents"):
            agents = ext_params.agents
            log.info("agents is " + str(agents))
            if agents is None:
                return
            separator = ","
            if ";" in str(agents):
                separator = ";"
            agent_arr = str(agents).split(separator)
            for agent in agent_arr:
                if len(agent) == 0:
                    continue
                resource = ResourceClient.query_v2_resource(agent)
                # 检查agent是否离线
                if resource.get("linkStatus") == str(LinkStatusEnum.Offline.value):
                    raise EmeiStorBizException(error=ResourceErrorCodes.HOST_OFFLINE, message="The host is offline.")


def check_hyper_v_disk_info(resource_obj):
    res = ResourceClient.query_v2_resource(resource_obj.get("uuid"))
    if res.get("subType") != ResourceSubTypeEnum.HYPER_V_VM.value:
        return
    disks = res.get("extendInfo").get("disks")
    if not disks:
        raise EmeiStorBizException(error=ResourceErrorCodes.VIRTUAL_MACHINE_DISK_INFO_IS_EMPTY,
                                   message='Failed to obtain the cloud server disk information.')
    disk_info_list = json.loads(disks)
    if len(disk_info_list) == 0:
        raise EmeiStorBizException(error=ResourceErrorCodes.VIRTUAL_MACHINE_DISK_INFO_IS_EMPTY,
                                   message='Failed to obtain the cloud server disk information.')
    for disk_info in disk_info_list:
        hyper_v_is_shared = disk_info.get("extendInfo").get("IsShared")
        if hyper_v_is_shared == 'true':
            raise EmeiStorBizException(error=ResourceErrorCodes.NOT_SUPPORT_SHARED_DISK,
                                       message='Fail to protect hyper-v, because not to support shared disk.')
        hyper_v_is_physical_hard_disk = disk_info.get("extendInfo").get("IsPhysicalHardDisk")
        if hyper_v_is_physical_hard_disk == 'true':
            raise EmeiStorBizException(error=ResourceErrorCodes.NOT_SUPPORT_PHYSICAL_HARD_DISK,
                                       message='Fail to protect hyper-v, because not to support physical hard disk.')
        hyper_v_format = disk_info.get("extendInfo").get("Format")
        if HypervFormatEnum.VHDSet == hyper_v_format:
            raise EmeiStorBizException(error=ResourceErrorCodes.NOT_SUPPORT_VHD_SET_DISK,
                                       message='Fail to protect hyper-v, because not to support vhdx set disk.')


def check_single_policy_and_worm_policy(resource_obj, policy):
    worm_switch = policy.get("worm_validity_type")
    if worm_switch != WormValidityType.WORM_NOT_OPEN:
        policy = AntiRansomwareClient.query_policy_by_resource_id(resource_obj.get("uuid"))
        if policy.get("id") and policy.get("schedule", {}).get("setWorm", False):
            log.error("Sla worm and anti worm policy turn on cannot together exist.")
            raise EmeiStorBizException(error=CommonErrorCodes.BOTH_SLA_WORM_AND_ANTI_RANSOMWARE_WORM_TURN_ON,
                                   message=f"Sla worm and anti worm policy turn on cannot together exist.")


def check_worm_switch_and_worm_policy(resource_obj, sla_obj):
    """
    校验SLA的worm开关和worm策略中的开关是否同时开启
    :param resource_obj:  资源对象
    :param sla_obj:  sla保护
    :return:
    """
    for policy in sla_obj.get("policy_list"):
        if policy.get("type") == PolicyTypeEnum.backup.value:
            check_single_policy_and_worm_policy(resource_obj, policy)


def batch_protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params):
    """
    根据资源类型进行保护前置校验
    :param sla_obj：sla对象
    :param resource_obj：资源对象
    :param first_os_type：操作系统类型
    :return:
    """
    resource_type = ResourceSubTypeEnum(resource_obj.get("sub_type")).value
    processor_manager.do_validate(resource_type, resource_obj)
    batch_protect_host_pre_check(resource_obj, first_os_type, ext_params)
    check_worm_switch_and_worm_policy(resource_obj, sla_obj)
    check_open_gauss_database_resource_matches_sla(resource_type, sla_obj)
    check_dameng_resource_matches_sla(resource_type, sla_obj)
    check_db2_tablespace_resource_matches_sla(resource_type, sla_obj)
    check_exchange_resource_matches_sla(resource_type, sla_obj)
    check_exchange_resource_has_related(resource_obj)
    check_tidb_resource_matches_sla(resource_type, sla_obj)
    check_ndmp_resource_matches_sla(resource_type, sla_obj)
    check_oceanbase_resource_matches_sla(resource_type, sla_obj)
    check_tdsql_resource_matches_sla(resource_type, sla_obj)
    check_apsara_stack_resource_matches_sla(resource_type, sla_obj)
    check_gaussdbt_single_matches_sla(resource_type, sla_obj)
    check_generaldb_matches_sla(resource_obj, sla_obj)
    check_eapp_mysql_matches_sla(resource_obj, sla_obj)
    check_oracle_matches_sla(resource_obj, sla_obj, ext_params)
    check_hyper_v_disk_info(resource_obj)
    check_vmware_matches_sla(resource_type, sla_obj)
    check_can_be_protected_for_anti_ransomware(resource_type, ext_params)
    if check_resource_not_need_validate(ResourceSubTypeEnum(resource_obj.get("sub_type"))):
        return
    sla_type = ResourceSubTypeEnum(sla_obj.get("application")).value
    check_resource_type(ext_params, resource_obj, resource_type, sla_obj)
    resource_type = get_resource_type(resource_type)
    # SQLServer,NasShare,NasFileSystem,FileSet,MongoDB,PostgreSQL,KingBase不能绑定common类型的sla
    if sla_type == ResourceSubTypeEnum.Common.value and (
            resource_type in [ResourceSubTypeEnum.NasShare.value, ResourceSubTypeEnum.NasFileSystem.value,
                              ResourceSubTypeEnum.NdmpBackupSet.value,
                              ResourceSubTypeEnum.Fileset.value, ResourceSubTypeEnum.Volume.value,
                              ResourceSubTypeEnum.MongoDB.value, ResourceSubTypeEnum.PostgreSQL.value,
                              ResourceSubTypeEnum.KingBase.value, ResourceSubTypeEnum.SQLServer.value,
                              ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.value, ResourceSubTypeEnum.AD.value]):
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   message="The SLA type is inconsistent with the resource type")

    if (sla_type != ResourceSubTypeEnum.Common.value and
            sla_type != resource_type and not
            (resource_type == ResourceSubTypeEnum.ORACLE_PDB.value and sla_type == ResourceSubTypeEnum.Oracle.value)):
        log.error(f"[batch_protect_pre_check] protect sla_type:{sla_type} resource_type:{resource_type}.")
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   message="The SLA type is inconsistent with the resource type")
    # 校验该资源之前的备份副本所在的存储单元：1.等于当前SLA中选择的存储单元2.被包含在在当前SLA选择的存储单元组中
    ProtectionClient.check_exist_copies_location_before_protect(sla_obj.get("uuid"), resource_obj.get('uuid'))


def check_resource_not_need_validate(resource_sub_type):
    return resource_sub_type not in \
        [ResourceSubTypeEnum.MicroSoftVirtualMachine,
         ResourceSubTypeEnum.VirtualMachine, ResourceSubTypeEnum.Fileset, ResourceSubTypeEnum.Volume,
         ResourceSubTypeEnum.KubernetesStatefulSet, ResourceSubTypeEnum.KubernetesNamespace,
         ResourceSubTypeEnum.HCSCloudHost, ResourceSubTypeEnum.HCSProject,
         ResourceSubTypeEnum.APSARA_STACK_ZONE, ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET,
         ResourceSubTypeEnum.APSARA_STACK_INSTANCE, ResourceSubTypeEnum.APSARA_STACK,
         ResourceSubTypeEnum.DB2Database, ResourceSubTypeEnum.DB2Tablespace, ResourceSubTypeEnum.FUSION_ONE_COMPUTE,
         ResourceSubTypeEnum.FusionCompute, ResourceSubTypeEnum.SQLServerInstance,
         ResourceSubTypeEnum.SQLServerClusterInstance, ResourceSubTypeEnum.SQLServerDatabase,
         ResourceSubTypeEnum.HDFSFileset, ResourceSubTypeEnum.HBaseBackupSet,
         ResourceSubTypeEnum.HiveBackupSet, ResourceSubTypeEnum.ElasticSearchBackupSet,
         ResourceSubTypeEnum.NasShare, ResourceSubTypeEnum.NasFileSystem,
         ResourceSubTypeEnum.NdmpBackupSet,
         ResourceSubTypeEnum.SQLServerAlwaysOn, ResourceSubTypeEnum.MysqlInstance,
         ResourceSubTypeEnum.MysqlClusterInstance, ResourceSubTypeEnum.MysqlDatabase,
         ResourceSubTypeEnum.DamengSingleNode, ResourceSubTypeEnum.DamengCluster,
         ResourceSubTypeEnum.OpenGaussDatabase, ResourceSubTypeEnum.OpenGaussInstance,
         ResourceSubTypeEnum.KingBaseInstance, ResourceSubTypeEnum.KingBaseClusterInstance,
         ResourceSubTypeEnum.DWSCluster, ResourceSubTypeEnum.DWSTable, ResourceSubTypeEnum.DWSSchema,
         ResourceSubTypeEnum.DWSDateBase,
         ResourceSubTypeEnum.PostgreClusterInstance, ResourceSubTypeEnum.PostgreInstance,
         ResourceSubTypeEnum.Oracle, ResourceSubTypeEnum.ORACLE_PDB, ResourceSubTypeEnum.GaussDBT,
         ResourceSubTypeEnum.Redis, ResourceSubTypeEnum.ClickHouse,
         ResourceSubTypeEnum.OPENSTACK_PROJECT, ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER,
         ResourceSubTypeEnum.HCS_GAUSSDB_PROJECT, ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE,
         ResourceSubTypeEnum.TPOPS_GAUSSDB_PROJECT, ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE,
         ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE,
         ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP, ResourceSubTypeEnum.SAPHANA_DATABASE,
         ResourceSubTypeEnum.GOLDENDB_CLUSTER_INSTANCE, ResourceSubTypeEnum.EXCHANGE_DATABASE,
         ResourceSubTypeEnum.EXCHANGE_MAILBOX, ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE,
         ResourceSubTypeEnum.MONGODB_CLUSTER, ResourceSubTypeEnum.MONGODB_SINGLE,
         ResourceSubTypeEnum.InformixSingleInstance, ResourceSubTypeEnum.InformixClusterInstance,
         ResourceSubTypeEnum.OCEANBASE_TENANT, ResourceSubTypeEnum.GAUSSDBT_SINGLE, ResourceSubTypeEnum.TiDB_CLUSTER,
         ResourceSubTypeEnum.TiDB_DATABASE, ResourceSubTypeEnum.TiDB_TABLE, ResourceSubTypeEnum.COMMON_SHARE,
         ResourceSubTypeEnum.SAP_ON_ORACLE, ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE,
         ResourceSubTypeEnum.EXCHANGE_ONLINE, ResourceSubTypeEnum.EXCHANGE_ONLINE_BACKUP_SET]


def get_resource_type(resource_type):
    resource_mapping = {
        ResourceSubTypeEnum.NUTANIX.value: ResourceSubTypeEnum.NUTANIX_VM.value,
        ResourceSubTypeEnum.NUTANIX_HOST.value: ResourceSubTypeEnum.NUTANIX_VM.value,
        ResourceSubTypeEnum.NUTANIX_CLUSTER.value: ResourceSubTypeEnum.NUTANIX_VM.value,
        ResourceSubTypeEnum.CNWARE.value: ResourceSubTypeEnum.CNWARE_VM.value,
        ResourceSubTypeEnum.CNWARE_HOST.value: ResourceSubTypeEnum.CNWARE_VM.value,
        ResourceSubTypeEnum.CNWARE_CLUSTER.value: ResourceSubTypeEnum.CNWARE_VM.value,
        ResourceSubTypeEnum.APSARA_STACK_ZONE.value: ResourceSubTypeEnum.APSARA_STACK.value,
        ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET.value: ResourceSubTypeEnum.APSARA_STACK.value,
        ResourceSubTypeEnum.APSARA_STACK_INSTANCE.value: ResourceSubTypeEnum.APSARA_STACK.value,
        ResourceSubTypeEnum.HYPER_V_HOST.value: ResourceSubTypeEnum.HYPER_V_VM.value,
        ResourceSubTypeEnum.KubernetesNamespace.value: ResourceSubTypeEnum.KubernetesStatefulSet.value,
        ResourceSubTypeEnum.HCSProject.value: ResourceSubTypeEnum.HCSCloudHost.value,
        ResourceSubTypeEnum.OPENSTACK_PROJECT.value: ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.value,
        ResourceSubTypeEnum.SQLServerInstance: ResourceSubTypeEnum.SQLServer.value,
        ResourceSubTypeEnum.SQLServerClusterInstance: ResourceSubTypeEnum.SQLServer.value,
        ResourceSubTypeEnum.SQLServerAlwaysOn: ResourceSubTypeEnum.SQLServer.value,
        ResourceSubTypeEnum.SQLServerDatabase: ResourceSubTypeEnum.SQLServer.value,
        ResourceSubTypeEnum.MysqlClusterInstance: ResourceSubTypeEnum.MySQL.value,
        ResourceSubTypeEnum.MysqlDatabase: ResourceSubTypeEnum.MySQL.value,
        ResourceSubTypeEnum.MysqlCluster: ResourceSubTypeEnum.MySQL.value,
        ResourceSubTypeEnum.MysqlInstance: ResourceSubTypeEnum.MySQL.value,
        ResourceSubTypeEnum.PostgreInstance: ResourceSubTypeEnum.PostgreSQL.value,
        ResourceSubTypeEnum.PostgreClusterInstance: ResourceSubTypeEnum.PostgreSQL.value,
        ResourceSubTypeEnum.PostgreCluster: ResourceSubTypeEnum.PostgreSQL.value,
        ResourceSubTypeEnum.OpenGaussInstance: ResourceSubTypeEnum.OpenGauss.value,
        ResourceSubTypeEnum.OpenGaussDatabase: ResourceSubTypeEnum.OpenGauss.value,
        ResourceSubTypeEnum.SAPHANA_INSTANCE: ResourceSubTypeEnum.SAPHANA_INSTANCE.value,
        ResourceSubTypeEnum.SAPHANA_DATABASE: ResourceSubTypeEnum.SAPHANA_DATABASE.value,
        ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.value: ResourceSubTypeEnum.SAP_ON_ORACLE.value,
        ResourceSubTypeEnum.EXCHANGE_ONLINE.value: ResourceSubTypeEnum.EXCHANGE_ONLINE.value,
        ResourceSubTypeEnum.EXCHANGE_ONLINE_BACKUP_SET.value: ResourceSubTypeEnum.EXCHANGE_ONLINE.value
    }
    resource_mapping.update(get_resource_mapping_extra())
    return resource_mapping.get(resource_type, resource_type)


def get_resource_mapping_extra():
    resource_mapping = {
        ResourceSubTypeEnum.DamengCluster: ResourceSubTypeEnum.Dameng.value,
        ResourceSubTypeEnum.DamengSingleNode: ResourceSubTypeEnum.Dameng.value,
        ResourceSubTypeEnum.DWSCluster: ResourceSubTypeEnum.DWSCluster.value,
        ResourceSubTypeEnum.DWSDateBase: ResourceSubTypeEnum.DWSCluster.value,
        ResourceSubTypeEnum.DWSSchema: ResourceSubTypeEnum.DWSCluster.value,
        ResourceSubTypeEnum.DWSTable: ResourceSubTypeEnum.DWSCluster.value,
        ResourceSubTypeEnum.KingBaseInstance: ResourceSubTypeEnum.KingBase.value,
        ResourceSubTypeEnum.KingBaseCluster: ResourceSubTypeEnum.KingBase.value,
        ResourceSubTypeEnum.KingBaseClusterInstance: ResourceSubTypeEnum.KingBase.value,
        ResourceSubTypeEnum.DB2Database.value: ResourceSubTypeEnum.DB2.value,
        ResourceSubTypeEnum.DB2Tablespace.value: ResourceSubTypeEnum.DB2.value,
        ResourceSubTypeEnum.GOLDENDB_CLUSTER_INSTANCE: ResourceSubTypeEnum.GOLDENDB.value,
        ResourceSubTypeEnum.MONGODB_SINGLE: ResourceSubTypeEnum.MongoDB.value,
        ResourceSubTypeEnum.MONGODB_CLUSTER: ResourceSubTypeEnum.MongoDB.value,
        ResourceSubTypeEnum.InformixSingleInstance: ResourceSubTypeEnum.InformixService.value,
        ResourceSubTypeEnum.InformixClusterInstance: ResourceSubTypeEnum.InformixService.value,
        ResourceSubTypeEnum.OCEANBASE_CLUSTER: ResourceSubTypeEnum.OCEANBASE_CLUSTER.value,
        ResourceSubTypeEnum.OCEANBASE_TENANT: ResourceSubTypeEnum.OCEANBASE_CLUSTER.value,
        ResourceSubTypeEnum.GAUSSDBT_SINGLE.value: ResourceSubTypeEnum.GaussDBT.value,
        ResourceSubTypeEnum.TiDB_CLUSTER: ResourceSubTypeEnum.TiDB.value,
        ResourceSubTypeEnum.TiDB_DATABASE: ResourceSubTypeEnum.TiDB.value,
        ResourceSubTypeEnum.TiDB_TABLE: ResourceSubTypeEnum.TiDB.value,
        ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON: ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.value,
        ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON: ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.value,
        ResourceSubTypeEnum.EXCHANGE_MAILBOX: ResourceSubTypeEnum.EXCHANGE_DATABASE.value,
        ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE: ResourceSubTypeEnum.EXCHANGE_DATABASE.value,
        ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE: ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE.value,
        ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP: ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE.value,
        ResourceSubTypeEnum.SAP_ON_ORACLE.value: ResourceSubTypeEnum.SAP_ON_ORACLE.value,
        ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.value: ResourceSubTypeEnum.SAP_ON_ORACLE.value,
        ResourceSubTypeEnum.EXCHANGE_ONLINE.value: ResourceSubTypeEnum.EXCHANGE_ONLINE.value,
        ResourceSubTypeEnum.EXCHANGE_ONLINE_BACKUP_SET.value: ResourceSubTypeEnum.EXCHANGE_ONLINE.value
    }
    return resource_mapping


def check_resource_type(ext_params, resource_obj, resource_type, sla_obj):
    if resource_type == ResourceSubTypeEnum.VirtualMachine.value:
        from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
        disks = VMwareDiscoveryService.get_vm_disks(resource_obj.get("uuid"))
        if not disks:
            raise EmeiStorBizException(error=ResourceErrorCodes.VIRTUAL_MACHINE_DISK_INFO_IS_EMPTY,
                                       message="Failed to get virtual machine disk information.")
    elif resource_type == ResourceSubTypeEnum.KubernetesStatefulSet.value:
        check_kubernetes_volume_names(resource_obj, ext_params)
    elif resource_type == ResourceSubTypeEnum.HCSCloudHost.value:
        check_hcs_disk_info(resource_obj, ext_params)
    elif resource_type == ResourceSubTypeEnum.Fileset.value and ext_params:
        check_action_and_small_file(sla_obj.get("policy_list"), ext_params)
    elif resource_type in [ResourceSubTypeEnum.SQLServerInstance, ResourceSubTypeEnum.SQLServerClusterInstance,
                           ResourceSubTypeEnum.SQLServerDatabase]:
        check_sql_server_protected_resource(resource_obj)
    elif resource_type in [ResourceSubTypeEnum.DB2Database, ResourceSubTypeEnum.DB2Tablespace]:
        check_db2_protected_resource(resource_obj)


def check_db2_protected_resource(resource_obj):
    with database.session() as session:
        resource_id = resource_obj.get("uuid")
        if resource_obj.get("sub_type") == ResourceSubTypeEnum.DB2Tablespace.value:
            count = session.query(ProtectedObject.uuid).filter(and_(
                ProtectedObject.resource_id == str(resource_obj.get("parent_uuid")),
                ProtectedObject.sub_type == ResourceSubTypeEnum.DB2Database.value)).count()
        else:
            tablespaces = session.query(ResourceTable.uuid).filter(ResourceTable.parent_uuid == resource_id).all()
            tablespace_uuids = list(i[0] for i in tablespaces)
            count = session.query(ProtectedObject.uuid).filter(
                ProtectedObject.resource_id.in_(tablespace_uuids)).count()
        if count > 0:
            log.error(f"Db2 protect {resource_id} fail: check related protect object size: {count}")
            raise EmeiStorBizException(error=CommonErrorCodes.CAN_NOT_PROTECT_BOTH_DATABASE_AND_TABLESPACE,
                                       message="Related resource is protected.")


def check_sql_server_protected_resource(resource_obj):
    with database.session() as session:
        resource_id = resource_obj.get("uuid")
        if resource_obj.get("sub_type") == ResourceSubTypeEnum.SQLServerDatabase.value:
            count = session.query(ProtectedObject).filter(and_(
                ProtectedObject.resource_id == str(resource_obj.get("parent_uuid")),
                ProtectedObject.sub_type.in_([ResourceSubTypeEnum.SQLServerInstance.value,
                                              ResourceSubTypeEnum.SQLServerClusterInstance.value]))).count()
        else:
            databases = session.query(ResourceTable).filter(ResourceTable.parent_uuid == resource_id).all()
            database_uuids = list(parent.uuid for parent in databases)
            count = session.query(ProtectedObject).filter(ProtectedObject.resource_id.in_(database_uuids)).count()
        sub_type = resource_obj.get("sub_type")
        log.info(f"[SQL Server] protect {resource_id} sub type:{sub_type}: check related protect obj len: {count}")
        if count > 0:
            log.error(f"[SQL Server] protect {resource_id} fail: check related protect obj len: {count}")
            raise EmeiStorBizException(error=CommonErrorCodes.CAN_NOT_PROTECT_BOTH_INSTANCE_AND_DATABASE,
                                       message="Related resource is protected.")


processor_manager = ValidatorManager()


def check_ext_params(sla_obj, resource_obj, ext_params, resource_type):
    # 文件集sla高级参数中小文件聚合和永久增量备份冲突
    if resource_type == ResourceSubTypeEnum.Fileset.value and ext_params:
        check_action_and_small_file(sla_obj.get("policy_list"), ext_params)
    # 校验kubernetes的volume_names是否合法
    elif resource_type == ResourceSubTypeEnum.KubernetesStatefulSet.value:
        check_kubernetes_volume_names(resource_obj, ext_params)
    # 校验hcs的磁盘信息是否合法
    elif resource_type == ResourceSubTypeEnum.HCSCloudHost.value:
        check_hcs_disk_info(resource_obj, ext_params)
    # 校验VMware是否包含共享类型是多写入器类型的磁盘（共享盘，不支持对此种磁盘进行备份）
    elif resource_type == ResourceSubTypeEnum.VirtualMachine.value:
        check_vmware_disk_sharing_type(resource_obj, ext_params)


def get_mappings_list():
    return {**get_db_mapping(), **get_misc_mapping()}


def get_db_mapping():
    return {
        # postgres 实例和集群实例都当做 PostgresSQL 处理
        ResourceSubTypeEnum.PostgreSQL.value: [ResourceSubTypeEnum.PostgreInstance.value,
                                               ResourceSubTypeEnum.PostgreClusterInstance.value],
        # Dameng 单机和集群都当做 Dameng 处理
        ResourceSubTypeEnum.Dameng.value: [ResourceSubTypeEnum.DamengSingleNode.value,
                                           ResourceSubTypeEnum.DamengCluster.value],
        ResourceSubTypeEnum.SAP_ON_ORACLE.value: [ResourceSubTypeEnum.SAP_ON_ORACLE.value,
                                                  ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.value],
        # KingBase 实例和集群实例都当做 KingBase 处理
        ResourceSubTypeEnum.KingBase.value: [ResourceSubTypeEnum.KingBaseInstance.value,
                                             ResourceSubTypeEnum.KingBaseClusterInstance.value],
        # MySQL 单实例，集群实例，数据库都当做 MySQL 来处理
        ResourceSubTypeEnum.MySQL.value: [ResourceSubTypeEnum.MysqlClusterInstance, ResourceSubTypeEnum.MysqlDatabase,
                                          ResourceSubTypeEnum.MysqlInstance],
        # OpenGauss 实例，数据库都当做 OpenGauss 来处理
        ResourceSubTypeEnum.OpenGauss.value: [ResourceSubTypeEnum.OpenGaussInstance.value,
                                              ResourceSubTypeEnum.OpenGaussDatabase.value],
        # DB2 数据库、表空间都当做 DB2 来处理
        ResourceSubTypeEnum.DB2.value: [ResourceSubTypeEnum.DB2Database.value, ResourceSubTypeEnum.DB2Tablespace.value],
        # SQL Server 单实例、集群实例、数据库、可用性组都当做 SQL Server 来处理
        ResourceSubTypeEnum.SQLServer.value: [ResourceSubTypeEnum.SQLServerInstance.value,
                                              ResourceSubTypeEnum.SQLServerDatabase.value,
                                              ResourceSubTypeEnum.SQLServerAlwaysOn.value,
                                              ResourceSubTypeEnum.SQLServerClusterInstance.value],
        # NasShare, NasFileSystem 不能绑定 common 类型的 SLA
        ResourceSubTypeEnum.DWSCluster.value: [ResourceSubTypeEnum.DWSCluster.value,
                                               ResourceSubTypeEnum.DWSDateBase.value,
                                               ResourceSubTypeEnum.DWSSchema.value, ResourceSubTypeEnum.DWSTable.value],
        ResourceSubTypeEnum.OCEANBASE_CLUSTER.value: [ResourceSubTypeEnum.OCEANBASE_CLUSTER.value,
                                                      ResourceSubTypeEnum.OCEANBASE_TENANT.value],
        # TDSQL 集群实例 TDSQL-clusterInstance 不当做 TDSQL 来处理
        ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE.value: [ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE.value,
                                                           ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP.value],
        # GoldenDB 集群实例当做 GoldenDB 来处理
        ResourceSubTypeEnum.GOLDENDB.value: [ResourceSubTypeEnum.GOLDENDB_CLUSTER_INSTANCE.value],
        ResourceSubTypeEnum.MongoDB.value: [ResourceSubTypeEnum.MONGODB_SINGLE.value,
                                            ResourceSubTypeEnum.MONGODB_CLUSTER.value],
        ResourceSubTypeEnum.InformixService.value: [ResourceSubTypeEnum.InformixSingleInstance.value,
                                                    ResourceSubTypeEnum.InformixClusterInstance.value],
        ResourceSubTypeEnum.GaussDBT.value: [ResourceSubTypeEnum.GAUSSDBT_SINGLE.value],
        ResourceSubTypeEnum.TiDB.value: [ResourceSubTypeEnum.TiDB_CLUSTER.value,
                                         ResourceSubTypeEnum.TiDB_DATABASE.value, ResourceSubTypeEnum.TiDB_TABLE.value],
        ResourceSubTypeEnum.AntDB.value: [ResourceSubTypeEnum.AntDBInstance.value,
                                          ResourceSubTypeEnum.AntDBClusterInstance.value],

        # Exchange 修改保护资源统一为 Database
        ResourceSubTypeEnum.EXCHANGE_DATABASE.value: [ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.value,
                                                      ResourceSubTypeEnum.EXCHANGE_MAILBOX.value,
                                                      ResourceSubTypeEnum.EXCHANGE_GROUP.value],
        ResourceSubTypeEnum.SAPHANA_DATABASE.value: [ResourceSubTypeEnum.SAPHANA_DATABASE.value],
        ResourceSubTypeEnum.EXCHANGE_ONLINE.value: [ResourceSubTypeEnum.EXCHANGE_ONLINE.value,
                                                    ResourceSubTypeEnum.EXCHANGE_ONLINE_BACKUP_SET.value]
    }


def get_misc_mapping():
    return {
        # k8s namespace 当做 statefulSet 处理
        ResourceSubTypeEnum.KubernetesStatefulSet.value: [ResourceSubTypeEnum.KubernetesNamespace.value],
        ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.value: [ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.value,
                                                              ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON.value],
        # HCSProject 当做 HCSCloudHost 处理
        ResourceSubTypeEnum.HCSCloudHost.value: [ResourceSubTypeEnum.HCSProject.value],
        # vmware 集群和 vmware 主机都当做 vmware 处理
        ResourceSubTypeEnum.VirtualMachine.value: [ResourceSubTypeEnum.ClusterComputeResource.value,
                                                   ResourceSubTypeEnum.HostSystem.value],
        ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.value: [ResourceSubTypeEnum.OPENSTACK_PROJECT.value],
        ResourceSubTypeEnum.CNWARE_VM.value: [ResourceSubTypeEnum.CNWARE.value, ResourceSubTypeEnum.CNWARE_HOST.value,
                                              ResourceSubTypeEnum.CNWARE_CLUSTER.value],
        ResourceSubTypeEnum.NUTANIX_VM.value: [ResourceSubTypeEnum.NUTANIX.value,
                                               ResourceSubTypeEnum.NUTANIX_HOST.value,
                                               ResourceSubTypeEnum.NUTANIX_CLUSTER.value],
        ResourceSubTypeEnum.APSARA_STACK.value: [ResourceSubTypeEnum.APSARA_STACK_ZONE.value,
                                                 ResourceSubTypeEnum.APSARA_STACK_RESOURCE_SET.value,
                                                 ResourceSubTypeEnum.APSARA_STACK_INSTANCE.value],
        ResourceSubTypeEnum.HYPER_V_VM.value: [ResourceSubTypeEnum.HYPER_V_HOST.value]
    }


def protect_pre_check(sla_obj, resource_obj, first_os_type, ext_params, is_sla_changed):
    """
    根据资源类型和sla类型进行保护前置校验
    :param resource_obj:  资源对象
    :param sla_obj:  sla保护
    :param first_os_type: 主机类型
    :param ext_params: 扩展参数
    :return:
    """
    batch_protect_host_pre_check(resource_obj, first_os_type, ext_params)
    check_worm_switch_and_worm_policy(resource_obj, sla_obj)
    resource_type = ResourceSubTypeEnum(resource_obj.get("sub_type")).value
    sla_type = ResourceSubTypeEnum(sla_obj.get("application")).value
    processor_manager.do_validate(resource_type, resource_obj)
    check_open_gauss_database_resource_matches_sla(resource_type, sla_obj)
    check_dameng_resource_matches_sla(resource_type, sla_obj)
    check_db2_tablespace_resource_matches_sla(resource_type, sla_obj)
    check_exchange_resource_matches_sla(resource_type, sla_obj)
    check_exchange_resource_has_related(resource_obj)
    check_tidb_resource_matches_sla(resource_type, sla_obj)
    check_antdb_resource_matches_sla(resource_type, sla_obj)
    check_ndmp_resource_matches_sla(resource_type, sla_obj)
    check_oceanbase_resource_matches_sla(resource_type, sla_obj)
    check_tdsql_resource_matches_sla(resource_type, sla_obj)
    check_apsara_stack_resource_matches_sla(resource_type, sla_obj)
    check_gaussdbt_single_matches_sla(resource_type, sla_obj)
    check_generaldb_matches_sla(resource_obj, sla_obj)
    check_eapp_mysql_matches_sla(resource_obj, sla_obj)
    check_ext_params(sla_obj, resource_obj, ext_params, resource_type)
    check_vmware_sla_modified(resource_type, sla_obj, is_sla_changed)
    mappings = get_mappings_list()
    matched_items = list(key for key, value in mappings.items() if resource_type in value) or [resource_type]
    resource_type = matched_items[0]
    # SQLServer,NasShare,NasFileSystem,MongoDB,PostgreSQL,KingBase,VMware不能绑定common类型的sla
    if sla_type == ResourceSubTypeEnum.Common.value and (
            resource_type in [ResourceSubTypeEnum.NasShare.value, ResourceSubTypeEnum.NasFileSystem.value,
                              ResourceSubTypeEnum.NdmpBackupSet.value,
                              ResourceSubTypeEnum.Fileset.value, ResourceSubTypeEnum.Volume.value,
                              ResourceSubTypeEnum.MongoDB.value, ResourceSubTypeEnum.PostgreSQL.value,
                              ResourceSubTypeEnum.KingBase.value, ResourceSubTypeEnum.SQLServer.value,
                              ResourceSubTypeEnum.TiDB_CLUSTER.value, ResourceSubTypeEnum.TiDB_DATABASE.value,
                              ResourceSubTypeEnum.TiDB_TABLE.value,
                              ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.value, ResourceSubTypeEnum.AD.value]):
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   message="The SLA type is inconsistent with the resource type")
    # oracle集群和oracle单机可以使用相同的sla
    if sla_type == ResourceSubTypeEnum.Oracle.value and (
            resource_type in [ResourceSubTypeEnum.Oracle.value, ResourceSubTypeEnum.ORACLE_CLUSTER.value,
                              ResourceSubTypeEnum.ORACLE_PDB.value]):
        return
    # ndmp和nas文件系统可以使用相同的sla
    if sla_type == ResourceSubTypeEnum.NasFileSystem.value and resource_type == ResourceSubTypeEnum.NdmpBackupSet.value:
        return
    # 资源类型和sla类型不一致抛异常
    if sla_type != ResourceSubTypeEnum.Common.value and sla_type != resource_type:
        log.error(f"[protect_pre_check] protect sla_type:{sla_type} resource_type:{resource_type}.")
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   message="The SLA type is inconsistent with the resource type")


def check_open_gauss_database_resource_matches_sla(resource_type, sla_obj):
    if resource_type != ResourceSubTypeEnum.OpenGaussDatabase.value:
        return
    for policy in sla_obj.get("policy_list"):
        if policy.get("action") == PolicyActionEnum.difference_increment.value:
            log.error("OpenGauss database does not support incremental backups")
            raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       message=f"OpenGauss database does not support incremental backups")


def check_dameng_resource_matches_sla(resource_type, sla_obj):
    if resource_type != ResourceSubTypeEnum.DamengCluster.value:
        return
    for policy in sla_obj.get("policy_list"):
        if policy.get("action") == PolicyActionEnum.log.value:
            log.error("Dameng cluster does not support log backup")
            raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       message=f"Dameng cluster does not support log backup")
        if policy.get("action") == PolicyActionEnum.archiving.value:
            log.error("Dameng cluster does not support archiving")
            raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       message=f"Dameng cluster does not support archiving")


def check_db2_tablespace_resource_matches_sla(resource_type, sla_obj):
    if resource_type != ResourceSubTypeEnum.DB2Tablespace.value:
        return
    for policy in sla_obj.get("policy_list", []):
        if policy.get("action") == PolicyActionEnum.log.value:
            log.error("Db2 tablespace does not support log backup.")
            raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       message=f"Db2 tablespace does not support log backup.")


def check_exchange_resource_matches_sla(resource_type, sla_obj):
    if resource_type in [ResourceSubTypeEnum.EXCHANGE_GROUP.value, ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.value,
                         ResourceSubTypeEnum.EXCHANGE_DATABASE.value]:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.cumulative_increment.value, PolicyActionEnum.difference_increment.value]:
                log.error(f'exchange {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'exchange {resource_type} does not support {action} backup.')

    if resource_type == ResourceSubTypeEnum.EXCHANGE_MAILBOX.value:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.permanent_increment.value, PolicyActionEnum.cumulative_increment.value,
                          PolicyActionEnum.log]:
                log.error(f"exchange {resource_type} does not support {action} backup.")
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f"exchange {resource_type} does not support {action} backup.")


def check_exchange_resource_has_related(resource_obj):
    resource_type = ResourceSubTypeEnum(resource_obj.get("sub_type")).value
    if resource_type in [ResourceSubTypeEnum.EXCHANGE_GROUP.value, ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.value]:
        child_resource_list: List[ResourceTable] = (resource_service.query_resource_by_parent_id(
            resource_obj.get("root_uuid")))
        for child_resouce in child_resource_list:
            if child_resouce.protection_status == ProtectionStatusEnum.protected and \
                    child_resouce.uuid != resource_obj.get("root_uuid") and \
                    child_resouce.sub_type != ResourceSubTypeEnum.EXCHANGE_MAILBOX:
                raise EmeiStorBizException(error=CommonErrorCodes.ERROR_EXCHANGE_PROTECT_CONFLICT,
                                           message=f'resource {child_resouce.name} has protected.')

    if resource_type == ResourceSubTypeEnum.EXCHANGE_DATABASE.value:
        parent_resource = resource_service.query_resource_by_id(resource_obj.get("root_uuid"))
        if parent_resource is not None and parent_resource.protection_status == ProtectionStatusEnum.protected:
            raise EmeiStorBizException(error=CommonErrorCodes.ERROR_EXCHANGE_PROTECT_CONFLICT,
                                       message=f'resource {parent_resource.name} has protected.')


def check_tidb_resource_matches_sla(resource_type, sla_obj):
    if resource_type == ResourceSubTypeEnum.TiDB_CLUSTER.value:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.cumulative_increment.value, PolicyActionEnum.difference_increment.value,
                          PolicyActionEnum.permanent_increment.value]:
                log.error(f'TiDB {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'TiDB {resource_type} does not support {action} backup.')

    if resource_type in [ResourceSubTypeEnum.TiDB_DATABASE.value, ResourceSubTypeEnum.TiDB_TABLE.value]:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.log.value, PolicyActionEnum.cumulative_increment.value,
                          PolicyActionEnum.difference_increment.value, PolicyActionEnum.permanent_increment.value]:
                log.error(f'TiDB {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'TiDB {resource_type} does not support {action} backup.')


def check_antdb_resource_matches_sla(resource_type, sla_obj):
    if resource_type in [ResourceSubTypeEnum.AntDBInstance.value, ResourceSubTypeEnum.AntDBClusterInstance.value]:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.cumulative_increment.value, PolicyActionEnum.difference_increment.value,
                          PolicyActionEnum.permanent_increment.value]:
                log.error(f'AntDB {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'AntDB {resource_type} does not support {action} backup.')


def check_ndmp_resource_matches_sla(resource_type, sla_obj):
    if resource_type == ResourceSubTypeEnum.NdmpBackupSet.value:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.log.value]:
                log.error(f'{resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'{resource_type} does not support {action} backup.')


def check_saphana_resource_matches_sla(resource_type, sla_obj):
    if resource_type == ResourceSubTypeEnum.SAPHANA_INSTANCE.value:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.cumulative_increment.value, PolicyActionEnum.permanent_increment.value]:
                log.error(f'Oceanbase {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'Oceanbase {resource_type} does not support {action} backup.')
    if resource_type == ResourceSubTypeEnum.SAPHANA_DATABASE.value:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.log.value, PolicyActionEnum.cumulative_increment.value,
                          PolicyActionEnum.difference_increment.value, PolicyActionEnum.permanent_increment.value]:
                log.error(f'Oceanbase {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'Oceanbase {resource_type} does not support {action} backup.')


def check_oceanbase_resource_matches_sla(resource_type, sla_obj):
    if resource_type == ResourceSubTypeEnum.OCEANBASE_CLUSTER.value:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.cumulative_increment.value, PolicyActionEnum.permanent_increment.value]:
                log.error(f'Oceanbase {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'Oceanbase {resource_type} does not support {action} backup.')
    if resource_type == ResourceSubTypeEnum.OCEANBASE_TENANT.value:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.log.value, PolicyActionEnum.cumulative_increment.value,
                          PolicyActionEnum.difference_increment.value, PolicyActionEnum.permanent_increment.value]:
                log.error(f'Oceanbase {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'Oceanbase {resource_type} does not support {action} backup.')


def check_tdsql_resource_matches_sla(resource_type, sla_obj):
    if resource_type == ResourceSubTypeEnum.TDSQL_CLUSTER_GROUP.value:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.cumulative_increment.value, PolicyActionEnum.difference_increment.value,
                          PolicyActionEnum.permanent_increment.value]:
                log.error(f'Tdsql {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'Tdsql {resource_type} does not support {action} backup.')
    if resource_type == ResourceSubTypeEnum.TDSQL_CLUSTER_INSTANCE.value:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.cumulative_increment.value, PolicyActionEnum.permanent_increment.value]:
                log.error(f'Tdsql {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'Tdsql {resource_type} does not support {action} backup.')


def check_apsara_stack_resource_matches_sla(resource_type, sla_obj):
    if resource_type == ResourceSubTypeEnum.APSARA_STACK_INSTANCE.value:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action in [PolicyActionEnum.cumulative_increment.value, PolicyActionEnum.log.value,
                          PolicyActionEnum.permanent_increment.value]:
                log.error(f'ApsaraStack {resource_type} does not support {action} backup.')
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f'ApsaraStack {resource_type} does not support {action} backup.')


def check_mongodb_single_instance_matches_sla(resource_type, sla_obj):
    if resource_type != ResourceSubTypeEnum.MONGODB_SINGLE.value:
        return
    for policy in sla_obj.get("policy_list", []):
        if policy.get("action") == PolicyActionEnum.log.value:
            log.error("MongoDB single instance does not support log backup.")
            raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       message=f"MongoDB single instance does not support log backup.")


def check_gaussdbt_single_matches_sla(resource_type, sla_obj):
    if resource_type != ResourceSubTypeEnum.GAUSSDBT_SINGLE.value:
        return
    sla_type = ResourceSubTypeEnum(sla_obj.get("application")).value
    if sla_type == ResourceSubTypeEnum.Common.value:
        log.error("GaussDBT single does not support common sla.")
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   message="GaussDBT single does not support common sla.")
    for policy in sla_obj.get("policy_list", []):
        if policy.get("action") in [PolicyActionEnum.difference_increment.value,
                                    PolicyActionEnum.cumulative_increment.value]:
            log.error("GaussDBT single does not support difference and cumulative backup.")
            raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       message=f"GaussDBT single does not support difference and cumulative backup.")


def check_generaldb_matches_sla(resource_obj, sla_obj):
    resource_sub_type = ResourceSubTypeEnum(resource_obj.get("sub_type")).value
    if resource_sub_type != ResourceSubTypeEnum.GENERAL_DB.value:
        return
    resource = ResourceClient.query_v2_resource(resource_obj.get('uuid'))
    if not resource.get("extendInfo") or "scriptConf" not in resource.get("extendInfo"):
        return
    script_conf = json.loads(resource.get("extendInfo").get("scriptConf"))
    if "backup" not in script_conf or "support" not in script_conf.get("backup"):
        return
    backup_type = []
    for support in script_conf.get("backup").get("support"):
        if "backupType" in support:
            backup_type.append(support.get("backupType"))
    if not backup_type:
        return
    database_type = ResourceSubTypeEnum.GENERAL_DB.value
    if "databaseType" in script_conf:
        database_type = script_conf.get("databaseType")
    for policy in sla_obj.get("policy_list", []):
        policy_action = policy.get("action")
        if policy_action in [PolicyActionEnum.replication.value, PolicyActionEnum.archiving.value]:
            continue
        if policy_action not in backup_type:
            log.error(f"{database_type} does not support {policy_action}.")
            raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       message=f"{database_type} does not support {policy_action}.")


def check_eapp_mysql_matches_sla(resource_obj, sla_obj):
    resource_sub_type = ResourceSubTypeEnum(resource_obj.get("sub_type")).value
    if resource_sub_type != ResourceSubTypeEnum.MysqlClusterInstance.value:
        return
    resource = ResourceClient.query_v2_resource(resource_obj.get('uuid'))
    extend_info = resource.get("extendInfo")
    if not extend_info or not extend_info.get("clusterType"):
        log.warn("Empty extend info or no cluster type")
        return
    cluster_type = extend_info.get("clusterType")
    if cluster_type != 'EAPP':
        log.info("No need check policy")
        return

    for policy in sla_obj.get("policy_list", []):
        action = policy.get("action")
        if action == PolicyActionEnum.log.value:
            log.error("eappmysql does not support %s", action)
            raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                       message=f"eappmysql instance does not support log backup.")


def check_oracle_matches_sla(resource_obj, sla_obj, ext_params):
    resource_sub_type = ResourceSubTypeEnum(resource_obj.get("sub_type")).value
    if resource_sub_type != ResourceSubTypeEnum.Oracle.value:
        return
    # storage_snapshot do not support cumulative increment
    if ext_params.storage_snapshot_flag:
        for policy in sla_obj.get("policy_list", []):
            action = policy.get("action")
            if action == PolicyActionEnum.cumulative_increment.value:
                raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                           message=f"storage snapshot Oracle does not support cumulative backup.")


def check_vmware_matches_sla(resource_type, sla_obj):
    sla_type = ResourceSubTypeEnum(sla_obj.get("application")).value
    # VMware不能绑定common类型的sla
    if sla_type == ResourceSubTypeEnum.Common.value and (
            resource_type in [ResourceSubTypeEnum.VirtualMachine.value, ResourceSubTypeEnum.HostSystem.value,
                              ResourceSubTypeEnum.ClusterComputeResource.value]):
        raise EmeiStorBizException(error=CommonErrorCodes.ILLEGAL_PARAMS,
                                   message="The SLA type is inconsistent with the resource type")


def check_vmware_sla_modified(resource_type, sla_obj, is_sla_changed):
    # 如果vmware的sla发生修改，则需要校验sla的类型
    if is_sla_changed:
        check_vmware_matches_sla(resource_type, sla_obj)


def get_validity_volume_path(resource_list):
    volumes = []
    types = []
    for resource in resource_list:
        if resource.filters is None:
            return tuple(volumes), tuple(types)
        filters = resource.filters
        for resource_filter in filters:
            volume = resource_filter.values
            filter_type = resource_filter.type
            volumes.append(volume)
            types.append(filter_type)
    return tuple(volumes), tuple(types)


# 校验主机卷下的路径参数是否合法
def check_validity_volume_path(resource_list):
    volumes, types = get_validity_volume_path(resource_list)
    for filter_type in types:
        if filter_type != "VOLUME":
            return
    for volume in volumes:
        if (volume is None) or len(volume) == 0:
            raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, error_message="volume path is illegal params")
        # 正则匹配路径
        win_regular = ProtectVolumeServiceEnum.WIN_REGULAR
        # 判断卷路径里面的每个path是否合法
        for path in volume:
            if path is None or len(path) == 0 or len(path) > ProtectVolumeServiceEnum \
                    .HOST_VOLUME_PATHS_MAX_LENGTH or path.isspace():
                raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, message="volume path is illegal params")
            if not (re.match(win_regular, path) or path.startswith('/')):
                raise EmeiStorBizException(CommonErrorCodes.ILLEGAL_PARAMS, message="volume path is illegal params")


# 校验k8s的pvc是否合法
def check_kubernetes_volume_names(resource_obj, ext_params):
    ext_volume_names = ext_params.volume_names
    res = ResourceClient.query_v2_resource(resource_obj.get("uuid"))
    volume_names = json.loads(res.get("extendInfo").get("sts")).get("volumeNames")
    if len(ext_volume_names) <= 0 or len(volume_names) <= 0:
        raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM, message="volume_names is invalid")
    for volume_name in ext_volume_names:
        if volume_name not in volume_names:
            raise EmeiStorBizException(CommonErrorCodes.ERR_PARAM, message="volume_names is invalid")


# 校验hcs的磁盘是否合法
def check_hcs_disk_info(resource_obj, ext_params):
    ext_disk_info = ext_params.disk_info
    res = ResourceClient.query_v2_resource(resource_obj.get("uuid"))
    disk_info = json.loads(res.get("extendInfo").get("host")).get("diskInfo")
    if not disk_info:
        raise EmeiStorBizException(ResourceErrorCodes.CLOUD_HOST_DISK_INFO_IS_EMPTY, message="disk_info is invalid")
    disk_id_info = []
    for disk in disk_info:
        disk_id_info.append(disk.get("id"))
    for ext_disk in ext_disk_info:
        if ext_disk not in disk_id_info:
            raise EmeiStorBizException(ResourceErrorCodes.CLOUD_HOST_DISK_INFO_IS_EMPTY, message="disk_info is invalid")


# 校验保护对象是否存在
def check_is_exist_projected_object(resource_list):
    resource_ids = []
    for protect_resource in resource_list:
        resource_ids.append(protect_resource.resource_id)
    with database.session() as session:
        projected_object_list = db.projected_object.query_by_resource_ids(db=session, resource_ids=resource_ids)
        if len(projected_object_list) > 0:
            raise EmeiStorBizException(error=CommonErrorCodes.SYSTEM_ERROR, message="projected_object exist")


def query_storage_info(storage_id):
    with database.session() as session:
        storage_info: StorageUnitTable = session.query(StorageUnitTable).filter(
            StorageUnitTable.id == storage_id).one_or_none()
        return storage_info


# 校验本地盘是否支持保护此资源
def check_is_local_disk_support_resource(resource_list, sla_id):
    storage_types = []
    # 支持本地盘备份的应用subtype集合
    local_disk_support_subtype = LocalDiskSupportSubType.local_disk_support_subtype
    sla = ProtectionClient.query_sla(sla_id)
    policy_lists = sla['policy_list']
    storage_ids = set()

    for policy in policy_lists:
        ext_parameters = policy.get('ext_parameters')
        if not ext_parameters:
            continue
        storage_info = ext_parameters.get('storage_info')
        if not storage_info:
            continue
        storage_id = storage_info.get('storage_id')
        if storage_id:
            storage_ids.add(storage_id)

    for storage_id in storage_ids:
        storage_info = query_storage_info(storage_id)
        if storage_info:
            storage_types.append(storage_info.device_type)
    log.info(f'disk_check storage_types: {storage_types} storage_ids: {storage_ids}')
    if not DeployType().is_dependent() and "BasicDisk" in storage_types:
        for protect_resource in resource_list:
            resource = ResourceClient.query_v2_resource(protect_resource.resource_id)
            resource_sub_type = get_resource_sub_type(resource)
            if resource_sub_type not in local_disk_support_subtype:
                raise EmeiStorBizException(error=CommonErrorCodes.BACK_UP_NOT_SUPPORT_BASIC_DISK,
                                           message="Operation failed because the "
                                                   "application cannot be backed up on the local disk.")


def get_resource_sub_type(resource):
    log.info(f'disk_check resources_subtype: {resource.get("subType")}')
    resource_sub_type = resource.get("subType")
    if resource_sub_type == ResourceSubTypeEnum.GENERAL_DB:
        # 资源类型是通用数据库时，取通用数据库的子类型判断是否支持保护
        if not resource.get("extendInfo"):
            return resource_sub_type
        general_db_sub_type = resource.get("extendInfo", {}).get("databaseTypeDisplay", "")
        resource_sub_type = GeneralDbSubTypeMappingEnum.get_resource_sub_type(general_db_sub_type)
        log.info(f'disk_check general_db_sub_type: {general_db_sub_type}, resource_sub_type: {resource_sub_type}')
    return resource_sub_type


# 将受保护对象按照资源类型分类
def group_by_sub_type(projected_object_list, type_set):
    projected_object_list_group = []
    for type_elem in type_set:
        single_projected_object_list = []
        for projected_object in projected_object_list:
            if projected_object.sub_type == type_elem:
                single_projected_object_list.append(projected_object)
        if single_projected_object_list:
            projected_object_list_group.append(single_projected_object_list)
    return projected_object_list_group


# 修改sla时，校验VMware是否包含共享类型是多写入器类型的磁盘（共享盘，不支持对此种磁盘进行备份）
def check_vmware_disk_sharing_type(resource_obj, ext_params):
    ext_disk_info = ext_params.disk_info
    res, root_rs = get_resource_by_res_id(resource_obj.get("uuid"))
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(res.root_uuid)
    from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
    service = VMwareDiscoveryService(service_instance, env)
    multi_writer_disk_info = service.get_disk_sharing_type_is_multi_writer(res)
    clear(env.password)
    multi_writer_disk_uuid_info = []
    for disk in multi_writer_disk_info:
        multi_writer_disk_uuid_info.append(disk.get("uuid"))
    for ext_disk in ext_disk_info:
        if ext_disk in multi_writer_disk_uuid_info:
            log.error(f"Sharing type of disk: {ext_disk} is multi-writer, this disk cannot be protected.")
            raise EmeiStorBizException(ResourceErrorCodes.NOT_SUPPORT_PROTECT_MULTI_WRITER_DISK,
                                       message="Disks of the multi-writer sharing type cannot be protected.")


def get_resource_by_res_id(res_id):
    resource = resource_service.query_resource_by_id(res_id)
    if resource is None:
        raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[])
    root_res = resource_service.query_environment({"uuid": resource.root_uuid})
    if root_res is None:
        raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[])
    return resource, root_res[0]


# 添加sla时，校验VMware是否包含共享类型是多写入器类型的磁盘（共享盘，不支持对此种磁盘进行备份）
def check_disk_sharing_multi_writer(protect_resource, resource):
    log.info(f"Start to check disk sharing type of resource: {protect_resource.resource_id}")
    resource_type = ResourceSubTypeEnum(resource.get("sub_type")).value
    if resource_type != ResourceSubTypeEnum.VirtualMachine.value:
        return
    res, root_rs = get_resource_by_res_id(protect_resource.resource_id)
    env = ScanEnvSchema(**root_rs)
    service_instance = service_instance_manager.get_service_instance(res.root_uuid)
    from app.resource.service.vmware.vmware_discovery_service import VMwareDiscoveryService
    service = VMwareDiscoveryService(service_instance, env)
    multi_writer_disk_info = service.get_disk_sharing_type_is_multi_writer(res)
    clear(env.password)
    multi_writer_disk_uuid_info = []
    for disk in multi_writer_disk_info:
        multi_writer_disk_uuid_info.append(disk.get("uuid"))
    if protect_resource.filters is None:
        raise EmeiStorBizException(
            error=CommonErrorCodes.OBJ_NOT_EXIST, parameters=[])
    filters = protect_resource.filters
    for resource_filter in list(filter(
            lambda x: x.type == FilterType.DISK and x.filter_by == FilterColumn.ID and
                      x.rule == FilterRule.ALL and x.mode == FilterMode.INCLUDE,
            filters)):
        disk_uuid_list = resource_filter.values
        if '*' in disk_uuid_list and len(multi_writer_disk_uuid_info) > 0:
            log.error(f"Has multi-writer sharing type disk, this disk cannot be protected.")
            raise EmeiStorBizException(ResourceErrorCodes.NOT_SUPPORT_PROTECT_MULTI_WRITER_DISK,
                                       message="Disks of the multi-writer sharing type cannot be protected.")
        for disk_uuid in disk_uuid_list:
            if disk_uuid in multi_writer_disk_uuid_info:
                log.error(f"Sharing type of disk: {disk_uuid} is multi-writer, this disk cannot be protected.")
                raise EmeiStorBizException(ResourceErrorCodes.NOT_SUPPORT_PROTECT_MULTI_WRITER_DISK,
                                           message="Disks of the multi-writer sharing type cannot be protected.")


def update_cyber_engine_self_learning_config(ext_parameters, device_id, resource_id):
    # 开启自学习
    self_learning_config = update_self_learning_config(resource_id,
                                                       device_id,
                                                       ext_parameters.is_open,
                                                       ext_parameters.type,
                                                       ext_parameters.duration)
    # 更新自学习的字段
    ext_parameters.is_open = self_learning_config.get('is_open')
    ext_parameters.type = self_learning_config.get('type')
    ext_parameters.duration = self_learning_config.get('duration')
    ext_parameters.progress = self_learning_config.get('progress')


def get_job(job_extend_params, job_message, request_id, resource, user_id):
    domain_id_list = domain_resource_object_service.get_domain_id_list(resource.get("uuid"))
    job_id = toolkit.create_job_center_task(request_id, {
        'requestId': request_id,
        'sourceId': resource.get("uuid"),
        'sourceName': resource.get("name"),
        'sourceType': resource.get("type"),
        'sourceSubType': resource.get("sub_type"),
        'sourceLocation': resource.get("path", ""),
        "type": JobType.RESOURCE_PROTECTION.value,
        'userId': user_id,
        'domain_id_list': domain_id_list,
        'enableStop': False,
        "extendField": job_extend_params
    }, job_message)
    return job_id


def check_can_be_protected_for_anti_ransomware(resource_type, ext_params):
    """
    校验主存防勒索文件系统是否配置了互斥特性
    :param resource_type: 资源类型
    :param ext_params: 扩展参数
    """
    if not DeployType().is_hyper_detect_deploy_type():
        return
    if resource_type in [ResourceSubTypeEnum.CloudBackupFileSystem.value]:
        file_system_id = ext_params.file_system_ids
        log.info(f"file_system_id: {file_system_id}")
        if not file_system_id:
            log.warning("not exists file_system_id.")
            return
        file_system_info = SystemBaseClient.query_filesystem(file_system_id[0])
        if file_system_info.get("hasSmartMobility", "") == "1":
            raise EmeiStorBizException(error=ProtectionErrorCodes.CLOUDBACKUP_SMART_MOBILITY_CANNOT_BACKUP,
                                       message="resource has smart mobility, can not protect.")


class BatchProtectionService:

    @staticmethod
    def submit_batch_protection_task(user_id: str, batch_create_req: BatchProtectionSubmitReq):
        resource_list = batch_create_req.resources
        sla_id = batch_create_req.sla_id
        for resource in resource_list:
            if (DeployType().is_dependent() and
                    projected_object_service.check_resource_has_anti_or_worm(resource.resource_id)):
                projected_object_service.check_is_support_worm_and_anti(sla_id)
        # 校验本地盘是否支持保护此资源
        check_is_local_disk_support_resource(resource_list, batch_create_req.sla_id)
        # 校验是否存在保护对象
        check_is_exist_projected_object(resource_list)
        # 增加values的参数校验
        check_validity_volume_path(resource_list)
        sla = ProtectionClient.query_sla(batch_create_req.sla_id)
        # 填充保护对象的扩展字段
        fill_protection_ext_parameters(sla, batch_create_req.ext_parameters)
        resource_ids = [resource.resource_id for resource in resource_list]
        first_os_type = None
        for protect_resource in resource_list:
            resource = ResourceClient.query_resource(
                protect_resource.resource_id)
            first_os_type = first_os_type if first_os_type else \
                (first_os_type if 'os_type' not in resource else resource.get('os_type'))
            batch_protect_pre_check(sla, resource, first_os_type, batch_create_req.ext_parameters)
            # 添加sla时，校验VMware是否包含共享类型是多写入器类型的磁盘（共享盘，不支持对此种磁盘进行备份）
            check_disk_sharing_multi_writer(protect_resource, resource)
        # 增加extendField字段，包含SLA信息
        job_extend_params = {
            "sla_name": sla.get("name"),
            "sla_id": sla.get("uuid")
        }
        job_ids = []
        BatchProtectionService.create_protection_schedule(batch_create_req, job_extend_params, job_ids, resource_list,
                                                          user_id)
        return job_ids

    @staticmethod
    def create_protection_schedule(batch_create_req, job_extend_params, job_ids, resource_list, user_id):
        for protect_resource in resource_list:
            request_id = str(uuid.uuid4())
            resource = ResourceClient.query_resource(protect_resource.resource_id)
            execute_req = BatchProtectionExecuteReq(
                sla_id=batch_create_req.sla_id, resources=[protect_resource],
                ext_parameters=batch_create_req.ext_parameters, post_action=batch_create_req.post_action,
                job_id=request_id, user_id=user_id, resource_group_id=batch_create_req.resource_group_id
            )
            # 此功能仅支持安全一体机并且接入设备为OP
            if (DeployType().is_cyber_engine_deploy_type()
                    and resource.get('environment_sub_type') == ResourceSubTypeEnum.CYBERENGINE_OCEAN_PROTECT):
                update_cyber_engine_self_learning_config(batch_create_req.ext_parameters,
                                                         resource.get('environment_uuid'),
                                                         protect_resource.resource_id)
                log.info(f"add sla and set self learning {batch_create_req.ext_parameters} " +
                         f"for resource {protect_resource.resource_id}")
            job_message = JobMessage(
                topic=topics.RESOURCE_PROTECTION_SCHEDULE, payload=execute_req.dict(),
            )
            job_id = get_job(job_extend_params, job_message, request_id, resource, user_id)
            if not job_id:
                raise EmeiStorBizException(
                    error=CommonErrorCodes.SYSTEM_ERROR, message="create resource protection job failed.")
            job_ids.append(job_id)
            # 创建保护时发送消息到副本管理微服务，处理副本清理。域内复制不移除复制链路
            resource_id = protect_resource.resource_id
            msg = ProtectionRemoveEvent(
                request_id=str(uuid.uuid4()), resource_id=resource_id, sla_id=batch_create_req.sla_id,
                is_remove_line=False)
            producer.produce(msg)
            # 将资源的保护状态更新为创建中
            BatchProtectionService.check_and_update_resource_status(resource)

    @staticmethod
    def check_and_update_resource_status(resource, is_resource_group: bool = False):
        with database.session() as session:
            if resource.get("protection_status") == ProtectionStatusEnum.protecting.value:
                raise EmeiStorBizException(
                    error=ResourceErrorCodes.RESOURCE_ALREADY_PROTECTED, message="The resource is not protecting.")
            else:
                resource_service.update_protection_status(session=session, resource_id_list=[resource.get("uuid")],
                                                          protection_status=ProtectionStatusEnum.protecting,
                                                          is_resource_group=is_resource_group)

    @staticmethod
    def execute_batch_protection(request_id: str, batch_create_req: BatchProtectionExecuteReq):
        """
        批量创建保护对象
        1.查询资源信息列表
        2.校验全部资源都没有保护对象
        3.校验全部资源为同一种类型
        4.查询SLA信息
        5.根据资源类型进行保护
        :param request_id:
        :param batch_create_req:
        :return:
        """
        job_id = batch_create_req.job_id

        BatchProtectionService.record_job_step(job_id, request_id, ResourceProtectionJobSteps.PROTECTION_START)
        BatchProtectionService.update_job(job_id, request_id, JobStatus.RUNNING, 10)
        protect_resource_list = batch_create_req.resources
        sla = ProtectionClient.query_sla(batch_create_req.sla_id)
        resource_list = list(ResourceClient.query_resource(
            protect_resource.resource_id) for protect_resource in protect_resource_list)
        check_result = BatchProtectionService.check_resource_is_protected(batch_create_req, sla)
        if len(check_result.failed_ids) > 0:
            BatchProtectionService.record_job_step(job_id, request_id, ResourceProtectionJobSteps.PROTECTION_FAILED,
                                                   JobLogLevel.FATAL)
            BatchProtectionService.update_job(job_id, request_id, JobStatus.FAIL, JobLogLevel.FATAL)
            return
        if len(resource_list) == 0:
            BatchProtectionService.record_job_step(job_id, request_id, ResourceProtectionJobSteps.PROTECTION_FAILED,
                                                   JobLogLevel.FATAL)
            BatchProtectionService.update_job(job_id, request_id, JobStatus.FAIL, JobLogLevel.FATAL)
            return
        result = protect_by_resource_type(request_id, resource_list, sla, batch_create_req)
        if result.is_success():
            is_manual_failed = BatchProtectionService._execute_protection_post_action(batch_create_req,
                                                                                      result.success_ids)
            BatchProtectionService.record_job_step(job_id, request_id, ResourceProtectionJobSteps.PROTECTION_FINISH)
            job_status = JobStatus.PARTIAL_SUCCESS if result.failed_ids or is_manual_failed else JobStatus.SUCCESS
            BatchProtectionService.update_job(job_id, request_id, job_status)
        else:
            BatchProtectionService.record_job_step(job_id, request_id, ResourceProtectionJobSteps.PROTECTION_FAILED,
                                                   JobLogLevel.FATAL)
            BatchProtectionService.update_job(job_id, request_id, JobStatus.FAIL, JobLogLevel.FATAL)

    @staticmethod
    def check_resource_is_protected(batch_create_req: BatchProtectionExecuteReq, sla) -> BatchOperationResult:
        job_id = batch_create_req.job_id
        request_id = batch_create_req.request_id
        batch_result = BatchOperationResult()
        with database.session() as session:
            for resource in batch_create_req.resources:
                projected_object = db.projected_object.query_one_by_resource_id(
                    db=session, resource_id=resource.resource_id)
                if projected_object:
                    batch_result.append_failed_id(resource.resource_id)
                    BatchProtectionService.record_job_step(
                        job_id, request_id, ResourceProtectionJobSteps.PROTECTION_EXECUTING_FAILED, JobLogLevel.ERROR,
                        [projected_object.name, sla.get("name")],
                        log_detail=ResourceErrorCodes.RESOURCE_ALREADY_PROTECTED.get("code"))
        return batch_result

    @staticmethod
    def _execute_protection_post_action(batch_create_req: BatchProtectionExecuteReq, success_id_list: List):
        is_manual_failed = False
        if batch_create_req.post_action != ProtectPostAction.BACKUP:
            return is_manual_failed

        for resource_id in success_id_list:
            job_log_level = JobLogLevel.INFO
            job_step_status = ResourceProtectionJobSteps.PROTECTION_START_MANUAL_SUCCESS
            resource_obj = ResourceClient.query_resource(resource_id)
            resource_name = resource_obj.get("name") or ""
            log_detail = None
            log_detail_param = None
            try:
                backup_req = CurrentManualBackupRequest(sla_id=batch_create_req.sla_id, action=PolicyActionEnum.full,
                                                        user_id=batch_create_req.user_id)
                with database.session() as session:
                    ProtectedObjectService.manual_backup(session, resource_id, backup_req)
            except EmeiStorBizException as es:
                is_manual_failed = True
                job_log_level = JobLogLevel.ERROR
                job_step_status = ResourceProtectionJobSteps.PROTECTION_START_MANUAL_FAILED
                log_detail = es.error_code if isinstance(es, EmeiStorBizException) else None
                log_detail_param = es.parameter_list if isinstance(es, EmeiStorBizException) else None
                log.exception(f"protection post action, resource(id={resource_id}, name={resource_name}) "
                              f"execute manual full backup failed")

            BatchProtectionService.record_job_step(batch_create_req.job_id, batch_create_req.request_id,
                                                   job_step_status, job_log_level, [resource_name],
                                                   log_detail_param=log_detail_param, log_detail=log_detail)
        return is_manual_failed

    @staticmethod
    def record_job_step(job_id: str, request_id: str, job_step_label: str,
                        log_level: JobLogLevel = JobLogLevel.INFO,
                        log_info_param=None, log_detail=None, log_detail_param=None):
        if not log_info_param:
            log_info_param = []
        log_step_req = toolkit.build_update_job_log_request(job_id,
                                                            job_step_label,
                                                            log_level,
                                                            log_info_param,
                                                            log_detail,
                                                            log_detail_param)
        toolkit.modify_task_log(request_id, job_id, log_step_req)

    @staticmethod
    def update_job(job_id: str, request_id: str, status: JobStatus, progress=None):
        update_req = {
            "status": status.value
        }
        if status is JobStatus.RUNNING:
            update_req.update(progress=progress)
        elif status is JobStatus.SUCCESS or status is JobStatus.PARTIAL_SUCCESS:
            update_req.update(progress=100)
        toolkit.complete_job_center_task(request_id, job_id, update_req)

    @staticmethod
    def modify_protection_task_submit(user_id: str, modify_submit_req: ModifyProtectionSubmitReq):
        resource_id = modify_submit_req.resource_id
        sla_id = modify_submit_req.sla_id
        # 保护资源时，如果资源已被防勒索或worm保护，选择的sla包含本地盘则报错
        if DeployType().is_dependent() and projected_object_service.check_resource_has_anti_or_worm(resource_id):
            projected_object_service.check_is_support_worm_and_anti(sla_id)
        log.info(f"modify protection task. resource id: {resource_id}")
        with database.session() as session:
            projected_object = db.projected_object.query_one_by_resource_id(
                db=session, resource_id=resource_id)
            if projected_object is None:
                raise EmeiStorBizException(
                    error=CommonErrorCodes.ILLEGAL_PARAMS, message="Resource is not protected.")
        sla = ProtectionClient.query_sla(str(modify_submit_req.sla_id))
        # 填充保护对象的扩展字段
        fill_protection_ext_parameters(sla, modify_submit_req.ext_parameters)
        _fill_base_esn_and_target_ext_parameters(modify_submit_req.ext_parameters, projected_object)
        if modify_submit_req.is_resource_group:
            resource_object = resource_service.query_resource_group_by_id(projected_object.resource_id).as_dict()
            resource_object["type"] = resource_object.get("source_type", "")
            resource_object["sub_type"] = resource_object.get("source_sub_type", "")
        else:
            resource_object = ResourceClient.query_resource(projected_object.resource_id)
        first_os_type = None
        if 'os_type' in resource_object:
            first_os_type = resource_object.get('os_type')
        is_sla_changed = projected_object.sla_id != sla.get("uuid")
        if not modify_submit_req.is_resource_group:
            # 根据资源类型和sla类型进行保护前置校验
            protect_pre_check(sla, resource_object, first_os_type, modify_submit_req.ext_parameters, is_sla_changed)
            # 校验脚本是否正确
            check_protected_obj_script(resource_object, modify_submit_req.ext_parameters)
        # 此功能仅支持安全一体机并且接入设备为OP
        if (DeployType().is_cyber_engine_deploy_type()
                and resource_object.get('environment_sub_type') == ResourceSubTypeEnum.CYBERENGINE_OCEAN_PROTECT):
            update_cyber_engine_self_learning_config(modify_submit_req.ext_parameters,
                                                     resource_object.get('environment_uuid'), resource_id)
            log.info(f"change sla and update self learning: {modify_submit_req.ext_parameters}" +
                     f" for resource {resource_id}")
        # 校验该资源之前的备份副本所在的存储单元：1.等于当前SLA中选择的存储单元2.被包含在在当前SLA选择的存储单元组中
        ProtectionClient.check_exist_copies_location_before_protect(sla.get("uuid"), resource_id)
        execute_req = ModifyProtectionExecuteReq(
            sla_id=str(modify_submit_req.sla_id), resource_id=resource_id,
            ext_parameters=modify_submit_req.ext_parameters,
            # 任务id会在creat job时候赋值，暂传空串以避免校验错误
            job_id='', is_sla_modify=is_sla_changed,
            resource_type=resource_object.get("type"), resource_sub_type=resource_object.get("sub_type"),
            origin_sla_id=projected_object.sla_id, is_resource_group=modify_submit_req.is_resource_group,
            is_group_sub_resource=modify_submit_req.is_group_sub_resource
        )
        job_message = JobMessage(
            topic=topics.RESOURCE_PROTECTION_MODIFY_SCHEDULE,
            payload=execute_req.dict(exclude_unset=True, exclude_none=True)
        )
        job_id = create_job(sla, user_id, resource_object, job_message)
        # 将资源的保护状态更新为保护中
        BatchProtectionService.check_and_update_resource_status(resource_object, modify_submit_req.is_resource_group)
        return job_id

    @staticmethod
    def modify_protection_execute(execute_req: ModifyProtectionExecuteReq):
        """
        修改保护对象
        :param request_id: 请求id
        :param execute_req: 保护对象更新请求
        :return:
        """
        job_id = execute_req.job_id
        request_id = execute_req.request_id
        BatchProtectionService.record_job_step(job_id, request_id,
                                               ResourceProtectionModifyJobSteps.PROTECTION_MODIFY_START)
        BatchProtectionService.update_job(job_id, request_id, JobStatus.RUNNING, 10)
        modify_result = modify_protection_by_resource_type(execute_req)
        if modify_result.is_success():
            BatchProtectionService.record_job_step(job_id, request_id,
                                                   ResourceProtectionModifyJobSteps.PROTECTION_MODIFY_FINISH)
            BatchProtectionService.update_job(job_id, request_id, JobStatus.PARTIAL_SUCCESS if len(
                modify_result.failed_ids) > 0 else JobStatus.SUCCESS)
        else:
            BatchProtectionService.record_job_step(job_id, request_id,
                                                   ResourceProtectionModifyJobSteps.PROTECTION_MODIFY_FAILED)
            BatchProtectionService.update_job(job_id, request_id, JobStatus.FAIL)

    @staticmethod
    def check_resource_status(session: Session, resource_ids: List[str]):
        projected_object_list = db.projected_object.query_by_resource_ids(db=session, resource_ids=resource_ids)
        all_protect_obj_list = get_protect_obj_by_resource_type(session, projected_object_list)
        all_resource_ids = list(projected_object.resource_id for projected_object in all_protect_obj_list)
        if len(all_resource_ids) == 0:
            log.info(f"current all resource not projected:{resource_ids}")
            return
        # 根据resource_id 查询运行期间备份job数量
        count = JobClient.count_job(all_resource_ids, WORKING_STATUS_LIST, CAN_NOT_REMOVE_PROTECT_TYPES)
        # 如果存在则不让移除保护
        if count and count > 0:
            raise EmeiStorBizException(error=ResourceErrorCodes.BACKUP_TASK_IS_WORKING,
                                       parameters=count,
                                       message=f"{count} backup jobs is working")

    @staticmethod
    def check_is_resource_group(resource_ids: List[str]):
        count = resource_service.query_resource_group_members_count(source_id_list=resource_ids)
        # 如果存在group下的资源未被释放 则不允许删除保护
        if count and count > 0:
            raise EmeiStorBizException(error=ResourceErrorCodes.RESOURCES_IN_RESOURCE_GROUP,
                                       parameters=count,
                                       message=f"{count} resources in resource group")

    @staticmethod
    def batch_remove_protection(
            session: Session,
            resource_ids: List[str],
            is_resource_group: bool = False
    ) -> List[str]:
        projected_object_list = db.projected_object.query_by_resource_ids(
            db=session, resource_ids=resource_ids)
        if len(projected_object_list) == 0:
            log.warn(f"protected object not exist queried by resource_ids:{resource_ids}")
            resource_service.update_protection_status(session=session, resource_id_list=resource_ids,
                                                      protection_status=ProtectionStatusEnum.unprotected,
                                                      is_resource_group=is_resource_group)
            return []
        type_set = {projected_object.sub_type for projected_object in projected_object_list}
        # 传入的是list，但是只有一个元素
        for resource_id, projected_object in zip(resource_ids, projected_object_list):
            # 此功能仅支持安全一体机并且接入设备为OP
            if DeployType().is_cyber_engine_deploy_type():
                resource = ResourceClient.query_resource(projected_object.resource_id)
                if resource.get('environment_sub_type') == ResourceSubTypeEnum.CYBERENGINE_OCEAN_PROTECT:
                    # 安全一体机移除SLA时关闭自学习，参数设置为默认值调用DEE接口
                    origin_ext_parameters = json.loads(projected_object.ext_parameters)
                    ext_parameters = CloudBackupExtParam(**{
                        "share_type": origin_ext_parameters["share_type"],
                        "file_system_ids": origin_ext_parameters["file_system_ids"],
                        "is_open": False,
                        "type": 0,
                        "duration": 15,
                        "progress": 0
                    })
                    update_cyber_engine_self_learning_config(ext_parameters, projected_object.env_id, resource_id)
                    log.info(f"remove sla and close self learning for resource {resource_id}")
        if len(type_set) != 1:
            # 不同类型资源，分类后依次批量操作
            projected_object_list_group = group_by_sub_type(projected_object_list, type_set)
            for single_projected_object_list in projected_object_list_group:
                BatchProtectionService.remove_update_protect_status(session, single_projected_object_list,
                                                                    is_resource_group)
        else:
            BatchProtectionService.remove_update_protect_status(session, projected_object_list, is_resource_group)
        return resource_ids

    @staticmethod
    def remove_update_protect_status(session, projected_object_list, is_resource_group: bool = False):
        protect_obj_list = get_protect_obj_by_resource_type(session, projected_object_list)
        remove_protection(session, protect_obj_list)
        if not is_resource_group:
            for tmp_obj in protect_obj_list:
                ResourceClient.umount_agent_and_lan_free(tmp_obj)
        for tmp_obj in protect_obj_list:
            handle_unmount_repo_for_sap_hana_db(tmp_obj)
        # 更新资源表中的保护状态为未保护
        resource_id_list = list(protect_obj.resource_id for protect_obj in protect_obj_list)
        resource_service.update_protection_status(session=session, resource_id_list=resource_id_list,
                                                  protection_status=ProtectionStatusEnum.unprotected,
                                                  is_resource_group=is_resource_group)

    @staticmethod
    def batch_deactivate(session: Session, resource_ids: List[str]):
        projected_object_list = db.projected_object.query_by_resource_ids(db=session, resource_ids=resource_ids)
        if len(projected_object_list) == 0:
            return
        type_set = {projected_object.sub_type for projected_object in projected_object_list}
        if len(type_set) != 1:
            # 不同类型资源，分类后依次批量操作
            projected_object_list_group = group_by_sub_type(projected_object_list, type_set)
            for single_projected_object_list in projected_object_list_group:
                BatchProtectionService.deactivate_update_protect_status(single_projected_object_list, session)
        else:
            BatchProtectionService.deactivate_update_protect_status(projected_object_list, session)

    @staticmethod
    def deactivate_update_protect_status(projected_object_list, session):
        protect_obj_list = get_protect_obj_by_resource_type(session, projected_object_list)
        process_resource_id_list = list(protect_obj.resource_id for protect_obj in protect_obj_list if
                                        protect_obj.status == Status.Active.value)
        db.projected_object.update_status(
            db=session, resource_ids=process_resource_id_list, status=Status.Inactive)
        # 更新资源表中的保护状态为未保护
        resource_service.update_protection_status(session=session, resource_id_list=process_resource_id_list,
                                                  protection_status=ProtectionStatusEnum.unprotected)

    @staticmethod
    def batch_activate(session: Session, resource_ids: List[str]):
        projected_object_list = db.projected_object.query_by_resource_ids(
            db=session, resource_ids=resource_ids)
        if len(projected_object_list) == 0:
            return
        type_set = {projected_object.sub_type for projected_object in projected_object_list}
        if len(type_set) != 1:
            # 不同类型资源，分类后依次批量操作
            projected_object_list_group = group_by_sub_type(projected_object_list, type_set)
            for single_projected_object_list in projected_object_list_group:
                BatchProtectionService.activate_update_protect_status(single_projected_object_list, session)
        else:
            BatchProtectionService.activate_update_protect_status(projected_object_list, session)

    @staticmethod
    def activate_update_protect_status(projected_object_list, session):
        protect_obj_list = get_protect_obj_by_resource_type(session, projected_object_list)
        process_resource_id_list = list(protect_obj.resource_id for protect_obj in protect_obj_list if
                                        protect_obj.status == Status.Inactive.value)
        db.projected_object.update_status(db=session, resource_ids=process_resource_id_list, status=Status.Active)
        # 更新资源表中的保护状态为已保护
        resource_service.update_protection_status(session=session, resource_id_list=process_resource_id_list,
                                                  protection_status=ProtectionStatusEnum.protected)

    @staticmethod
    def batch_deactivate_resource_group(session: Session, resource_ids: List[str], is_resource_group: bool = False):
        projected_object_list = db.projected_object.query_by_resource_ids(
            db=session, resource_ids=resource_ids)
        if len(projected_object_list) == 0:
            return
        type_set = {projected_object.sub_type for projected_object in projected_object_list}
        if len(type_set) != 1:
            # 不同类型资源，分类后依次批量操作
            projected_object_list_group = group_by_sub_type(projected_object_list, type_set)
            for single_projected_object_list in projected_object_list_group:
                BatchProtectionService.deactivate_update_resource_group_protect_status(single_projected_object_list,
                                                                                       session,
                                                                                       is_resource_group)
        else:
            BatchProtectionService.deactivate_update_resource_group_protect_status(projected_object_list, session,
                                                                                   is_resource_group)

    @staticmethod
    def deactivate_update_resource_group_protect_status(projected_object_list, session, is_resource_group):
        process_resource_id_list = list(protect_obj.resource_id for protect_obj in projected_object_list if
                                        protect_obj.status == Status.Active.value)
        db.projected_object.update_status(
            db=session, resource_ids=process_resource_id_list, status=Status.Inactive)
        # 更新资源表中的保护状态为未保护
        resource_service.update_protection_status(session=session, resource_id_list=process_resource_id_list,
                                                  protection_status=ProtectionStatusEnum.unprotected,
                                                  is_resource_group=is_resource_group)

    @staticmethod
    def batch_activate_resource_group(session: Session, resource_ids: List[str], is_resource_group: bool = False):
        projected_object_list = db.projected_object.query_by_resource_ids(db=session, resource_ids=resource_ids)
        if len(projected_object_list) == 0:
            return
        type_set = {projected_object.sub_type for projected_object in projected_object_list}
        if len(type_set) != 1:
            # 不同类型资源，分类后依次批量操作
            projected_object_list_group = group_by_sub_type(projected_object_list, type_set)
            for single_projected_object_list in projected_object_list_group:
                BatchProtectionService.activate_update_resource_group_protect_status(single_projected_object_list,
                                                                                     session,
                                                                                     is_resource_group)
        else:
            BatchProtectionService.activate_update_resource_group_protect_status(projected_object_list, session,
                                                                                 is_resource_group)

    @staticmethod
    def activate_update_resource_group_protect_status(projected_object_list, session, is_resource_group):
        process_resource_id_list = list(protect_obj.resource_id for protect_obj in projected_object_list if
                                        protect_obj.status == Status.Inactive.value)
        db.projected_object.update_status(
            db=session, resource_ids=process_resource_id_list, status=Status.Active)
        # 更新资源表中的保护状态为已保护
        resource_service.update_protection_status(session=session, resource_id_list=process_resource_id_list,
                                                  protection_status=ProtectionStatusEnum.protected,
                                                  is_resource_group=is_resource_group)

    @staticmethod
    def sync_resource_add(resource_id: str):
        """
        同步新增资源
            1. 判断资源子类型，根据子类型进行处理
        """
        resource = resource_service.query_resource_by_id(resource_id)
        with database.session() as session:
            projected_object = db.projected_object.query_one_by_resource_id(db=session, resource_id=resource_id)
            if not resource or projected_object:
                return
            if resource.sub_type in [ResourceSubTypeEnum.VirtualMachine.value,
                                     ResourceSubTypeEnum.KubernetesStatefulSet.value,
                                     ResourceSubTypeEnum.HCSCloudHost.value,
                                     ResourceSubTypeEnum.FusionCompute.value,
                                     ResourceSubTypeEnum.FUSION_ONE_COMPUTE.value,
                                     ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.value,
                                     ResourceSubTypeEnum.HYPER_V_VM.value,
                                     ResourceSubTypeEnum.CNWARE_VM.value,
                                     ResourceSubTypeEnum.NUTANIX_VM.value
                                     ]:
                sync_sub_resource_add(session, resource.dict())

    @staticmethod
    def check_auth(domain_id, sla_id):
        sla = ProtectionClient.query_sla(sla_id)
        policy_type_list = []
        for policy in sla.get("policy_list"):
            if policy.get("type") not in policy_type_list:
                policy_type_list.append(policy.get("type"))
        for policy_type in policy_type_list:
            if policy_type == PolicyTypeEnum.backup.value:
                if not get_default_role_auth_list(domain_id, [AuthOperationEnum.BACKUP]):
                    log.error(f'Current user has no backup permission, domain_id:{domain_id}')
                    raise EmeiStorBizException(UserErrorCodes.SLA_POLICY_OPERATION_ACCESS_DENIED,
                                               *["common_backup_label"])
            if policy_type == PolicyTypeEnum.archiving.value:
                if not get_default_role_auth_list(domain_id, [AuthOperationEnum.ARCHIVE]):
                    log.error(f'Current user has no archive permission, domain_id:{domain_id}')
                    raise EmeiStorBizException(UserErrorCodes.SLA_POLICY_OPERATION_ACCESS_DENIED,
                                               *["common_archive_label"])
            if policy_type == PolicyTypeEnum.replication.value:
                if not get_default_role_auth_list(domain_id, [AuthOperationEnum.REPLICATION]):
                    log.error(f'Current user has no replication permission, domain_id:{domain_id}')
                    raise EmeiStorBizException(UserErrorCodes.SLA_POLICY_OPERATION_ACCESS_DENIED,
                                               *["common_replication_label"])

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
from threading import Thread
from typing import List, Optional

from fastapi import Query, Header, Depends
from fastapi.params import Body, Path

import app.copy_catalog.client.copy_client as client
from app.archive.schemas.archive_request import ArchiveMsg
from app.archive.service.service import handle_schedule_archiving
from app.common.auth.check_ath import CheckAuthModel
from app.common.clients.resource_client import ResourceClient
from app.common.concurrency import async_route
from app.common.deploy_type import DeployType
from app.common.enums.copy_enum import GenerationType
from app.common.enums.job_enum import JobType
from app.common.enums.rbac_enum import ResourceSetTypeEnum, OperationTypeEnum, AuthOperationEnum
from app.common.enums.sla_enum import BackupTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import IllegalParamException, EmeiStorBizException
from app.common.extension import define_page_query_api_for_model
from app.common.exter_attack import exter_attack
from app.common.log.common_operation_code import OperationAlarmCode
from app.common.log.operation_log import operation_log
from app.common.logger import get_logger
from app.common.schemas.common_schemas import UuidObject
from app.common.security.jwt_utils import get_user_id, get_role_list, get_user_domain_id
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum
from app.copy_catalog.common.common import (
    HttpStatusDetail,
    GenIndexType,
    CopyRetentionTypeLabel,
    CopyRetentionDurationUnitLabel,
    OperationLabel, CopyWormValidityTypeLabel
)
from app.copy_catalog.copy_error_code import CopyErrorCode
from app.copy_catalog.models.req_param import NoArchiveReq
from app.copy_catalog.models.tables_and_sessions import CopyTable, database
from app.copy_catalog.schemas import (
    CopyStatisticView,
    CopyRetentionPolicySchema,
    CopyInfoSchema,
    CopyStatistic,
    CopyRestoreInformation,
    CopySchema,
    CopyProtectionSchema,
    ProtectedCopyBatchOperationReq,
    CopyArchiveMapSchema,
    CopyInfoWithArchiveFieldsSchema,
    CopyAntiRansomwareReport,
    CopyDetail,
    CopyStatusUpdate,
    ReplicatedCopiesSchema,
    CopyStatusUpdateByDeviceEsnSchemas, UpdateCopyIndexStatusRequest, CopyWormRetentionSettingSchema
)
from app.copy_catalog.schemas.copy_schemas import UpdateCopyPropertiesRequest, CopyWormStatusUpdate, \
    DeleteExcessCopiesRequest
from app.copy_catalog.service import (
    anti_ransomware_service,
    copy_expire_service,
    copy_delete_workflow
)
from app.copy_catalog.service.copy_delete_workflow import CopyDeleteParam
from app.copy_catalog.service.curd import copy_query_service, copy_create_service, copy_update_service, \
    copy_delete_service
from app.copy_catalog.service.curd.copy_query_service import (
    query_resource_type_name,
    query_copy_backup_id,
    copy_data_condition_filter,
    copy_authenticate, get_delete_copy_cyber_log_data, get_update_retention_cyber_log_data,
    query_copy_info_by_backup_id, query_copy_info_by_uuid_and_esn, query_deleted_copy_subtype,
    query_user_related_copy_by_resource_id
)
from app.copy_catalog.service.delete_excess_copy_service import delete_excess_copy
from app.copy_catalog.service.import_copy_service import query_copy_info_by_copy_id
from app.copy_catalog.util.copy_auth_verify_util import check_resource_related_copy_auth
from app.copy_catalog.util.copy_util import is_worm_copy, is_snapshot_in_period
from app.resource.service.common import resource_service

log = get_logger(__name__)

api = async_route()
copy_api = async_route()

define_page_query_api_for_model(
    copy_api,
    database,
    CopyTable,
    initiator=copy_data_condition_filter,
    extra_conditions=["date", "gn_range", "cluster_name", "storage_unit_name",
                      "resource_set_id", "labelName", "labelList", "origin_date"],
    default_conditions={"deleted": False},
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR},
    authenticate=copy_authenticate,
    summary="副本列表查询",
    converter_response=resource_service.common_add_label_json_info_with_query_response
)
define_page_query_api_for_model(
    copy_api,
    database,
    CopyTable,
    path="/internal/copies",
    initiator=copy_data_condition_filter,
    extra_conditions=["year", "month", "week", "date", "hour", "gn_range", "starts_time", "ends_time"]
)
define_page_query_api_for_model(
    copy_api,
    database,
    CopyTable,
    path="/copies/summary/resources",
    relation_config="__resource_summary__",
    initiator=copy_data_condition_filter,
    extra_conditions=["date", "gn_range"],
    default_conditions={"deleted": False},
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR},
    authenticate=copy_authenticate,
    summary="副本资源列表查询"
)
define_page_query_api_for_model(
    copy_api,
    database,
    CopyTable,
    path="/internal/copies/summary/resources",
    relation_config="__resource_summary__",
)
COPY_TAG = 'copies'
COPY_DETECT_TAG = 'copies-detect-report'


@exter_attack
@copy_api.post(
    "/internal/copies",
    status_code=200,
    summary="Save copy",
    tags=[COPY_TAG],
    response_model=UuidObject,
)
def save_copy_info(copy: CopyInfoSchema = Body(None, description="副本信息"),
                   override: bool = Query(default=False, description="是否覆盖")):
    return copy_create_service.save_copy_info(copy, override=override)


# 保留策略：永久/指定时间
def trans_retention_type_to_operation_record(params):
    retention_policy = params.get("retention_policy")
    retention_type = retention_policy.retention_type
    if retention_type is None:
        return '--'
    return CopyRetentionTypeLabel[retention_type.name.upper()].value


# 保留时间单位：单位。（比如：天）
def trans_retention_duration_unit_to_operation_record(params):
    retention_policy = params.get("retention_policy")
    retention_duration_unit = retention_policy.duration_unit
    if retention_duration_unit is None:
        return '--'
    return CopyRetentionDurationUnitLabel[retention_duration_unit.name.upper()].value


#是否转换worm
def trans_worm_convert_worm_switch_to_operation_record(params):
    worm_setting = params.get("worm_setting")
    convert_worm_switch = worm_setting.convert_worm_switch
    if convert_worm_switch:
        return "common_yes_label"
    return "common_no_label"


# 保留时间单位：单位。（比如：天）
def trans_worm_retention_duration_unit_to_operation_record(params):
    worm_setting = params.get("worm_setting")
    duration_unit = worm_setting.duration_unit
    if duration_unit is None:
        return '--'
    return CopyRetentionDurationUnitLabel[duration_unit.name.upper()].value


# worm保留时间类型
def trans_worm_validity_type_to_operation_record(params):
    worm_setting = params.get("worm_setting")
    worm_validity_type = worm_setting.worm_validity_type
    if worm_validity_type is None:
        return '--'
    return CopyWormValidityTypeLabel[worm_validity_type.name.upper()].value


@exter_attack
@copy_api.post(
    "/internal/batchCopies",
    status_code=200,
    summary="Save copies",
    tags=[COPY_TAG],
)
# 注意调用此接口前需删除原数据，此处批量保存的副本数据需保持uuid不变
def save_copy_list(copy_infos: List[CopySchema] = Body(None, description="副本信息列表")):
    copy_create_service.save_copy_list(copy_infos)


@exter_attack
@copy_api.get(
    "/copies/summary/statistics",
    status_code=200,
    tags=[COPY_TAG],
    summary="资源状态统计"
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources="resource:resource_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.QUERY,
                              target="resource_id")
)
def resource_status_statistics(resource_id: str = Query(..., min_length=1, max_length=128, description="资源ID"),
                               token: str = Header(..., alias="X-Auth-Token", description="授权token")):
    user_id = get_user_id(token)
    role = get_role_list(token)[0]
    return copy_query_service.resource_status_statistics(resource_id, role, user_id)


@exter_attack
@copy_api.get(
    "/copies/statistics",
    status_code=200,
    tags=[COPY_TAG],
    response_model=List[CopyStatistic],
    summary="指定日期副本数量"
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources="resource:resource_id")
def query_copy_statistics(
        view: CopyStatisticView = Query(CopyStatisticView.YEAR, description="统计视图（枚举）。取值范围：year/month"),
        time_point: str = Query(None, description="时间点"),
        resource_id: str = Query(None, description="资源ID"),
        token: str = Header(..., alias="X-Auth-Token", description="授权token")):
    domain_id = get_user_domain_id(token)
    role = get_role_list(token)[0]
    return copy_query_service.query_copy_statistics(view, time_point, resource_id, role, domain_id)


@exter_attack
# 操作日志：用户（{0}：{1}）更新资源（{2}：{3}）在（{4}）时间点副本的保留策略（保留类型：{5}，保留时间：{6}）。
@copy_api.post(
    "/copies/{copy_id}/action/update-retention",
    status_code=200,
    tags=[COPY_TAG],
    summary="修改副本保留策略")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="copy:copy_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.COPY, operation=OperationTypeEnum.QUERY,
                              target="copy_id")
)
@operation_log(
    name=OperationAlarmCode.UPDATE_COPY_RETENTION,
    target="@Copycatalog",
    detail=('copy_id!copy.resource_sub_type', 'copy_id!copy.resource_name', 'copy_id!copy.display_timestamp!timestamp',
            trans_retention_type_to_operation_record, 'retention_policy.retention_duration',
            trans_retention_duration_unit_to_operation_record)
)
def update_copy_retention(
        copy_id: str = Path(..., min_length=1, max_length=64, description="副本ID"),
        retention_policy: CopyRetentionPolicySchema = Body(None, description="副本保留策略")
):
    copy_expire_service.update_copy_retention(copy_id, retention_policy)


@exter_attack
# 用户（{0}：{1}）修改副本{2}的worm设置（是否转换worm副本：{3}，WORM有效保留类型：{4}，副本保留周期：{5}，副本保留周期单位：{6}）
@copy_api.post(
    "/copies/{copy_id}/action/update-worm-setting",
    status_code=200,
    tags=[COPY_TAG],
    summary="修改WORM副本设置")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="copy:copy_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.COPY, operation=OperationTypeEnum.QUERY,
                              target="copy_id")
)
@operation_log(
    name=OperationAlarmCode.UPDATE_WORM_COPY_SETTING,
    target="@Copycatalog",
    detail=('copy_id', trans_worm_convert_worm_switch_to_operation_record,
            trans_worm_validity_type_to_operation_record,
            'worm_setting.retention_duration',
            trans_worm_retention_duration_unit_to_operation_record)
)
def update_worm_copy_setting(
        copy_id: str = Path(..., min_length=1, max_length=64, description="副本ID"),
        worm_setting: CopyWormRetentionSettingSchema = Body(None, description="WORM副本设置")
):
    copy_expire_service.update_worm_copy_setting(copy_id, worm_setting)


@exter_attack
# 操作日志：
# 用户（{0}：{1}）更新资源（存储设备名（{2}）、设备序列号（{3}）、设备类型（{4}）、租户名（{5}）、租户ID（{6}）、资源ID（{7}）、资源名（{8}））
# 在（{9}）时间点快照的保留策略（保留类型：{10}，保留时间：{11}{12}）
@copy_api.post(
    "/copies/{copy_id}/action/update-retention-cyber",
    status_code=200,
    tags=[COPY_TAG],
    summary="修改副本保留策略")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="copy:copy_id"
)
@operation_log(
    name=OperationAlarmCode.UPDATE_COPY_RETENTION_CYBER,
    target="@Copycatalog",
    detail=(get_update_retention_cyber_log_data, 'copy_id!copy.display_timestamp!timestamp',
            trans_retention_type_to_operation_record, 'retention_policy.retention_duration',
            trans_retention_duration_unit_to_operation_record)
)
def update_copy_retention_cyber(
        copy_id: str = Path(..., min_length=1, max_length=64, description="副本ID"),
        retention_policy: CopyRetentionPolicySchema = Body(None, description="副本保留策略")
):
    copy_expire_service.update_copy_retention(copy_id, retention_policy)


@exter_attack
@copy_api.post(
    "/internal/copies/{copy_id}/action/update-retention",
    status_code=200,
    tags=[COPY_TAG])
def internal_update_copy_retention(
        copy_id: str,
        retention_policy: CopyRetentionPolicySchema = Body(None, description="副本保留策略")
):
    copy_expire_service.update_copy_retention(copy_id, retention_policy)


@exter_attack
@copy_api.put(
    "/internal/copies/{copy_id}/index-status",
    status_code=200,
    tags=[COPY_TAG])
def internal_update_copy_index_status(
        copy_id: str,
        index_status: str,
        error_code: str = OperationLabel.INDEX_RESPONSE_ERROR_LABEL.value
):
    copy_update_service.update_copy_index_status(copy_id, index_status, error_code)


@exter_attack
@copy_api.put(
    "/internal/copies/{copy_id}/browse-mount-status",
    status_code=200,
    tags=[COPY_TAG])
def internal_update_copy_browse_mount_status(
        copy_id: str,
        browse_mount_status: str,
        error_code: str = OperationLabel.INDEX_RESPONSE_ERROR_LABEL.value
):
    copy_update_service.update_copy_browse_mount_status(copy_id, browse_mount_status, error_code)


@exter_attack
@copy_api.delete("/copies/index",
                 status_code=200,
                 tags=[COPY_TAG],
                 summary="删除资源对应的副本索引")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="resource:resource_id"
)
@operation_log(
    name=OperationAlarmCode.DELETE_RESOURCE_COPY_INDEX,
    target="@Resource",
    detail=(query_resource_type_name)
)
def delete_resource_copy_index(resource_id: str = Query(..., description="资源Id"),
                               token: str = Header(..., alias="X-Auth-Token", description="授权token")):
    user_id = get_user_id(token)
    domain_id = get_user_domain_id(token)
    check_resource_related_copy_auth(resource_id, [AuthOperationEnum.copyIndex], domain_id)
    copy_delete_service.delete_resource_copy_index(resource_id, user_id)


@exter_attack
# 日志记录:当前用户{0}从源地址{1}删除资源({2})在({3})时间点的副本。
@copy_api.delete("/copies/{copy_id}", status_code=200, tags=[COPY_TAG], summary="删除副本")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="copy:copy_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.COPY, operation=OperationTypeEnum.DELETE,
                              auth_operation_list={AuthOperationEnum.copyDelete}, target="copy_id")
)
@operation_log(
    name=OperationAlarmCode.DELETE_COPY,
    target="@Copycatalog",
    detail=(
            (query_deleted_copy_subtype), 'copy_id!copy.resource_name',
            'copy_id!copy.display_timestamp!timestamp'),
)
def delete_copy(
        copy_id: str = Path(..., min_length=1, max_length=64, description="副本ID"),
        token: str = Header(..., alias="X-Auth-Token", description="授权token"),
        create_job: bool = Query(True, description="是否创建任务"),
        is_forced: bool = Query(False, description="是否强制删除"),
        job_type: str = Query(JobType.COPY_DELETE.value, description="job的类型")
):
    check_copy_can_be_delete(copy_id)
    user_id = get_user_id(token)
    copy_delete_param = CopyDeleteParam(user_id=user_id, strict=True, create_job=create_job, job_type=job_type,
                                        is_forced=is_forced)
    copy_delete_workflow.request_delete_copy_by_id(copy_id, copy_delete_param)


@exter_attack
# 日志记录:用户（{0}：{1}）删除资源（存储设备名（{2}）、设备序列号（{3}）、设备类型（{4}）、租户名（{5}）、租户ID（{6}）、资源ID（{7}）、资源名（{8}））在（{9}）时间点的快照。
@copy_api.delete("/copies/{copy_id}/cyber", status_code=200, tags=[COPY_TAG], summary="删除副本")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN}, resources="copy:copy_id")
@operation_log(
    name=OperationAlarmCode.DELETE_COPY_CYBER,
    target="@Copycatalog",
    detail=(get_delete_copy_cyber_log_data, 'copy_id!copy.display_timestamp!timestamp'),
)
def delete_copy_cyber(
        copy_id: str = Path(..., min_length=1, max_length=64, description="副本ID"),
        token: str = Header(..., alias="X-Auth-Token", description="授权token"),
        create_job: bool = Query(True, description="是否创建任务"),
        is_forced: bool = Query(False, description="是否强制删除"),
        job_type: str = Query(JobType.COPY_DELETE.value, description="job的类型")
):
    check_copy_can_be_delete(copy_id)
    user_id = get_user_id(token)
    copy_delete_param = CopyDeleteParam(user_id=user_id, strict=True, create_job=create_job, job_type=job_type,
                                        is_forced=is_forced)
    copy_delete_workflow.request_delete_copy_by_id(copy_id, copy_delete_param)


# 只对前端接口校验，internal接口不校验
def check_copy_can_be_delete(copy_id: str):
    copy = copy_delete_workflow.get_deleting_copy(copy_id, True)
    if copy is None:
        return
    # 日志备份不允许手动删除
    if BackupTypeEnum.log == copy.backup_type:
        raise IllegalParamException(CommonErrorCodes.ERR_PARAM, [], message=f"log copy can not be deleted by manual")
    # 非安全一体机, 校验是否是WORM副本, 安全一体机不在此处校验
    if is_worm_copy(copy.as_dict()) and not DeployType().is_cyber_engine_deploy_type() and is_snapshot_in_period(copy):
        log.error(f"Worm copy can not be deleted. copy id: {copy.uuid}")
        raise EmeiStorBizException.build_from_error(CopyErrorCode.DELETE_WORM_COPY_FAIL)


@exter_attack
@copy_api.delete("/internal/copies/{copy_id}", status_code=200, tags=[COPY_TAG])
def internal_delete_copy(
        copy_id: str = Path(..., description="副本ID"),
        user_id: str = Query(None, description="用户ID"),
        create_job: bool = Query(True, description="是否创建任务"),
        is_forced: bool = Query(False, description="是否强制删除"),
        is_associated: bool = Query(False, description="是否删除关联的"),
        job_type: str = Query(default=JobType.COPY_DELETE.value, description="任务类型"),
        is_delete_data: bool = Query(True, description="是否删除数据")
):
    log.info(f"internal delete copy.copy id is {copy_id}, job type is {job_type}.")
    copy_delete_param = CopyDeleteParam(user_id=user_id, strict=True, create_job=create_job, is_forced=is_forced,
                                        is_associated=is_associated, job_type=job_type, is_delete_data=is_delete_data)
    copy_delete_workflow.request_delete_copy_by_id(copy_id, copy_delete_param)


@exter_attack
@copy_api.delete("/internal/async/copies", status_code=200, tags=[COPY_TAG])
def async_delete_copes(
        copy_list: list = Query(None, description="副本列表"),
        user_id: str = Query(None, description="用户ID"),
        is_forced: bool = Query(False, description="是否强制删除"),
        is_associated: bool = Query(False, description="是否删除关联的"),
        job_type: str = Query(default=JobType.COPY_DELETE.value, description="任务类型"),
):
    log.info(f"Start async delete copy, copy num is {len(copy_list)}")
    copy_delete_param = CopyDeleteParam(user_id=user_id, strict=True, is_forced=is_forced,
                                        is_associated=is_associated, job_type=job_type)
    args = {
        'copy_list': copy_list,
        'copy_delete_param': copy_delete_param
    }
    rp_thread = Thread(target=copy_delete_workflow.batch_delete_copy, kwargs=args, name='batch_delete_copy')
    rp_thread.start()


@exter_attack
@copy_api.delete("/internal/copies", status_code=200, tags=[COPY_TAG])
def internal_delete_copy_info(
        copy_id_list: List[str] = Query(..., description="副本列表")
):
    for copy_id in copy_id_list:
        copy_delete_service.delete_single_copy(copy_id)


@exter_attack
@copy_api.delete("/internal/batchCopies", status_code=200, summary="delete copies", tags=[COPY_TAG])
def internal_delete_copy_list(
        copy_id_list: List[str] = Query(..., description="副本列表")
):
    copy_delete_service.delete_copy_list(copy_id_list)


@exter_attack
@copy_api.put("/internal/copies/{copy_id}/status", status_code=200, tags=[COPY_TAG])
def internal_update_copy_status(copy_id: str = Path(..., description="副本ID"),
                                copy_status: CopyStatusUpdate = Body(..., description="副本状态")):
    return copy_update_service.update_copy_status_by_id(copy_id, copy_status)


@exter_attack
@copy_api.put("/internal/copies/{copy_id}/worm-status", status_code=200, tags=[COPY_TAG])
def internal_update_copy_worm_status(copy_id: str = Path(..., description="副本ID"),
                                     req: CopyWormStatusUpdate = Body(..., description="副本WORM状态")):
    return copy_update_service.update_copy_worm_status_by_id(copy_id, req.worm_status)


@exter_attack
@copy_api.get(
    "/internal/copies/action/verify",
    status_code=200,
    tags=[COPY_TAG],
)
def internal_verify_copy_ownership(user_id: str = Query(..., description="用户ID"),
                                   copy_uuid_list: List[str] = Query(..., description="副本列表")):
    copy_query_service.verify_copy_ownership(user_id, copy_uuid_list)


@exter_attack
@copy_api.get(
    '/copies/extended_parameters',
    status_code=200,
    summary="查询Oracle恢复目标主机参数配置",
    response_model=CopyRestoreInformation,
    tags=[COPY_TAG],
    responses={500: {"detail": "Internal error"}}
)
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources="copy:copy_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.COPY, operation=OperationTypeEnum.QUERY,
                              target="copy_id")
)
def get_copy_extended_parameter(
        copy_id: str = Query(None, min_length=1, max_length=64, description="副本id"),
        system_change_number: str = Query(None, max_length=15, description="Oracle scn"),
        time: str = Query(None, description="Oracle 时间点"),
        resource_id: str = Query(None, min_length=1, max_length=128, description="资源id")):
    log.debug(f'get /copies/extended_parameters')

    return ResourceClient.query_database_target_host(query_copy_backup_id(copy_id), system_change_number, time,
                                                     resource_id)


@exter_attack
# 日志记录：用户（{0}:{1}）手动创建资源（{2}）在({3})时间点的副本索引
@copy_api.post(
    "/copies/{copy_id}/action/create-index",
    status_code=200,
    tags=[COPY_TAG],
    summary="创建副本索引")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources="copy:copy_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.COPY, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.copyIndex}, target="copy_id")
)
@operation_log(
    name=OperationAlarmCode.CREATE_COPY_INDEX,
    target="@Copycatalog",
    detail=('copy_id!copy.resource_name', 'copy_id!copy.display_timestamp!timestamp')
)
def create_copy_index(
        copy_id: str = Path(..., description="副本id", min_length=1, max_length=64)
):
    copy_create_service.send_copy_save_event_if_need_forward(copy_id=copy_id, gen_index=GenIndexType.MANUAL)


@exter_attack
@copy_api.get("/internal/copies/{copy_id}/latest-related-copy",
              status_code=200,
              summary="get copy info by condition",
              response_model=Optional[CopySchema],
              tags=[COPY_TAG])
def query_copy_by_condition(
        copy_id: str = Path(..., description="副本id"),
        sorted_key: bool = Query(None, description="排序字段"),
        conditions: str = Query(None, description="查询条件")):
    """
    :param sorted_key: 排序字段（倒叙）
    :param conditions: 过滤条件
    :return: 单个副本对象
    """
    return copy_query_service.query_copy_by_condition(copy_id, sorted_key, conditions)


@exter_attack
@copy_api.get("/internal/copies/resource",
              status_code=200,
              summary="get copy info by resource_id",
              response_model=Optional[CopySchema],
              tags=[COPY_TAG])
def query_copy_by_resource_id(
        resource_id: str = Query(None, description="资源id"),
        generated_by: str = Query(None, description="类型")):
    """
    :param generated_by:
    :param resource_id: 资源 ID
    :return: 单个副本对象
    """
    return copy_query_service.query_copy_by_resource_id(resource_id, generated_by)


@exter_attack
@copy_api.post("/internal/copies/action/create-protection",
               status_code=200,
               tags=[COPY_TAG])
def create_copy_protection(
        copy_protection: CopyProtectionSchema = Body(..., description="副本保护对象")):
    """
    :param copy_protection:副本保护对象
    :return: 200
    """
    copy_create_service.create_copy_protection(copy_protection)


@exter_attack
@copy_api.delete(
    "/internal/copies/action/delete-protection",
    status_code=200,
    tags=[COPY_TAG])
def delete_copy_protection(
        req: ProtectedCopyBatchOperationReq = Body(..., description="副本保护对象资源IDs")):
    copy_delete_service.delete_copy_protection(req.resource_ids)


@exter_attack
@copy_api.put("/internal/copies/action/deactivate-protection",
              status_code=200,
              tags=[COPY_TAG])
def deactivate_copy_protection(
        req: ProtectedCopyBatchOperationReq = Body(..., description="副本保护对象资源IDs")):
    return copy_update_service.deactivate_copy_protection(req.resource_ids)


@exter_attack
@copy_api.put("/internal/copies/action/activate-protection",
              status_code=200,
              tags=[COPY_TAG])
def activated_copy_protection(
        req: ProtectedCopyBatchOperationReq = Body(..., description="副本保护对象资源IDs")):
    return copy_update_service.activated_copy_protection(req.resource_ids)


@exter_attack
@copy_api.put("/internal/copies/action/revoke/{user_id}",
              status_code=200,
              tags=[COPY_TAG]
              )
def revoke_copy_user_id(user_id: str = Path(..., description="用户ID"), ):
    copy_update_service.revoke_copy_user_id(user_id)


@exter_attack
@copy_api.get('/copies/action/query',
              status_code=200,
              summary="根据Oracle scn查询副本",
              response_model=List[CopySchema],
              response_description="The response is the list of copies id",
              responses={500: {"detail": "Internal error"}},
              tags=[COPY_TAG])
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}, resources="resource:db_id",
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.RESOURCE, operation=OperationTypeEnum.QUERY,
                              target="db_id")
)
def query_copy_by_scn(
        db_id: str = Query(..., min_length=1, max_length=64, description="数据库的id"),
        scn: str = Query(..., max_length=15, description="scn号"),
        database_type: str = Query(..., description="资源的类型"),
):
    return client.query_copy_by_scn(db_id, scn, database_type)


@exter_attack
@copy_api.get('/internal/copies/no-archive',
              status_code=200,
              summary="query no-archived copies",
              response_model=List[CopySchema],
              response_description="The response is the list of copies id",
              responses={500: {"detail": "Internal error"}},
              tags=[COPY_TAG])
def get_no_archive_copy_list(
        req_param: NoArchiveReq = Depends()
):
    return copy_query_service.query_no_archive_copy_list(req_param)


@exter_attack
@copy_api.post("/internal/copies/action/archive",
               status_code=200,
               tags=[COPY_TAG])
def create_copy_archive_map(copy_archive_map: CopyArchiveMapSchema = Body(..., description="副本与存储对象映射")):
    copy_create_service.generate_copy_archive_map(copy_archive_map)


@exter_attack
@copy_api.post("/internal/replicated-copies",
               status_code=200,
               tags=[COPY_TAG])
def create_replicated_copies(replicated_copies_schema: ReplicatedCopiesSchema = Body(..., description="副本与esn映射")):
    copy_create_service.create_replicated_copies(replicated_copies_schema)


@exter_attack
@copy_api.get("/internal/replicated_copies",
              status_code=200,
              summary="get replicated copy list",
              response_description="replicated copy list",
              tags=[COPY_TAG])
def query_replicated_copies(
        copy_id: str = Query(None, description="copy_id"),
        esn: str = Query(None, description="esn")):
    """
    :param esn: esn
    :param copy_id: 指定副本ID
    :return: 指定副本ID对应的esn_list
    """
    return copy_query_service.query_replicated_copies(copy_id, esn)


@exter_attack
@copy_api.get("/internal/copies/{chain_id}/signed-deleted-copies",
              status_code=200,
              summary="get copies info by chain id",
              response_model=List[CopySchema],
              response_description="The response is the list of copies id",
              responses={500: {"detail": "Internal error"}},
              tags=[COPY_TAG])
def query_delete_copy_list(
        chain_id: str = Path(..., description="chain id")):
    """
    :param chain_id: 指定chain id
    :return: 指定chain id标记deleted为true的副本对象列表
    """
    return copy_query_service.query_delete_copy_list(chain_id)


@exter_attack
@copy_api.get("/internal/copies/archive/{storage_id}",
              status_code=200,
              summary="get archive copy count by storage id",
              response_description="The count of archive copy",
              tags=[COPY_TAG])
def query_archive_copy_count_by_storage_id(
        storage_id: str = Path(..., description="storage_id")):
    """
    :param storage_id: 指定归档存储ID
    :return: 指定存储ID对应的副本个数
    """
    return copy_query_service.query_archive_copy_count_by_storage_id(storage_id)


@exter_attack
@copy_api.get("/internal/copies/resource-properties",
              status_code=200,
              summary="get copy info by resource-properties",
              response_model=Optional[CopySchema],
              tags=[COPY_TAG])
def query_copy_by_resource_properties(
        root_uuid: str = Query(None, description="资源的root_uuid"),
        generated_by: str = Query(None, description="副本类型"),
        resource_sub_type: str = Query(None, description="资源子类型")):
    """
    :param root_uuid: 副本对应资源的root_uuid
    :param generated_by: 副本生成方式
    :param resource_sub_type: 资源子类型
    :return: 副本对象
    """
    return copy_query_service.query_copy_by_resource_properties(root_uuid, generated_by, resource_sub_type)


@copy_api.get(
    "/internal/copies/backup-id",
    status_code=200,
    tags=[COPY_TAG],
    summary="根据备份ID和esn查询副本信息",
    response_model=Optional[CopySchema],
)
def query_copy_by_backup_id(origin_backup_id: str = Query(..., min_length=1, max_length=64,
                                                          description="备份副本ID"),
                            esn: str = Query(..., min_length=1, max_length=64, description="设备esn"),
                            generated_by_list: List[GenerationType] = Query(..., min_items=1, max_items=10,
                                                                            description="副本类型列表")):
    return query_copy_info_by_backup_id(origin_backup_id, esn, generated_by_list)


@copy_api.get(
    "/internal/copies/uuid",
    status_code=200,
    tags=[COPY_TAG],
    summary="根据uuid和esn查询副本信息",
    response_model=Optional[CopySchema],
)
def query_copy_by_backup_id(uuid: str = Query(..., description="uuid"),
                            esn: str = Query(..., description="设备esn")):
    return query_copy_info_by_uuid_and_esn(uuid, esn)


@exter_attack
@copy_api.get(
    "/internal/copies/{copy_id}",
    status_code=200,
    tags=[COPY_TAG],
    summary="根据副本ID查询副本信息",
    response_model=CopyInfoWithArchiveFieldsSchema
)
def query_copy_by_copy_id(copy_id: str = Path(..., description="副本ID")):
    return query_copy_info_by_copy_id(copy_id)


@exter_attack
@copy_api.put("/internal/copies/{copy_id}/detail",
              status_code=200,
              tags=[COPY_TAG],
              summary="更新副本详情")
def update_copy_detail(copy_id: str = Path(..., min_length=1, max_length=64, description="副本ID"),
                       detail: CopyDetail = Body(..., description="副本详情")):
    log.info(f"update copy_id:{copy_id}, detail:{detail.detail}")
    copy_update_service.update_copy_detail(copy_id, detail.detail)


@exter_attack
@copy_api.put("/internal/copies/index-status",
              status_code=200,
              tags=[COPY_TAG],
              summary="更新资源对应的副本索引状态")
def update_resource_copy_index_status(resource_id: str, index_status: str, error_code: str = ""):
    copy_update_service.update_resource_copy_index_status(resource_id, index_status, error_code)


@exter_attack
@copy_api.get("/internal/copies/{copy_id}/detection-reports",
              status_code=200,
              tags=[COPY_TAG],
              response_model=CopyAntiRansomwareReport,
              summary="获得副本防勒索检测报告")
def query_anti_ransomware_detail(
        copy_id: str = Path(..., min_length=1, max_length=64, description="副本id")):
    return anti_ransomware_service.query_anti_ransomware_detail(copy_id)


@exter_attack
@copy_api.delete("/internal/copies/action/delete-storage/{storage_id}",
                 status_code=200,
                 summary="delete archive copy by storage id",
                 tags=[COPY_TAG])
def delete_archive_copy_by_storage_id(storage_id: str = Path(..., description="storage_id")):
    """
    删除归档存储中全部副本
    :param storage_id: 指定归档存储ID
    :return: 该归档存储下所有副本
    """
    return copy_delete_service.delete_archive_copy_by_storage_id(storage_id)


@exter_attack
@copy_api.get("/internal/copies/{copy_id}/action/count",
              status_code=200,
              summary="count copy by parent id",
              tags=[COPY_TAG])
def count_copy_by_parent_id(copy_id: str = Path(..., min_length=1, max_length=64, description="副本id")):
    """
    根据副本id，统计该副本经过live mount产生的子副本数量
    :return: 副本数量
    """
    return copy_query_service.count_copy_by_parent_id(copy_id)


@exter_attack
@copy_api.get("/internal/copies/action/check-delete",
              status_code=200,
              summary="check copy can delete",
              tags=[COPY_TAG])
def check_copy_can_delete(copy_id_list: List[str] = Query(..., description="副本列表")):
    """
    判断副本能否被删除
    :param copy_id_list: 副本id列表
    :return: 检查结果
    """
    return copy_delete_service.check_copy_can_delete(copy_id_list)


@copy_api.put("/internal/copies/{copy_id}/properties",
              status_code=200,
              summary="update copy properties by key",
              tags=[COPY_TAG])
def update_properties_by_key(copy_id: str = Path(..., min_length=1, max_length=64, description="副本id"),
                             request: UpdateCopyPropertiesRequest = Body(..., description="更新副本属性请求")):
    """
    更新副本扩展参数中的key对应的value

    :param copy_id: 副本id
    :param request: 更新副本扩展参数请求
    :return: 检查结果
    """
    copy_update_service.update_copy_properties_by_key(copy_id, request.key, request.value)


@copy_api.delete("/internal/copies/{resource_id}/excess-copies", status_code=200)
def delete_excess_copies(resource_id: str = Path(..., min_length=1, max_length=64, description="资源id"),
                         request: DeleteExcessCopiesRequest = Body(..., description="删除资源多余副本请求")):
    """
    按数量保留副本时，删除资源下多余副本

    :param resource_id: 资源id
    :param request: 删除多余副本参数请求
    """
    log.info(f"Delete resource: {resource_id} excess copies.")
    delete_excess_copy(resource_id=resource_id, request=request)


@exter_attack
@copy_api.put("/internal/copies/status", status_code=200, tags=[COPY_TAG])
def update_status_by_device_esn(request: CopyStatusUpdateByDeviceEsnSchemas
                                = Body(..., description="修改指定集群上的副本的副本状态请求")):
    """
    修改指定集群上的副本的副本状态

    :param request: 修改指定集群上的副本的副本状态请求参数
    """
    copy_update_service.update_status_by_device_esn(request.device_esn, request.status)


@exter_attack
@copy_api.put("/internal/archive/dispatch", status_code=200, tags=[COPY_TAG])
def dispatch_archive(msg: ArchiveMsg
                     = Body(..., description="归档转发消息")):
    """
    归档转发消息

    :param msg: 归档转发消息
    """
    handle_schedule_archiving(msg)


@copy_api.get("/internal/user/{domain_id}/copies/{resource_id}", status_code=200, tags=[COPY_TAG])
def query_user_indexed_copies_resource_related(
        resource_id: str = Path(..., min_length=1, max_length=256, description="资源id"),
        domain_id: str = Path(..., min_length=1, max_length=256, description="用户域id"),
        index_status: str = Query(..., min_length=1, max_length=32, description="副本索引状态")
):
    """
    查询用户域下指定资源索引状态的副本

    :param resource_id: 资源id
    :param domain_id: 域id
    :param index_status 索引状态
    """
    return query_user_related_copy_by_resource_id(resource_id=resource_id, domain_id=domain_id,
                                                  index_status=index_status)


@exter_attack
@copy_api.put("/internal/batch/copies/index/status",
              status_code=200,
              tags=[COPY_TAG],
              summary="更新副本索引状态")
def update_resource_copy_index_status(
        request: UpdateCopyIndexStatusRequest = Body(..., description="更新副本索引状态")):
    """
    更新指定副本的索引状态

    :param request: 更新副本索引状态请求
    """
    copy_update_service.update_copy_list_index_status(copy_id_list=request.copy_id_list,
                                                      index_status=request.index_status, error_code=request.error_code)


api.include_router(copy_api, prefix="/v1", responses={404: {"detail": HttpStatusDetail.detail_404}})

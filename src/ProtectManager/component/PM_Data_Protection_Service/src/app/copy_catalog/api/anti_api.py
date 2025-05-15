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
from typing import List, Dict

from fastapi import Query, Header
from fastapi.params import Body, Path

from app.common.auth.check_ath import CheckAuthModel
from app.common.concurrency import async_route
from app.common.deploy_type import DeployType
from app.common.enums.anti_ransomware_enum import AntiRansomwareEnum
from app.common.enums.rbac_enum import ResourceSetTypeEnum, AuthOperationEnum, OperationTypeEnum
from app.common.enums.resource_enum import DeployTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.exter_attack import exter_attack
from app.common.log.common_operation_code import OperationAlarmCode
from app.common.log.operation_log import operation_log
from app.common.logger import get_logger
from app.common.schemas.common_schemas import BasePage
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum
from app.copy_catalog.common.common import (
    HttpStatusDetail
)
from app.copy_catalog.common.constant import AntiRansomwareCopyConstant
from app.copy_catalog.schemas import (
    CopySchema,
    CopyAntiRansomwareReq,
    CopyAntiRansomwareReport,
    CopyAntiRansomware,
    CopyAntiRansomwareStatistics,
    CopyAntiRansomwareSummary,
    ModifyCopyAntiRansomwareStatusBody,
    ModifyCopyAntiRansomwareDetectionStatusReq
)
from app.copy_catalog.schemas.copy_schemas import CopyAntiRansomwareQuery
from app.copy_catalog.service import (
    anti_ransomware_service
)
from app.copy_catalog.service.curd.copy_query_service import get_detect_status_cyber_log_data

log = get_logger(__name__)

api = async_route()
copy_anti_api = async_route()

COPY_DETECT_TAG = 'copies-detect-report'
COPY_TAG = 'copies'


@exter_attack
@copy_anti_api.get("/internal/copies/status/undetected",
                   status_code=200,
                   summary="获得防勒索未检测副本",
                   response_model=BasePage[CopySchema],
                   tags=[COPY_DETECT_TAG])
def query_undetect_resoruces(
        resource_id: str = Query(..., description="资源id"),
        generated_by_list: List[str] = Query(None, description="副本类型集合"),
        copy_start_time: str = Query(None, description="该时间之后完成的副本(微秒)"),
        page_no: int = Query(..., description="分页页面编码"),
        page_size: int = Query(..., description="分页数据条数")):
    copy_anti_ransoware_query = CopyAntiRansomwareQuery(**{
        "resource_id": resource_id,
        "generated_by_list": generated_by_list,
        "copy_start_time": copy_start_time,
        "page_no": page_no,
        "page_size": page_size,
        "status": AntiRansomwareEnum.UNDETECTED.value,
    })
    return anti_ransomware_service.query_copy_anti_ransomware(copy_anti_ransoware_query)


@exter_attack
@copy_anti_api.post(
    "/internal/copies/{copy_id}/detection-reports",
    status_code=200,
    summary="写入勒索报告信息",
    tags=[COPY_DETECT_TAG]
)
def create_detection_reports(
        copy_id: str = Path(..., min_length=1, max_length=64, description="副本id"),
        request: CopyAntiRansomwareReq = Body(None, description="副本防勒索检测信息")):
    """
    :param request: 副本防勒索信息
    :param copy_id: 副本id
    :return:
    """
    log.info(f"create_copy_detection_reports, copy_id:{copy_id}, status: {request.status}")
    anti_ransomware_service.create_detection_reports(copy_id, request)


@exter_attack
@copy_anti_api.put(
    "/internal/copies/detection-status",
    status_code=200,
    summary="批量修改防勒索副本检测状态",
    tags=[COPY_DETECT_TAG]
)
def modify_detection_status(
        request: ModifyCopyAntiRansomwareDetectionStatusReq = Body(None, description="副本防勒索检测信息")):
    """
    :param request: 副本防勒索信息
    :return:
    """
    anti_ransomware_service.modify_detection_status(request)


@exter_attack
@copy_anti_api.get("/copies/{copy_id}/detect-reports",
                   status_code=200,
                   tags=[COPY_DETECT_TAG],
                   response_model=CopyAntiRansomwareReport,
                   summary="获得副本防勒索检测报告")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR},
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.COPY, operation=OperationTypeEnum.QUERY,
                              target="copy_id"))
def query_anti_ransomware_report(
        copy_id: str = Path(..., min_length=1, max_length=64, description="副本id"),
        token: str = Header(..., min_length=1, max_length=10000, alias="X-Auth-Token", title="X-Auth-Token",
                            description="访问令牌")):
    return anti_ransomware_service.query_anti_ransomware_report(token, copy_id)


@exter_attack
@copy_anti_api.get("/copies/detect-details",
                   status_code=200,
                   tags=[COPY_DETECT_TAG],
                   response_model=BasePage[CopyAntiRansomware],
                   summary="获得副本防勒索信息")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR}
)
def query_anti_ransomware_copies(
        resource_id: str = Query(None, min_length=1, max_length=64, description="资源的uuid"),
        page_no: int = Query(..., ge=0, le=10000, description="分页页面编码"),
        page_size: int = Query(..., ge=0, le=200, description="分页数据条数"),
        orders: str = Query(None, min_length=0, max_length=1024, description="排序"),
        conditions: str = Query(None, max_length=1024, description="条件参数"),
        token: str = Header(..., min_length=1, max_length=10000, alias="X-Auth-Token", title="X-Auth-Token",
                            description="访问令牌")):
    if DeployType().is_cyber_engine_deploy_type():
        return anti_ransomware_service.query_anti_ransomware_copies_cyber_engine(token, resource_id, page_no, page_size,
                                                                                 orders,
                                                                                 conditions)
    else:
        return anti_ransomware_service.query_anti_ransomware_copies(token, resource_id, page_no, page_size, orders,
                                                                    conditions)


@exter_attack
@copy_anti_api.get("/internal/copies/detect/details",
                   status_code=200,
                   tags=[COPY_DETECT_TAG],
                   response_model=BasePage[CopyAntiRansomware],
                   summary="获得副本防勒索信息, 内部接口")
def query_anti_ransomware_copies(
        resource_id: str = Query(..., min_length=1, max_length=64, description="资源的uuid"),
        page_no: int = Query(..., ge=0, le=10000, description="分页页面编码"),
        page_size: int = Query(..., ge=0, le=200, description="分页数据条数"),
        orders: str = Query(None, min_length=0, max_length=1024, description="排序"),
        conditions: str = Query(None, max_length=1024, description="条件参数")):
    return anti_ransomware_service.query_anti_ransomware_copies_cyber_engine(None, resource_id, page_no, page_size,
                                                                             orders,
                                                                             conditions)


@exter_attack
@copy_anti_api.put("/copies/{copy_id}/detect-status",
                   status_code=200,
                   tags=[COPY_DETECT_TAG],
                   summary="更改副本勒索状态")
@right_control(
    roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN},
    check_auth=CheckAuthModel(resource_set_type=ResourceSetTypeEnum.COPY, operation=OperationTypeEnum.MODIFY,
                              auth_operation_list={AuthOperationEnum.PREVENT_EXTORTION_AND_WORM}, target="copy_id")
)
@operation_log(
    name=OperationAlarmCode.RANSOMWARE_DETECTION_FALSE_ALARM_HANDLED,
    target="@Copycatalog",
    detail=('copy_id!copy.resource_name', 'copy_id!copy.display_timestamp!timestamp', 'copy_id!copy.uuid')
)
def modify_anti_ransomware_copies(
        copy_id: str = Path(..., min_length=1, max_length=64, description="副本id"),
        ext_parameters: ModifyCopyAntiRansomwareStatusBody = Body(None, description="修改防勒索状态扩展参数"),
        token: str = Header(..., min_length=1, max_length=10000, alias="X-Auth-Token", title="X-Auth-Token",
                            description="访问令牌")):
    log.info(f"modify anti-ransomware copy {copy_id} detect status.")
    return anti_ransomware_service.modify_anti_ransomware_copy(token, copy_id, ext_parameters)


@exter_attack
@copy_anti_api.put("/copies/{copy_id}/detect-status-cyber",
                   status_code=200,
                   tags=[COPY_DETECT_TAG],
                   summary="更改副本勒索状态")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN})
@operation_log(
    name=OperationAlarmCode.RANSOMWARE_DETECTION_FALSE_ALARM_HANDLED_CYBER,
    target="@Copycatalog",
    detail=(get_detect_status_cyber_log_data, 'copy_id!copy.display_timestamp!timestamp', 'copy_id!copy.uuid')
)
def modify_anti_ransomware_copies_cyber(
        copy_id: str = Path(..., min_length=1, max_length=64, description="副本id"),
        ext_parameters: ModifyCopyAntiRansomwareStatusBody = Body(None, description="修改防勒索状态扩展参数"),
        token: str = Header(..., min_length=1, max_length=10000, alias="X-Auth-Token", title="X-Auth-Token",
                            description="访问令牌")):
    log.info(f"modify anti-ransomware copy {copy_id} detect status.")
    return anti_ransomware_service.modify_anti_ransomware_copy(token, copy_id, ext_parameters)


@exter_attack
@copy_anti_api.get("/copies/detect-statistics",
                   status_code=200,
                   tags=[COPY_DETECT_TAG],
                   response_model=BasePage[CopyAntiRansomwareStatistics],
                   summary="获得资源副本防勒索统计列表")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR})
def query_anti_ransomware_copies_resource(
        resource_sub_type: str = Query(..., min_length=1, max_length=1024, description="资源子类型"),
        page_no: int = Query(..., ge=0, le=10000, description="分页页面编码"),
        page_size: int = Query(..., ge=0, le=200, description="分页数据条数"),
        orders: str = Query(None, max_length=1024, description="排序"),
        conditions: str = Query(None, max_length=1024, description="条件参数"),
        token: str = Header(..., min_length=1, max_length=10000, alias="X-Auth-Token", title="X-Auth-Token",
                            description="访问令牌")):
    if DeployType().is_cyber_engine_deploy_type():
        return anti_ransomware_service.query_anti_ransomware_resources_cyber_engine(
            (token, page_no, page_size,
             orders, conditions))
    else:
        return anti_ransomware_service.query_anti_ransomware_copies_resource(
            (token, resource_sub_type, page_no, page_size,
             orders, conditions))


@exter_attack
@copy_anti_api.get("/internal/copies/detect/statistics",
                   status_code=200,
                   tags=[COPY_DETECT_TAG],
                   response_model=BasePage[CopyAntiRansomwareStatistics],
                   summary="获得资源副本防勒索统计列表, 内部接口")
def query_anti_ransomware_copies_resource(
        page_no: int = Query(..., ge=0, le=10000, description="分页页面编码"),
        page_size: int = Query(..., ge=0, le=200, description="分页数据条数"),
        orders: str = Query(None, max_length=1024, description="排序"),
        conditions: str = Query(None, max_length=1024, description="条件参数")):
    return anti_ransomware_service.query_anti_ransomware_resources_cyber_engine(
        (None, page_no, page_size,
         orders, conditions))


@exter_attack
@copy_anti_api.get("/copies/detect-summary",
                   status_code=200,
                   tags=[COPY_DETECT_TAG],
                   response_model=List[CopyAntiRansomwareSummary],
                   summary="获得副本检测统计")
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_DP_ADMIN, RoleEnum.ROLE_AUDITOR})
def query_anti_ransomware_copies_summary(
        resource_sub_type: List[str] = Query(None, min_length=1, description="资源子类型"),
        period: str = Query(None, max_length=16, regex="week|month|half-year", description="查询周期"),
        token: str = Header(..., min_length=1, max_length=10000, alias="X-Auth-Token", title="X-Auth-Token",
                            description="访问令牌")):
    if not resource_sub_type:
        resource_sub_type = AntiRansomwareCopyConstant.SUPPORT_ANTI_RESOURCE_TYPE_LIST
    return anti_ransomware_service.query_anti_ransomware_copies_summary(token, resource_sub_type, period)


api.include_router(copy_anti_api, prefix="/v1", responses={404: {"detail": HttpStatusDetail.detail_404}})

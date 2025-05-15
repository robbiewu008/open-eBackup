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
import re
from datetime import datetime
from typing import Union, Optional, List

from pydantic import BaseModel, Field, Json, validator, root_validator

from app.common import logger
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.enums.sla_enum import PolicyActionEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException, IllegalParamException
from app.common.schemas.common_schemas import PageRequest
from app.protection.object import db
from app.protection.object.common import db_config
from app.protection.object.common.protection_enums import ProtectPostAction
from app.protection.object.schemas.cyber_engine_validator import CyberEngineValidator
from app.protection.object.schemas.extends.base_ext_param import BaseExtParam
from app.protection.object.schemas.extends.extends_params_manager import ExtendsParamsManager
from app.protection.object.schemas.extends.params.common_ext_param import CommonResourceExtParam
from app.protection.object.schemas.extends.params.vmware_ext_param import ProtectResource
from app.resource.common.common_enum import FilesetTemplateOsTypeEnum
from app.resource.service.common import resource_service

log = logger.get_logger(__name__)

NON_WINDOWS_PATH_REG = '^(.+)[\.]{1}(sh){1}$'
WINDOWS_PATH_REG = '^(.+)[\.]{1}(bat){1}$'


class ProtectedObjectId(BaseModel):
    uuid: str = Field(None, description="保护对象的ID")


class ProtectedObjectSlaCompliance(BaseModel):
    in_compliance: int = Field(None, description="SLA遵从")
    out_of_compliance: int = Field(None, description="SLA不遵从")


class ProtectedObjectTime(BaseModel):
    latest_time: Optional[Union[datetime, None]] = Field(description="最近一次备份时间")
    earliest_time: Optional[Union[datetime, None]] = Field(description="最早一次备份时间")
    next_time: Optional[Union[datetime, None]] = Field(description="下一次备份时间")


class ProtectedObjectCreate(BaseModel):
    sla_id: str = Field(description="关联的SLA id")
    sla_name: str = Field(description="SLA名称")
    resource_id: str = Field(description="保护对象对应的资源id")
    name: str = Field(None, description="保护对象名称")
    path: str = Field(None, description="资源路径")
    env_id: str = Field(None, description="环境id【资源id】")
    env_type: str = Field(None, description="环境类型：如主机、虚拟化平台、大数据平台")
    type: str = Field(None, description="资源类型：如主机、数据库、虚拟机等")
    sub_type: str = Field(None, description="资源子类型：如文件集，Oracle，VMware 等")
    ext_parameters: str = Field(None, description="扩展属性")
    chain_id: str = Field(description="chain id")

    class Config:
        orm_mode = True


class ProtectedObjectQuery(ProtectedObjectTime):
    sla_id: str = Field(description="关联SLA的ID")
    sla_name: str = Field(description="SLA名称")
    resource_id: str = Field(description="保护对象对应资源的ID")
    name: str = Field(None, description="保护对象名称")
    path: str = Field(None, description="资源位置")
    env_id: str = Field(None, description="环境ID")
    env_type: str = Field(None, description="环境类型")
    type: str = Field(None, description="资源类型：如Host、Fileset、Database、VM等")
    sub_type: str = Field(None, description="资源子类型：如DBBackupAgent、Fileset、Oracle、vim.VirtualMachine等")
    status: int = Field(None, description="保护状态")
    ext_parameters: Json = Field(None, description="扩展属性")
    consistent_status: str = Field(None, description="备份数据完整性状态")
    chain_id: str = Field(None, description="chain id")

    class Config:
        orm_mode = True


class ManualBackupReq(BaseModel):
    sla_id: str = Field(description="绑定SLA的ID", max_length=36)
    action: PolicyActionEnum = Field(description="备份动作")
    is_resource_group: bool = Field(False, description="是否为资源组")
    copy_name: str = Field(None, description="副本名称", max_length=550)
    is_security_snap: bool = Field(False, description="未感染快照锁定")
    is_backup_detect_enable: bool = Field(False, description="开启备份副本侦测")
    upper_bound: int = Field(None, description="熵值侦测感染阈值")
    retention_type: int = Field(None, description="保留类型, 1永久保留, 2按时间保留")
    retention_duration: int = Field(None, description="保留时间")
    duration_unit: str = Field(None, description="保留单位")

    @root_validator
    def validate_retention(cls, backup_req):
        # 兼容非安全一体机部署形态, 手动备份未传入保留类型
        retention_type = backup_req.get("retention_type", None)
        if retention_type is None:
            return backup_req
        cyber_engine_validate_result = CyberEngineValidator.validate_backup_req(backup_req)
        if cyber_engine_validate_result:
            return backup_req
        else:
            raise EmeiStorBizException(error=CommonErrorCodes.ERR_PARAM,
                                       message="retention_type or unit or duration is illegal")

    @validator("copy_name", pre=True)
    def validate_copy_name(cls, v, values, **kwargs):
        if not v:
            return v
        if values.get("is_resource_group", False) and len(v) > 400:
            raise EmeiStorBizException(error=CommonErrorCodes.ERR_PARAM,
                                       message="copy name of manual backup for resource group can not larger than 400.")
        return v


class CurrentManualBackupRequest(ManualBackupReq):
    user_id: str = Field(None, description="执行手动备份的用户域id")


class BatchOperationReq(BaseModel):
    is_resource_group: bool = Field(False, description="是否是资源组")
    resource_ids: List[str] = Field(..., description="批量操作资源ID列表")

    @validator("resource_ids", pre=True)
    def validate_resource_list(cls, v, values, **kwargs):
        is_resource_group = values.get("is_resource_group", False)
        if is_resource_group:
            resource_list = resource_service.query_resource_group_info(v)
        else:
            resource_list = resource_service.query_resource_info(v)
        if len(resource_list) != len(v):
            raise IllegalParamException(CommonErrorCodes.OBJ_NOT_EXIST, v, "some resource not exist")
        with db_config.get_session() as session:
            obj_list = db.projected_object.query_by_resource_ids(
                db=session, resource_ids=v)
            if len(obj_list) != len(v):
                log.warn(f"protected resource:{len(obj_list)} is less than resource_ids:{len(v)}")
        return v


class BatchOperationResp(BatchOperationReq):
    pass


class ComplianceUpdate(BaseModel):
    resource_id: str = Field(..., description="资源ID")
    compliance: bool = Field(..., description="SLA遵从度, True：遵从，False：不遵从")


extend_manager = ExtendsParamsManager()


class ModifyProtectionSubmitReq(BaseModel):
    sla_id: str = Field(description="新的SLA ID", max_length=36)
    resource_id: str = Field(description="保护对象对应的资源ID", max_length=64)
    resource_group_id: str = Field("", description="资源组id", max_length=64)
    is_resource_group: bool = Field(False, description="是否给资源组创建保护对象")
    is_group_sub_resource: bool = Field(False, description="是否给子资源组子资源创建保护对象")
    ext_parameters: BaseExtParam = Field(..., description="扩展属性")

    @validator('ext_parameters', pre=True)
    def check_ext_parameters(cls, v, values, **kwargs):
        resource_id = values.get('resource_id')
        if values.get("is_resource_group"):
            res = resource_service.query_resource_group_by_id(resource_id)
            sub_type = res.source_sub_type
        else:
            res = resource_service.query_resource_by_id(resource_id)
            sub_type = res.sub_type
        if not res:
            raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                       message=f"resource [{resource_id}] is not existed")
        return extend_manager.get_ext_params_class(sub_type, v)


class ModifyProtectObjExtInfoReq(BaseModel):
    resource_id: str = Field(min_length=1, max_length=256, description="保护对象对应的资源ID")
    delete_keys: List[str] = Field(None, description="要删除的扩展属性key")
    ext_parameters: BaseExtParam = Field(..., description="扩展属性")

    @validator('ext_parameters', pre=True)
    def check_ext_parameters(cls, ext, values, **kwargs):
        return CommonResourceExtParam(**ext)


class UpdateSelfLearningProgressReq(BaseModel):
    resource_id: str = Field(description="资源ID", min_length=1, max_length=1024)
    progress: int = Field(0, description="进度", ge=0, le=60)


class ModifyProtectionExecuteReq(BaseModel):
    sla_id: str = Field(description="新的SLA id")
    resource_id: str = Field(description="保护对象对应的资源id")
    ext_parameters: BaseExtParam = Field(
        None, description="扩展属性")
    job_id: str = Field(..., description="批量保护执行请求")
    request_id: str = Field(None, description="任务请求id")
    resource_type: str = Field(None, description="保护对象对应的资源id")
    resource_sub_type: str = Field(None, description="保护对象对应的资源id")
    origin_sla_id: str = Field(None, description="原的SLA id")
    is_sla_modify: bool = Field(..., description="sla是否修改")
    is_resource_group: bool = Field(False, description="是否给资源组创建保护对象")
    is_group_sub_resource: bool = Field(False, description="是否给子资源组子资源创建保护对象")


def check_os_type_path(os_type: str, path_list: list):
    rule = WINDOWS_PATH_REG if os_type == FilesetTemplateOsTypeEnum.WINDOWS.value else NON_WINDOWS_PATH_REG
    for path in path_list:
        if path and not re.match(rule, path, flags=0):
            raise EmeiStorBizException(error=CommonErrorCodes.ERR_PARAM, message="script path is illegal")


class BatchProtectionSubmitReq(BaseModel):
    sla_id: str = Field(..., description="关联的SLA ID", max_length=36)
    resources: List[ProtectResource] = Field(..., description="保护的资源列表")
    ext_parameters: BaseExtParam = Field(..., description="扩展属性")
    post_action: ProtectPostAction = Field(None, description="绑定SLA成功后执行的操作")
    resource_group_id: str = Field("", description="资源组id", max_length=64)

    @validator('ext_parameters', pre=True)
    def check_ext_parameters(cls, ext_parameters, values, **kwargs):
        resources = values.get('resources')
        resource_type_set = set()
        for resource in resources:
            res = resource_service.query_resource_by_id(
                resource.resource_id)
            if not res:
                raise EmeiStorBizException(error=CommonErrorCodes.OBJ_NOT_EXIST,
                                           message=f"resource [{resource.resource_id}] is not existed")
            if res.sub_type == ResourceSubTypeEnum.VirtualMachine and not resource.filters:
                raise EmeiStorBizException(error=CommonErrorCodes.ERR_PARAM,
                                           message="The filters field is required to protect resources in the VMware")
            if res.sub_type == ResourceSubTypeEnum.Fileset.value or res.sub_type == ResourceSubTypeEnum.Volume.value:
                os_type = res.environment_os_type
                ext_param = extend_manager.get_ext_params_class(res.sub_type, ext_parameters)
                script_path = [ext_param.pre_script, ext_param.post_script, ext_param.failed_script]
                check_os_type_path(os_type, script_path)
            resource_type_set.add(res.sub_type)
        if len(resource_type_set) > 1:
            raise EmeiStorBizException(error=CommonErrorCodes.ERR_PARAM,
                                       message="resource in param resources are not of the same type")

        return extend_manager.get_ext_params_class(resource_type_set.pop(), ext_parameters)


class BatchProtectionExecuteReq(BaseModel):
    sla_id: str = Field(..., description="关联的SLA id")
    resources: List[ProtectResource] = Field(..., description="保护的资源列表")
    ext_parameters: BaseExtParam = Field(
        None, description="扩展属性")
    post_action: ProtectPostAction = Field(None, description="绑定SLA成功后执行的操作")
    job_id: str = Field(..., description="批量保护执行请求")
    request_id: str = Field(None, description="任务请求id")
    user_id: str = Field(..., description="用户id")
    resource_group_id: str = Field("", description="资源组id", max_length=64)


class ProtectedObjectQueryResponse(ProtectedObjectQuery):
    sla_compliance: bool = Field(None, description="SLA遵从度，True：遵从，False：不遵从")
    ext_parameters: dict = Field(None, description="扩展属性")

    @validator('ext_parameters', pre=True)
    def check_ext_parameters(cls, v):
        res = None
        if v is None:
            v = {}
        if isinstance(v, dict):
            return v
        return res

    class Config:
        orm_mode = True


class ProtectedObjectQueryRequest(PageRequest):
    domain_id: str = None
    sla_id: str
    name: str = None
    sla_compliance: List[bool] = None
    sub_type: List[ResourceSubTypeEnum] = None
    path: str = None

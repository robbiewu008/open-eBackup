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
from app.base.db_base import database
from app.common.enums.resource_enum import ResourceSubTypeEnum
from app.common.exception.common_error_codes import CommonErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException
from app.common.logger import get_logger

from app.protection.object.service.validator import Validator
from app.resource.models.resource_models import ResExtendInfoTable

log = get_logger(__name__)


class FusionComputeValidator(Validator):

    @staticmethod
    def is_support(sub_type) -> bool:
        return sub_type == ResourceSubTypeEnum.FusionCompute.value\
            or sub_type == ResourceSubTypeEnum.FUSION_ONE_COMPUTE.value

    @staticmethod
    def query_resource_extend_info(resource_id):
        with database.session() as session:
            exist_extend_infos = session.query(ResExtendInfoTable).filter(*{
                ResExtendInfoTable.resource_id == resource_id}).all()
            return exist_extend_infos

    @staticmethod
    def do_validate(resource):
        # 只是对FC虚拟机做状态校验
        if resource.get('type') != 'VM':
            return
        resource_id = resource.get('uuid')
        resource_extend_infos = FusionComputeValidator.query_resource_extend_info(resource_id)
        status = None
        for resource_extend_info in resource_extend_infos:
            if resource_extend_info.key == 'status':
                status = resource_extend_info.value
                break
        if status is None:
            return
        # 运行中和已停止为正常状态
        normal_status_list = ['running', 'stopped']

        # 状态异常的虚拟机
        if status not in normal_status_list:
            log.error(f"Resource: {resource_id} status: {status} is invalid can not protect.")
            raise EmeiStorBizException(error=CommonErrorCodes.STATUS_ERROR,
                                       message="resource verify_status is false, cannot be protected")

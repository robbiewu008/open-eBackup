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
import uuid

from app.resource.schemas.database_schema import DBRestoreCreateResourceSchema

from app.resource.db import db_res_db_api
from app.base.db_base import database
from app.resource.service.common import db_desesitization_service
from app.common.exception.resource_error_codes import ResourceErrorCodes
from app.common.exception.unified_exception import EmeiStorBizException


def create_res(params: DBRestoreCreateResourceSchema):
    with database.session():
        host_info = db_res_db_api.query_host_info_by_uuid(params.host_id)
        if host_info is None:
            raise EmeiStorBizException(ResourceErrorCodes.HOST_NOT_EXISTS,
                                       message=f"Environment {params.host_id} is not exists.")
        db_instance_info_temp = None
        if db_instance_info_temp is None:
            raise EmeiStorBizException(ResourceErrorCodes.HOST_NOT_EXISTS,
                                       message=f"db instance is not exists.")
        db_uuid = str(uuid.uuid5(uuid.NAMESPACE_X500, params.host_id
                                 + params.sub_type
                                 + db_instance_info_temp.get("instance_name")
                                 + params.database_name))
        db_desesitization_service.add_desestitation_info(db_uuid)

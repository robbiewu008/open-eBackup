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
from typing import List

from app.common.exter_attack import exter_attack
from app.common.security.right_control import right_control
from app.common.security.role_dict import RoleEnum
from app.resource.schemas.resource_catalog_schema import ResourceCatalogSchema
from app.resource.service.common import resource_catalog_service
from app.common.concurrency import async_route

resource_catalog_router = async_route()


@exter_attack
@resource_catalog_router.get(
    path="",
    response_model=List[ResourceCatalogSchema],
    summary="查询资源目录",
    status_code=200)
@right_control(roles={RoleEnum.ROLE_SYS_ADMIN, RoleEnum.ROLE_AUDITOR, RoleEnum.ROLE_DP_ADMIN})
def list_resource_catalog():
    catalogs = resource_catalog_service.list_resource_catalog()
    return list(ResourceCatalogSchema(**catalog) for catalog in catalogs)

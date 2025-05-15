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
from app.resource.api import (resource_catalog_api,
                              link_status_api, host_res_api, vmware_api,
                              res_discovery_api, agent_penetrate_api
                              )

from app.resource.api.resource_api import resource_api
from app.common.concurrency import async_route

api = async_route()
api.include_router(
    resource_api,
    prefix="/v1",
    responses={404: {"detail": 'Not found'}}
)

api.include_router(
    res_discovery_api.discovery_router,
    prefix='/v1',
    tags=['Protected Environment'],
    responses={404: {'description': 'Not found'}},
)
api.include_router(
    host_res_api.host_router,
    prefix='/v1',
    tags=['host'],
    responses={404: {'description': 'Not found'}},
)

api.include_router(
    vmware_api.vmware_api,
    prefix='/v1',
    tags=['vmware'],
    responses={404: {'description': 'Not found'}},
)

api.include_router(
    resource_catalog_api.resource_catalog_router,
    prefix="/v1/resource-catalogs",
    tags=['resource-catalog'],
    responses={404: {'description': 'Not found'}},
)

api.include_router(
    link_status_api.sl_api,
    prefix="/v1",
    tags=['service-links'],
    responses={404: {'description': 'Not found'}},
)

api.include_router(
    agent_penetrate_api.agent_penetrate_router,
    prefix="/v1",
    tags=['agent_api'],
    responses={404: {'description': 'Not found'}},
)
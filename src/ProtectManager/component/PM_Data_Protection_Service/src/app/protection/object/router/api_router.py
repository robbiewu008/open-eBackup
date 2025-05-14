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
from app.protection.object.api.protected_object_api import api as projected_object_api
from app.protection.object.api.protected_copy_object_api import api as protected_copy_object_api
from app.common.concurrency import async_route

api = async_route()
api.include_router(projected_object_api, prefix='/v1', tags=["projected_object_api"])
api.include_router(protected_copy_object_api, prefix='/v1', tags=["protected_copy_object_api"])

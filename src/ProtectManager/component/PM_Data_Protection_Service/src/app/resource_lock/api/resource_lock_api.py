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
from fastapi import APIRouter, Body, Path, Query

from app.common import logger
from app.common.concurrency import async_route
from app.common.exter_attack import exter_attack
from app.copy_catalog.common.common import HttpStatusDetail
from app.resource_lock.schemas.lock_schemas import LockRequest, LockResponse
from app.resource_lock.service import lock_service

lock_api = async_route()
api = APIRouter()
LOGGER = logger.get_logger(__name__)

TAG = "ResourceLock"


@exter_attack
@api.post("/internal/locks", status_code=200, summary="创建资源锁", tags=[TAG], response_model=LockResponse)
def lock_resources(lock_request: LockRequest = Body(None, description="资源锁加锁请求", alias="lockRequest")):
    LOGGER.info(f"sync lock resources, request id: {lock_request.request_id}, lock id: {lock_request.lock_id}")
    try:
        result, resource = lock_service.lock(lock_request)
    except Exception:
        return LockResponse(isSuccess=False)
    LOGGER.info(f"sync lock resources finished, result:{result}, failed resources: {resource}")
    if result:
        return LockResponse(isSuccess=result)
    return LockResponse(isSuccess=result, failedResource=resource)


@exter_attack
@api.delete("/internal/locks", status_code=200, summary="清空资源锁", tags=[TAG])
def clear_lock_resources():
    lock_service.clear()


@exter_attack
@api.delete("/internal/locks/{lock_id}", status_code=200, summary="释放资源锁", tags=[TAG])
def unlock_resources(lock_id: str = Path(..., description="需要释放的资源锁id"),
                     request_id: str = Query(None, description="请求id", alias="requestId")):
    lock_service.unlock(request_id, lock_id)


lock_api.include_router(api, prefix="/v1", responses={404: {"detail": HttpStatusDetail.detail_404}})

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

from fastapi import Body

from app.common import logger
from app.common.concurrency import async_route
from app.common.exter_attack import exter_attack
from app.smart_balance.schemas import ExecuteCluster
from app.smart_balance.service import reschedule_job, train_start

log = logger.get_logger(__name__)
smart_balance_router = async_route()


@exter_attack
@smart_balance_router.post("/v1/internal/smart-balance/action/sort",
                           status_code=200,
                           description="sort execute clusters")
def sort_clusters(execute_clusters: List[ExecuteCluster] = Body(None, description="Execute clusters need to sort")):
    if not execute_clusters:
        log.info(f"Clusters is null or empty : {execute_clusters}.")
        return []
    log.info(f"Smart balance sort clusters size: {len(execute_clusters)}")
    return reschedule_job(execute_clusters)


@smart_balance_router.get(
    "/v1/internal/smart-balance/action/retrain",
    status_code=200,
    description="Retrain the smart model")
def retrain():
    log.info(f"Start to train the smart model.")
    train_start()

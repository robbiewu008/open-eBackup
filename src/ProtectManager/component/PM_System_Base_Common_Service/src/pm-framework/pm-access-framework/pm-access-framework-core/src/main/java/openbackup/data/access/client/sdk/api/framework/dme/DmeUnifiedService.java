/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.access.client.sdk.api.framework.dme;

import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.util.AgentApiUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.service.AvailableAgentManagementDomainService;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.net.URI;

/**
 * 功能描述: DmeUnifiedService
 *
 */
@Slf4j
@Service
@AllArgsConstructor
public class DmeUnifiedService {
    private final DmeUnifiedRestApi dmeUnifiedRestApi;
    private final AvailableAgentManagementDomainService domainService;

    /**
     * 下发即时挂载销毁任务
     *
     * @param task 任务参数
     */
    public void cancelLiveMount(LiveMountCancelTask task) {
        URI uri = domainService.getUrlByAgents(AgentApiUtil.getAgentIds(task.getAgents()));
        log.info("Unmount uri: {}, task id: {}", uri, task.getRequestId());
        if (VerifyUtil.isEmpty(uri)) {
            dmeUnifiedRestApi.cancelLiveMount(task);
        } else {
            dmeUnifiedRestApi.cancelLiveMountWithUri(uri, task);
        }
        log.info("Call dme unmount rest api success, uri: {}, jobId: {}.", uri, task.getRequestId());
    }
}
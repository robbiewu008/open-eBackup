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
package openbackup.tidb.resources.access.restore;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;

import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

/**
 * 表恢复
 *
 */
@Slf4j
@Component
public class TidbTableRestoreInterceptor extends TidbDatabaseRestoreInterceptor {
    /**
     * 构造器
     *
     * @param tidbService tidbService
     * @param copyRestApi copyRestApi
     * @param tidbAgentProvider tidbAgentProvider
     * @param resourceService resourceService
     * @param defaultSelector defaultSelector
     */
    public TidbTableRestoreInterceptor(TidbService tidbService, TidbAgentProvider tidbAgentProvider,
        CopyRestApi copyRestApi, ResourceService resourceService, DefaultProtectAgentSelector defaultSelector) {
        super(tidbService, tidbAgentProvider, copyRestApi, resourceService, defaultSelector);
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TIDB_TABLE.getType().equals(object);
    }

    private TaskEnvironment convertToTaskEnvironment(Endpoint endpoint) {
        ProtectedEnvironment protectedEnvironment = resourceService.getResourceById(endpoint.getId())
            .filter(env -> env instanceof ProtectedEnvironment)
            .map(env -> (ProtectedEnvironment) env)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Protected resource is not exists!"));
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(protectedEnvironment, taskEnvironment);
        return taskEnvironment;
    }
}

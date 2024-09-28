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

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 数据库恢复
 *
 */
@Slf4j
@Component
public class TidbDatabaseRestoreInterceptor extends TidbClusterRestoreInterceptor {
    /**
     * 构造器
     *
     * @param tidbService tidbService
     * @param tidbAgentProvider tidbAgentProvider
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     * @param defaultSelector defaultSelector
     */
    public TidbDatabaseRestoreInterceptor(TidbService tidbService, TidbAgentProvider tidbAgentProvider,
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
        return ResourceSubTypeEnum.TIDB_DATABASE.getType().equals(object);
    }

    /**
     * 设置恢复任务endPoint信息
     *
     * @param task task
     */
    @Override
    public void setRestoreTaskEndpoint(RestoreTask task) {
        ProtectedResource resource = BeanTools.copy(task.getTargetObject(), ProtectedResource::new);
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.RESTORE.getValue())
            .build();
        List<Endpoint> endpointList = tidbAgentProvider.getSelectedAgents(agentSelectParam);

        ProtectedResource clusterResource = tidbService.getResourceByCondition(task.getTargetObject().getUuid());
        task.setAgents(endpointList);
        setClusterInfo(task, clusterResource);
    }

    /**
     * 设置集群信息
     *
     * @param task RestoreTask
     * @param clusterResource clusterResource
     */
    public void setClusterInfo(RestoreTask task, ProtectedResource clusterResource) {
        task.getTargetObject()
            .getExtendInfo()
            .put(TidbConstants.CLUSTER_INFO_LIST, clusterResource.getExtendInfo().get(TidbConstants.CLUSTER_INFO_LIST));

        task.getTargetObject()
            .getExtendInfo()
            .put(TidbConstants.TIUP_UUID, clusterResource.getExtendInfo().get(TidbConstants.TIUP_UUID));

        task.getTargetObject()
            .getExtendInfo()
            .put(TidbConstants.TIUP_PATH, clusterResource.getExtendInfo().get(TidbConstants.TIUP_PATH));

        task.getTargetObject().setAuth(clusterResource.getAuth());
    }
}

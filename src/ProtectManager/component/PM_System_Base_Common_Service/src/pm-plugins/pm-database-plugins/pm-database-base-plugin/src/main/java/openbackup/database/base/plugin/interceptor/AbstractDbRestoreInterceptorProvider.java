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
package openbackup.database.base.plugin.interceptor;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupModifyReq;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;

import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.Optional;

/**
 * 数据库恢复插件通用功能
 *
 */
@Slf4j
public abstract class AbstractDbRestoreInterceptorProvider implements RestoreInterceptorProvider {
    /**
     * providerManager
     */
    @Autowired
    protected ProviderManager providerManager;

    @Autowired
    @Qualifier("unifiedConnectionCheckProvider")
    private ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    @Autowired
    private ResourceService resourceService;

    @Override
    public RestoreTask initialize(RestoreTask task) {
        // 检查连通性
        checkConnention(task);
        return supplyRestoreTask(task);
    }

    @Override
    public void postProcess(RestoreTask task, ProviderJobStatusEnum jobStatus) {
        // 数据库应用恢复任务成功后，也需要在后置处理中，指定恢复目标资源的下次备份任务为全量
        log.info("data base restore post process. job status is {}", jobStatus.getStatus());
        if (!Objects.equals(jobStatus, ProviderJobStatusEnum.SUCCESS)) {
            return;
        }
        NextBackupModifyReq nextBackupModifyReq = NextBackupModifyReq.build(findAssociatedResourcesToSetNextFull(task),
            NextBackupChangeCauseEnum.RESTORE_SUCCESS_TO_FULL);
        resourceService.modifyNextBackup(nextBackupModifyReq, false);
    }

    /**
     * 获取与此相关联的资源，用于恢复成功后下次转全量
     *
     * @param task 资源恢复对象
     * @return 关联资源，若包含自身，也需要返回
     */
    protected List<String> findAssociatedResourcesToSetNextFull(RestoreTask task) {
        return Collections.singletonList(task.getTargetObject().getUuid());
    }

    /**
     * 检查连通性
     *
     * @param task RestoreTask
     */
    protected void checkConnention(RestoreTask task) {
        Optional<ProtectedResource> resOptional = resourceService.getResourceById(task.getTargetEnv().getUuid());
        if (!resOptional.isPresent()) {
            return;
        }
        ResourceConnectionCheckProvider provider = providerManager.findProviderOrDefault(
            ResourceConnectionCheckProvider.class, resOptional.get(), resourceConnectionCheckProvider);
        ResourceCheckContext resourceCheckContext = provider.checkConnection(resOptional.get());
        if (resourceCheckContext == null || resourceCheckContext.getActionResults() == null) {
            return;
        }
        for (ActionResult actionResult : resourceCheckContext.getActionResults()) {
            if (!Objects.equals(actionResult.getCode(), 0L)) {
                throw new LegoCheckedException("restore connect failed.");
            }
        }
    }

    /**
     * 数据库各自应用信息
     *
     * @param task RestoreTask
     * @return RestoreTask
     */
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        return task;
    }
}

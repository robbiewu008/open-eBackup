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
package openbackup.openstack.protection.access.provider;

import openbackup.openstack.protection.access.common.OpenstackQuotaService;
import com.huawei.oceanprotect.system.base.quota.enums.UpdateQuotaType;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.copy.CopyDeleteInterceptor;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Optional;

/**
 * Openstack副本删除拦截器
 *
 */
@Slf4j
@Component
public class OpenstackCopyDeleteInterceptor implements CopyDeleteInterceptor {
    private final OpenstackQuotaService quotaService;
    private final ResourceService resourceService;

    public OpenstackCopyDeleteInterceptor(OpenstackQuotaService quotaService, ResourceService resourceService) {
        this.quotaService = quotaService;
        this.resourceService = resourceService;
    }

    @Override
    public void finalize(Copy copy, TaskCompleteMessageBo taskMessage) {
        log.info("openstack copy delete post process, jobId:{}", taskMessage.getJobId());
        Optional.ofNullable(copy.getResourceId())
            .flatMap(resourceService::getResourceById)
            .ifPresent(res -> resourceService.getResourceById(res.getRootUuid())
                .filter(quotaService::isRegisterOpenstack)
                .ifPresent(env -> quotaService.updateUsedQuota(res.getParentUuid(), copy, UpdateQuotaType.REDUCE))
            );
    }

    @Override
    public void initialize(DeleteCopyTask task, CopyInfoBo copy) {
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.equalsSubType(subType);
    }
}

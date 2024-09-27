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
package openbackup.obs.plugin.provider;

import openbackup.data.access.framework.protection.common.util.JobExtendInfoUtil;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobCommonProvider;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.RequiredArgsConstructor;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.Optional;

/**
 * ObjectStorageCommonJobProvider
 *
 * @author c00826511
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-06-13
 */
@Component
@RequiredArgsConstructor
public class ObjectStorageCommonJobProvider implements JobCommonProvider {
    private static final String STORAGE_TYPE = "storageType";

    private final ResourceService resourceService;

    @Override
    public boolean applicable(String subtype) {
        return ResourceSubTypeEnum.OBJECT_SET.equalsSubType(subtype);
    }

    @Override
    public void intercept(Job insertJob) {
        if (JobTypeEnum.BACKUP.getValue().equals(insertJob.getType())) {
            Optional<ProtectedResource> resource = resourceService.getBasicResourceById(false, true,
                insertJob.getSourceId());
            resource.ifPresent(
                res -> JobExtendInfoUtil.insertValueToExtStr(insertJob, Collections.singletonList(STORAGE_TYPE),
                    res.getEnvironment().getExtendInfoByKey(STORAGE_TYPE)));
        }
    }
}

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
package openbackup.exchange.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobCommonProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * ExchangeDatabaseJobProvider
 * <p>
 * 拦截exchange database备份，添加区分数据库属于单机和DAG参数
 *
 */
@Slf4j
@Component
@AllArgsConstructor
public class ExchangeDatabaseJobProvider implements JobCommonProvider {
    private final ResourceService resourceService;

    @Override
    public boolean applicable(String subtype) {
        return ResourceSubTypeEnum.EXCHANGE_DATABASE.equalsSubType(subtype);
    }

    @Override
    public void intercept(Job insertJob) {
        if (isAgentLessBackup(insertJob)) {
            ProtectedResource resource = resourceService.getResourceById(insertJob.getSourceId())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                    "Protected resource not exist. uuid: " + insertJob.getSourceId()));
            String isGroup = resource.getEnvironment().getExtendInfo().get("isGroup");
            JSONObject extendStr = JSONObject.fromObject(insertJob.getExtendStr());
            extendStr.put("isGroup", isGroup);
            insertJob.setExtendStr(extendStr.toString());
        }
    }

    private boolean isAgentLessBackup(Job insertJob) {
        return JobTypeEnum.BACKUP.getValue().equals(insertJob.getType())
            && ResourceSubTypeEnum.EXCHANGE_DATABASE.equalsSubType(insertJob.getSourceSubType());
    }
}

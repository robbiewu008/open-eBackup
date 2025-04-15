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
package openbackup.tdsql.resources.access.interceptor;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;

import org.apache.commons.collections.MapUtils;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.Map;

/**
 * 功能描述
 *
 */
@Slf4j
@Component
public class TdsqlClusterGroupCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    /**
     * Constructor
     *
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     */
    public TdsqlClusterGroupCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType().equals(subType);
    }

    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return false;
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        log.info("TdsqlClusterGroupCopyDeleteInterceptor, handleTask, start.");
        Map<String, String> advanceParams = task.getAdvanceParams();
        if (MapUtils.isEmpty(advanceParams)) {
            advanceParams = new HashMap<>();
        }
        advanceParams.put(TdsqlConstant.RESOURCE_EXISTS, String.valueOf(super.isResourceExists(task)));
        task.setAdvanceParams(advanceParams);
    }
}

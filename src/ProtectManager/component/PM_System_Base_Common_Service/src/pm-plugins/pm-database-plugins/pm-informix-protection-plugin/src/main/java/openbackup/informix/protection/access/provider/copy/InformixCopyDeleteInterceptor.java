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
package openbackup.informix.protection.access.provider.copy;

import static openbackup.data.access.framework.copy.mng.util.CopyUtil.getNextCopyByType;

import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.informix.protection.access.constant.InformixConstant;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.MapUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * informix副本删除拦截器
 *
 * @author hwx1164326
 * @version [DataBackup 1.5.0]
 * @since 2023-08-03
 */
@Component
@Slf4j
public class InformixCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    /**
     * 构造函数
     *
     * @param copyRestApi 副本api
     * @param resourceService 资源服务
     */
    public InformixCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType(),
                        ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType()).contains(subType);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsCumulativeIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 删除差异副本，会删除该差异副本后面所有的增量副本
        Copy nextCumulativeCopy = getNextCopyByType(copies, BackupTypeConstants.CUMULATIVE_INCREMENT, thisCopy.getGn());
        Copy nextCopy = CopyUtil.getSmallerCopy(nextFullCopy, nextCumulativeCopy);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextCopy);
    }
    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        log.info("InformixCopyDeleteInterceptor,handleTask,start.");
        Map<String, String> advanceParams = task.getAdvanceParams();
        if (MapUtils.isEmpty(advanceParams)) {
            advanceParams = new HashMap<>();
        }
        advanceParams.put(InformixConstant.RESOURCE_EXISTS, String.valueOf(super.isResourceExists(task)));
        task.setAdvanceParams(advanceParams);
    }
    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return false;
    }
}

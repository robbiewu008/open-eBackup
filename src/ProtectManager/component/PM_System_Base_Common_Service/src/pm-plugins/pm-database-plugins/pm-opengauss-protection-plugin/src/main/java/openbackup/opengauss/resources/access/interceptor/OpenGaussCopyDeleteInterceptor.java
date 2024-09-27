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
package openbackup.opengauss.resources.access.interceptor;

import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * OpenGauss副本删除Provider
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-17
 */
@Slf4j
@Component
public class OpenGaussCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    /**
     * 构造器注入
     *
     * @param copyRestApi 副本查询接口
     * @param resourceService resourceService
     */
    public OpenGaussCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType(),
            ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType()).contains(subType);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        List<Copy> differenceCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.DIFFERENCE_INCREMENT);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(differenceCopies, thisCopy, nextFullCopy);
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        task.setIsForceDeleted(true);
    }
}

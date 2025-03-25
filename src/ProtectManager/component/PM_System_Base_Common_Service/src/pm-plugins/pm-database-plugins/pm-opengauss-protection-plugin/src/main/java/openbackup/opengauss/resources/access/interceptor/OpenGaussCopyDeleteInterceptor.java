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

import static openbackup.data.access.framework.copy.mng.util.CopyUtil.getCopiesBetweenTwoCopy;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * OpenGauss副本删除Provider
 *
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
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        if (isContainsMorePreviousFullCopy(thisCopy)) {
            return getCopiesBetweenTwoCopy(copies, thisCopy, nextFullCopy).stream()
                .filter(copy -> copy.getBackupType() != BackupTypeConstants.LOG.getAbBackupType())
                .map(Copy::getUuid)
                .collect(Collectors.toList());
        }
        log.info("delete log, copyid: {}", thisCopy.getUuid());
        return getCopiesBetweenTwoCopy(copies, thisCopy, nextFullCopy).stream()
            .map(Copy::getUuid)
            .collect(Collectors.toList());
    }

    private boolean isContainsMorePreviousFullCopy(Copy thisCopy) {
        List<Copy> copies = copyRestApi.queryCopiesByResourceId(thisCopy.getResourceId());
        return copies.stream()
            .anyMatch(copy -> copy.getBackupType() == BackupTypeConstants.FULL.getAbBackupType()
                && copy.getGn() < thisCopy.getGn() && thisCopy.getGeneratedBy().equals(copy.getGeneratedBy()));
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        // 不需要UBC后置任务删除日志备份副本，设置为false
        Map<String, String> map = new HashMap<>(1);
        map.put(OpenGaussConstants.IS_DELETE_RELATIVE_COPIES, String.valueOf(false));
        task.addParameters(map);
    }
}

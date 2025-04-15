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
package openbackup.kingbase.protection.access.provider.copy;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

/**
 * Kingbase副本删除Provider
 *
 */
@Slf4j
@Component
public class KingBaseCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    private final CopyService copyService;

    /**
     * 构造函数
     *
     * @param copyRestApi 副本API
     * @param resourceService 资源服务
     * @param copyService 获取副本相关的信息服务
     */
    public KingBaseCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService,
        CopyService copyService) {
        super(copyRestApi, resourceService);
        this.copyService = copyService;
    }

    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return false;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return Arrays.asList(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType(),
            ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType()).contains(resourceSubType);
    }

    @Override
    public List<String> getAssociatedCopy(String copyId) {
        return super.getAssociatedCopy(copyId);
    }

    @Override
    protected List<String> collectDeleteAssociatedCopy(Copy thisCopy) {
        return super.collectDeleteAssociatedCopy(thisCopy);
    }

    @Override
    protected List<String> getShouldDeleteCopies(List<Copy> copies, Copy thisCopy) {
        return super.getShouldDeleteCopies(copies, thisCopy);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 删除下一个全量副本前所有日志/增量副本
        List<BackupTypeConstants> associatedTypes = Arrays.asList(BackupTypeConstants.LOG,
            BackupTypeConstants.DIFFERENCE_INCREMENT);
        List<Copy> confContainCopies = copies.stream().filter(copy -> {
            BackupTypeConstants copyBackupType = BackupTypeConstants.getBackupTypeByAbBackupType(copy.getBackupType());
            return associatedTypes.contains(copyBackupType);
        }).collect(Collectors.toList());
        List<Copy> associatedCopies = CopyUtil.getCopiesBetweenTwoCopy(confContainCopies, thisCopy, nextFullCopy);
        if (associatedCopies.isEmpty()) {
            return Collections.emptyList();
        }
        return associatedCopies.stream().map(Copy::getUuid).collect(Collectors.toList());
    }

    @Override
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 删除下一个全量副本前所有日志/增量副本
        return getCopiesCopyTypeIsFull(copies, thisCopy, nextFullCopy);
    }
}

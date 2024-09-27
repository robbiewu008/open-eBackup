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
package openbackup.data.access.framework.backup.controller;

import openbackup.data.access.framework.backup.controller.vo.TransferBackupVo;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupFeatureService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.util.EnumUtil;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import java.util.Collections;
import java.util.List;

/**
 * 备份内部Controller
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/5/27
 */
@RestController
@RequestMapping("/v2/internal/backup")
public class BackupInternalController {
    @Autowired
    private BackupFeatureService backupFeatureService;

    private ProviderManager providerManager;

    private ResourceService resourceService;

    @Autowired
    public void setProviderManager(ProviderManager providerManager) {
        this.providerManager = providerManager;
    }

    @Autowired
    public void setResourceService(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    /**
     * 查询是否支持数据备份与日志备份并行备份
     *
     * @param resourceId 资源Id
     * @return 是否支持并行
     */
    @ExterAttack
    @GetMapping("allowDataAndLogParallel")
    public boolean isSupportDataAndLogParallelBackup(@RequestParam("resourceId") String resourceId) {
        return backupFeatureService.isSupportDataAndLogParallelBackup(resourceId);
    }

    /**
     * 转换备份类型
     *
     * @param transferBackupVo 转换备份类型结构体
     * @return 转换后的备份类型
     */
    @ExterAttack
    @PostMapping("transferBackupType")
    BackupTypeConstants transferBackupType(@RequestBody TransferBackupVo transferBackupVo) {
        BackupTypeConstants backupTypeConstants = EnumUtil.get(BackupTypeConstants.class,
                BackupTypeConstants::getBackupType, transferBackupVo.getBackupType(), false, true);
        if (backupTypeConstants == null) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                    "The param is error: " + transferBackupVo.getBackupType());
        }
        return backupFeatureService.transferBackupType(backupTypeConstants, transferBackupVo.getResourceId());
    }

    /**
     * 备份流程获取自定义的资源锁
     *
     * @param resourceId 资源Id
     * @return 加锁资源
     */
    @ExterAttack
    @GetMapping("getBackupResourceLock")
    public List<LockResourceBo> getBackupResourceLock(@RequestParam("resourceId") String resourceId) {
        ProtectedResource protectedResource = getProtectedResource(resourceId);
        BackupInterceptorProvider backupInterceptorProvider =
            providerManager.findProvider(BackupInterceptorProvider.class, protectedResource.getSubType(), null);
        if (backupInterceptorProvider == null) {
            return Collections.emptyList();
        }
        return backupInterceptorProvider.getLockResources(protectedResource);
    }

    private ProtectedResource getProtectedResource(String resourceId) {
        return resourceService.getBasicResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Resource not found."));
    }
}

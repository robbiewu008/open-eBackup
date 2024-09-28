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
package openbackup.openstack.adapter.service;

import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.generator.SlaGenerator;
import com.huawei.oceanprotect.sla.sdk.api.SlaOperateService;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.CreateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * SLA相关操作管理器
 *
 */
@Component
@Slf4j
public class SlaManager {
    private final SlaOperateService slaOperateService;
    private final SlaQueryService slaQueryService;
    private final OpenStackUserManager userManager;

    public SlaManager(SlaOperateService slaOperateService, SlaQueryService slaQueryService,
        OpenStackUserManager userManager) {
        this.slaOperateService = slaOperateService;
        this.slaQueryService = slaQueryService;
        this.userManager = userManager;
    }

    /**
     * 创建SLA
     *
     * @param backupJob {@link OpenStackBackupJobDto} backupJob
     * @return SLA id
     */
    public String createSla(OpenStackBackupJobDto backupJob) {
        CreateSlaCommand sla = SlaGenerator.generateCreateSlaCommand(backupJob);
        String userId = userManager.obtainUserId();
        sla.setUserId(userId);

        String slaId = slaOperateService.create(sla).getUuid();
        log.info("Create sla success, sla id: {}.", slaId);
        return slaId;
    }

    /**
     * 修改SLA
     *
     * @param backupJob {@link OpenStackBackupJobDto} backupJob
     * @param oldSla 旧SLA
     * @return SLA id
     */
    public String updateSla(OpenStackBackupJobDto backupJob, SlaDto oldSla) {
        UpdateSlaCommand sla = SlaGenerator.generateUpdateSlaCommand(backupJob, oldSla);
        // 此功能暂时未使用 RBAC需求先不适配这里 后续使用RBAC框架需要增加domainId到command
        String slaId = slaOperateService.update(sla, false).getUuid();
        log.info("Update sla success, sla id: {}", slaId);
        return slaId;
    }

    /**
     * 查询SLA
     *
     * @param slaId SLA id
     * @return {@link SlaDto} SLA
     */
    public SlaDto querySla(String slaId) {
        return slaQueryService.querySlaById(slaId);
    }

    /**
     * 删除SLA
     *
     * @param slaId SLA id
     */
    public void deleteSla(String slaId) {
        if (isGlobalSla(slaId)) {
            log.info("Sla: {} is global, do not delete.", slaId);
            return;
        }
        slaOperateService.delete(slaId);
        log.info("Delete sla: {} success.", slaId);
    }

    private boolean isGlobalSla(String slaId) {
        SlaDto sla = slaQueryService.querySlaById(slaId);
        return sla.isGlobal();
    }
}

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

import openbackup.openstack.adapter.constants.OpenStackErrorCodes;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.exception.OpenStackException;
import openbackup.openstack.adapter.generator.ProtectionGenerator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ManualBackupReq;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ProtectionBatchOperationReq;
import openbackup.system.base.sdk.resource.model.ProtectionCreationDto;
import openbackup.system.base.sdk.resource.model.ProtectionModifyDto;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;

/**
 * 保护相关操作管理器
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-17
 */
@Slf4j
@Component
public class ProtectionManager {
    private final ProtectObjectRestApi protectObjectRestApi;
    private final OpenStackUserManager userManager;

    public ProtectionManager(ProtectObjectRestApi protectObjectRestApi, OpenStackUserManager userManager) {
        this.protectObjectRestApi = protectObjectRestApi;
        this.userManager = userManager;
    }

    /**
     * 创建保护
     *
     * @param slaId 创建保护使用的SLA id
     * @param resourceId 资源id
     * @param backupJob {@link OpenStackBackupJobDto} OpenStack备份任务
     * @return 保护任务id
     */
    public String createProtection(String slaId, String resourceId, OpenStackBackupJobDto backupJob) {
        ProtectionCreationDto protection = ProtectionGenerator.generateProtectionCreation(slaId, resourceId,
            backupJob);
        String userId = userManager.obtainUserId();
        try {
            List<String> jobIds = protectObjectRestApi.createProtectedObject(userId, protection);
            log.info("Create protection job: {} of resource: {} and sla: {} success.", jobIds, resourceId, slaId);
            return jobIds.get(0);
        } catch (LegoUncheckedException | FeignException ex) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "call create protection error.");
        }
    }

    /**
     * 修改保护
     *
     * @param slaId SLA id
     * @param backupJob {@link OpenStackBackupJobDto} OpenStack备份任务
     * @param protectedObject {@link ProtectedObjectInfo} 修改前保护对象信息
     * @return 修改保护任务id
     */
    public String modifyProtection(String slaId, OpenStackBackupJobDto backupJob, ProtectedObjectInfo protectedObject) {
        ProtectionModifyDto modifyReq = ProtectionGenerator.generateProtectionModifyReq(slaId,
            protectedObject.getResourceId(), backupJob, protectedObject);

        String userId = userManager.obtainUserId();
        try {
            String jobId = protectObjectRestApi.modifyProtectedObject(userId, modifyReq).replace("\"", "");
            log.info("Create modify protection job: {} of resource: {} and sla: {} success.", jobId,
                protectedObject.getResourceId(), slaId);
            return jobId;
        } catch (LegoUncheckedException | FeignException ex) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "call modify protection error.");
        }
    }

    /**
     * 查询受保护对象
     *
     * @param resourceId 资源id
     * @param isStrict 受保护对象是否必须存在
     * @return {@link ProtectedObjectInfo} 受保护对象
     */
    public ProtectedObjectInfo queryProtectedObject(String resourceId, boolean isStrict) {
        try {
            ProtectedObjectInfo protectObject = protectObjectRestApi.getProtectObject(resourceId);
            if (isStrict) {
                PowerAssert.notNull(protectObject, () -> new OpenStackException(OpenStackErrorCodes.NOT_FOUND,
                    String.format("Backup job: %s not found.", resourceId)));
            }
            return protectObject;
        } catch (LegoUncheckedException | FeignException ex) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "call query protected object error.");
        }
    }

    /**
     * 激活保护
     *
     * @param resourceId 待激活保护的资源id
     */
    public void activate(String resourceId) {
        ProtectionBatchOperationReq operationReq = new ProtectionBatchOperationReq();
        operationReq.setResourceIds(Collections.singletonList(resourceId));
        try {
            protectObjectRestApi.activate(operationReq);
        } catch (LegoUncheckedException | FeignException ex) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "call activate error.");
        }
    }

    /**
     * 取消激活
     *
     * @param resourceId 资源id
     */
    public void deactivate(String resourceId) {
        ProtectionBatchOperationReq operationReq = new ProtectionBatchOperationReq();
        operationReq.setResourceIds(Collections.singletonList(resourceId));
        try {
            protectObjectRestApi.deactivate(operationReq);
        } catch (LegoUncheckedException | FeignException ex) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "call deactivate error.");
        }
    }

    /**
     * 执行手动备份
     *
     * @param resourceId 执行手动备份的资源id
     * @param slaId 资源绑定的SLA的id
     * @param backupAction {@link com.huawei.oceanprotect.sla.sdk.enums.PolicyAction} 手动备份类型
     */
    public void manualBackup(String resourceId, String slaId, String backupAction) {
        String userId = userManager.obtainUserId();
        ManualBackupReq backupReq = new ManualBackupReq();
        backupReq.setAction(backupAction);
        backupReq.setSlaId(slaId);
        try {
            List<String> jobIds = protectObjectRestApi.manualBackup(resourceId, userId, backupReq);
            log.info("Create manual backup job success, job id: {}", jobIds);
        } catch (LegoUncheckedException | FeignException ex) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "call manual backup error.");
        }
    }

    /**
     * 删除保护
     *
     * @param resourceId 受保护对象资源id
     */
    public void delete(String resourceId) {
        ProtectionBatchOperationReq params = new ProtectionBatchOperationReq();
        params.setResourceIds(Collections.singletonList(resourceId));
        try {
            protectObjectRestApi.deleteProtectedObjects(params);
            log.info("Delete protected object: {} success.", resourceId);
        } catch (LegoUncheckedException | FeignException ex) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "call delete protected object error.");
        }
    }
}

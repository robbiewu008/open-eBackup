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
import openbackup.openstack.adapter.dto.OpenStackQuotaDto;
import openbackup.openstack.adapter.exception.OpenStackException;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.unit.CapabilityUnitType;
import openbackup.system.base.common.utils.unit.UnitConvert;
import com.huawei.oceanprotect.system.base.quota.enums.UserQuotaErrorCode;
import com.huawei.oceanprotect.system.base.quota.po.UserQuota;
import com.huawei.oceanprotect.system.base.quota.po.UserQuotaPo;
import com.huawei.oceanprotect.system.base.quota.service.UserQuotaService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;

/**
 * OpenStack配额管理器
 *
 */
@Slf4j
@Component
public class OpenStackQuotaManager {
    private static final long NO_LIMIT_QUOTA = -1;
    private static final int SCALE = 2;

    private final UserQuotaService userQuotaService;
    private final OpenStackUserManager userManager;

    public OpenStackQuotaManager(UserQuotaService userQuotaService, OpenStackUserManager userManager) {
        this.userQuotaService = userQuotaService;
        this.userManager = userManager;
    }

    /**
     * 设置配额
     *
     * @param projectId OpenStack项目id
     * @param quota 配额
     */
    public void setQuota(String projectId, OpenStackQuotaDto quota) {
        String userId = userManager.obtainUserId();
        UserQuota userQuota = buildUserQuota(projectId, userId, quota.getSize());
        try {
            userQuotaService.setUserQuota(userQuota);
            log.info("Set project: {} quota: {}(B) success.", projectId, userQuota.getBackupTotalQuota());
        } catch (LegoCheckedException exception) {
            long errorCode = exception.getErrorCode();
            // 该错误场景需返回接口文档中指定错误码
            if (errorCode == UserQuotaErrorCode.USER_TOTAL_QUOTA_NOT_ENOUGH.getCode()) {
                throw new OpenStackException(OpenStackErrorCodes.INIT_QUOTA_LESS_THAN_USED,
                    String.format("Set project: %s of user: %s quota: %s fail.", projectId, userId,
                        userQuota.getBackupTotalQuota()));
            }
            throw exception;
        }
    }

    private UserQuota buildUserQuota(String projectId, String userId, long size) {
        UserQuota userQuota = new UserQuota();

        userQuota.setUserId(userId);
        userQuota.setResourceId(projectId);
        userQuota.setBackupTotalQuota(
            size == NO_LIMIT_QUOTA ? size : UnitConvert.convert(size, CapabilityUnitType.GB, CapabilityUnitType.BYTE));
        userQuota.setCloudArchiveTotalQuota(NO_LIMIT_QUOTA);
        return userQuota;
    }

    /**
     * 获取配额
     *
     * @param projectId OpenStack项目id
     * @return 项目配额
     */
    public List<OpenStackQuotaDto> getQuota(String projectId) {
        List<UserQuotaPo> userQuota =
                userQuotaService.listUserQuotaInfoByResourceId(Collections.singletonList(projectId));
        return userQuota.stream().map(this::buildOpenStackQuota).collect(Collectors.toList());
    }

    private OpenStackQuotaDto buildOpenStackQuota(UserQuotaPo userQuota) {
        double total =
                userQuota.getBackupTotalQuota() == NO_LIMIT_QUOTA
                        ? NO_LIMIT_QUOTA
                        : UnitConvert.convert(
                                userQuota.getBackupTotalQuota(), CapabilityUnitType.BYTE, CapabilityUnitType.GB, SCALE);
        double used =
                UnitConvert.convert(
                        userQuota.getBackupUsedQuota(), CapabilityUnitType.BYTE, CapabilityUnitType.GB, SCALE);
        log.info(
                "OpenStack resource: {} total quota: {}, used: {}(B).",
                userQuota.getResourceId(),
                userQuota.getBackupTotalQuota(),
                userQuota.getBackupUsedQuota());
        OpenStackQuotaDto quota = new OpenStackQuotaDto();
        quota.setAll((int) total);
        quota.setUsed((int) used);
        return quota;
    }
}

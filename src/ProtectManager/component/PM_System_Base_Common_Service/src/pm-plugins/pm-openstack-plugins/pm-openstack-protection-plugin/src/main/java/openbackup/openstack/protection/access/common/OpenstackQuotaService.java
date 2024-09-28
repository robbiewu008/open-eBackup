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
package openbackup.openstack.protection.access.common;

import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.common.utils.JSONObject;
import com.huawei.oceanprotect.system.base.quota.enums.QuotaTaskTypeEnum;
import com.huawei.oceanprotect.system.base.quota.enums.UpdateQuotaType;
import com.huawei.oceanprotect.system.base.quota.service.UserQuotaService;
import openbackup.system.base.sdk.copy.model.CopyInfo;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Optional;

/**
 * openstack 公共服务
 *
 */
@Slf4j
@Component
public class OpenstackQuotaService {
    private final UserQuotaService userQuotaService;

    public OpenstackQuotaService(UserQuotaService userQuotaService) {
        this.userQuotaService = userQuotaService;
    }

    /**
     * 检查是否为云核场景
     *
     * @param environment 环境资源
     * @return 判断结果
     */
    public boolean isRegisterOpenstack(ProtectedResource environment) {
        log.info("check environment is register openstack.");
        return Optional.ofNullable(environment)
            .map(ProtectedResource::getExtendInfo)
            .flatMap(extendInfo -> Optional.ofNullable(extendInfo.get(OpenstackConstant.REGISTER_SERVICE)))
            .filter(OpenstackConstant.REGISTER_OPENSTACK::equals)
            .isPresent();
    }

    /**
     * 更新项目配额
     *
     * @param projectId 项目id
     * @param copy      副本
     * @param quotaType 配额更新类型
     */
    public void updateUsedQuota(String projectId, CopyInfo copy, UpdateQuotaType quotaType) {
        log.info("openstack update quota. project:{}, copy:{}, userId:{}", projectId, copy.getUuid(), copy.getUserId());
        JSONObject properties = JSONObject.fromObject(copy.getProperties());
        String quota = properties.getString(CopyPropertiesKeyConstant.SIZE, "0");
        userQuotaService.updateUserUsedQuota(copy.getUserId(), projectId, copy.getGeneratedBy(), quotaType, quota);
    }

    /**
     * 检查备份配额
     *
     * @param userId    用户id
     * @param projectId 项目id
     */
    public void checkBackupQuota(String userId, String projectId) {
        log.info("User: {} check quota of resource: {} to backup.", userId, projectId);
        userQuotaService.checkUserQuotaInSrc(userId, projectId, QuotaTaskTypeEnum.TASK_BACKUP, false);
    }
}

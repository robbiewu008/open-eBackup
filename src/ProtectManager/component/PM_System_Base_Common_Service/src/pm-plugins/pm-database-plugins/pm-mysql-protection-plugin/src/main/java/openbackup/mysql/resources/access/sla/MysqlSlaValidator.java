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
package openbackup.mysql.resources.access.sla;

import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.common.MysqlErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述:
 * MySQL的sla校验器
 *
 */
@Slf4j
@Component
public class MysqlSlaValidator implements SlaValidateProvider {
    private ProtectObjectRestApi protectObjectRestApi;

    private ProtectedEnvironmentService protectedEnvironmentService;

    @Autowired
    public void setProtectObjectRestApi(ProtectObjectRestApi protectObjectRestApi) {
        this.protectObjectRestApi = protectObjectRestApi;
    }

    @Autowired
    public void setProtectedEnvironmentService(ProtectedEnvironmentService protectedEnvironmentService) {
        this.protectedEnvironmentService = protectedEnvironmentService;
    }

    /**
     * Validator 过滤接口，过滤MySQL
     *
     * @param object object 受保护资源
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.MYSQL.getType().equals(object);
    }

    /**
     * Validator 配置接口，对MySQL的备份类型进行配置
     *
     * @return slaValidateConfig 配置
     */
    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig slaValidateConfig = new SlaValidateConfig();

        // Mysql共需要配置四种备份类型（全量，差异增量，累积增量，日志）/复制/归档
        slaValidateConfig.getSpecificationConfig()
                .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                        SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                        SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION_LOG,
                        SlaConstants.REPLICATION_LOG_POLICY_COUNT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT))
                .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT));
        log.info("set MySQL sla PolicyLimitConfig success.");
        return slaValidateConfig;
    }

    @Override
    public void validateSLA(SlaBase slaBase) {
        SlaValidateProvider.super.validateSLA(slaBase);
        if (!(slaBase instanceof UpdateSlaCommand)) {
            return;
        }
        UpdateSlaCommand updateSlaCommand = (UpdateSlaCommand) slaBase;
        List<String> policyActions = updateSlaCommand.getPolicyList()
            .stream()
            .map(PolicyDto::getAction)
            .map(PolicyAction::getAction)
            .collect(Collectors.toList());
        boolean isContainsLogAction = policyActions.stream().anyMatch(PolicyAction.LOG.getAction()::equals);
        if (!isContainsLogAction) {
            log.info("Do not need to check the action of the sla policy.");
            return;
        }
        List<ProtectedObjectInfo> items;
        int page = 0;
        do {
            BasePage<ProtectedObjectInfo> data = protectObjectRestApi.pageQueryProtectObject(updateSlaCommand.getUuid(),
                page, IsmNumberConstant.HUNDRED);
            items = data.getItems();
            Optional<ProtectedObjectInfo> databaseProject = items.stream().filter(project -> {
                if (!ResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.equalsSubType(project.getSubType())) {
                    return false;
                }
                ProtectedEnvironment env = protectedEnvironmentService.getEnvironmentById(project.getEnvId());
                String clusterType = env.getExtendInfoByKey(DatabaseConstants.CLUSTER_TYPE);
                return MysqlConstants.EAPP.equals(clusterType);
            }).findFirst();
            // sla已绑定eappmysql资源
            if (databaseProject.isPresent()) {
                throw new LegoCheckedException(MysqlErrorCode.EAPP_MYSQL_NOT_SUPPORT_LOG_BACKUP,
                    "Eappmysql do not allow policy for log backup.");
            }
            page++;
        } while (items.size() >= IsmNumberConstant.HUNDRED);
    }
}

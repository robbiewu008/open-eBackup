/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tpops.protection.access.sla;

import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * TPOPS GaussDb SLA校验器
 *
 * @author x30021699
 * @since 2023-05-09
 */

@Slf4j
@Component
public class TpopsGaussDBValidatorProvider implements SlaValidateProvider {
    private static final String STORAGE_ID = "storage_id";

    private static final String EXTERNAL_STORAGE_ID = "external_storage_id";

    private final BackupStorageApi backupStorageApi;

    public TpopsGaussDBValidatorProvider(BackupStorageApi backupStorageApi) {
        this.backupStorageApi = backupStorageApi;
    }

    @Override
    public boolean applicable(String object) {
        return Objects.equals(object, ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig config = new SlaValidateConfig();
        log.info("start to config sla");
        config.getSpecificationConfig()
            .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
        return config;
    }

    @Override
    public void validateSLA(SlaBase slaBase) {
        // 校验本地存储库id是否可信
        checkStorageIdExists(slaBase);

        // 外部存储id 校验存储库id是否可信
        checkExternalStorageIdExists(slaBase);
    }

    private void checkStorageIdExists(SlaBase slaBase) {
        slaBase.getPolicyList().forEach(policyDto -> {
            if (policyDto.getExtParameters().get(STORAGE_ID) == null) {
                return;
            }
            String storageId = policyDto.getExtParameters().get(STORAGE_ID).textValue();

            // 备份的存储库ID不为空，且能查询到相关信息
            if (!StringUtils.isEmpty(storageId) && policyDto.getType() == PolicyType.BACKUP && !hasStorageInfo(
                storageId)) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "storage id: " + storageId + " not exists");
            }
        });
        log.debug("check sla: {} external storage id success.", slaBase.getName());
    }

    private boolean hasStorageInfo(String storageId) {
        try {
            NasDistributionStorageDetail detail = backupStorageApi.getDetail(storageId);
            return !StringUtils.isEmpty(detail.getUuid());
        } catch (LegoCheckedException e) {
            log.error("get storage detail fail.", e);
            return false;
        }
    }

    private void checkExternalStorageIdExists(SlaBase slaBase) {
        List<PolicyDto> policyDtos = slaBase.getPolicyList()
            .stream()
            .filter(policy -> PolicyType.REPLICATION.getType().equals(policy.getType().toString()))
            .collect(Collectors.toList());

        // 没有复制策略时直接返回,不校验
        policyDtos.forEach(policyDto -> {
            // 不存在该字段
            if (policyDto.getExtParameters().get(EXTERNAL_STORAGE_ID) == null) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "external storage id not exists");
            }
            String storageId = policyDto.getExtParameters().get(EXTERNAL_STORAGE_ID).textValue();

            // 备份的存储库ID不为空
            if (StringUtils.isEmpty(storageId)) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "external storage id: " + storageId + " not exists");
            }
        });
    }
}

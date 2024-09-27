package openbackup.gaussdbdws.protection.access.sla;

import com.huawei.oceanprotect.base.cluster.repository.ClusterRepository;
import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.ArrayTargetClusterService;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.protection.access.provider.sdk.protection.ProtectionQueryService;
import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.constant.DwsErrorCode;
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

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Objects;
import java.util.stream.Collectors;

/**
 * dws校验器
 *
 * @author mwx1009754
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-30
 */

@Slf4j
@Component
public class DwsValidator implements SlaValidateProvider {
    private static final String STORAGE_ID = "storage_id";

    private static final String EXTERNAL_STORAGE_ID = "external_storage_id";

    private static final String EXTERNAL_SYSTEM_ID = "external_system_id";

    private final BackupStorageApi backupStorageApi;

    private final ProtectionQueryService protectionQueryService;

    private final ArrayTargetClusterService arrayTargetClusterService;

    private final ClusterRepository clusterRepository;

    public DwsValidator(BackupStorageApi backupStorageApi, ProtectionQueryService protectionQueryService,
        ArrayTargetClusterService arrayTargetClusterService, ClusterRepository clusterRepository) {
        this.backupStorageApi = backupStorageApi;
        this.protectionQueryService = protectionQueryService;
        this.arrayTargetClusterService = arrayTargetClusterService;
        this.clusterRepository = clusterRepository;
    }

    @Override
    public boolean applicable(String object) {
        return Objects.equals(object, ResourceSubTypeEnum.GAUSSDB_DWS.getType());
    }

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig config = new SlaValidateConfig();
        config.getBackupConfig().checkBackupUnitRange(false);
        config.getSpecificationConfig()
            .setLimit(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT))
            .setLimit(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
        return config;
    }

    @Override
    public void validateSLA(SlaBase slaBase) {
        // 校验本地存储库id是否可信
        checkStorageIdExists(slaBase);

        // 校验当前关联资源是否不允许创建该类型保护策略
        checkApplicationIsSupportAction(slaBase);
        // 外部存储id 校验存储库id是否可信
        checkExternalStorageIdExists(slaBase);
    }

    private void checkApplicationIsSupportAction(SlaBase slaBase) {
        PolicyAction policyAction = PolicyAction.DIFFERENCE_INCREMENT;
        List<PolicyAction> actions = slaBase.getPolicyList()
            .stream()
            .map(PolicyDto::getAction)
            .distinct()
            .collect(Collectors.toList());
        if (!actions.contains(PolicyAction.DIFFERENCE_INCREMENT) && !actions.contains(
            PolicyAction.CUMULATIVE_INCREMENT)) {
            return;
        }
        if (!actions.contains(policyAction)) {
            policyAction = PolicyAction.CUMULATIVE_INCREMENT;
        }
        int count = protectionQueryService.countBySubTypeAndSlaName(slaBase.getName(), null,
            ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType());
        if (count != 0) {
            log.error("Count size: {}", count);
            throw new LegoCheckedException(DwsErrorCode.NO_SUPPORT_SLA,
                new String[] {"explore_dws_schema_label", DwsConstant.SLA_ACTION.get(policyAction)},
                ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType() + " not support " + policyAction.getAction());
        }

        count = protectionQueryService.countBySubTypeAndSlaName(slaBase.getName(), null,
            ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType());
        if (count != 0) {
            log.error("Count size: {}", count);
            throw new LegoCheckedException(DwsErrorCode.NO_SUPPORT_SLA,
                new String[] {"explore_dws_table_label", DwsConstant.SLA_ACTION.get(policyAction)},
                ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType() + " not support " + policyAction.getAction());
        }
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
        // 备份策略未配置或配置非并行备份存储单元组，复制策略不配置备份存储单元组
        // 备份策略配置并行备份存储单元组，复制策略需配置并行备份存储单元组
        PolicyDto backupPolicy = slaBase.getPolicyList()
            .stream()
            .filter(policy -> PolicyType.BACKUP.equals(policy.getType()))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "No backup policy."));
        JsonNode backupPolicyExtParameters = backupPolicy.getExtParameters();
        if (!backupPolicyExtParameters.has(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY)) {
            return;
        }
        JsonNode storageInfo = backupPolicyExtParameters.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY);
        if (!storageInfo.has(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY) || !Objects.equals(
            BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE,
            storageInfo.get(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY).textValue())) {
            return;
        }
        List<PolicyDto> policyDtos = slaBase.getPolicyList()
            .stream()
            .filter(policy -> Objects.equals(PolicyType.REPLICATION, policy.getType()))
            .collect(Collectors.toList());
        // 备份策略未配置备份存储单元组
        if (!storageInfo.has(STORAGE_ID) || StringUtils.isEmpty(storageInfo.get(STORAGE_ID).asText())) {
            // 复制策略不能配置备份存储单元组
            checkRepExternalStorageShouldEmpty(policyDtos);
        } else {
            String storageId = storageInfo.get(STORAGE_ID).textValue();
            NasDistributionStorageDetail detail = backupStorageApi.getDetail(storageId);
            // 备份策略配置并行备份存储单元组，复制策略需配置并行备份存储单元组
            if (detail.isHasEnableParallelStorage()) {
                checkRepStorageParallel(policyDtos);
            } else {
                // 备份策略配置非并行备份存储单元组，复制策略不能配置备份存储单元组
                checkRepExternalStorageShouldEmpty(policyDtos);
            }
        }
    }

    /**
     * 检查复制存储单元组为并行数据存储
     *
     * @param policyDtos 复制策略
     */
    private void checkRepStorageParallel(List<PolicyDto> policyDtos) {
        policyDtos.forEach(policy -> {
            JsonNode externalStorageId = policy.getExtParameters().get(EXTERNAL_STORAGE_ID);
            if (externalStorageId == null || StringUtils.isEmpty(externalStorageId.textValue())) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "Replication policy backup storage unit group should not be empty.");
            }
            String targetClusterId = policy.getExtParameters().get(EXTERNAL_SYSTEM_ID).textValue();
            TargetCluster cluster = clusterRepository.getTargetClusterById(Integer.parseInt(targetClusterId));
            NasDistributionStorageDetail distribution = arrayTargetClusterService.getTargetDistribution(cluster,
                    externalStorageId.textValue())
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "Replication backup storage unit group not exists."));
            if (!distribution.isHasEnableParallelStorage()) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "Replication backup storage unit group is non-parallel.");
            }
        });
    }

    /**
     * 检查复制策略备份存储单元组
     *
     * @param policyDtos 复制策略
     */
    private void checkRepExternalStorageShouldEmpty(List<PolicyDto> policyDtos) {
        policyDtos.forEach((policyDto) -> {
            JsonNode externalStorageId = policyDto.getExtParameters().get(EXTERNAL_STORAGE_ID);
            if (externalStorageId != null && StringUtils.isNotEmpty(externalStorageId.textValue())) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "Replication policy backup storage unit group should be empty.");
            }
        });
    }
}

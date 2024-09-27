package openbackup.mongodb.protection.access.provider.sla;

import static com.huawei.oceanprotect.sla.sdk.enums.PolicyAction.ARCHIVING;
import static com.huawei.oceanprotect.sla.sdk.enums.PolicyAction.CUMULATIVE_INCREMENT;
import static com.huawei.oceanprotect.sla.sdk.enums.PolicyAction.DIFFERENCE_INCREMENT;
import static com.huawei.oceanprotect.sla.sdk.enums.PolicyAction.FULL;
import static com.huawei.oceanprotect.sla.sdk.enums.PolicyAction.LOG;
import static com.huawei.oceanprotect.sla.sdk.enums.PolicyAction.PERMANENT_INCREMENT;
import static com.huawei.oceanprotect.sla.sdk.enums.PolicyAction.REPLICATION;
import static openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum.MONGODB;
import static openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum.MONGODB_SINGLE;

import openbackup.data.protection.access.provider.sdk.sla.SlaValidateProvider;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;

import com.google.common.collect.ImmutableSet;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * MongoDB的sla provider
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
@Component
@Slf4j
public class MongoDBSlaProvider implements SlaValidateProvider {
    private static final Set<String> DEFAULT_BACKUP_NEED_CHECK_ACTION_SET = ImmutableSet.of(LOG.getAction(),
        CUMULATIVE_INCREMENT.getAction(), DIFFERENCE_INCREMENT.getAction(), PERMANENT_INCREMENT.getAction());

    private static final String COMMON_LOG_BACKUP_LABEL = "common_log_backup_label";

    @Autowired
    private ProtectObjectRestApi protectObjectRestApi;

    @Override
    public SlaValidateConfig getConfig() {
        SlaValidateConfig sla = new SlaValidateConfig();
        sla.getSpecificationConfig()
            .setLimit(PolicyLimitConfig.of(FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT))
            .setLimit(PolicyLimitConfig.of(ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT))
            .setLimit(PolicyLimitConfig.of(REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
        return sla;
    }

    @Override
    public void validateSLA(SlaBase slaBase) {
        if (!(slaBase instanceof UpdateSlaCommand)) {
            log.info("Current operation of this sla: {} is not modification, no need to check.",
                slaBase.getPolicyList().get(0).getUuid());
            return;
        }
        UpdateSlaCommand updateSlaCommand = (UpdateSlaCommand) slaBase;
        List<String> policyActions = updateSlaCommand.getPolicyList()
            .stream()
            .filter(policyDto -> PolicyType.BACKUP.equals(policyDto.getType()))
            .map(policyDto -> policyDto.getAction().getAction())
            .collect(Collectors.toList());
        boolean isContainsNeedCheckAction = policyActions.stream()
            .anyMatch(DEFAULT_BACKUP_NEED_CHECK_ACTION_SET::contains);
        if (!isContainsNeedCheckAction) {
            log.info("Do not need to check the action of the sla policy.");
            return;
        }
        List<ProtectedObjectInfo> items;
        int page = 0;
        do {
            BasePage<ProtectedObjectInfo> data = protectObjectRestApi.pageQueryProtectObject(updateSlaCommand.getUuid(),
                page, IsmNumberConstant.HUNDRED);
            items = data.getItems();
            if (VerifyUtil.isEmpty(items)) {
                log.info("No any resource is bound to this sla: {}, no need to check.", updateSlaCommand.getUuid());
                return;
            }
            page++;
        } while (items.size() >= IsmNumberConstant.HUNDRED);
    }

    private void checkSupportContainsSla(List<String> policyActions, ProtectedObjectInfo protectedObjectInfo) {
        String subType = protectedObjectInfo.getSubType();
        if (MONGODB_SINGLE.equalsSubType(subType) && policyActions.contains(LOG.getAction())) {
            throw new LegoCheckedException(CommonErrorCode.SLA_NOT_SUPPORT_BACKUP_POLICY,
                new String[] {subType, COMMON_LOG_BACKUP_LABEL},
                "Resource: " + subType + " bound to this sla do not support type: LOG");
        }
    }

    /**
     * MongoDB的sla适配器
     *
     * @param resourceSubType 资源子类型
     * @return 检查结果
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return MONGODB.equalsSubType(resourceSubType);
    }
}

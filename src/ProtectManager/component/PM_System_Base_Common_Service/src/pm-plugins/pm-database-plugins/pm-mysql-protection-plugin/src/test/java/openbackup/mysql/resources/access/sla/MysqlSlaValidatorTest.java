/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.mysql.resources.access.sla;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.mysql.resources.access.common.MysqlConstants;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述：
 * mysql的sla校验器
 *
 * @author wWX1146064
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-17
 */
public class MysqlSlaValidatorTest {
    private final MysqlSlaValidator mysqlValidator = new MysqlSlaValidator();

    private final ProtectObjectRestApi protectObjectRestApi = Mockito.mock(ProtectObjectRestApi.class);

    private final ProtectedEnvironmentService protectedEnvironmentService = Mockito.mock(ProtectedEnvironmentService.class);

    @Rule
    public ExpectedException exception = ExpectedException.none();

    @Before
    public void init() {
        mysqlValidator.setProtectedEnvironmentService(protectedEnvironmentService);
        mysqlValidator.setProtectObjectRestApi(protectObjectRestApi);
    }

    /**
     * 用例场景：策略模式策略识别-MySQL
     * 前置条件：类型参数为MySQL
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(mysqlValidator.applicable(ResourceSubTypeEnum.MYSQL.getType()));
        Assert.assertFalse(mysqlValidator.applicable(ResourceSubTypeEnum.OPENGAUSS.getType()));
    }

    /**
     * 用例场景：MySQL类型sla创建配置备份类型
     * 前置条件：MySQL类型识别成功，服务正常
     * 检查点: 添加全量备份、差异增量备份、累积增量备份、日志备份、复制、归档成功
     */
    @Test
    public void add_sla_config_limit_success() {
        SlaValidateConfig slaValidateConfig = mysqlValidator.getConfig();
        SlaValidateConfig.SpecificationConfig config = slaValidateConfig.getSpecificationConfig();
        List<PolicyLimitConfig> policiesConfig = config.getPoliciesConfig();

        List<PolicyLimitConfig> policyLimitConfigs = Arrays.asList(
                PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
                PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                        SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
                PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                        SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
                PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
                PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT),
                PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT));
        Assert.assertEquals(6, policiesConfig.size());
        Assert.assertEquals(JsonUtil.json(policiesConfig), JsonUtil.json(policyLimitConfigs));
    }

    @Test
    public void validate_failed() {
        UpdateSlaCommand sla = new UpdateSlaCommand();
        PolicyDto policy = new PolicyDto();
        policy.setAction(PolicyAction.LOG);
        sla.setPolicyList(Lists.newArrayList(policy));
        sla.setUuid("uuid");
        BasePage<ProtectedObjectInfo> data = new BasePage<>();
        ProtectedObjectInfo poi = new ProtectedObjectInfo();
        poi.setEnvId("envId");
        poi.setSubType(ResourceSubTypeEnum.MYSQL_CLUSTER_INSTANCE.getType());
        data.setItems(Lists.newArrayList(poi));
        PowerMockito.when(protectObjectRestApi.pageQueryProtectObject("uuid",0,100)).thenReturn(data);
        ProtectedEnvironment env = new ProtectedEnvironment();
        env.setExtendInfo(ImmutableMap.of(DatabaseConstants.CLUSTER_TYPE, MysqlConstants.EAPP));
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById("envId")).thenReturn(env);
        exception.expect(LegoCheckedException.class);
        mysqlValidator.validateSLA(sla);
    }
}
package openbackup.tdsql.resources.access.sla;

import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tdsql.resources.access.sla.TdsqlSlaValidatorProvider;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-07-03
 */
public class TdsqlSlaValidatorProviderTest {
    private final ProtectObjectRestApi protectObjectRestApi = Mockito.mock(ProtectObjectRestApi.class);

    private final TdsqlSlaValidatorProvider tdsqlSlaValidatorProvider = new TdsqlSlaValidatorProvider(
        protectObjectRestApi);

    /**
     * 用例场景：策略模式策略识别-TDSQL
     * 前置条件：类型参数为TDSQL-clusterInstance
     * 检查点：识别成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(tdsqlSlaValidatorProvider.applicable(ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType()));
        Assert.assertFalse(tdsqlSlaValidatorProvider.applicable(ResourceSubTypeEnum.TDSQL_CLUSTER.getType()));
    }

    /**
     * 用例场景：TDSQL类型sla创建配置备份类型
     * 前置条件：TDSQL类型识别成功，服务正常
     * 检查点：添加全量备份、差异增量备份、累积增量备份、日志备份、复制、归档成功
     */
    @Test
    public void add_sla_config_limit_success() {
        SlaValidateConfig slaValidateConfig = tdsqlSlaValidatorProvider.getConfig();
        SlaValidateConfig.SpecificationConfig config = slaValidateConfig.getSpecificationConfig();
        List<PolicyLimitConfig> policyLimitConfigList = config.getPoliciesConfig();
        Assert.assertEquals(6, policyLimitConfigList.size());
        List<PolicyAction> actions = policyLimitConfigList.stream()
            .map(PolicyLimitConfig::getAction)
            .collect(Collectors.toList());
        Assert.assertTrue(actions.containsAll(
            Arrays.asList(PolicyAction.FULL, PolicyAction.DIFFERENCE_INCREMENT, PolicyAction.CUMULATIVE_INCREMENT,
                PolicyAction.LOG, PolicyAction.ARCHIVING, PolicyAction.REPLICATION)));
    }

    /**
     * 用例名称：sla所绑定的资源部支持sla修改的备份策略<br/>
     * 前置条件：<br/>
     * check点：不合法的参数成功校验<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_resource_not_support_sla() {
        PowerMockito.when(
            protectObjectRestApi.pageQueryProtectObject(Mockito.anyString(), Mockito.anyInt(), Mockito.anyInt()))
            .thenReturn(prepareBasePages());
        Assert.assertThrows(LegoCheckedException.class,
            () -> tdsqlSlaValidatorProvider.validateSLA(prepareUpdateSlaCommand()));
    }

    private BasePage<ProtectedObjectInfo> prepareBasePages() {
        ProtectedObjectInfo protectedObjectInfo1 = new ProtectedObjectInfo();
        protectedObjectInfo1.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType());
        protectedObjectInfo1.setResourceId("123");
        ProtectedObjectInfo protectedObjectInfo2 = new ProtectedObjectInfo();
        protectedObjectInfo2.setResourceId("456");
        List<ProtectedObjectInfo> items = new ArrayList<>();
        items.add(protectedObjectInfo1);
        items.add(protectedObjectInfo2);
        BasePage<ProtectedObjectInfo> basePages = new BasePage<>();
        basePages.setItems(items);
        return basePages;
    }

    private List<PolicyDto> preparePolicyList() {
        PolicyDto policyDto1 = new PolicyDto();
        policyDto1.setType(PolicyType.BACKUP);
        policyDto1.setAction(PolicyAction.FULL);
        policyDto1.setSlaId("789");
        PolicyDto policyDto2 = new PolicyDto();
        policyDto2.setType(PolicyType.BACKUP);
        policyDto2.setAction(PolicyAction.DIFFERENCE_INCREMENT);
        policyDto2.setSlaId("789");
        List<PolicyDto> policyDtoList = new ArrayList<>();
        policyDtoList.add(policyDto1);
        policyDtoList.add(policyDto2);
        return policyDtoList;
    }

    private UpdateSlaCommand prepareUpdateSlaCommand() {
        UpdateSlaCommand updateSlaCommand = new UpdateSlaCommand();
        updateSlaCommand.setPolicyList(preparePolicyList());
        updateSlaCommand.setUuid("123456");
        return updateSlaCommand;
    }
}

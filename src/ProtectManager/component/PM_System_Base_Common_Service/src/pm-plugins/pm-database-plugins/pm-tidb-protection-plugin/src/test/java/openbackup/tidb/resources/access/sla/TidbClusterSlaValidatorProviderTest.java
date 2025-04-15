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
package openbackup.tidb.resources.access.sla;

import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.sla.TidbClusterSlaValidatorProvider;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * TidbClusterSlaValidatorProvider测试
 *
 */
public class TidbClusterSlaValidatorProviderTest {
    TidbClusterSlaValidatorProvider tidbClusterSlaValidatorProvider;

    private final ProtectObjectRestApi protectObjectRestApi = Mockito.mock(ProtectObjectRestApi.class);

    @Before
    public void before() {
        tidbClusterSlaValidatorProvider = new TidbClusterSlaValidatorProvider(protectObjectRestApi);
    }

    /**
     * 用例场景：通用数据库sla校验
     * 前置条件：无
     * 检查点：有全量策略
     */
    @Test
    public void success_check_config() {
        Assert.assertTrue(tidbClusterSlaValidatorProvider.applicable(ResourceSubTypeEnum.TIDB.getType()));
        SlaValidateConfig.SpecificationConfig specificationConfig = tidbClusterSlaValidatorProvider.getConfig()
            .getSpecificationConfig();
        boolean hasFull = specificationConfig.getPoliciesConfig()
            .stream()
            .anyMatch(e -> Objects.equals(e.getAction(), PolicyAction.FULL));
        Assert.assertTrue(hasFull);
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
            () -> tidbClusterSlaValidatorProvider.validateSLA(prepareUpdateSlaCommand()));
    }

    private BasePage<ProtectedObjectInfo> prepareBasePages() {
        ProtectedObjectInfo protectedObjectInfo1 = new ProtectedObjectInfo();
        protectedObjectInfo1.setSubType(ResourceSubTypeEnum.TIDB_TABLE.getType());
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
        policyDto2.setAction(PolicyAction.LOG);
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
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
package openbackup.database.base.plugin.provider.sla;

import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.model.ProtectedResourceExtendInfo;
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

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

/**
 * GeneralDbSlaValidateProvider测试
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-30
 */
public class GeneralDbSlaValidateProviderTest {
    GeneralDbSlaValidateProvider generalDbSlaValidateProvider;

    private final ProtectObjectRestApi protectObjectRestApi = Mockito.mock(ProtectObjectRestApi.class);

    private final ResourceExtendInfoService resourceExtendInfoService = Mockito.mock(ResourceExtendInfoService.class);

    @Before
    public void before() {
        generalDbSlaValidateProvider = new GeneralDbSlaValidateProvider(protectObjectRestApi,
                resourceExtendInfoService);
    }

    /**
     * 用例场景：通用数据库sla校验
     * 前置条件：无
     * 检查点：有全量策略
     */
    @Test
    public void success_check_config() {
        Assert.assertTrue(generalDbSlaValidateProvider.applicable(ResourceSubTypeEnum.GENERAL_DB.getType()));
        SlaValidateConfig.SpecificationConfig specificationConfig = generalDbSlaValidateProvider.getConfig()
                .getSpecificationConfig();
        boolean hasFull = specificationConfig.getPoliciesConfig().stream()
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
        PowerMockito.when(resourceExtendInfoService.queryExtendInfo(Mockito.anyList(), Mockito.anyString()))
                .thenReturn(prepareExtendInfos());
        Assert.assertThrows(LegoCheckedException.class,
                () -> generalDbSlaValidateProvider.validateSLA(prepareUpdateSlaCommand()));
    }

    private BasePage<ProtectedObjectInfo> prepareBasePages() {
        ProtectedObjectInfo protectedObjectInfo1 = new ProtectedObjectInfo();
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

    private List<ProtectedResourceExtendInfo> prepareExtendInfos() {
        String scriptConf1 = "{\"backup\":{\"support\":[{\"backupType\":\"full\"}, " +
                "{\"backupType\":\"log\"}, " +
                "{\"backupType\":\"cumulative_increment\"}, " +
                "{\"backupType\":\"difference_increment\"}]}, " +
                "\"databaseType\":\"sap hana\"}";
        String scriptConf2 = "{\"backup\":{\"support\":[{\"backupType\":\"full\"}, " +
                "{\"backupType\":\"cumulative_increment\"}]}, " +
                "\"databaseType\":\"gbase 8a\" }";
        ProtectedResourceExtendInfo protectedResourceExtendInfo1 = new ProtectedResourceExtendInfo();
        ProtectedResourceExtendInfo protectedResourceExtendInfo2 = new ProtectedResourceExtendInfo();
        protectedResourceExtendInfo1.setResourceId("123");
        protectedResourceExtendInfo1.setValue(scriptConf1);
        protectedResourceExtendInfo2.setResourceId("456");
        protectedResourceExtendInfo2.setValue(scriptConf2);
        List<ProtectedResourceExtendInfo> extendInfos = new ArrayList<>();
        extendInfos.add(protectedResourceExtendInfo1);
        extendInfos.add(protectedResourceExtendInfo2);
        return extendInfos;
    }
}

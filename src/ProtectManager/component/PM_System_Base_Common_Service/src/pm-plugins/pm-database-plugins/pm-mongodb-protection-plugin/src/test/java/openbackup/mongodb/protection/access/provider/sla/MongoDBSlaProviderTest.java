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
package openbackup.mongodb.protection.access.provider.sla;

import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.mongodb.protection.access.provider.sla.MongoDBSlaProvider;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * MongoDB的sla provider 测试类
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
public class MongoDBSlaProviderTest {
    private final ProtectObjectRestApi protectObjectRestApi = Mockito.mock(ProtectObjectRestApi.class);

    private final MongoDBSlaProvider mongoDBSlaProvider = new MongoDBSlaProvider();

    @Before
    public void setup() {
        ReflectionTestUtils.setField(mongoDBSlaProvider, "protectObjectRestApi", protectObjectRestApi);
    }

    /**
     * 用例场景：MongoDB sla provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(mongoDBSlaProvider.applicable(ResourceSubTypeEnum.MONGODB.getType()));
    }

    @Test
    public void test_validate() {
        UpdateSlaCommand updateSlaCommand = prepareUpdateSlaCommand();
        Mockito.when(
                protectObjectRestApi.pageQueryProtectObject(Mockito.anyString(), Mockito.anyInt(), Mockito.anyInt()))
            .thenReturn(prepareBasePages());
        Assert.assertThrows(LegoCheckedException.class, () -> mongoDBSlaProvider.validateSLA(updateSlaCommand));
    }

    private BasePage<ProtectedObjectInfo> prepareBasePages() {
        ProtectedObjectInfo protectedObjectInfo1 = new ProtectedObjectInfo();
        protectedObjectInfo1.setSubType(ResourceSubTypeEnum.MONGODB_SINGLE.getType());
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

    /**
     * 用例场景：MongoDB配置文件的限制条件
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void get_config_success() {
        SlaValidateConfig config = mongoDBSlaProvider.getConfig();
        List<PolicyLimitConfig> policiesConfig = config.getSpecificationConfig().getPoliciesConfig();
        for (PolicyLimitConfig policyLimitConfig : policiesConfig) {
            if (PolicyAction.FULL == policyLimitConfig.getAction()) {
                Assert.assertEquals(SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT, policyLimitConfig.getLimit());
            }
            if (PolicyAction.LOG == policyLimitConfig.getAction()) {
                Assert.assertEquals(SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT, policyLimitConfig.getLimit());
            }
            if (PolicyAction.ARCHIVING == policyLimitConfig.getAction()) {
                Assert.assertEquals(SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT, policyLimitConfig.getLimit());
            }
            if (PolicyAction.REPLICATION == policyLimitConfig.getAction()) {
                Assert.assertEquals(SlaConstants.REPLICATION_POLICY_COUNT_LIMIT, policyLimitConfig.getLimit());
            }
        }
        Assert.assertNotNull(config);
    }
}

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
package openbackup.database.base.plugin.service.impl;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;

import openbackup.database.base.plugin.service.impl.SlaValidService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.Collections;

/**
 * SlaValidService测试类
 *
 */
public class SlaValidServiceTest {
    /**
     * 用例名称：修改sla时，该sla多个策略需要满足最小集合关系
     * 前置条件：无
     * check点：修改sla时，该sla多个策略需要满足最小集合关系，多余会报错
     */
    @Test
    public void should_throw_LegoCheckedException_when_modify_sla_check_resource_public_policy() {
        ProtectObjectRestApi protectObjectRestApi = PowerMockito.mock(ProtectObjectRestApi.class);
        BasePage<ProtectedObjectInfo> data = new BasePage<>();
        data.setTotal(100);
        ProtectedObjectInfo protectedObjectInfo = new ProtectedObjectInfo();
        protectedObjectInfo.setSubType(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType());
        data.setItems(Collections.singletonList(protectedObjectInfo));

        PowerMockito.when(protectObjectRestApi.pageQueryProtectObject(any(), anyInt(), anyInt())).thenReturn(data);

        SlaValidService slaValidService = new SlaValidService(protectObjectRestApi);
        UpdateSlaCommand updateSlaCommand = new UpdateSlaCommand();
        // Given
        PolicyDto first = new PolicyDto();
        first.setType(PolicyType.BACKUP);
        first.setAction(PolicyAction.FULL);

        PolicyDto second = new PolicyDto();
        second.setType(PolicyType.BACKUP);
        second.setAction(PolicyAction.DIFFERENCE_INCREMENT);
        updateSlaCommand.setPolicyList(Arrays.asList(first, second, first, first, first));

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> slaValidService.modifySlaCheckResourcePublicPolicy(updateSlaCommand,
                ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType(), PolicyAction.DIFFERENCE_INCREMENT.getAction()));
        Assert.assertEquals(CommonErrorCode.SLA_NOT_SUPPORT_BACKUP_POLICY, legoCheckedException.getErrorCode());
    }
}

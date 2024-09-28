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
package openbackup.oceanbase.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.verify;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.oceanbase.OceanBaseTest;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Collections;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
public class OceanBaseTenantProviderTest extends OceanBaseTest {
    private OceanBaseTenantProvider oceanBaseTenantProvider;

    @Override
    @Before
    public void init() {
        super.init();
        oceanBaseTenantProvider = new OceanBaseTenantProvider(oceanBaseService);
        mockGetEvnById();
    }

    /**
     * 用例场景：注册租户集成功
     * 前置条件：传入注册信息正确
     * 检查点：过程无异常
     */
    @Test
    public void register_tenant_success() {
        ProtectedResource protectedResource = mockTenantResource();
        oceanBaseTenantProvider.beforeCreate(protectedResource);

        verify(oceanBaseService).getExistingOceanBaseTenant(protectedResource.getParentUuid(),
            protectedResource.getUuid());
        verify(oceanBaseService).checkTenantSetConnect(protectedResource);
    }

    /**
     * 用例场景：修改租户集成功
     * 前置条件：传入注册信息正确
     * 检查点：过程无异常
     */
    @Test
    public void update_tenant_success() {
        ProtectedResource protectedResource = mockTenantResource();
        ProtectedResource dbRecord = new ProtectedResource();
        dbRecord.setParentUuid(protectedResource.getParentUuid());
        PowerMockito.when(oceanBaseService.getResourceById(protectedResource.getUuid()))
            .thenReturn(Optional.of(dbRecord));
        oceanBaseTenantProvider.beforeUpdate(protectedResource);

        verify(oceanBaseService).getExistingOceanBaseTenant(protectedResource.getParentUuid(),
            protectedResource.getUuid());
        verify(oceanBaseService).checkTenantSetConnect(protectedResource);
    }

    /**
     * 用例场景：租户集的管理集群不能修改
     * 前置条件：传入注册信息正确
     * 检查点：过程无异常
     */
    @Test
    public void update_tenant_cannot_modify_parent_uuid_error() {
        ProtectedResource protectedResource = mockTenantResource();
        ProtectedResource dbRecord = new ProtectedResource();
        dbRecord.setParentUuid("11111");
        PowerMockito.when(oceanBaseService.getResourceById(protectedResource.getUuid()))
            .thenReturn(Optional.of(dbRecord));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseTenantProvider.beforeUpdate(protectedResource));

        Assert.assertEquals("Can not modify the parent cluster", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：注册租户集时， 某个租户已经注册在其他租户集中了
     * 前置条件：OB运行正常
     * 检查点：注册失败
     */
    @Test
    public void register_tenant_when_tenant_is_registered_error() {
        ProtectedResource protectedResource = mockTenantResource();

        PowerMockito.when(oceanBaseService.getExistingOceanBaseTenant(any(), any()))
            .thenReturn(Collections.singletonList("tenant3"));

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> oceanBaseTenantProvider.beforeCreate(protectedResource));
        Assert.assertEquals("The tenant name: tenant3 already register.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }
}

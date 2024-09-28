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
package openbackup.openstack.protection.access.common;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.provider.MockFactory;
import com.huawei.oceanprotect.system.base.quota.enums.UpdateQuotaType;
import com.huawei.oceanprotect.system.base.quota.service.UserQuotaService;
import openbackup.system.base.sdk.copy.model.CopyInfo;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;

/**
 * 功能描述: OpenstackQuotaServiceTest
 *
 */
public class OpenstackQuotaServiceTest {
    private static OpenstackQuotaService openstackQuotaService;

    private static final UserQuotaService userQuotaService = Mockito.mock(UserQuotaService.class);

    @BeforeClass
    public static void init() {
        openstackQuotaService = new OpenstackQuotaService(userQuotaService);
    }

    /**
     * 用例场景：测试是否注册到OpenStack <br/>
     * 前置条件：开启注册到OpenStack <br/>
     * 检查点：返回结果为true
     */
    @Test
    public void test_is_register_openstack_success() {
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.getExtendInfo().put(OpenstackConstant.REGISTER_SERVICE, OpenstackConstant.REGISTER_OPENSTACK);
        boolean registerOpenstack = openstackQuotaService.isRegisterOpenstack(environment);
        Assert.assertTrue(registerOpenstack);
    }

    /**
     * 用例场景：测试检查配额成功 <br/>
     * 前置条件：参数正常 <br/>
     * 检查点：无异常发生
     */
    @Test
    public void test_check_backup_quota_success() {
        Mockito.doNothing().when(userQuotaService).checkUserQuotaInSrc(any(), any(), any(), anyBoolean());
        openstackQuotaService.checkBackupQuota("user_id","projectId");
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：测试更新配额成功 <br/>
     * 前置条件：参数正常 <br/>
     * 检查点：无异常抛出
     */
    @Test
    public void test_update_used_quota_success() {
        Mockito.doNothing().when(userQuotaService)
            .updateUserUsedQuota(any(),any(), any(), any(), anyString());
        CopyInfo copy = new CopyInfo();
        copy.setProperties("{\"size\": \"1\"}");
        openstackQuotaService.updateUsedQuota("projectId", copy, UpdateQuotaType.INCREASE);
        Assert.assertTrue(true);
    }
}

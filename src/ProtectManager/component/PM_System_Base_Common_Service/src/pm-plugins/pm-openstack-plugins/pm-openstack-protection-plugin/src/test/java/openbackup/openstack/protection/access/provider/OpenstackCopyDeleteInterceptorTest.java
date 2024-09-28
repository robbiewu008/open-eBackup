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
package openbackup.openstack.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.common.OpenstackQuotaService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Optional;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

/**
 * 功能描述: OpenstackCopyDeleteInterceptorTest
 *
 */
public class OpenstackCopyDeleteInterceptorTest {
    private static OpenstackCopyDeleteInterceptor openstackCopyDeleteInterceptor;
    private static final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private static final OpenstackQuotaService openstackQuotaService = Mockito.mock(OpenstackQuotaService.class);

    @BeforeClass
    public static void init() {
        openstackCopyDeleteInterceptor = new OpenstackCopyDeleteInterceptor(openstackQuotaService, resourceService);
    }

    /**
     * 测试场景：applicable匹配成功 <br/>
     * 前置条件：传入资源信息subtype类型为OpenStackCloudServer <br/>
     * 检查点：返回True
     */
    @Test
    public void test_applicable_success() {
        boolean applicable = openstackCopyDeleteInterceptor.applicable(
            ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());
        Assert.assertTrue(applicable);
    }

    /**
     * 测试场景：校验副本删除后置处理成功 <br/>
     * 前置条件：云核场景 <br/>
     * 检查点：无异常
     */
    @Test
    public void test_post_process_success() {
        Copy copy = new Copy();
        copy.setResourceId("res_id");
        TaskCompleteMessageBo taskMessage = new TaskCompleteMessageBo();
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.getExtendInfo().put(OpenstackConstant.REGISTER_SERVICE, OpenstackConstant.REGISTER_OPENSTACK);
        Mockito.when(resourceService.getResourceById(anyString()))
            .thenReturn(Optional.of(MockFactory.mockProtectedResource()))
            .thenReturn(Optional.of(environment));
        Mockito.when(openstackQuotaService.isRegisterOpenstack(any())).thenReturn(true);
        Mockito.doNothing().when(openstackQuotaService).updateUsedQuota(any(), any(), any());
        openstackCopyDeleteInterceptor.finalize(copy, taskMessage);
        Assert.assertTrue(true);
    }
}

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
package openbackup.database.base.plugin.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.database.base.plugin.util.TestConfHelper;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * GeneralDbRestoreInterceptorProvider测试类
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-30
 */
@SpringBootTest(classes = {GeneralDbRestoreInterceptorProvider.class, GeneralDbProtectAgentService.class})
@RunWith(SpringRunner.class)
public class GeneralDbRestoreInterceptorProviderTest {
    @Autowired
    private GeneralDbRestoreInterceptorProvider generalDbRestoreInterceptorProvider;
    @Autowired
    private GeneralDbProtectAgentService generalDbProtectAgentService;

    @MockBean
    protected ProviderManager providerManager;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private AgentUnifiedService agentUnifiedService;

    @MockBean
    @Qualifier("unifiedConnectionCheckProvider")
    private ResourceConnectionCheckProvider resourceConnectionCheckProvider;

    @MockBean
    @Qualifier("unifiedResourceConnectionChecker")
    private ProtectedResourceChecker protectedResourceChecker;

    @MockBean
    private CopyRestApi copyRestApi;

    @Before
    public void init() {
        ProtectedEnvironment singleDb = TestConfHelper.mockDatabase(true);
        List<ProtectedEnvironment> hosts = TestConfHelper.mockHost();
        GeneralDbUtil.setProtectResourceFullHostInfo(singleDb,hosts);

        Mockito.when(resourceService.getResourceById(Mockito.any()))
            .thenReturn(Optional.of(singleDb));
        HostDto hostDto = new HostDto();
        hostDto.setUuid("host1");
        hostDto.setPort(1);
        hostDto.setEndpoint("1.1.1.1");
        Mockito.when(agentUnifiedService.getHost(Mockito.any(), Mockito.any())).thenReturn(hostDto);
    }

    /**
     * 用例场景：通用数据库设置恢复参数
     * 前置条件：无
     * 检查点：恢复参数设置成功
     */
    @Test
    public void restore_interceptor_success(){
        RestoreTask task = new RestoreTask();
        task.setRestoreType("normalRestore");
        task.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        task.setTargetEnv(new TaskEnvironment());
        task.getTargetEnv().setExtendInfo(new HashMap<>());
        task.getTargetEnv().getExtendInfo().put(GeneralDbConstant.EXTEND_SCRIPT_CONF, TestConfHelper.getHanaConf());

        Copy copy = new Copy();
        copy.setBackupType(1);
        ProtectedResource copyResource = new ProtectedResource();
        copyResource.setVersion("1.0");
        copy.setResourceProperties(JsonUtil.json(copyResource));

        Mockito.when(copyRestApi.queryCopyByID(Mockito.any())).thenReturn(copy);

        generalDbRestoreInterceptorProvider.initialize(task);

        Assert.assertEquals(task.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS), "1");
        Assert.assertEquals(task.getAdvanceParams().get(DatabaseConstants.MULTI_POST_JOB), "true");
    }
}

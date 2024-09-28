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
package openbackup.data.access.framework.copy.mng.provider;

import java.util.UUID;

import openbackup.data.access.framework.copy.mng.provider.UnifiedCopyProvider;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;

import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.common.constants.ContextConstants;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;

import static org.mockito.ArgumentMatchers.any;

/**
 * UnifiedCopyProvider LLT
 *
 */
public class UnifiedCopyProviderTest {
    private static final String COPY_PROP = "{\"repositories\":[{\"id\":\"\",\"protocol\":1, \"type\": 1}]}";
    private static final String COPY_PROP_WITH_S3_REPO = "{\"repositories\":[{\"id\":\"1111\",\"protocol\":2, \"type\": 1}]}";
    private static final String COPY_PROP_WITH_ERROR_REPO = "{\"repositories\":[{\"id\":\"1111\",\"protocol\":9, \"type\": 1}]}";
    private String requestId = UUID.randomUUID().toString();
    private final SanClientService sanClientService = Mockito.mock(SanClientService.class);
    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    /**
     * 基本冒烟测试
     */
    @Test
    public void smoke_test() {
        UnifiedCopyProvider provider = mockCopyProvider();
        provider.setSanClientService(sanClientService);
        CopyInfoBo copyInfo = mockCopyInfo();
        provider.deleteCopy(requestId, copyInfo);
        Assert.assertTrue(!provider.applicable(new CopyBo()));
    }

    /**
     * 测试场景：存储库不是本地的，必须获取存储库相关信息
     * 前置条件：副本中保存了存储库相关信息
     * 检查点：无异常抛出
     */
    @Test
    public void should_send_repository_to_dme_if_backup_rep_is_not_local() {
        UnifiedCopyProvider provider = mockCopyProvider();
        provider.setSanClientService(sanClientService);
        CopyInfoBo copyInfo = mockCopyInfo();
        copyInfo.setProperties(COPY_PROP_WITH_S3_REPO);
        provider.deleteCopy(requestId, copyInfo);
        Assert.assertNotNull(provider);
    }

    /**
     * 测试场景：下发删除命令给DME失败
     * 前置条件：DME执行删除副本失败
     * 检查点：抛出指定异常
     */
    @Test
    public void should_throws_LegoUncheckedException_if_delete_copy_failed() {
        expectedException.expect(LegoUncheckedException.class);

        DmeUnifiedRestApi dmeUnifiedRestApi = PowerMockito.mock(DmeUnifiedRestApi.class);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);
        ResourceService resourceService = PowerMockito.mock(ResourceService.class);
        RepositoryStrategyManager strategyManager = PowerMockito.mock(RepositoryStrategyManager.class);
        PowerMockito.when(strategyManager.getStrategy(any())).thenReturn(PowerMockito.mock(RepositoryStrategy.class));
        PowerMockito.doThrow(new LegoUncheckedException("")).when(dmeUnifiedRestApi).deleteCopy(any(), any());
        UnifiedCopyProvider provider = new UnifiedCopyProvider(dmeUnifiedRestApi, strategyManager,
            mockRedissonClient(),copyManagerService, resourceService);
        provider.setSanClientService(sanClientService);
        ProviderManager providerManager = Mockito.mock(ProviderManager.class);
        Mockito.when(providerManager.findProvider(Mockito.any(), Mockito.any(), Mockito.any())).thenReturn(null);
        provider.setProviderManager(providerManager);
        TaskResource taskResource = new TaskResource();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Mockito.when(copyManagerService.buildTaskResource(Mockito.any())).thenReturn(taskResource);
        Mockito.when(copyManagerService.buildTaskEnvironment(Mockito.any())).thenReturn(taskEnvironment);
        CopyInfoBo copyInfo = mockCopyInfo();
        copyInfo.setProperties(COPY_PROP_WITH_S3_REPO);
        provider.deleteCopy(requestId, copyInfo);
    }

    /**
     * 测试场景：存储库协议或类型错误
     * 前置条件：副本中保存的存储协议或类型错误
     * 检查点：抛出指定异常
     */
    @Test
    public void should_throws_LegoCheckedException_if_rep_protocol_or_type_error() {
        expectedException.expect(LegoCheckedException.class);

        UnifiedCopyProvider provider = mockCopyProvider();
        CopyInfoBo copyInfo = mockCopyInfo();
        copyInfo.setProperties(COPY_PROP_WITH_ERROR_REPO);
        provider.deleteCopy(requestId, copyInfo);
    }

    private CopyInfoBo mockCopyInfo() {
        CopyInfoBo copyInfo = new CopyInfoBo();
        copyInfo.setUuid(UUID.randomUUID().toString());
        copyInfo.setProperties(COPY_PROP);
        return copyInfo;
    }

    private UnifiedCopyProvider mockCopyProvider() {
        DmeUnifiedRestApi dmeUnifiedRestApi = PowerMockito.mock(DmeUnifiedRestApi.class);
        RepositoryStrategyManager strategyManager = PowerMockito.mock(RepositoryStrategyManager.class);
        ResourceService resourceService = PowerMockito.mock(ResourceService.class);
        PowerMockito.when(strategyManager.getStrategy(any())).thenReturn(PowerMockito.mock(RepositoryStrategy.class));
        ProviderManager providerManager = Mockito.mock(ProviderManager.class);
        Mockito.when(providerManager.findProvider(Mockito.any(), Mockito.any(), Mockito.any())).thenReturn(null);
        CopyManagerService copyManagerService = PowerMockito.mock(CopyManagerService.class);
        UnifiedCopyProvider copyProvider = new UnifiedCopyProvider(dmeUnifiedRestApi, strategyManager,
            mockRedissonClient(), copyManagerService, resourceService);
        copyProvider.setProviderManager(providerManager);
        TaskResource taskResource = new TaskResource();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Mockito.when(copyManagerService.buildTaskResource(Mockito.any())).thenReturn(taskResource);
        Mockito.when(copyManagerService.buildTaskEnvironment(Mockito.any())).thenReturn(taskEnvironment);

        JobService jobService = PowerMockito.mock(JobService.class);
        Mockito.doNothing().when(jobService).updateJob(any(), any());
        copyProvider.setJobService(jobService);
        return copyProvider;
    }

    private RedissonClient mockRedissonClient() {
        RedissonClient redissonClient = PowerMockito.mock(RedissonClient.class);
        RMap rMap = Mockito.mock(RMap.class);
        Mockito.when(rMap.get(ContextConstants.IS_FORCED)).thenReturn("True");
        Mockito.when(redissonClient.getMap(Mockito.any(), Mockito.any(StringCodec.class))).thenReturn(rMap);
        return redissonClient;
    }
}

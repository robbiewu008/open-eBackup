/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.opengauss.resources.access.provider;

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * {@link OpenGaussAgentProvider 测试类}
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-25
 */
public class OpenGaussAgentProviderTest {
    private final OpenGaussAgentService openGaussAgentService = PowerMockito.mock(OpenGaussAgentService.class);
    private final OpenGaussAgentProvider agentProvider = new OpenGaussAgentProvider(openGaussAgentService);

    /**
     * 用例场景：填充agent
     * 前置条件：资源类型为OpenGauss-instance
     * 检查点：填充成功
     */
    @Test
    public void select_agent_success() {
        AgentSelectParam agentSelectParam = getAgentSelectParam();
        PowerMockito.when(openGaussAgentService.getAgentEndpoint(anyString())).thenReturn(new ArrayList<>());
        Assert.assertNotNull(agentProvider.getSelectedAgents(agentSelectParam));
    }

    /**
     * 用例场景：过滤OpenGauss类型
     * 前置条件：资源类型为OpenGauss-instance
     * 检查点：类过滤检查返回成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(agentProvider.applicable(getAgentSelectParam()));
    }

    private AgentSelectParam getAgentSelectParam() {
        ProtectedResource resource = new ProtectedResource();
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("env-uuid");
        resource.setEnvironment(environment);
        resource.setSubType(ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType());
        return AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(new HashMap<>())
            .build();
    }
}

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.gaussdbdws.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * {@link GaussDBDWSAgentProvider 测试类}
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-25
 */
public class GaussDBDWSAgentProviderTest {
    private final GaussDBBaseService gaussDBBaseService = PowerMockito.mock(GaussDBBaseService.class);
    private final GaussDBDWSAgentProvider agentProvider = new GaussDBDWSAgentProvider(gaussDBBaseService);

    /**
     * 用例场景：填充agent
     * 前置条件：资源类型为DWS-table
     * 检查点：填充agent成功
     */
    @Test
    public void select_agent_success() {
        AgentSelectParam agentSelectParam = getAgentSelectParam();
        PowerMockito.when(gaussDBBaseService.supplyAgent(any())).thenReturn(new ArrayList<>());
        Assert.assertNotNull(agentProvider.getSelectedAgents(agentSelectParam));
    }

    /**
     * 用例场景：DWS类型过滤
     * 前置条件：资源类型为DWS-table
     * 检查点：类过滤检查返回成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(agentProvider.applicable(getAgentSelectParam()));
    }

    private AgentSelectParam getAgentSelectParam() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType());
        return AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(new HashMap<>())
            .build();
    }
}

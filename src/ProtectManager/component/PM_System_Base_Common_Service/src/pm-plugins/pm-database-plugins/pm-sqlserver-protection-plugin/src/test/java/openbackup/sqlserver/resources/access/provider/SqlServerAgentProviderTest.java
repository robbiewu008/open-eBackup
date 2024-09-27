/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.sqlserver.resources.access.provider;

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;

/**
 * {@link SqlServerAgentProvider 测试类}
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-25
 */
public class SqlServerAgentProviderTest {
    private final SqlServerBaseService sqlServerBaseService = PowerMockito.mock(SqlServerBaseService.class);
    private final SqlServerAgentProvider agentProvider = new SqlServerAgentProvider(sqlServerBaseService);

    /**
     * 用例场景：SQL Server agent填充
     * 前置条件：资源类型为SQLServer-database
     * 检查点：填充成功
     */
    @Test
    public void select_agent_success() {
        AgentSelectParam agentSelectParam = getAgentSelectParam();
        PowerMockito.when(sqlServerBaseService.convertNodeListToAgents(anyString())).thenReturn(new ArrayList<>());
        Assert.assertNotNull(agentProvider.getSelectedAgents(agentSelectParam));
    }

    /**
     * 用例场景：SQL Server实例检查类provider过滤
     * 前置条件：资源类型为SQL Server单实例
     * 检查点：类过滤检查返回成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(agentProvider.applicable(getAgentSelectParam()));
    }

    private AgentSelectParam getAgentSelectParam() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType());
        return AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(new HashMap<>())
            .build();
    }
}

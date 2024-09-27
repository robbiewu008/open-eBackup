package openbackup.exchange.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedResourceServiceImpl;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import openbackup.exchange.protection.access.service.ExchangeService;
import openbackup.exchange.protection.access.service.impl.ExchangeServiceImpl;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * ExchangeAgentProvider Test
 *
 * @author z00693144
 * @since 2024-03-08
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {ExchangeAgentProvider.class, ProtectedResourceServiceImpl.class, ExchangeServiceImpl.class})
public class ExchangeAgentProviderTest {
    private ResourceService resourceService = Mockito.mock(ProtectedResourceServiceImpl.class);

    private ExchangeService exchangeService = Mockito.mock(ExchangeServiceImpl.class);

    private final ExchangeAgentProvider provider = new ExchangeAgentProvider(resourceService, exchangeService);

    /**
     * 用例场景：测试资源能否执行
     * 前置条件：无
     * 检查点：检查通过
     */
    @Test
    public void testApplicable() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType());
        AgentSelectParam param = AgentSelectParam.builder().resource(resource).build();
        Assert.assertTrue(provider.applicable(param));
        param.getResource().setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        Assert.assertFalse(provider.applicable(param));
    }

    /**
     * 用例场景：测试选择agent
     * 前置条件：无
     * 检查点：选择成功
     */
    @Test
    public void testSelect() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType());
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("agentUuid", "xxxx");
        environment.setExtendInfo(extendInfo);
        resource.setEnvironment(environment);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("111");
        protectedResource.setEndpoint("192.168.0.1");
        protectedResource.setPort(333);
        AgentSelectParam param = AgentSelectParam.builder()
            .resource(resource).jobType(JobTypeEnum.BACKUP.getValue()).build();
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.any()))
            .thenReturn(Optional.of(protectedResource));
        List<Endpoint> agents = provider.getSelectedAgents(param);
        Assert.assertFalse(agents.isEmpty());
        Assert.assertEquals(333, agents.get(0).getPort());
        param.setJobType(JobTypeEnum.RESOURCE_SCAN.getValue());
        agents = provider.getSelectedAgents(param);
        Assert.assertFalse(agents.isEmpty());
        Assert.assertEquals(333, agents.get(0).getPort());
    }
}
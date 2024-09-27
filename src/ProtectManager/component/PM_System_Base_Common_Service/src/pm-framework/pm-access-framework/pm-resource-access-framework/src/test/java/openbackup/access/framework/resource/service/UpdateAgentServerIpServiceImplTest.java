package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.service.UpdateAgentServerIpServiceImpl;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.model.host.ManagementIp;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.support.membermodification.MemberModifier;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 更新AgentServerIp 测试类
 *
 * @author swx1010572
 * @version [OceanProtect X8000 2.1.0]
 * @since 2022-07-27
 */
public class UpdateAgentServerIpServiceImplTest {

    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private final UpdateAgentServerIpServiceImpl updateAgentServerIpService = new UpdateAgentServerIpServiceImpl(
        resourceService, agentUnifiedService);

    /**
     * 用例场景：测试更新server ip成功
     * 前置条件：无
     * 检查点：不抛出异常
     */
    @Test
    public void update_agent_server_ip_success() throws IllegalAccessException {
        MemberModifier.field(UpdateAgentServerIpServiceImpl.class, "agentUnifiedService")
            .set(updateAgentServerIpService, agentUnifiedService);
        Map<String, Object> filter = new HashMap<>();
        filter.put("type", ResourceTypeEnum.HOST.getType());
        filter.put("scenario", Arrays.asList(Arrays.asList("!="), "1"));
        filter.put("isCluster", false);
        filter.put("subType",
            Arrays.asList(ResourceSubTypeEnum.DB_BACKUP_AGENT.getType(), ResourceSubTypeEnum.VM_BACKUP_AGENT.getType(),
                ResourceSubTypeEnum.U_BACKUP_AGENT.getType()));
        Mockito.when(resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.ONE, filter))
            .thenReturn(getProtectedResourcePageListResponse());
        Mockito.when(resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.THOUSAND, filter))
            .thenReturn(getProtectedResourcePageListResponse());
        ManagementIp managementIp = new ManagementIp();
        managementIp.setManagerServerList(Arrays.asList("192.168.100.100", "192.168.100.102"));
        updateAgentServerIpService.updateAgentServer(managementIp);
        Assert.assertTrue(true);
    }

    private PageListResponse<ProtectedResource> getProtectedResourcePageListResponse() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(2);
        List<ProtectedResource> protectedResources = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        protectedEnvironment.setRootUuid("xxxxxx");
        protectedEnvironment.setExtendInfo(new HashMap<>());
        protectedEnvironment.setUuid("bbbb");
        protectedEnvironment.setName("/database1/schema1");
        protectedEnvironment.setEndpoint("10.244.102.105");
        protectedEnvironment.setPort(25080);
        protectedResources.add(protectedEnvironment);
        pageListResponse.setRecords(protectedResources);
        return pageListResponse;
    }
}

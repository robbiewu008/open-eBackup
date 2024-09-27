package openbackup.informix.protection.access.provider.checker;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.informix.protection.access.constant.InformixConstant;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Informix Connection Checker Test
 *
 * @since 2023-09-01
 */
public class InformixConnectionCheckerTest {
    ProtectedEnvironmentRetrievalsService retrievalsService = Mockito.mock(ProtectedEnvironmentRetrievalsService.class);
    AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
    InformixService informixService = Mockito.mock(InformixService.class);

    InformixConnectionChecker informixChecker =
            new InformixConnectionChecker(retrievalsService, agentUnifiedService, informixService);

    /**
     * 用例场景：检查subType是否可以进入informix的检查
     * 前置条件：无
     * 检查点：进入informix的检查
     */
    @Test
    public void applicable_test() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType());
        Assert.assertTrue(informixChecker.applicable(protectedResource));
    }

    /**
     * 用例场景：检查集群注册成功
     * 前置条件：无
     * 检查点：注册检查
     */
    @Test
    public void collect_connectable_resources_register_test() {
        Map<ProtectedResource, List<ProtectedEnvironment>> mockMap = new HashMap<>();
        mockMap.put(new ProtectedResource(), new ArrayList<>());
        Mockito.when(retrievalsService.collectConnectableResources(any(ProtectedResource.class)))
                .thenReturn(mockMap);

        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        protectedResource.setDependencies(dependencies);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceListMap =
                                            informixChecker.collectConnectableResources(protectedResource);
        Assert.assertEquals(protectedResourceListMap.size(), 1);
    }

    /**
     * 用例场景：检查集群连通性成功
     * 前置条件：无
     * 检查点：连通性检查
     */
    @Test
    public void collect_connectable_resources_connect_test() {
        Map<ProtectedResource, List<ProtectedEnvironment>> mockMap = new HashMap<>();
        mockMap.put(new ProtectedResource(), new ArrayList<>());
        Mockito.when(retrievalsService.collectConnectableResources(any(ProtectedResource.class)))
                .thenReturn(mockMap);
        Mockito.when(informixService.getResourceById(anyString())).thenReturn(new ProtectedResource());

        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        ProtectedResource child = new ProtectedResource();
        child.setUuid("123-123-123");
        child.setExtendInfoByKey(InformixConstant.INSTANCESTATUS, InformixConstant.MASTER_NODE_STATUS);
        ArrayList<ProtectedResource> childrenList = new ArrayList<>();
        childrenList.add(child);
        dependencies.put("children", childrenList);
        protectedResource.setDependencies(dependencies);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceListMap =
                informixChecker.collectConnectableResources(protectedResource);
        Assert.assertEquals(protectedResourceListMap.size(), 1);
    }
}

package openbackup.postgre.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * poostgre集群实例ConnectionChecker测试类
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.6.0]
 * @since 2024-04-25
 */
public class PostgreClusterInstanceConnectionCheckerTest {
    private static final String ENDPOINT = "8.40.99.187";

    private final ProtectedEnvironmentService environmentService = Mockito.mock(ProtectedEnvironmentService.class);

    private final InstanceResourceService instanceResourceService = Mockito.mock(InstanceResourceService.class);

    private final ProtectedEnvironmentRetrievalsService environmentRetrievalsService = Mockito.mock(
        ProtectedEnvironmentRetrievalsService.class);

    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);

    private PostgreClusterInstanceConnectionChecker connectionChecker = new PostgreClusterInstanceConnectionChecker(
        environmentRetrievalsService, agentUnifiedService, environmentService, instanceResourceService);

    @Before
    public void init() {
        PowerMockito.when(environmentService.getEnvironmentById(any())).thenReturn(mockEnv());
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入资源
     * 检查点：是否返回true
     */
    @Test
    public void applicable_postgre_cluster_instance_provider_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType());
        Assert.assertTrue(connectionChecker.applicable(resource));
        resource.setSubType(ResourceSubTypeEnum.MYSQL.getType());
        Assert.assertFalse(connectionChecker.applicable(resource));
    }

    @Test
    public void execute_check_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setExtendInfo(new HashMap<>());
        resource.setParentUuid("ParentUuid");
        List<ProtectedResource> result = new ArrayList<>();
        result.add(mockEnv());
        resource.setDependencies(ImmutableMap.of(DatabaseConstants.CHILDREN, result));
        PowerMockito.doNothing().when(instanceResourceService).setClusterInstanceNodeRole(any());
        PowerMockito.when(instanceResourceService.checkIsClusterInstance(any())).thenReturn(mockCheckResult());
        Assert.assertThrows(LegoCheckedException.class, () -> connectionChecker.collectConnectableResources(resource));
    }

    private ProtectedEnvironment mockEnv() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setExtendInfo(new HashMap<>());
        environment.setEndpoint(ENDPOINT);
        return environment;
    }

    private AgentBaseDto mockCheckResult() {
        Map<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.VERSION, "9.6.0");
        AgentBaseDto checkResult = new AgentBaseDto();
        checkResult.setErrorMessage(JSONObject.fromObject(map).toString());
        return checkResult;
    }
}
package openbackup.tdsql.resources.access.livemount;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.when;

import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.livemount.common.constants.LiveMountConstants;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tdsql.resources.access.service.TdsqlService;

import com.alibaba.fastjson.JSON;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-07-04
 */
@RunWith(MockitoJUnitRunner.class)
public class TdsqlLiveMountIntercreptorProviderTest {
    @Mock
    private AgentUnifiedService mockAgentUnifiedService;
    @Mock
    private TdsqlService mockTdsqlService;
    @Mock
    private CopyRestApi mockCopyRestApi;

    private TdsqlLiveMountIntercreptorProvider tdsqlLiveMountIntercreptorProviderUnderTest;
    @Before
    public void setUp() {
        tdsqlLiveMountIntercreptorProviderUnderTest =
            new TdsqlLiveMountIntercreptorProvider(mockAgentUnifiedService, mockTdsqlService, mockCopyRestApi);
    }

    /**
     * 用例场景：TDSQL环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(tdsqlLiveMountIntercreptorProviderUnderTest.applicable(
            ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType())
        );
        Assert.assertFalse(tdsqlLiveMountIntercreptorProviderUnderTest.applicable("object"));
    }

    /**
     * 用例场景：即时挂载参数准备
     * 前置条件：参数正常
     * 检查点：不报错，参数正确
     */
    @Test
    public void test_init_liveMount_create_task_param_success() {
        when(mockTdsqlService.getEnvironmentById(anyString())).thenReturn(getEnvironment());
        when(mockTdsqlService.getAgentEndpoint(any())).thenReturn(getEndpoint());
        when(mockAgentUnifiedService.getHost(anyString(), anyInt())).thenReturn(getAgent());
        LiveMountCreateTask task = getLiveMountCreateTask();
        tdsqlLiveMountIntercreptorProviderUnderTest.initialize(task);
        Assert.assertNotNull(task.getAgents());
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(), task.getTargetEnv().getExtendInfo()
            .get(DatabaseConstants.DEPLOY_TYPE));
        tdsqlLiveMountIntercreptorProviderUnderTest.initialize(getLiveMountCreateTask());
    }

    /**
     * 用例场景：取消即时挂载参数准备
     * 前置条件：参数正常
     * 检查点: 无异常，参数正确
     */
    @Test
    public void test_init_live_mount_cancel_task_param_success() {
        when(mockTdsqlService.getEnvironmentById(anyString())).thenReturn(getEnvironment());
        when(mockTdsqlService.getAgentEndpoint(any())).thenReturn(getEndpoint());
        when(mockAgentUnifiedService.getHost(anyString(), any())).thenReturn(getAgent());
        LiveMountCancelTask liveMountCancelTask = getLiveMountCancelTask();
        tdsqlLiveMountIntercreptorProviderUnderTest.finalize(liveMountCancelTask);
        Assert.assertNotNull(liveMountCancelTask.getAgents());
    }

    /**
     * 用例场景：是否刷新资源
     * 前置条件：无
     * 检查点: 返回值正确，无异常
     */
    @Test
    public void test_is_refresh_target_environment_success() {
        boolean result = tdsqlLiveMountIntercreptorProviderUnderTest.isRefreshTargetEnvironment();
        Assert.assertFalse(result);
    }

    private Endpoint getEndpoint() {
        Endpoint endpoint = new Endpoint();
        endpoint.setId("872a77ba-3d18-4751-91df-812831c86acc");
        endpoint.setIp("192.168.160.62");
        endpoint.setPort(59530);
        return endpoint;
    }

    private LiveMountCreateTask getLiveMountCreateTask() {
        LiveMountCreateTask liveMountCreateTask = new LiveMountCreateTask();
        ProtectedEnvironment protectedEnvironment = getEnvironment();
        liveMountCreateTask.setTargetEnv(BeanTools.copy(protectedEnvironment, TaskEnvironment::new));
        HashMap<String, Object> advanceParams = new HashMap<>();
        LiveMountFileSystemShareInfo liveMountFileSystemShareInfo = new LiveMountFileSystemShareInfo();
        liveMountFileSystemShareInfo.setType(1);
        advanceParams.put(LiveMountConstants.FILE_SYSTEM_SHARE_INFO,
            JSONObject.writeValueAsString(Collections.singletonList(liveMountFileSystemShareInfo)));
        advanceParams.put("preScript", "test.sh");
        liveMountCreateTask.setAdvanceParams(advanceParams);
        TaskResource taskResource = new TaskResource();
        taskResource.setParentUuid("parent_uuid");
        taskResource.setSubType("TDSQL-clusterInstance");
        liveMountCreateTask.setTargetObject(taskResource);
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(RepositoryTypeEnum.DATA.getType());
        liveMountCreateTask.setRepositories(Collections.singletonList(storageRepository));
        return liveMountCreateTask;
    }

    private LiveMountCancelTask getLiveMountCancelTask() {
        LiveMountCancelTask liveMountCancelTask = new LiveMountCancelTask();
        HashMap<String, Object> advanceParams = new HashMap<>();
        LiveMountFileSystemShareInfo liveMountFileSystemShareInfo = new LiveMountFileSystemShareInfo();
        liveMountFileSystemShareInfo.setType(1);
        advanceParams.put(LiveMountConstants.FILE_SYSTEM_SHARE_INFO,
            JSONObject.writeValueAsString(Collections.singletonList(liveMountFileSystemShareInfo)));
        liveMountCancelTask.setAdvanceParams(advanceParams);
        TaskResource taskResource = new TaskResource();
        taskResource.setParentUuid("parent_uuid");
        taskResource.setRootUuid("root_uuid");
        liveMountCancelTask.setTargetObject(taskResource);
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(RepositoryTypeEnum.DATA.getType());
        liveMountCancelTask.setRepositories(Collections.singletonList(storageRepository));
        liveMountCancelTask.getTargetObject().setSubType("TDSQL-clusterInstance");
        return liveMountCancelTask;
    }

    private HostDto getAgent() {
        HostDto host = new HostDto();
        host.setUuid("872a77ba-3d18-4751-91df-812831c86acc");
        host.setName("8-40-160-62");
        host.setType("agent");
        host.setSubType("UBackupAgent");
        Map<String, String> agentIpList = new HashMap<>();
        String agentIpListStr = "192.168.160.62,8.40.160.62,fe80::d143:5c06:d2cf:2287,fe80::41af:571c:a2e4:f061,fe80::9bf9:6e9c:d1a2:f15e,fe80::d094:f372:f7ba:9d2d,fe80::4655:46db:f055:4a5,fe80::5c03:1bd0:4b26:4a95";
        agentIpList.put("agentIpList", agentIpListStr);
        host.setExtendInfo(JSON.toJSONString(agentIpList));
        host.setEndpoint("192.168.160.62");
        host.setPort(59530);
        return host;
    }

    private ProtectedEnvironment getEnvironment() {
        String json = "{\"uuid\":\"872a77ba-3d18-4751-91df-812831c86acc\",\"name\":\"8-40-160-62\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"path\":\"\",\"createdTime\":\"2023-06-02 22:31:19.0\",\"rootUuid\":\"872a77ba-3d18-4751-91df-812831c86acc\",\"sourceType\":\"\",\"version\":\"1.5.RC1.027\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.160.62,8.40.160.62,fe80::d143:5c06:d2cf:2287,fe80::41af:571c:a2e4:f061,fe80::9bf9:6e9c:d1a2:f15e,fe80::d094:f372:f7ba:9d2d,fe80::4655:46db:f055:4a5,fe80::5c03:1bd0:4b26:4a95\",\"agentId\":\"1665020602290819074\",\"$citations_agents_aaf26dfe643f40b2acf54cc74cf9d8b6\":\"34a90a3dd9ec48ef9319479ec62a57f8\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"agentUpgradeable\":\"1\",\"agentUpgradeableVersion\":\"1.5.RC1.029\"},\"endpoint\":\"192.168.160.62\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"location\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }
}

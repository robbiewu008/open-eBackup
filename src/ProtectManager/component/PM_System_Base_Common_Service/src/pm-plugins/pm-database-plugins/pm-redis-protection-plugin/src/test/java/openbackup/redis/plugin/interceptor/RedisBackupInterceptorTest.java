package openbackup.redis.plugin.interceptor;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.redis.plugin.provider.RedisAgentProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import com.google.common.collect.Maps;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;

/**
 * Redis 数据库备份拦截器实现类 测试类
 *
 * @author w00439064
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/06/24
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {RedisBackupInterceptor.class})
public class RedisBackupInterceptorTest {
    @InjectMocks
    private RedisBackupInterceptor redisBackupInterceptor;

    @Mock
    private ResourceService resourceService;

    @Mock
    private KerberosService kerberosService;

    @Mock
    private EncryptorService encryptorService;

    @Before
    public void setup() {
        RedisAgentProvider redisAgentProvider = new RedisAgentProvider();
        redisBackupInterceptor = new RedisBackupInterceptor(resourceService, kerberosService, encryptorService,
            redisAgentProvider);
    }

    @Test
    public void test_supply_backup_task_success() {
        BackupTask backupTask = redisBackupInterceptor.supplyBackupTask(mockBackupTask());
        Set<String> keySet = backupTask.getProtectEnv().getExtendInfo().keySet();
        Assert.assertTrue(keySet.contains(DatabaseConstants.DEPLOY_TYPE));
    }

    @Test
    public void test_supply_agent_success() {
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.anyString()))
            .thenReturn(mockProtectedResource());
        BackupTask backupTask = mockBackupTask();
        redisBackupInterceptor.supplyAgent(backupTask);
        Assert.assertEquals(1, backupTask.getAgents().size());
        Assert.assertEquals(6379, backupTask.getAgents().get(0).getPort());
    }

    @Test
    public void test_supply_nodes_success() {
        ProtectedEnvironment cluster = new ProtectedEnvironment();
        ProtectedResource node = new ProtectedResource();
        node.setExtendInfo(Maps.newHashMap());
        node.setExtendInfoByKey("pair", "10.1.10.10:8080");
        node.setUuid("123");
        Authentication auth = new Authentication();
        auth.setExtendInfo(Maps.newHashMap());
        node.setAuth(auth);
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("456");
        node.setDependencies(Collections.singletonMap(DatabaseConstants.AGENTS, Arrays.asList(agent)));
        cluster.setDependencies(Collections.singletonMap(DatabaseConstants.CHILDREN, Arrays.asList(node)));
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.anyString()))
            .thenReturn(Optional.of(cluster));
        BackupTask backupTask = mockBackupTask();
        redisBackupInterceptor.supplyNodes(backupTask);
        Assert.assertEquals(1, backupTask.getProtectEnv().getNodes().size());
    }

    /**
     * 用例场景：redis备份的时候检查连通性
     * 前置条件：无
     * 检查点： 连通，允许备份
     */
    @Test
    public void test_supply_nodes_scan_not_complete_error() {
        ProtectedEnvironment cluster = new ProtectedEnvironment();
        ProtectedResource node = new ProtectedResource();
        node.setExtendInfo(Maps.newHashMap());
        node.setUuid("123");
        Authentication auth = new Authentication();
        auth.setExtendInfo(Maps.newHashMap());
        node.setAuth(auth);
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid("456");
        node.setDependencies(Collections.singletonMap(DatabaseConstants.AGENTS, Arrays.asList(agent)));
        cluster.setDependencies(Collections.singletonMap(DatabaseConstants.CHILDREN, Arrays.asList(node)));
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.anyString()))
            .thenReturn(Optional.of(cluster));
        BackupTask backupTask = mockBackupTask();
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> redisBackupInterceptor.supplyNodes(backupTask));
        Assert.assertEquals(CommonErrorCode.SCAN_JOB_NOT_COMPLETE, exception.getErrorCode());
    }

    /**
     * 用例场景：redis备份的时候检查连通性
     * 前置条件：无
     * 检查点： 连通，允许备份
     */
    @Test
    public void test_check_connection_success() {
        BackupTask backupTask = mockBackupTask();
        TaskEnvironment environment = new TaskEnvironment();
        // 资源状态为在线，连通可以备份
        environment.setLinkStatus(String.valueOf(LinkStatusEnum.ONLINE.getStatus()));
        backupTask.setProtectEnv(environment);
        redisBackupInterceptor.checkConnention(backupTask);
        // 资源状态为异常，连通可以备份
        environment.setLinkStatus(String.valueOf(LinkStatusEnum.PARTLY_ONLING.getStatus()));
        backupTask.setProtectEnv(environment);
        redisBackupInterceptor.checkConnention(backupTask);
    }

    /**
     * 用例场景：redis备份的时候检查连通性
     * 前置条件：无
     * 检查点： 不连通，不允许备份
     */
    @Test(expected = LegoCheckedException.class)
    public void test_check_connection_failed() {
        BackupTask backupTask = mockBackupTask();
        TaskEnvironment environment = new TaskEnvironment();
        environment.setLinkStatus(String.valueOf(LinkStatusEnum.OFFLINE.getStatus()));
        backupTask.setProtectEnv(environment);
        redisBackupInterceptor.checkConnention(backupTask);
    }

    @Test
    public void test_applicable_redis() {
        Assert.assertTrue(redisBackupInterceptor.applicable("Redis"));
    }

    private BackupTask mockBackupTask() {
        BackupTask backupTask = new BackupTask();
        TaskEnvironment environment = new TaskEnvironment();
        Map<String, String> envExtendInfo = new HashMap<>();
        environment.setExtendInfo(envExtendInfo);
        backupTask.setProtectEnv(environment);
        TaskResource resource = new TaskResource();
        resource.setUuid(UUID.randomUUID().toString());
        backupTask.setProtectObject(resource);
        return backupTask;
    }

    private Optional<ProtectedResource> mockProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        ProtectedResource resource = new ProtectedResource();
        Map<String, List<ProtectedResource>> agents = new HashMap<>();
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setUuid(UUID.randomUUID().toString());
        agent.setEndpoint("127.0.0.1");
        agent.setPort(6379);
        agents.put(DatabaseConstants.AGENTS, Arrays.asList(agent));
        resource.setDependencies(agents);
        resource.setUuid(UUID.randomUUID().toString());
        resource.setExtendInfo(new HashMap<>());
        dependencies.put(DatabaseConstants.CHILDREN, Arrays.asList(resource));
        protectedResource.setDependencies(dependencies);
        return Optional.of(protectedResource);
    }
}

package openbackup.redis.plugin.interceptor;

import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.redis.plugin.provider.RedisAgentProvider;
import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Maps;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Optional;

/**
 * Redis 数据库备份拦截器实现类 测试类
 *
 * @author w00504341
 * @since 2022-07-11
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {RedisRestoreInterceptor.class})
public class RedisRestoreInterceptorTest {
    @InjectMocks
    private RedisRestoreInterceptor redisRestoreInterceptor;

    @Mock
    private ResourceService resourceService;

    @Mock
    private DmeUnifiedRestApi dmeUnifiedRestApi;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private KerberosService kerberosService;

    @Mock
    private EncryptorService encryptorService;

    @Before
    public void setup() {
        RedisAgentProvider redisAgentProvider = new RedisAgentProvider();
        redisRestoreInterceptor = new RedisRestoreInterceptor(resourceService, dmeUnifiedRestApi, copyRestApi,
            kerberosService, encryptorService, redisAgentProvider);
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入Redis宏
     * 检查点：是否返回true
     */
    @Test
    public void applicable_restore_interceptor_provider_success() {
        Assert.assertTrue(redisRestoreInterceptor.applicable(ResourceSubTypeEnum.REDIS.getType()));
    }

    /**
     * 用例场景：特性开关，用于判断是否需要对目标资源是否在线进行校验
     * 前置条件：无
     * 检查点：是否返回false
     */
    @Test
    public void get_restore_feature_success() {
        Assert.assertFalse(redisRestoreInterceptor.getRestoreFeature().isShouldCheckEnvironmentIsOnline());
    }

    /**
     * 用例场景：应用自己填参数
     * 前置条件：无
     * 检查点：是否返回false
     */
    @Test
    @Ignore
    public void intercept_success() {
        PowerMockito.when(dmeUnifiedRestApi.getCopyInfo(ArgumentMatchers.anyString()))
            .thenReturn(buildDmeCopyRestResponse());
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.anyBoolean(), ArgumentMatchers.anyString()))
            .thenReturn(Optional.of(buildProtectedResource("0")));
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString())).thenReturn(buildCopyInfo());
        RestoreTask task = buildRestoreTask();
        redisRestoreInterceptor.initialize(task);
    }

    /**
     * 用例场景：应用自己填参数
     * 前置条件：无
     * 检查点：aof持久化模式，不符合，抛异常
     */
    @Test(expected = LegoCheckedException.class)
    @Ignore
    public void intercept_failed() {
        PowerMockito.when(dmeUnifiedRestApi.getCopyInfo(ArgumentMatchers.anyString()))
            .thenReturn(buildDmeCopyRestResponse());
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.anyBoolean(), ArgumentMatchers.anyString()))
            .thenReturn(Optional.of(buildProtectedResource("1")));
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString())).thenReturn(buildCopyInfo());
        RestoreTask task = buildRestoreTask();
        redisRestoreInterceptor.initialize(task);
    }

    private RestoreTask buildRestoreTask() {
        RestoreTask task = new RestoreTask();
        task.setTaskId("123");
        task.setRequestId("123");
        task.setCopyId("abc");
        task.setRestoreType("normalRestore");
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid("nodeId1");
        taskEnvironment.setLinkStatus("0");
        taskEnvironment.setExtendInfo(new HashMap<String, String>() {{
            put(RedisConstant.SLOT, "0-100");
        }});
        task.setTargetEnv(taskEnvironment);
        return task;
    }

    private DmeCopyInfo buildDmeCopyRestResponse() {
        DmeCopyInfo dmeCopyInfo = new DmeCopyInfo();
        List<TaskEnvironment> srcNodes = buildNodes();
        TaskEnvironment protectEnv = new TaskEnvironment();
        protectEnv.setNodes(srcNodes);
        dmeCopyInfo.setProtectEnv(protectEnv);
        return dmeCopyInfo;
    }

    private List<TaskEnvironment> buildNodes() {
        List<TaskEnvironment> nodes = new ArrayList<>();
        TaskEnvironment node1 = new TaskEnvironment();
        node1.setExtendInfo(new HashMap<String, String>() {{
            put(RedisConstant.SLOT, "0-100");
            put(DatabaseConstants.STATUS, "28");
        }});
        TaskEnvironment node2 = new TaskEnvironment();
        node2.setExtendInfo(new HashMap<String, String>() {{
            put(RedisConstant.SLOT, "");
            put(DatabaseConstants.STATUS, "28");
        }});
        nodes.add(node1);
        nodes.add(node2);
        return nodes;
    }

    private ProtectedResource buildProtectedResource(String aofEnable) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setDependencies(new HashMap<String, List<ProtectedResource>>() {{
            put("children", buildProtectChildren(aofEnable));
        }});
        return protectedResource;
    }

    private List<ProtectedResource> buildProtectChildren(String aofEnable) {
        List<ProtectedResource> children = new ArrayList<>();
        ProtectedResource child1 = new ProtectedResource();
        Authentication auth = new Authentication();
        auth.setExtendInfo(Maps.newHashMap());
        child1.setAuth(auth);
        child1.setExtendInfo(new HashMap<String, String>() {{
            put("slot", "0-100");
            put("status", "28");
            put(RedisConstant.RDB_DB_FILENAME, "dbfile1.rdb");
            put(RedisConstant.AOF_ENABLED, aofEnable);
            put(RedisConstant.RDB_DIR, "/root");
        }});
        child1.setDependencies(new HashMap<String, List<ProtectedResource>>() {{
            put("agents", buildAgents("123"));
        }});
        child1.setUuid("123");
        ProtectedResource child2 = new ProtectedResource();
        child2.setAuth(auth);
        child2.setExtendInfo(new HashMap<String, String>() {{
            put("slot", "");
            put("status", "28");
            put(RedisConstant.RDB_DB_FILENAME, "dbfile2.rdb");
            put(RedisConstant.AOF_ENABLED, aofEnable);
            put(RedisConstant.RDB_DIR, "/root");
        }});
        child2.setDependencies(new HashMap<String, List<ProtectedResource>>() {{
            put("agents", buildAgents("456"));
        }});
        child2.setUuid("456");
        children.add(child1);
        children.add(child2);
        return children;
    }

    private List<ProtectedResource> buildAgents(String uuid) {
        List<ProtectedResource> agents = new ArrayList<>();
        ProtectedEnvironment agent = new ProtectedEnvironment();
        agent.setLinkStatus("1");
        agent.setUuid(uuid);
        agent.setEndpoint("endpoint");
        agent.setPort(12);
        agents.add(agent);
        return agents;
    }

    private Copy buildCopyInfo() {
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value());
        return copy;
    }

    /**
     * 用例场景：下发恢复任务时 针对资源进行锁定
     * 前置条件：构造restoreTask结构体
     * 检查点: 成功返回加锁列表
     */
    @Test
    public void restore_getLock_resources() {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setTargetObject(new TaskResource());
        restoreTask.getTargetObject().setUuid("test_uuid");
        List<LockResourceBo> lockResources = redisRestoreInterceptor.getLockResources(restoreTask);
        Assert.assertEquals(lockResources.get(0).getLockType(), LockType.WRITE);
    }
}

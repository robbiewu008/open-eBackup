/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.access.framework.resource.service;

import static org.assertj.core.api.Assertions.assertThatNoException;
import static org.assertj.core.api.Assertions.assertThatThrownBy;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.argThat;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import openbackup.access.framework.resource.persistence.dao.ProtectedResourceMapper;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceRepositoryImpl;
import openbackup.access.framework.resource.service.provider.AgentDefaultLinkStatusProvider;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.CyberEngineResourceService;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.pack.lock.Lock;
import openbackup.system.base.pack.lock.LockService;
import openbackup.system.base.pack.lock.zookeeper.pack.LockServiceImpl;
import openbackup.system.base.pack.lock.zookeeper.zookeeper.ZookeeperService;
import openbackup.system.base.query.SessionService;
import com.huawei.oceanprotect.system.base.schedule.service.SchedulerService;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.lock.ResourceLockRestApi;
import openbackup.system.base.sdk.resource.EnvironmentScanRestApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import openbackup.system.base.sdk.schedule.model.Schedule;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.RedisContextService;

import org.apache.curator.framework.recipes.locks.InterProcessMutex;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.quartz.SchedulerException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@SpringBootTest
@RunWith(SpringRunner.class)
@ContextConfiguration(
        classes = {
            ProtectedEnvironmentServiceImpl.class,
            JobScheduleService.class,
            ProtectedEnvironmentListenerWrapper.class,
            LockServiceImpl.class,
            SessionService.class, SqlInitializationAutoConfiguration.class
        })
@MockBean({
    ScheduleRestApi.class,
    ProtectedResourceMapper.class,
    MessageTemplate.class,
    JobCenterRestApi.class,
    ProtectedResourceRepositoryImpl.class,
    MessageTemplate.class,
    ZookeeperService.class,
    CommonAlarmService.class,
    ResourceLockRestApi.class,
    RedisContextService.class
})
public class ProtectedEnvironmentServiceImplTest {
    private static final String ENV_ID = "266ea41d-adf5-480b-af50-15b940c2b846";
    private static final String PARENT_ID = "266ea41d-adf5-480b-af50-15b940c2b850";
    private static final String ROOT_ID = "366ea41d-adf5-480b-af50-15b940c2b850";

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private SchedulerService scheduleService;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private MemberClusterService memberClusterService;

    @MockBean
    private ResourceExtendInfoService resourceExtendInfoService;

    @MockBean
    private CyberEngineResourceService cyberEngineResourceService;

    @MockBean
    private LockService lockService;

    @MockBean
    private Lock lock;

    @Autowired
    private ProtectedEnvironmentServiceImpl protectedEnvironmentService;

    @Autowired
    private ProtectedEnvironmentListenerWrapper protectedEnvironmentListener;

    @Autowired
    private ZookeeperService zookeeperService;

    @MockBean
    private DeployTypeService deployTypeService;

    @MockBean
    private JobService jobService;

    @MockBean
    private ResourceScanService resourceScanService;

    @MockBean
    private SessionService sessionService;

    @MockBean
    private AgentDefaultLinkStatusProvider agentDefaultLinkStatusProvider;

    @MockBean
    private StringRedisTemplate stringRedisTemplate;

    @MockBean
    private EnvironmentScanRestApi environmentScanRestApi;

    @MockBean
    private ProtectedResourceRepository protectedResourceRepository;

    @Before
    public void prepare() throws Exception {
        mockMutex(true);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        PowerMockito.doNothing().when(protectedResourceRepository).deleteProtectResourceExtendInfoByResourceId(Mockito.any(), Mockito.any());
        Mockito.when(sessionService.call(Mockito.any(),Mockito.any(String.class))).thenReturn(response);
    }

    private void mockMutex(boolean isAcquired) throws Exception {
        InterProcessMutex mutex = PowerMockito.mock(InterProcessMutex.class);
        PowerMockito.when(zookeeperService.createLock(any())).thenReturn(mutex);
        PowerMockito.when(mutex.acquire(anyLong(), any())).thenReturn(isAcquired);
        PowerMockito.doNothing().when(mutex).release();
        PowerMockito.when(lockService.createDistributeLock(anyString())).thenReturn(lock);
    }

    /**
     * 用例场景： 测试注册环境成功<br/>
     * 前置条件： 输入正确的参数，插件check没有自行设置root_uuid<br/>
     * 检查 点： 无异常抛出, 且environment的rootUUID被框架设置为对应的UUID<br/>
     *
     * @throws SchedulerException 调度异常
     */
    @Test
    public void test_register_environment_success_when_plugin_not_set_root_uuid() throws SchedulerException {
        ProtectedEnvironment environment = mockProtectedEnvironment();
        EnvironmentProvider provider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.doNothing()
                .when(provider)
                .register(
                        argThat(
                                env -> {
                                    env.setUuid(ENV_ID);
                                    return false;
                                }));
        String[] arr = {ENV_ID};
        PowerMockito.when(resourceService.create(any(), eq(true))).thenReturn(arr);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any()))
                .thenReturn(mockHasNoProtectedResource());
        PowerMockito.when(scheduleService.startScheduler(Mockito.mock(Schedule.class))).thenReturn(ENV_ID);
        PowerMockito.doNothing().when(protectedResourceRepository).deleteProtectResourceExtendInfoByResourceId(anyString(),
            anyString());
        protectedEnvironmentService.register(environment);
        Assert.assertEquals(ENV_ID, environment.getRootUuid());
    }

    /**
     * 用例场景： 测试注册环境成功<br/>
     * 前置条件： 输入正确的参数，插件check自行设置root_uuid<br/>
     * 检查 点： 无异常抛出, 且environment的rootUUID未被框架设置<br/>
     *
     * @throws SchedulerException 调度异常
     */
    @Test
    public void test_register_environment_success_when_plugin_set_root_uuid() throws SchedulerException {
        ProtectedEnvironment environment = mockProtectedEnvironment();
        EnvironmentProvider provider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.doNothing()
                .when(provider)
                .register(
                        argThat(
                                env -> {
                                    env.setUuid(ENV_ID);
                                    env.setRootUuid(ROOT_ID); // 插件自行设置了相关ROOT_ID
                                    return false;
                                }));
        String[] arr = {ENV_ID};
        PowerMockito.when(resourceService.create(any(), eq(true))).thenReturn(arr);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any()))
                .thenReturn(mockHasNoProtectedResource());
        PowerMockito.when(scheduleService.startScheduler(Mockito.mock(Schedule.class))).thenReturn(ENV_ID);
        PowerMockito.doNothing().when(protectedResourceRepository).deleteProtectResourceExtendInfoByResourceId(anyString(),
            anyString());
        protectedEnvironmentService.register(environment);
        Assert.assertEquals(ROOT_ID, environment.getRootUuid());
    }

    /**
     * 用例场景： 名称重复抛出异常<br/>
     * 前置条件： 数据库已经存储名称相同的环境，且注册的名称重复<br/>
     * 检查 点： 抛出指定异常和错误信息<br/>
     */
    @Test
    public void test_should_throws_LegoCheckedException_when_register_environment_if_name_duplicate() {
        String envName = "TestEnvName";
        ProtectedEnvironment environment = mockProtectedEnvironment();
        environment.setName(envName);
        EnvironmentProvider provider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.doNothing().when(provider).register(Mockito.mock(ProtectedEnvironment.class));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any()))
                .thenReturn(mockDuplicateNameResource(envName));
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(Collections.singletonList(new ProtectedResource()));
        Mockito.when(sessionService.call(Mockito.any(),Mockito.any(String.class))).thenReturn(response);
        PowerMockito.doNothing().when(protectedResourceRepository).deleteProtectResourceExtendInfoByResourceId(anyString(),
            anyString());
        LegoCheckedException legoCheckedException =
                Assert.assertThrows(
                        LegoCheckedException.class, () -> protectedEnvironmentService.register(environment));
        Assert.assertTrue(legoCheckedException.getMessage().contains("Duplicate environment name exists"));
    }

    /**
     * 用例场景： 测试处理扫描环境资源消息成功
     * 前置条件： 输入正确的参数
     * 检查 点： 无异常抛出
     */
    @Test
    public void test_handle_scan_environment_message_success() {
        Acknowledgment acknowledgment = Mockito.mock(Acknowledgment.class);
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        ProtectedEnvironment environment = mockProtectedEnvironment();
        JSONObject jsonObject = JSONObject.fromObject(environment);
        String scanEnvMessage = jsonObject.toString();
        EnvironmentProvider provider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        List<ProtectedResource> protectedResourceList = Collections.singletonList(new ProtectedResource());
        PowerMockito.when(provider.scan(Mockito.any(ProtectedEnvironment.class))).thenReturn(protectedResourceList);
        String[] arr = {"266ea41d-adf5-480b-af50-15b940c2b847"};
        PowerMockito.when(resourceService.create(new ProtectedResource[]{Mockito.mock(ProtectedEnvironment.class)}))
                .thenReturn(arr);
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(environment));
        protectedEnvironmentListener.handleScanEnvMessage(scanEnvMessage, acknowledgment);
    }

    /**
     * 用例场景： 测试处理扫描环境资源消息加锁失败<br/>
     * 前置条件： 输入正确的参数，无法正常加锁<br/>
     * 检查 点： 无异常抛出<br/>
     *
     * @throws Exception Exception
     */
    @Test
    public void test_handler_scan_environment_message_failure() throws Exception {
        Acknowledgment acknowledgment = Mockito.mock(Acknowledgment.class);
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        ProtectedEnvironment environment = mockProtectedEnvironment();
        JSONObject jsonObject = JSONObject.fromObject(environment);
        String scanEnvMessage = jsonObject.toString();
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(Optional.of(environment));
        mockMutex(false);
        protectedEnvironmentListener.handleScanEnvMessage(scanEnvMessage, acknowledgment);
        verify(resourceService, times(1)).getResourceById(any());
    }

    /**
     * 用例场景： 测试浏览环境资源成功
     * 前置条件： 输入正确的参数
     * 检查 点： 无异常抛出
     */
    @Test
    public void test_browse_protected_environment_success() {
        EnvironmentProvider provider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.when(resourceService.getResourceById(anyString()))
                .thenReturn(Optional.of(mockProtectedEnvironment()));
        PowerMockito.when(provider.browse(Mockito.any(ProtectedEnvironment.class),any()))
                .thenReturn(any());
        protectedEnvironmentService.browse(getBrowseEnvironmentResourceConditions());
    }

    /**
     * 用例场景：受保护环境不存在，抛出相应的异常
     * 前置条件：输入不存在的受保护环境ID
     * 检查 点： 检查异常信息是否和期望的一致
     */
    @Test
    public void test_should_throws_LegoCheckedException_when_browse_resource_if_env_is_not_exists() {
        EnvironmentProvider provider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        PowerMockito.when(provider.browse(Mockito.any(ProtectedEnvironment.class), any()))
                .thenReturn(any());
        LegoCheckedException legoCheckedException =
                Assert.assertThrows(
                        LegoCheckedException.class,
                        () -> protectedEnvironmentService.browse(getBrowseEnvironmentResourceConditions()));
        Assert.assertTrue(legoCheckedException.getMessage().contains("Protected environment is not exists!"));
    }


    private BrowseEnvironmentResourceConditions getBrowseEnvironmentResourceConditions() {
        BrowseEnvironmentResourceConditions environmentConditions = new BrowseEnvironmentResourceConditions();
        environmentConditions.setEnvId(ENV_ID);
        environmentConditions.setPageNo(0);
        environmentConditions.setPageSize(100);
        environmentConditions.setParentId(PARENT_ID);
        environmentConditions.setResourceType(null);
        return environmentConditions;
    }

    /**
     * 用例场景：删除受保护环境时，如受保护环境不存在，直接返回
     * 前置条件：输入不存在的受保护环境ID
     * 检查 点： 检查直接返回，方法执行一次无错误信息
     */
    @Test
    public void test_should_direct_return_when_delete_protected_env_if_env_is_not_exists() {
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        protectedEnvironmentService.deleteEnvironmentById(ENV_ID);
    }

    /**
     * 用例场景：查询受保护环境
     * 前置条件：无
     * 检查 点： 查询成功
     */
    @Test
    public void test_should_get_basic_environment_success() {
        ProtectedEnvironment env = new ProtectedEnvironment();
        env.setUuid("11");
        env.setName("name");

        PowerMockito.when(resourceService.getBasicResourceById("11")).thenReturn(Optional.of(env));
        Optional<ProtectedEnvironment> environmentOptional = protectedEnvironmentService.getBasicEnvironmentById("11");
        Assert.assertTrue(environmentOptional.isPresent());
        Assert.assertEquals(environmentOptional.get().getName(), "name");
    }

    /**
     * 用例场景：删除受保护环境时，如果受保护环境为升级中，不能被删除
     * 前置条件：受保护环境正在升级中
     * 检查 点： 升级中环境不能被删除
     */
    @Test
    public void test_should_do_nothing_when_delete_protected_env_if_status_is_upgrading() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        ProtectedEnvironmentServiceImpl spy = Mockito.spy(protectedEnvironmentService);
        protectedEnvironment.setLinkStatus(LinkStatusEnum.AGENT_STATUS_QUEUED.getStatus().toString());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(protectedEnvironment));
        assertThatNoException().isThrownBy(() -> spy.deleteEnvironmentById(ENV_ID));
    }

    /**
     * 用例场景：测试受保护环境没有保护对象删除成功
     * 前置条件：受保护环境没有对象
     * 检查 点：查询不到资源
     */
    @Test
    public void test_delete_protected_env_success_when_env_has_no_protected_object() {
        PowerMockito.when(resourceService.getResourceById(anyString()))
                .thenReturn(Optional.of(mockProtectedEnvironment()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any()))
                .thenReturn(mockHasNoProtectedObject());
        PowerMockito.doNothing().when(resourceService).delete(any(ResourceDeleteParams.class));

        EnvironmentProvider provider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.doNothing().when(provider).remove(Mockito.mock(ProtectedEnvironment.class));
        PowerMockito.when(lock.tryLockAndRun(anyLong(), any(), any())).thenReturn(true);
        protectedEnvironmentService.deleteEnvironmentById(ENV_ID);

        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        LegoCheckedException legoCheckedException =
                Assert.assertThrows(
                        LegoCheckedException.class, () -> protectedEnvironmentService.getEnvironmentById(ENV_ID));
        Assert.assertTrue(legoCheckedException.getMessage().contains("Protected environment is not exists!"));
    }

    /**
     * 用例场景：测试受保护环境没有资源时删除成功
     * 前置条件：受保护环境没有资源
     * 检查 点： 查询不到资源
     */
    @Test
    public void test_delete_protected_env_success_when_env_has_no_protected_resource() {
        PowerMockito.when(resourceService.getResourceById(anyString()))
                .thenReturn(Optional.of(mockProtectedEnvironment()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any()))
                .thenReturn(mockHasNoProtectedResource());
        PowerMockito.doNothing().when(resourceService).delete(any(ResourceDeleteParams.class));

        EnvironmentProvider provider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.doNothing().when(provider).remove(Mockito.mock(ProtectedEnvironment.class));
        PowerMockito.when(lock.tryLockAndRun(anyLong(), any(), any())).thenReturn(true);
        protectedEnvironmentService.deleteEnvironmentById(ENV_ID);

        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.empty());
        LegoCheckedException legoCheckedException =
                Assert.assertThrows(
                        LegoCheckedException.class, () -> protectedEnvironmentService.getEnvironmentById(ENV_ID));
        Assert.assertTrue(legoCheckedException.getMessage().contains("Protected environment is not exists!"));
    }

    /**
     * 用例场景：测试受保护环境时，如果存在绑定SLA的资源，则抛出指定的错误
     * 前置条件：受保护环境有资源且已经绑定SLA
     * 检查 点： 抛出指定的异常
     */
    @Test
    public void test_should_throws_LegoCheckedException_when_delete_protected_env_if_protected_object_exists() {
        PowerMockito.when(resourceService.getResourceById(anyString()))
                .thenReturn(Optional.of(mockProtectedEnvironment()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(), any())).thenReturn(mockHasProtectedObject());
        PowerMockito.doThrow(
                new LegoCheckedException(
                        ProtectedResourceRepositoryImpl.RESOURCE_ALREADY_BIND_SLA,
                        "Having resources are bound to SLAs."))
                .when(resourceService)
                .delete(any(String[].class));

        EnvironmentProvider provider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(provider);
        PowerMockito.doNothing().when(provider).remove(Mockito.mock(ProtectedEnvironment.class));
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid(ENV_ID);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> ReflectionTestUtils.invokeMethod(protectedEnvironmentService, "deleteEnvironment", environment));
        Assert.assertEquals(legoCheckedException.getErrorCode(), ProtectedResourceRepositoryImpl.RESOURCE_ALREADY_BIND_SLA);
    }

    /**
     * 用例场景：是否存在同样的环境
     * 前置条件：无
     * 检查 点： 不存在返回false，否则为true
     */
    @Test
    public void has_same_environment_in_db() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("11");
        Mockito.when(resourceService.getResourceById(environment.getUuid()))
            .thenReturn(Optional.empty())
            .thenReturn(Optional.of(new ProtectedEnvironment()));
        Assert.assertFalse(protectedEnvironmentService.hasSameEnvironmentInDb(environment));
        EnvironmentProvider environmentProvider = Mockito.mock(EnvironmentProvider.class);
        Mockito.when(providerManager.findProvider(eq(EnvironmentProvider.class), Mockito.any()))
            .thenReturn(environmentProvider);
        Assert.assertTrue(protectedEnvironmentService.hasSameEnvironmentInDb(environment));
    }

    /**
     * 用例场景：是否存在同样的环境
     * 前置条件：无
     * 检查 点： 不存在返回false，否则为true
     */
    @Test
    public void should_throw_LegoCheckedException_if_has_same_endpoint_environment() {
        ProtectedEnvironment environment = mockProtectedEnvironment();
        List<ProtectedResource> resources = Collections.singletonList(environment);
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(resources);
        Mockito.when(resourceService.query(anyBoolean(), anyInt(), anyInt(), anyMap())).thenReturn(response);
        assertThatThrownBy(() -> protectedEnvironmentService.checkHasSameEndpointEnvironment(environment)).hasMessage(
                "Duplicate environment endpoint exists.");
    }

    /**
     * 用例场景：存在同样的环境删除原有定时任务成功
     * 前置条件：存在同样的环境
     * 检查 点： 删除原有定时任务
     */
    @Test
    public void has_same_environment_delete_job_success() throws SchedulerException {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("11");
        Mockito.when(resourceService.getResourceById(environment.getUuid()))
            .thenReturn(Optional.of(mockProtectedEnvironment()));
        protectedEnvironmentService.hasSameEnvironmentInDb(environment);
        verify(scheduleService, times(1)).removeJob(any());
    }

    private ProtectedEnvironment mockProtectedEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("HDFS_Cluster");
        environment.setType("BigData");
        environment.setSubType("HDFS_Cluster");
        environment.setEndpoint("127.0.0.1");
        environment.setPort(8888);
        return environment;
    }

    private PageListResponse<ProtectedResource> mockHasNoProtectedResource() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(0);
        response.setRecords(new ArrayList<>());
        return response;
    }

    private PageListResponse<ProtectedResource> mockDuplicateNameResource(String name) {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource resource = new ProtectedResource();
        resource.setName(name);
        response.setRecords(list);
        list.add(resource);
        return response;
    }

    private PageListResponse<ProtectedResource> mockHasNoProtectedObject() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        response.setRecords(null);
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource res = new ProtectedResource();
        list.add(res);
        return response;
    }

    private PageListResponse<ProtectedResource> mockHasProtectedObject() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(1);
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedResource res = new ProtectedResource();
        ProtectedObject protectedObject = new ProtectedObject();
        res.setProtectedObject(protectedObject);
        list.add(res);
        response.setRecords(list);
        return response;
    }
}

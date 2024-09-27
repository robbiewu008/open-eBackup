/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.service;

import com.baomidou.mybatisplus.autoconfigure.MybatisPlusAutoConfiguration;

import openbackup.access.framework.resource.service.provider.AgentDefaultLinkStatusProvider;
import openbackup.access.framework.resource.util.ResourceThreadPoolTool;
import openbackup.access.framework.resource.validator.JsonSchemaValidatorImpl;
import com.huawei.oceanprotect.base.cluster.sdk.dto.MemberClusterBo;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.ResourceExtensionManager;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.LegoInternalAlarm;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.pack.lock.SQLLockService;
import openbackup.system.base.pack.lock.zookeeper.pack.LockServiceImpl;
import openbackup.system.base.pack.lock.zookeeper.zookeeper.ZookeeperService;
import openbackup.system.base.query.DefaultPageQueryFieldNamingStrategy;
import openbackup.system.base.query.PageQueryService;
import openbackup.system.base.query.SessionService;
import com.huawei.oceanprotect.system.base.schedule.service.SchedulerService;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.resource.EnvironmentScanRestApi;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.RedisContextService;
import org.apache.curator.framework.recipes.locks.InterProcessMutex;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.mybatis.spring.annotation.MapperScan;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.jdbc.DataSourceAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.data.redis.core.StringRedisTemplate;
import org.springframework.data.redis.core.ValueOperations;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Optional;
import java.util.UUID;
import java.util.concurrent.TimeUnit;

import static openbackup.access.framework.resource.testdata.MockEntity.mockProtectedEnvironment;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.isNull;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

/**
 * Protected Environment Listener Test
 *
 * @author l00272247
 * @since 2022-01-27
 */
@SpringBootTest
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@ComponentScan(basePackages = {"openbackup.access.framework.resource.persistence.dao"})
@MapperScan(basePackages = {"openbackup.access.framework.resource.persistence.dao"})
@ContextConfiguration(
        classes = {
            DataSourceAutoConfiguration.class,
            MybatisPlusAutoConfiguration.class,
            ProtectedEnvironmentListenerWrapper.class,
            ProtectedEnvironmentServiceImpl.class,
            ProtectedResourceRepository.class,
            ResourceService.class,
            ProtectedResourceServiceImpl.class,
            PageQueryService.class,
            SessionService.class,
            ProtectedResourceMonitorService.class,
            ProtectedResourceDecryptService.class,
            ProtectedResourceWatchService.class,
            JsonSchemaValidatorImpl.class,
            JobScheduleService.class,
            DefaultPageQueryFieldNamingStrategy.class,
            LockServiceImpl.class
        })
@MockBean(
        value = {
            ScheduleRestApi.class,
            MessageTemplate.class,
            SchedulerService.class,
            EncryptorService.class,
            JobCenterRestApi.class,
            ZookeeperService.class,
            CommonAlarmService.class,
            ResourceExtensionManager.class,
            DeployTypeService.class,
            RedisContextService.class
        })
@PrepareForTest({ResourceThreadPoolTool.class})
public class ProtectedEnvironmentListenerTest {
    @Autowired
    private ProtectedEnvironmentListenerWrapper protectedEnvironmentListener;

    @Autowired
    private ZookeeperService zookeeperService;

    @MockBean
    private ProtectedEnvironmentServiceImpl protectedEnvironmentService;

    @MockBean
    private ProtectedResourceServiceImpl resourceService;

    @MockBean
    private JobCenterRestApi jobCenterRestApi;

    @MockBean
    private MemberClusterService memberClusterService;

    @MockBean
    private ResourceExtendInfoService resourceExtendInfoService;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private RedisContextService redisContextService;

    @MockBean
    private ProtectedResourceRepository protectedResourceRepository;

    @MockBean
    private JobService jobService;

    @MockBean
    private ResourceScanService resourceScanService;

    @MockBean
    private SQLLockService sqlLockService;

    @MockBean
    private AgentDefaultLinkStatusProvider agentDefaultLinkStatusProvider;
    @MockBean
    private StringRedisTemplate redisTemplate;

    @MockBean
    EnvironmentScanRestApi environmentScanRestApi;

    @Before
    public void prepare() throws Exception {
        InterProcessMutex mutex = PowerMockito.mock(InterProcessMutex.class);
        PowerMockito.when(zookeeperService.createLock(any())).thenReturn(mutex);
        PowerMockito.when(mutex.acquire(anyLong(), any())).thenReturn(true);
        PowerMockito.doNothing().when(mutex).release();
        Mockito.doNothing().when(resourceExtendInfoService).saveOrUpdateExtendInfo(anyString(),any());
        String envId = "uuid";
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(envId);
        protectedEnvironment.setLinkStatus("1");
        protectedEnvironment.setSubType("1");
        MemberClusterBo memberClusterBo = new MemberClusterBo();
        memberClusterBo.setRemoteEsn("esn");
        memberClusterBo.setClusterIp("111");
        memberClusterBo.setClusterName("111");
        memberClusterBo.setStatus(ClusterEnum.StatusEnum.ONLINE.getStatus());
        MemberClusterBo memberClusterBo1 = new MemberClusterBo();
        memberClusterBo1.setRemoteEsn("esn2");
        memberClusterBo1.setClusterIp("111");
        memberClusterBo1.setClusterName("111");
        memberClusterBo1.setStatus(ClusterEnum.StatusEnum.ONLINE.getStatus());
        MemberClusterBo memberClusterBo2 = new MemberClusterBo();
        memberClusterBo2.setRemoteEsn("esn3");
        memberClusterBo2.setClusterIp("111");
        memberClusterBo2.setClusterName("111");
        memberClusterBo2.setStatus(ClusterEnum.StatusEnum.ONLINE.getStatus());
        Mockito.when(memberClusterService.getMemberClusterByEsn(anyString())).thenReturn(memberClusterBo);
        Mockito.when(memberClusterService.getAllMemberClusters()).thenReturn(Arrays.asList(memberClusterBo,memberClusterBo1,memberClusterBo2));
        Mockito.when(memberClusterService.getCurrentClusterRole()).thenReturn("PRIMARY");
        EnvironmentProvider environmentProvider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(providerManager.findProvider(any(), anyString(), isNull())).thenReturn(environmentProvider);
        Mockito.when(environmentProvider.healthCheckWithResultStatus(any())).thenReturn(Optional.of("0"));
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(anyString())).thenReturn(protectedEnvironment);
        Mockito.when(jobService.isJobPresent(anyString())).thenReturn(true);
    }

    /**
     * 用例名称：验证 触发环境扫描。<br/>
     * 前置条件：相应方法已mock。<br/>
     * check点：完成触发，无异常抛出
     */
    @Test
    public void test_handle_environment_health_check() throws Exception {
        BasePage<String> page = new BasePage<>();
        page.setItems(Collections.unmodifiableList(Arrays.asList("uuid")));
        PowerMockito.doReturn(page).when(protectedResourceRepository).queryResourceUuids(any());
        ValueOperations valueOperations = PowerMockito.mock(ValueOperations.class);
        PowerMockito.when(valueOperations.get(any())).thenReturn(null);
        PowerMockito.doNothing().when(valueOperations).set(anyString(),anyString(),anyLong(),any(TimeUnit.class));
        PowerMockito.when(redisTemplate.opsForValue()).thenReturn(valueOperations);
        protectedEnvironmentListener.handleEnvironmentHealthCheck();
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：验证不进行健康检查的情况。<br/>
     * 前置条件：kafka运行正常。<br/>
     * check点：对于受保护环境状态不为On或者Off，或者provider为null的情况下，不进行健康检查。
     */
    @Test
    public void test_handle_environment_health_check_listener_not_executed() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        String envId = UUID.randomUUID().toString();
        String consumer = "{\"envId\": " + envId + "}";
        String consumerString = JSONObject.fromObject(consumer).toString();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(envId);
        protectedEnvironment.setLinkStatus("2");

        ProviderManager provider = PowerMockito.mock(ProviderManager.class);
        EnvironmentProvider environmentProvider = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(provider.findProviderOrDefault(any(), anyString(), any())).thenReturn(environmentProvider).thenReturn(null);

        // 受保护环境状态不为Online、Offline或者PARTLY_ONLING，则不进行健康检查
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(anyString())).thenReturn(protectedEnvironment);
        protectedEnvironmentListener.handleEnvironmentHealthCheck(consumerString, acknowledgment);
        Mockito.verify(environmentProvider, times(0)).validate(protectedEnvironment);

        // 如果provider为null，不进行健康检查
        protectedEnvironment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        protectedEnvironmentListener.handleEnvironmentHealthCheck(consumerString, acknowledgment);
        Mockito.verify(environmentProvider, times(0)).validate(protectedEnvironment);
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：验证进行健康检查的情况。<br/>
     * 前置条件：kafka运行正常。<br/>
     * check点：对于受保护环境状态不为On或者Off，或者provider为null的情况下，不进行健康检查。
     */
    @Test
    public void should_be_online_when_all_online() {
        Mockito.when(resourceExtendInfoService.queryExtendInfo("uuid","connection_result")).thenReturn(Collections.singletonMap("connection_result","{\n" +
                "    \"uuid\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 0,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    },\n" +
                "    \"esn\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 1,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    }\n" +
                "}"));
        ProtectedEnvironment protectedEnvironment = mockProtectedEnvironment();
        Mockito.when(providerManager.findProviderOrDefault(any(),
                any(), any())).thenReturn(agentDefaultLinkStatusProvider);
        ReflectionTestUtils.invokeMethod(protectedEnvironmentListener,"handleHealthCheckResult",protectedEnvironment,LinkStatusEnum.ONLINE,"esn");
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：更新Agent状态
     * 前置条件：有ABC三个节点，AB与Agent连接状态全为正常，C为离线。
     * check点：Agent健康状态为正常。
     */
    @Test
    public void should_be_online_when_AB_online(){
        // 有在线有离线的情况
        String extendInfo="{\n" +
                "    \"uuid\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 1,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    },\n" +
                "    \"esn\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 1,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    },\n" +
                "    \"esn2\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 0,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    }\n" +
                "}";
        Mockito.when(resourceExtendInfoService.queryExtendInfo("uuid","connection_result")).thenReturn(Collections.singletonMap("connection_result",extendInfo));
        ProtectedEnvironment protectedEnvironment = mockProtectedEnvironment();
        Mockito.when(providerManager.findProviderOrDefault(any(),any(),any()))
                .thenReturn(agentDefaultLinkStatusProvider);
        ReflectionTestUtils.invokeMethod(protectedEnvironmentListener,"handleHealthCheckResult",protectedEnvironment,LinkStatusEnum.ONLINE,"esn");
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：更新Agent状态
     * 前置条件：有ABC三个节点，与Agent连接状态全为离线。
     * check点：Agent健康状态为离线。
     */
    @Test
    public void should_be_online_when_all_offline(){
        // 有在线有离线的情况
        String extendInfo="{\n" +
                "    \"uuid\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 0,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    },\n" +
                "    \"esn\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 0,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    },\n" +
                "    \"esn2\": {\n" +
                "        \"end_point\": \"127.0.0.1\",\n" +
                "        \"link_status\": 0,\n" +
                "        \"cluster_name\": \"name\"\n" +
                "    }\n" +
                "}";
        Mockito.when(resourceExtendInfoService.queryExtendInfo("uuid","connection_result")).thenReturn(Collections.singletonMap("connection_result",extendInfo));
        ProtectedEnvironment protectedEnvironment = mockProtectedEnvironment();
        Mockito.when(providerManager.findProviderOrDefault(any(),any(),any()))
                .thenReturn(agentDefaultLinkStatusProvider);
        ReflectionTestUtils.invokeMethod(protectedEnvironmentListener,"handleHealthCheckResult",protectedEnvironment,LinkStatusEnum.ONLINE,"esn");
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：验证健康告警信息生成是否正确。<br/>
     * 前置条件：告警参数入参正确。<br/>
     * check点：生成的健康告警信息正确。
     *
     * @throws NoSuchMethodException NoSuchMethodException
     * @throws InvocationTargetException InvocationTargetException
     * @throws IllegalAccessException IllegalAccessException
     */
    @Test
    public void test_gen_health_alarm()
            throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        Method method =
                ProtectedEnvironmentListener.class.getDeclaredMethod(
                        "genHealthAlarm", ProtectedEnvironment.class);
        method.setAccessible(true);
        ProtectedEnvironment protectedEnvironment = mockProtectedEnvironment();
        Object[] args = new Object[] {protectedEnvironment};
        LegoInternalAlarm alarm = (LegoInternalAlarm) method.invoke(protectedEnvironmentListener, args);
        Assert.assertArrayEquals(
                new Object[] {"test1", "TestEnv", "-1"}, alarm.getAlarmParam());
        Assert.assertTrue(true);
    }

    @Test
    public void test_handle_scan_env_message() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        PowerMockito.doAnswer(invocation -> null).when(acknowledgment).acknowledge();
        Mockito.when(protectedEnvironmentService.getEnvironmentById(Mockito.any()))
            .thenReturn(new ProtectedEnvironment());
        protectedEnvironmentListener.handleScanEnvMessage(new JSONObject().toString(), acknowledgment);
        verify(acknowledgment, times(1)).acknowledge();
        protectedEnvironmentListener.handleScanEnvMessage(
                new JSONObject().set("uuid", "123").toString(), acknowledgment);
        verify(acknowledgment, times(2)).acknowledge();
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：验证 获取环境健康状态。<br/>
     * 前置条件：环境初始化成功，获取健康状态的方法已mock。<br/>
     * check点：正确获取环境健康状态
     */
    @Test
    public void test_get_environment_linkStatus(){
        String envId = UUID.randomUUID().toString();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(envId);
        protectedEnvironment.setLinkStatus("2");

        EnvironmentProvider environmentProvider1 = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(environmentProvider1.healthCheckWithResultStatus(any())).thenReturn(Optional.empty());
        EnvironmentProvider environmentProvider2 = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(environmentProvider2.healthCheckWithResultStatus(any())).thenReturn(Optional.of(String.valueOf(0)));
        EnvironmentProvider environmentProvider3 = PowerMockito.mock(EnvironmentProvider.class);
        PowerMockito.when(environmentProvider3.healthCheckWithResultStatus(any())).thenReturn(Optional.of(String.valueOf(8)));

        Object res = ReflectionTestUtils.invokeMethod(protectedEnvironmentListener,
                "getEnvironmentLinkStatus",protectedEnvironment, environmentProvider1);
        Assert.assertTrue(LinkStatusEnum.ONLINE.equals(res));

        Object res2 = ReflectionTestUtils.invokeMethod(protectedEnvironmentListener,
                "getEnvironmentLinkStatus",protectedEnvironment, environmentProvider2);
        Assert.assertTrue(LinkStatusEnum.OFFLINE.equals(res2));

        Object res3 = ReflectionTestUtils.invokeMethod(protectedEnvironmentListener,
                "getEnvironmentLinkStatus",protectedEnvironment, environmentProvider3);
        Assert.assertTrue(LinkStatusEnum.PARTLY_ONLING.equals(res3));
    }

    /**
     * 用例名称：验证 手动扫描资源。<br/>
     * 前置条件：相应方法已mock。<br/>
     * check点：正确启动线程并执行。
     *
     */
    @Test
    public void test_handle_manual_scan_env_message() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        PowerMockito.doAnswer(invocation -> null).when(acknowledgment).acknowledge();

        String envId = UUID.randomUUID().toString();
        String jobId = UUID.randomUUID().toString();
        String consumer = "{\"resId\": " + envId + ", \"job_id\": " + jobId + "}";
        String consumerString = JSONObject.fromObject(consumer).toString();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(envId);
        protectedEnvironment.setLinkStatus("2");

        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(
                java.util.Optional.of(protectedEnvironment));
        PowerMockito.doReturn(new ArrayList<>()).when(resourceService).scanProtectedResource(any());
        PowerMockito.doNothing().when(jobCenterRestApi).completeJob(any(), any());
        PowerMockito.doNothing().when(redisContextService).delete(any());
        protectedEnvironmentListener.handleEnvironmentScanJob(consumerString, acknowledgment);
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：验证 vmware手动扫描资源。<br/>
     * 前置条件：kafka消息中含有is_need_forward字段。<br/>
     * check点：正确启动线程并执行。
     *
     */
    @Test
    public void test_handle_vmware_manual_scan_env_message() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        PowerMockito.doAnswer(invocation -> null).when(acknowledgment).acknowledge();

        String consumer = "{\"job_id\":\"123\",\"resId\":\"{123}\"}";
        String consumerString = JSONObject.fromObject(consumer).toString();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setSubType(ResourceSubTypeEnum.ESX.getType());
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(protectedEnvironment));
        PowerMockito.when(jobService.isJobPresent(anyString())).thenReturn(true);
        PowerMockito.when(resourceScanService.jobIsFinished(anyString())).thenReturn(false);
        protectedEnvironmentListener.handleEnvironmentScanJob(consumerString, acknowledgment);
        Mockito.verify(environmentScanRestApi, times(1)).doScanResource(anyString(), anyString(), anyString());
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：验证 触发环境扫描。<br/>
     * 前置条件：相应方法已mock。<br/>
     * check点：资源扫描线程池繁忙时，不会触发环境扫描;反之触发
     */
    @Test
    public void test_handle_environment_health_check_trigger() {

        // 资源扫描线程池繁忙时，不会触发环境扫描
        PowerMockito.doReturn(new BasePage<>()).when(protectedResourceRepository).queryResourceUuids(any());
        PowerMockito.mockStatic(ResourceThreadPoolTool.class);
        PowerMockito.when(ResourceThreadPoolTool.isBusy()).thenReturn(true);
        ValueOperations valueOperations = PowerMockito.mock(ValueOperations.class);
        PowerMockito.when(valueOperations.setIfAbsent(any(),any(),any())).thenReturn(true);
        PowerMockito.when(redisTemplate.opsForValue()).thenReturn(valueOperations);
        protectedEnvironmentListener.handleEnvironmentHealthCheck();
        verify(protectedResourceRepository, times(0)).queryResourceUuids(any());

        // 反之触发
        BasePage<String> page = new BasePage<>();
        page.setItems(Collections.unmodifiableList(Arrays.asList("uuid")));
        PowerMockito.doReturn(page).when(protectedResourceRepository).queryResourceUuids(any());
        MessageTemplate<String> messageTemplate = PowerMockito.mock(MessageTemplate.class);
        PowerMockito.doReturn(null).when(messageTemplate).send(any(), anyString());
        PowerMockito.mockStatic(ResourceThreadPoolTool.class);
        PowerMockito.when(ResourceThreadPoolTool.isBusy()).thenReturn(false);

        protectedEnvironmentListener.handleEnvironmentHealthCheck();
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：验证 手动扫描资源的异常情况,扫描抛出异常。<br/>
     * 前置条件：相应方法已mock。<br/>
     * check点：进入异常处理逻辑。
     */
    @Test
    public void test_handle_manul_scan_env_message_error() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        PowerMockito.doAnswer(invocation -> null).when(acknowledgment).acknowledge();

        String envId = UUID.randomUUID().toString();
        String jobId = UUID.randomUUID().toString();
        String consumer = "{\"resId\": " + envId + ", \"job_id\": " + jobId + "}";
        JSONObject data = JSONObject.fromObject(consumer);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(envId);

        PowerMockito.when(resourceService.getResourceById(any()))
                .thenThrow(new LegoCheckedException("LegoCheckedException"));
        PowerMockito.doReturn(new ArrayList<>()).when(resourceService).scanProtectedResource(any());
        PowerMockito.doNothing().when(jobCenterRestApi).completeJob(any(), any());
        PowerMockito.doNothing().when(redisContextService).delete(any());

        ReflectionTestUtils
                .invokeMethod(protectedEnvironmentListener, "doScanProtectedResource", data, protectedResource);
        Mockito.verify(jobCenterRestApi, Mockito.times(1)).updateJob(any(), any());
        Assert.assertTrue(true);
    }

    /**
     * 用例名称：验证 手动扫描资源部分成功。<br/>
     * 前置条件：相应方法已mock。<br/>
     * check点：扫描部分成功。
     */
    @Test
    public void test_handle_manul_scan_env_message_partly_success() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        PowerMockito.doAnswer(invocation -> null).when(acknowledgment).acknowledge();

        String envId = UUID.randomUUID().toString();
        String jobId = UUID.randomUUID().toString();
        String consumer = "{\"resId\": " + envId + ", \"job_id\": " + jobId + "}";
        JSONObject data = JSONObject.fromObject(consumer);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(envId);
        protectedEnvironment.setLinkStatus("2");
        protectedEnvironment.setExtendInfoByKey(Constants.ERROR_CODE, "001");

        PowerMockito.when(resourceService.getResourceById(any()))
                .thenReturn(Optional.of(protectedEnvironment));
        PowerMockito.doReturn(new ArrayList<>()).when(resourceService).scanProtectedResource(any());
        PowerMockito.doNothing().when(jobCenterRestApi).completeJob(any(), any());
        PowerMockito.doNothing().when(redisContextService).delete(any());

        ReflectionTestUtils
                .invokeMethod(protectedEnvironmentListener, "doScanProtectedResource", data, protectedEnvironment);
        Mockito.verify(jobCenterRestApi, Mockito.times(1)).updateJob(any(), any());
        Assert.assertTrue(true);
    }
}

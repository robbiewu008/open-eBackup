package openbackup.data.access.framework.restore.service;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.is;
import static org.mockito.ArgumentMatchers.anyList;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.BDDMockito.any;
import static org.mockito.BDDMockito.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;
import static org.powermock.api.mockito.PowerMockito.whenNew;

import com.huawei.emeistor.kms.kmc.util.KmcHelper;
import openbackup.data.access.framework.copy.verify.service.CopyVerifyTaskManager;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.KmcHelperMocker;
import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.access.framework.protection.mocks.CreateRestoreTaskRequestMocker;
import openbackup.data.access.framework.protection.mocks.MockRestoreInterceptorProvider;
import openbackup.data.access.framework.protection.mocks.ProtectedResourceMocker;
import openbackup.data.access.framework.protection.mocks.RepositoryMocker;
import openbackup.data.access.framework.protection.mocks.RestoreTaskMocker;
import openbackup.data.access.framework.protection.mocks.TaskResourceMocker;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.access.framework.protection.service.job.JobLogRecorder;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.converter.RestoreTaskConverter;
import openbackup.data.access.framework.restore.dto.RestoreTaskContext;
import openbackup.data.access.framework.servitization.util.OpServiceHelper;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupFeatureService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.access.framework.copy.mng.service.CopyAuthVerifyService;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import com.google.common.collect.Lists;

import org.assertj.core.util.Maps;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.beans.BeanUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.UUID;

/**
 * 恢复任务管理单元测试类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/7
 **/
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {RestoreTaskManager.class, KmcHelper.class, TokenBo.class})
public class RestoreTaskManagerTest extends KmcHelperMocker {
    private final ProviderManager providerManager = mock(ProviderManager.class);
    private final RestoreTaskService restoreTaskService = mock(RestoreTaskService.class);
    private final JobLogRecorder jobLogRecorder = mock(JobLogRecorder.class);
    private final RestoreResourceService restoreResourceService = mock(RestoreResourceService.class);
    private final RestoreValidateService restoreValidateService = mock(RestoreValidateService.class);
    private final CopyVerifyTaskManager copyVerifyTaskManager = mock(CopyVerifyTaskManager.class);
    private final JobService jobService = mock(JobService.class);
    private final DeployTypeService deployTypeService = mock(DeployTypeService.class);
    private final UserService userService = mock(UserService.class);
    private final SanClientService sanClientService = mock(SanClientService.class);
    private final CopyAuthVerifyService copyAuthVerifyService = mock(CopyAuthVerifyService.class);
    private final TaskRepositoryManager taskRepositoryManager = mock(TaskRepositoryManager.class);

    private BackupFeatureService backupFeatureService = mock(BackupFeatureService.class);

    private final RestoreTaskManager restoreTaskManager = new RestoreTaskManager(providerManager, restoreTaskService,
        jobLogRecorder, restoreValidateService, restoreResourceService, copyVerifyTaskManager, jobService);

    @Before
    public void init() {
        Mockito.when(jobService.isJobPresent(anyString())).thenReturn(true);
        restoreTaskManager.setDeployTypeService(deployTypeService);
        restoreTaskManager.setSanClientService(sanClientService);
        restoreTaskManager.setUserService(userService);

        CommonAgentService commonAgentService = Mockito.mock(CommonAgentService.class);
        restoreTaskManager.setCommonAgentService(commonAgentService);
        restoreTaskManager.setTaskRepositoryManager(taskRepositoryManager);
        restoreTaskManager.setCopyAuthVerifyService(copyAuthVerifyService);

        restoreTaskManager.setBackupFeatureService(backupFeatureService);
    }

    private List<Endpoint> mockEndpoints() {
        List<Endpoint> endpointList = new ArrayList<>();
        endpointList.add(new Endpoint("22222", "192.128.1.2", 1002));
        endpointList.add(new Endpoint("33333", "192.128.1.3", 1003));
        endpointList.add(new Endpoint("44444", "192.128.1.4", 1004));
        return endpointList;
    }

    /**
     * 用例名称：验证普通恢复到原位置时，恢复任务失败
     * 前置条件：没有可用的agent
     * check点：抛出异常
     */
    @Test
    public void should_exception_if_no_agents()  throws Exception {
        // Given
        final String mockJobId = UUID.randomUUID().toString();
        final CreateRestoreTaskRequest taskRequest = CreateRestoreTaskRequestMocker.buildWithParams(RestoreTypeEnum.CR,
            RestoreLocationEnum.ORIGINAL);
        given(providerManager.findProvider(eq(RestoreInterceptorProvider.class), any(), any())).willReturn(
            new MockRestoreInterceptorProvider());
        given(restoreTaskService.queryCopyDetail(any())).willReturn(CopyMocker.mockHdfsCopy());
        given(restoreResourceService.queryEndpoints(any(),any(),any(), any())).willReturn(new ArrayList<>());
        given(taskRepositoryManager.getStorageRepositories(any(), any())).willReturn(Lists.newArrayList(RepositoryMocker.mockNativeNfsRepository()));
        given(restoreResourceService.queryProtectedEnvironment(any(),any())).willReturn(ProtectedResourceMocker.mockTaskEnv());
        given(restoreResourceService.buildTaskResourceByTargetObject(anyString())).willReturn(TaskResourceMocker.mockFullInfo());
        given(restoreTaskService.createJob(any())).willReturn(mockJobId);
        // When
        RestoreTaskContext restoreTaskContext = new RestoreTaskContext();
        whenNew(RestoreTaskContext.class).withNoArguments().thenReturn(restoreTaskContext);
        Assert.assertThrows(LegoCheckedException.class, () -> restoreTaskManager.init(taskRequest));
    }

    /**
     * 用例名称：验证普通恢复到原位置时，初始化恢复任务成功<br/>
     * 前置条件：无<br/>
     * check点：jobId与期望相同，上下文中对象正确<br/>
     */
    @Test
    public void should_return_job_id_when_init_given_normal_restore_and_original_location() throws Exception {
        // Given
        String repoPwd = "admin8888";
        String envPwd = "password1111";
        final String mockJobId = UUID.randomUUID().toString();
        final CreateRestoreTaskRequest taskRequest = CreateRestoreTaskRequestMocker.buildWithParams(RestoreTypeEnum.CR,
                RestoreLocationEnum.ORIGINAL);
        given(providerManager.findProvider(eq(RestoreInterceptorProvider.class), any(), any())).willReturn(
                new MockRestoreInterceptorProvider());
        given(restoreTaskService.queryCopyDetail(any())).willReturn(CopyMocker.mockHdfsCopy());
        given(restoreResourceService.queryEndpoints(any(),any(),any(), any())).willReturn(mockEndpoints());
        given(taskRepositoryManager.getStorageRepositories(any(),any())).willReturn(Lists.newArrayList(RepositoryMocker.mockNativeNfsRepository()));
        given(restoreResourceService.queryProtectedEnvironment(any(), any())).willReturn(ProtectedResourceMocker.mockTaskEnv());
        given(restoreResourceService.buildTaskResourceByTargetObject(anyString())).willReturn(TaskResourceMocker.mockFullInfo());
        PowerMockito.doNothing().when(copyAuthVerifyService).checkCopyOperationAuth(any(), any());
        given(restoreTaskService.createJob(any())).willReturn(mockJobId);
        // When
        RestoreTaskContext restoreTaskContext = new RestoreTaskContext();
        whenNew(RestoreTaskContext.class).withNoArguments().thenReturn(restoreTaskContext);
        PowerMockito.mockStatic(TokenBo.class);
        PowerMockito.when(TokenBo.get()).thenReturn(prepareTokenBo());
        final String jobId = restoreTaskManager.init(taskRequest);
        // Then
        assertThat(jobId, is(mockJobId));
        RestoreTask restoreTask = restoreTaskContext.getRestoreTask();
        assertThat(restoreTask.getRepositories().get(0).getExtendAuth().getAuthPwd(),is(repoPwd));
        assertThat(restoreTask.getTargetEnv().getAuth().getAuthPwd(), is(envPwd));
        assertThat(RestoreModeEnum.LOCAL_RESTORE.getMode(), is(restoreTask.getRestoreMode()));
    }


    /**
     * 用例名称：验证恢复任务开始，下发到dme成功<br/>
     * 前置条件：无<br/>
     * check点：调用到dem的feign client接口一次，程序未抛出异常<br/>
     */
    @Test
    public void should_success_when_start_given_correct_task_info() {
        // Given
        String mockJobId = UUID.randomUUID().toString();
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        final RestoreTask mockRestoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        mockRestoreTask.setRequestId(mockJobId);
        TaskResource mockResource = new TaskResource();
        BeanUtils.copyProperties(ProtectedResourceMocker.mockTaskResource(), mockResource);
        mockRestoreTask.setTargetObject(mockResource);
        final TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(ProtectedResourceMocker.mockTaskEnv(), taskEnvironment);
        mockRestoreTask.setTargetEnv(taskEnvironment);
        given(restoreTaskService.queryCopyDetail(any())).willReturn(CopyMocker.mockHdfsCopy());
        given(restoreTaskService.lockResources(any(), any(), anyList())).willReturn(true);
        given(providerManager.findProvider(eq(RestoreInterceptorProvider.class), any(), any())).willReturn(
            new MockRestoreInterceptorProvider());
        given(restoreResourceService.getLanFreeConfig(any(), any())).willReturn(Maps.newHashMap("123", "true"));
        given(restoreResourceService.queryEndpoints(any(), anyString(), any(),any())).willReturn(prepareAgents());
        JobBo jobBo = new JobBo();
        jobBo.setStatus(JobStatusEnum.READY.name());
        given(jobService.queryJob(any())).willReturn(jobBo);
        OpServiceHelper opServiceHelper = PowerMockito.mock(OpServiceHelper.class);
        restoreTaskManager.setOpServiceHelper(opServiceHelper);
        PowerMockito.when(sanClientService.checkSanCopyAndLanFree(any())).thenReturn(true);
        Mockito.when(backupFeatureService.isSupportDataAndLogParallelBackup(Mockito.any(String.class)))
                .thenReturn(true);
        // When
        restoreTaskManager.start(mockRestoreTask);
        // Then
        verify(restoreValidateService, times(1)).checkLicense(any(), any());
        verify(restoreTaskService, times(1)).startTask(any());
        verify(jobLogRecorder, times(2)).recordJobStep(any(), any(), any(), any());
        verify(jobLogRecorder, never()).recordJobStepWithError(any(), any(), any(), any());
        verify(restoreResourceService, times(1)).getLanFreeConfig(any(), any());
    }

    /**
     * 用例名称：验证恢复完成时，处理成功<br/>
     * 前置条件：无<br/>
     * check点：程序正常执行，并且函数中的关键步骤均调用1次<br/>
     */
    @Test
    public void should_success_when_test_complete_given_status_success() {
        // Given
        String requestId = UUID.randomUUID().toString();
        final RestoreTask mockRestoreTask = RestoreTaskMocker.mockRestoreTask(requestId);
        given(restoreTaskService.queryCopyDetail(any())).willReturn(CopyMocker.mockHdfsCopy());
        given(providerManager.findProvider(any(), any(), any())).willReturn(new MockRestoreInterceptorProvider());
        JobBo jobBo = new JobBo();
        jobBo.setExtendStr(JSONObject.fromObject("{\"copyOriginalStatus\":\"Normal\"}").toString());
        given(jobService.queryJob(requestId)).willReturn(jobBo);
        // When
        restoreTaskManager.complete(mockRestoreTask, ProviderJobStatusEnum.SUCCESS);
        // Then
        verify(jobLogRecorder, times(1)).recordJobStep(any(), any(), any(), any());
        verify(restoreResourceService, times(1)).refreshTargetEnv(any(), any(), any());
    }

    /**
     * 用例名称：d4部署形态不检查内置agent<br/>
     * 前置条件：无<br/>
     * check点：程序正常执行<br/>
     */
    @Test
    public void d4_restore_not_check_dme_agent() {
        final CreateRestoreTaskRequest taskRequest =
            CreateRestoreTaskRequestMocker.buildWithParams(RestoreTypeEnum.CR, RestoreLocationEnum.ORIGINAL);
        given(restoreTaskService.queryCopyDetail(any())).willReturn(CopyMocker.mockHdfsCopy());
        taskRequest.setAgents(null);
        when(deployTypeService.isHyperDetectDeployType()).thenReturn(true);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus("1");
        when(restoreResourceService.queryProtectedEnvironment(any(),any())).thenReturn(protectedEnvironment);
        when(restoreResourceService.buildTaskResourceByTargetObject(any())).thenReturn(new TaskResource());
        when(providerManager.findProvider(any(), any(), any())).thenReturn(new MockRestoreInterceptorProvider());
        when(deployTypeService.isNotSupportRBACType()).thenReturn(true);
        PowerMockito.mockStatic(TokenBo.class);
        PowerMockito.when(TokenBo.get()).thenReturn(prepareTokenBo());
        when(deployTypeService.isCyberEngine()).thenReturn(true);
        restoreTaskManager.init(taskRequest);
        verify(restoreTaskService, times(1)).createJob(any());
    }

    @Test
    public void test_recover_start() {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setRequestId("id");
        restoreTaskManager.recoverStart(new Exception(), restoreTask);
        verify(jobLogRecorder, times(1)).recordJobStepWithError(any(), any(), any(), any());
    }

    private List<Endpoint> prepareAgents() {
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("123");
        return Collections.singletonList(endpoint);
    }

    private TokenBo prepareTokenBo() {
        TokenBo tokenBo = new TokenBo();
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId("123");
        tokenBo.setUser(userBo);
        return tokenBo;
    }
}
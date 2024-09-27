package openbackup.data.access.framework.restore.service;

import static org.hamcrest.MatcherAssert.assertThat;
import static org.hamcrest.Matchers.empty;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.not;
import static org.hamcrest.Matchers.notNullValue;
import static org.hamcrest.Matchers.samePropertyValuesAs;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;
import static org.powermock.api.mockito.PowerMockito.whenNew;

import com.huawei.emeistor.kms.kmc.util.KmcHelper;
import openbackup.data.access.client.sdk.api.framework.dee.DeeCopiesManagementRestApi;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.core.dao.CopiesAntiRansomwareDao;
import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.access.framework.protection.mocks.CreateRestoreTaskRequestMocker;
import openbackup.data.access.framework.protection.mocks.JobBoMocker;
import openbackup.data.access.framework.protection.mocks.ProtectedResourceMocker;
import openbackup.data.access.framework.protection.mocks.RepositoryMocker;
import openbackup.data.access.framework.protection.mocks.RestoreTaskMocker;
import openbackup.data.access.framework.protection.mocks.TaskResourceMocker;
import openbackup.data.access.framework.protection.service.lock.ResourceLockService;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.converter.RestoreTaskConverter;
import openbackup.data.access.framework.restore.dto.RestoreTaskContext;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.job.model.request.CreateJobRequest;
import openbackup.system.base.service.AvailableAgentManagementDomainService;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.base.user.service.ResourceSetApi;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.beans.BeanUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

/**
 * RestoreTaskService 的单元测试用例集合
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/6
 **/
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {KmcHelper.class, RestoreTaskHelper.class, RestoreTaskService.class})
public class RestoreTaskServiceTest {
    private final CopyRestApi copyRestApi = mock(CopyRestApi.class);

    private final JobService jobService = mock(JobService.class);

    private final DeeCopiesManagementRestApi deeCopiesManagementRestApi = mock(DeeCopiesManagementRestApi.class);

    private final DmeUnifiedRestApi dmeUnifiedRestApi = mock(DmeUnifiedRestApi.class);

    private final RepositoryStrategyManager repositoryStrategyManager = mock(RepositoryStrategyManager.class);

    private final ResourceLockService resourceLockService = mock(ResourceLockService.class);

    private final DeployTypeService deployTypeService = mock(DeployTypeService.class);

    private final CopiesAntiRansomwareDao copiesAntiRansomwareDao = mock(CopiesAntiRansomwareDao.class);

    private final AvailableAgentManagementDomainService domainService = mock(AvailableAgentManagementDomainService.class);

    private final TaskRepositoryManager taskRepositoryManager = mock(TaskRepositoryManager.class);

    private final ResourceSetApi resourceSetApi = mock(ResourceSetApi.class);

    private final RestoreTaskService restoreTaskService = new RestoreTaskService(copyRestApi, jobService,
            dmeUnifiedRestApi, repositoryStrategyManager, resourceLockService);

    @Rule
    public final ExpectedException exception = ExpectedException.none();

    @Before
    public void before(){
        restoreTaskService.setDeployTypeService(deployTypeService);
        restoreTaskService.setAvailableAgentManagementDomainService(domainService);
        restoreTaskService.setCopiesAntiRansomwareDao(copiesAntiRansomwareDao);
        restoreTaskService.setDeeCopiesManagementRestApi(deeCopiesManagementRestApi);
        restoreTaskService.setResourceSetApi(resourceSetApi);
    }

    /**
     * 用例名称：验证创建恢复任务成功<br/>
     * 前置条件：无<br/>
     * check点：返回的jobId为指定的jobId<br/>
     */
    @Test
    public void should_return_job_id_when_createJob_given_correct_request() throws Exception {
        // Given
        String mockJobId = UUID.randomUUID().toString();
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        final RestoreTask mockRestoreTask = RestoreTaskConverter.convertToRestoreTask(mockRequest);
        final Copy mockCopy = CopyMocker.mockCommonCopy();
        final TaskResource taskResource = TaskResourceMocker.mockFullInfo();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(ProtectedResourceMocker.mockTaskEnv(), taskEnvironment);
        mockRestoreTask.setTargetEnv(taskEnvironment);
        mockRestoreTask.setTargetObject(taskResource);
        RestoreTaskContext mockContext = new RestoreTaskContext();
        mockContext.setTaskRequest(mockRequest);
        mockContext.setRestoreTask(mockRestoreTask);
        mockContext.setCopy(mockCopy);
        given(jobService.createJob(any())).willReturn(mockJobId);
        given(jobService.extractJobQueueScope(anyString(), anyString())).willReturn("root_uuid");
        CreateJobRequest jobRequest = new CreateJobRequest();
        whenNew(CreateJobRequest.class).withNoArguments().thenReturn(jobRequest);

        String passwdEncrypted = "Admin@123456789";
        PowerMockito.mockStatic(KmcHelper.class);
        KmcHelper kmcHelper = PowerMockito.mock(KmcHelper.class);
        PowerMockito.when(KmcHelper.getInstance()).thenReturn(kmcHelper);
        PowerMockito.when(kmcHelper.encrypt(anyString())).thenReturn(passwdEncrypted);

        // When
        String jobId = restoreTaskService.createJob(mockContext);
        // Then
        Assert.assertNotNull(jobId);
        Assert.assertEquals(jobId, mockJobId);
        Assert.assertFalse(jobRequest.isEnableStop());
        Assert.assertEquals(mockRestoreTask.getTargetEnv().getAuth().getAuthPwd(), passwdEncrypted);
    }

    /**
     * 用例名称：验证从任务信息获取恢复任务信息<br/>
     * 前置条件：无<br/>
     * check点：数据获取成功，且属性正确<br/>
     */
    @Test
    public void should_success_when_getRestoreTaskFromJob_given_job_contain_task_info() {
        // Given
        String jobId = UUID.randomUUID().toString();
        final RestoreTask mockRestoreTask = RestoreTaskMocker.mockRestoreTask(jobId);
        final JobBo jobBo = JobBoMocker.buildRestoreJobBo(mockRestoreTask);
        given(jobService.queryJob(eq(jobId))).willReturn(jobBo);
        // When
        final RestoreTask restoreTaskFromJob = restoreTaskService.getRestoreTaskFromJob(jobId);
        // Then
        assertThat(restoreTaskFromJob, is(notNullValue()));
        // 恢复目标字段不序列化， 故从job中取出来此字段为空
        assertThat(restoreTaskFromJob.getTargetLocation(), is(RestoreLocationEnum.NEW));
        assertThat(restoreTaskFromJob,
                samePropertyValuesAs(mockRestoreTask, "targetEnv", "agents", "filters", "repositories", "taskId",
                        "subObjects", "targetObject", "targetLocation", "restoreMode", "dataLayout"));
        assertThat(restoreTaskFromJob.getTargetObject(), samePropertyValuesAs(mockRestoreTask.getTargetObject()));
        assertThat(restoreTaskFromJob.getTargetEnv(),
                samePropertyValuesAs(mockRestoreTask.getTargetEnv(), "auth", "nodes", "linkStatus"));
        assertThat(restoreTaskFromJob.getTargetEnv().getAuth(),
                samePropertyValuesAs(mockRestoreTask.getTargetEnv().getAuth()));
    }

    /**
     * 用例名称：验证从副本扩展参数中获取并构造存储库列表成功<br/>
     * 前置条件：无<br/>
     * check点：存储库数量与期望一致<br/>
     */
    @Test
    public void should_return_repository_when_test_getStorageRepositories_given_properties_contain_repositories() {
        // Given
        String test
                = "{\"snapshots\":[{\"id\":\"7@5d6cac00-ef69-4a3f-9541-922fa33258c6\",\"parentName\":\"Storage_c1360673-c60f-333b-91c4-b0a2d1d1a595\"}],\"repositories\":[{\"type\":1,\"protocol\":5,\"extendInfo\":{\"fileSystemId\":\"38\",\"productEsn\":\"2102353GTH10L9000005\"}},{\"type\":0,\"protocol\":5,\"extendInfo\":{\"fileSystemId\":\"38\",\"productEsn\":\"2102353GTH10L9000005\"}},{\"type\":2,\"protocol\":5,\"extendInfo\":{\"fileSystemId\":\"38\",\"productEsn\":\"2102353GTH10L9000005\"}},{\"id\":\"d5f97cd7e0b640d08cbb6053d88b3ab9\",\"type\":1,\"protocol\":2,\"extendInfo\":null}],\"isAggregation\":\"false\",\"multiFileSystem\":\"false\",\"archive_id\":{\"present\":true},\"storage_id\":\"f071ace28a064a8c9f28a429d7aba50a\"}";
        String passwdEncrypted = "Admin@123456789";
        PowerMockito.mockStatic(KmcHelper.class);
        KmcHelper kmcHelper = PowerMockito.mock(KmcHelper.class);
        PowerMockito.when(KmcHelper.getInstance()).thenReturn(kmcHelper);
        PowerMockito.when(kmcHelper.encrypt(anyString())).thenReturn(passwdEncrypted);
        RepositoryStrategy mockStrategy = mock(RepositoryStrategy.class);
        given(mockStrategy.getRepository(any())).willReturn(RepositoryMocker.mockS3Repository());
        given(repositoryStrategyManager.getStrategy(any())).willReturn(mockStrategy);
        // When
        final List<StorageRepository> storageRepositories = taskRepositoryManager.getStorageRepositories(eq(test),any());
        // Then
        assertThat(storageRepositories, not(empty()));
    }

    /**
     * 用例名称：校验副本状态为非normal时，调用校验副本接口报错<br/>
     * 前置条件：无<br/>
     * check点：1.异常与期望一致 2. 异常信息与期望一致<br/>
     */
    @Test
    public void should_return_resource_when_test_buildTaskResourceByCopy_given_correct_copy_info() {
        // Given
        final Copy mockCopy = CopyMocker.mockHdfsCopy();
        final JSONObject resourceJsonObj = JSONObject.fromObject(mockCopy.getResourceProperties());
        // When
        final TaskResource taskResource = RestoreTaskHelper.buildTaskResourceByCopy(mockCopy);
        // Then
        Assert.assertNotNull(taskResource);
    }

    /**
     * 用例名称：验证更新任務狀態成功<br/>
     * 前置条件：无<br/>
     * check点：更新任務狀態成功<br/>
     */
    @Test(expected = NullPointerException.class)
    public void should_update_job_status_success() {
        PowerMockito.doNothing().when(copyRestApi).updateCopyStatus(anyString(), any());
        restoreTaskService.updateCopyStatus("copyId", CopyStatus.INVALID);
        Mockito.verify(copyRestApi, Mockito.times(1))
                .updateCopyStatus("copyId",RestoreTaskHelper.buildUpdateCopyStatusReq(CopyStatus.INVALID));
    }

    /**
     * 用例名称：验证更新任務资源锁狀態成功<br/>
     * 前置条件：无<br/>
     * check点：更新任务资源锁狀態成功<br/>
     */
    @Test
    public void should_update_job_lock_id_success() {
        String JobId = "jobId";
        restoreTaskService.updateJobLockId(JobId);
        Mockito.doNothing().when(jobService).updateJob(anyString(), any());
        Mockito.verify(jobService, Mockito.times(1)).updateJob(any(), any());
    }

    /**
     * 用例名称：验证更新任務强制停止状态成功<br/>
     * 前置条件：无<br/>
     * check点：更新任务强制停止状态成功<br/>
     */
    @Test
    public void should_update_job_enforce_stop_to_false_success() {
        PowerMockito.doNothing().when(jobService).updateJob(anyString(), any());
        restoreTaskService.updateJobEnforceStopToFalse("jobId");
        restoreTaskService.updateJobStatus("jobId", ProviderJobStatusEnum.FAIL);
        Mockito.verify(jobService, Mockito.times(2)).updateJob(anyString(),any());
    }

    /**
     * 用例名称：验证获取副本信息的gn号与查询出来的保持一致<br/>
     * 前置条件：无<br/>
     * check点：获取副本信息的gn号与查询出来的保持一致<br/>
     */
    @Test
    public void should_query_copy_detail_success() {
        Copy copyInfo = new Copy();
        copyInfo.setGn(10);
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copyInfo);
        Copy copy = restoreTaskService.queryCopyDetail("jobId");
        Assert.assertEquals(copy.getGn(), copyInfo.getGn());
    }

    /**
     * 用例名称：验证恢复任务调用dme结束后清理敏感信息成功<br/>
     * 前置条件：无<br/>
     * check点：验证恢复任务调用dme结束后清理敏感信息成功<br/>
     */
    @Test
    public void should_start_restore_task_clean_auth_info_success() {
        PowerMockito.doNothing().when(dmeUnifiedRestApi).createRestoreTask(any(), any());
        RestoreTask restoreTask = new RestoreTask();
        TaskResource taskResource = new TaskResource();
        Authentication authentication = new Authentication();
        authentication.setAuthPwd("pwd1");
        HashMap<String, String> objectHashMap = new HashMap<>();
        objectHashMap.put("secret", "secret");
        authentication.setExtendInfo(objectHashMap);
        taskResource.setAuth(authentication);
        restoreTask.setTargetObject(taskResource);
        TaskEnvironment environment = new TaskEnvironment();
        Authentication auth2 = new Authentication();
        auth2.setAuthPwd("pwd2");
        environment.setAuth(auth2);
        restoreTask.setTargetEnv(environment);
        List<StorageRepository> repositories = new ArrayList<>();
        for (int i = 0; i < 2; i++) {
            StorageRepository storageRepository = new StorageRepository();
            storageRepository.setType(i);
            storageRepository.setProtocol(1);
            Authentication auth = new Authentication();
            auth.setAuthPwd("repository pwd" + i);
            storageRepository.setAuth(auth);
            repositories.add(storageRepository);
        }
        restoreTask.setRepositories(repositories);
        restoreTaskService.startTask(restoreTask);
        Assert.assertNotEquals(restoreTask.getTargetObject().getAuth().getAuthPwd(), "");
        Assert.assertNotEquals(restoreTask.getTargetObject().getAuth().getExtendInfo().get("secret"), "");
        Assert.assertNotEquals(restoreTask.getTargetEnv().getAuth().getAuthPwd(), "");
        Assert.assertNotEquals(restoreTask.getRepositories().get(0).getAuth().getAuthPwd(), "");
    }

    /**
     * 用例名称：验证恢复任务为事中快照时调用dee接口<br/>
     * 前置条件：无<br/>
     * check点：无异常抛出<br/>
     */
    @Test
    public void should_start_task_success_when_is_io_detect() {
        PowerMockito.when(deployTypeService.isHyperDetectDeployType()).thenReturn(false);
        PowerMockito.when(deployTypeService.isCyberEngine()).thenReturn(true);
        PowerMockito.when(copiesAntiRansomwareDao.isIoDetectCopy(anyString())).thenReturn(true);
        given(restoreTaskService.queryCopyDetail(any())).willReturn(CopyMocker.mockHdfsCopy());
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setCopyId("test_1");
        restoreTask.setRequestId("req_1");
        restoreTask.setTaskId("task_1");
        restoreTaskService.startTask(restoreTask);
    }

    @Test
    public void should_start_restore_task() {
        RestoreTask restoreTask = new RestoreTask();
        restoreTask.setTaskId("id");
        restoreTask.setCopyId("id");
        PowerMockito.when(deployTypeService.isHyperDetectDeployType()).thenReturn(true);
        Copy copy = new Copy();
        copy.setProperties(null);
        PowerMockito.when(restoreTaskService.queryCopyDetail(anyString())).thenReturn(copy);
        PowerMockito.doNothing().when(deeCopiesManagementRestApi).restoreFsSnapshot(any());
        restoreTaskService.startTask(restoreTask);
        Mockito.verify(deeCopiesManagementRestApi,Mockito.times(1)).restoreFsSnapshot(any());
    }

    @Test(expected = NullPointerException.class)
    public void should_update_copy_to_original_status() {
        JobBo jobBo = new JobBo();
        JSONObject json = new JSONObject();
        json.put("copyOriginalStatus", "Normal");
        jobBo.setExtendStr(json.toString());
        PowerMockito.when(jobService.queryJob("job_id")).thenReturn(jobBo);
        restoreTaskService.updateCopyToOriginalStatus("job_id", "copy_id");
    }
}

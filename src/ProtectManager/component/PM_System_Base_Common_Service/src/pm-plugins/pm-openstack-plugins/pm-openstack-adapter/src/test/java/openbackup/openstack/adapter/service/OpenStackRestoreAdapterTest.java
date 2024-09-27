package openbackup.openstack.adapter.service;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.adapter.constants.OpenStackErrorCodes;
import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import openbackup.openstack.adapter.exception.OpenStackException;
import openbackup.openstack.adapter.generator.OpenStackModelsGenerator;
import openbackup.openstack.adapter.testdata.TestDataGenerator;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.stream.IntStream;

/**
 * {@link OpenStackRestoreAdapter} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-16
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(OpenStackModelsGenerator.class)
public class OpenStackRestoreAdapterTest {
    private final OpenStackResourceManager resourceManager = Mockito.mock(OpenStackResourceManager.class);

    private final OpenStackRestoreManager restoreManager = Mockito.mock(OpenStackRestoreManager.class);

    private final OpenStackJobManager jobManager = Mockito.mock(OpenStackJobManager.class);

    private final OpenStackRestoreAdapter adapter = new OpenStackRestoreAdapter(resourceManager, restoreManager,
        jobManager);

    /**
     * 用例名称：给定虚拟机恢复任务，创建恢复任务时成功<br/>
     * 前置条件：资源存在<br/>
     * check点：1.响应体存在任务id；2.调用一次queryResourceById方法<br/>
     */
    @Test
    public void test_shouldSuccess_when_createJob_givenServerRestoreJob() throws IOException {
        OpenStackRestoreJobDto restoreJob = TestDataGenerator.createServerRestoreJob();
        ProtectedResource resource = new ProtectedResource();
        Mockito.when(resourceManager.queryResourceById(restoreJob.getInstanceId())).thenReturn(Optional.of(resource));
        String jobId = UUIDGenerator.getUUID();
        Mockito.when(restoreManager.createRestoreTask(restoreJob, resource)).thenReturn(jobId);

        OpenStackRestoreJobDto resp = adapter.createJob(restoreJob);
        assertThat(resp).usingRecursiveComparison().ignoringFields("id").isEqualTo(restoreJob);
        assertThat(resp.getId()).isEqualTo(jobId);
        Mockito.verify(resourceManager, Mockito.times(1)).queryResourceById(restoreJob.getInstanceId());
    }

    /**
     * 用例名称：给定卷恢复任务，创建恢复任务时成功<br/>
     * 前置条件：资源存在<br/>
     * check点：1.响应体存在任务id；2.调用一次queryResourceByVolumeId方法<br/>
     */
    @Test
    public void test_shouldSuccess_when_createJob_givenVolumeRestoreJob() throws IOException {
        OpenStackRestoreJobDto restoreJob = TestDataGenerator.createVolumeRestoreJob();
        ProtectedResource resource = new ProtectedResource();
        Mockito.when(resourceManager.queryResourceByVolumeId(restoreJob.getInstanceId()))
            .thenReturn(Optional.of(resource));
        String jobId = UUIDGenerator.getUUID();
        Mockito.when(restoreManager.createRestoreTask(restoreJob, resource)).thenReturn(jobId);

        OpenStackRestoreJobDto resp = adapter.createJob(restoreJob);
        assertThat(resp).usingRecursiveComparison().ignoringFields("id").isEqualTo(restoreJob);
        assertThat(resp.getId()).isEqualTo(jobId);
        Mockito.verify(resourceManager, Mockito.times(1)).queryResourceByVolumeId(restoreJob.getInstanceId());
    }

    /**
     * 用例名称：如果根据卷id查找的资源不存在，则创建恢复任务失败<br/>
     * 前置条件：资源不存在<br/>
     * check点：创建恢复任务时，根据卷id查找的资源必须存在<br/>
     */
    @Test
    public void test_shouldThrowLegoCheckedException_when_createJob_givenNonExistVolumeId() throws IOException {
        OpenStackRestoreJobDto restoreJob = TestDataGenerator.createVolumeRestoreJob();
        ProtectedResource resource = new ProtectedResource();
        Mockito.when(resourceManager.queryResourceByVolumeId(restoreJob.getInstanceId())).thenReturn(Optional.empty());

        Assert.assertThrows(LegoCheckedException.class, () -> adapter.createJob(restoreJob));
        Mockito.verify(restoreManager, Mockito.times(0)).createRestoreTask(restoreJob, resource);
    }

    /**
     * 用例名称：如果根据卷虚拟机id查找的资源不存在，则创建恢复任务失败<br/>
     * 前置条件：资源不存在<br/>
     * check点：创建恢复任务时，根据虚拟机id查找的资源必须存在<br/>
     */
    @Test
    public void test_shouldThrowLegoCheckedException_when_createJob_givenNonExistServerId() throws IOException {
        OpenStackRestoreJobDto restoreJob = TestDataGenerator.createServerRestoreJob();
        ProtectedResource resource = new ProtectedResource();
        Mockito.when(resourceManager.queryResourceById(restoreJob.getInstanceId())).thenReturn(Optional.empty());

        Assert.assertThrows(LegoCheckedException.class, () -> adapter.createJob(restoreJob));
        Mockito.verify(restoreManager, Mockito.times(0)).createRestoreTask(restoreJob, resource);
    }

    /**
     * 用例名称：如果任务不存在，则查询任务抛出异常<br/>
     * 前置条件：任务不存在<br/>
     * check点：查询恢复任务时，任务不存在应抛出异常<br/>
     */
    @Test
    public void should_throwOpenStackException_when_queryJob_given_nonExistJob() {
        String jobId = UUIDGenerator.getUUID();
        Mockito.doThrow(LegoCheckedException.class).when(jobManager).queryJob(jobId);

        OpenStackException exception = Assert.assertThrows(OpenStackException.class, () -> adapter.queryJob(jobId));
        assertThat(exception.getErrorCode()).isEqualTo(OpenStackErrorCodes.NOT_FOUND);
        assertThat(exception.getMessage()).isEqualTo(String.format("Job: %s is not exists.", jobId));
    }

    /**
     * 用例名称：查询全部恢复任务时，给定项目id下存在两个资源，每个资源有两个恢复任务，则返回4个任务<br/>
     * 前置条件：1.资源存在；2.存在恢复任务<br/>
     * check点：查询恢复任务时，返回正确的恢复任务数量<br/>
     */
    @Test
    public void should_return4Jobs_when_queryJob_given_twoResourcesHaveTwoJobsEach() throws Exception {
        String projectId = UUIDGenerator.getUUID();
        List<ProtectedResource> resources = new ArrayList<>();
        IntStream.range(0, 2).forEach(i -> {
            ProtectedResource resource = new ProtectedResource();
            resource.setUuid(UUIDGenerator.getUUID());
            resources.add(resource);
        });
        Mockito.when(resourceManager.queryResourcesByProjectId(projectId, false)).thenReturn(resources);

        List<JobBo> jobs = new ArrayList<JobBo>() {{
            add(new JobBo());
            add(new JobBo());
        }};
        Mockito.when(jobManager.queryAllJobs(resources.get(0).getUuid(), JobTypeEnum.RESTORE.getValue()))
            .thenReturn(jobs);
        Mockito.when(jobManager.queryAllJobs(resources.get(1).getUuid(), JobTypeEnum.RESTORE.getValue()))
            .thenReturn(jobs);
        PowerMockito.mockStatic(OpenStackModelsGenerator.class);
        PowerMockito.when(OpenStackModelsGenerator.class, "generateRestoreJob", any())
            .thenReturn(new OpenStackRestoreJobDto());

        List<OpenStackRestoreJobDto> restoreJobs = adapter.queryJobs(projectId);
        assertThat(restoreJobs).hasSize(4);
    }
}

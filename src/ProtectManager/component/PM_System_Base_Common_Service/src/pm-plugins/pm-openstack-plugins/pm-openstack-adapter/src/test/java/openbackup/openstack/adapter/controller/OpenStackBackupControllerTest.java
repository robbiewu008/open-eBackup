package openbackup.openstack.adapter.controller;

import static org.mockito.ArgumentMatchers.anyString;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.delete;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.put;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import openbackup.openstack.adapter.dto.BatchCreateBackupJobDto;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.dto.UpdateBackupJobDto;
import openbackup.openstack.adapter.exception.OpenStackException;
import openbackup.openstack.adapter.exception.OpenStackExceptionHandler;
import openbackup.openstack.adapter.service.OpenStackBackupAdapter;
import openbackup.openstack.adapter.testdata.TestDataGenerator;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;

import org.assertj.core.api.Assertions;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.stream.IntStream;

import javax.validation.ConstraintViolationException;

/**
 * {@link OpenStackBackupController} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-28
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = OpenStackBackupController.class)
@AutoConfigureWebMvc
@AutoConfigureMockMvc
public class OpenStackBackupControllerTest {
    private static final String RESOURCE_ID = "2b0c3d83-9236-4c02-bf3d-0b68ad736081";
    private static final String X_AUTH_TOKE = "X-Auth-Token";
    private static final String MOCK_TOKEN = "mock_token";
    private static final String BASE_URL = "/v2/backup_jobs";
    private static final String SWITCH_JOB_URL = BASE_URL + "/{id}/action";

    private final OpenStackExceptionHandler exceptionHandler = new OpenStackExceptionHandler();

    @Autowired
    private MockMvc mvc;

    @MockBean
    private OpenStackBackupAdapter adapter;

    @Autowired
    private OpenStackBackupController controller;

    /**
     * 用例名称：验证开启备份任务成功后，返回状态码202<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return202_when_requestSwitchJobWithStartAction() throws Exception {
        Mockito.doNothing().when(adapter).startJob(anyString());
        String content = "{\"start\": null}";
        mvc.perform(
                        post(SWITCH_JOB_URL, RESOURCE_ID)
                                .header(X_AUTH_TOKE, MOCK_TOKEN)
                                .contentType(MediaType.APPLICATION_JSON)
                                .content(content))
                .andExpect(status().isAccepted());
        Mockito.verify(adapter, Mockito.times(1)).startJob(anyString());
    }

    /**
     * 用例名称：验证停止备份任务成功后，返回状态码202<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return202_when_requestSwitchJobWithStopAction() throws Exception {
        Mockito.doNothing().when(adapter).startJob(anyString());
        String content = "{\"stop\": null}";
        mvc.perform(
                        post(SWITCH_JOB_URL, RESOURCE_ID)
                                .header(X_AUTH_TOKE, MOCK_TOKEN)
                                .contentType(MediaType.APPLICATION_JSON)
                                .content(content))
                .andExpect(status().isAccepted());
        Mockito.verify(adapter, Mockito.times(1)).stopJob(anyString());
    }

    /**
     * 用例名称：请求开关任务时，如果action不合法，则返回400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：action只能为stop或start<br/>
     */
    @Test
    public void should_return400_when_requestSwitchJob_given_invalidAction() throws Exception {
        Mockito.doNothing().when(adapter).startJob(anyString());
        String content = "{\"error\": null}";
        mvc.perform(
                        post(SWITCH_JOB_URL, RESOURCE_ID)
                                .header(X_AUTH_TOKE, MOCK_TOKEN)
                                .contentType(MediaType.APPLICATION_JSON)
                                .content(content))
                .andExpect(status().isBadRequest())
                .andDo(print());
    }

    /**
     * 用例名称：请求开关任务时，如果action不存在，则抛出ConstraintViolationException<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：action必须存在<br/>
     */
    @Test
    public void should_throwConstraintViolationException_when_requestSwitchJob_given_emptyAction() throws Exception {
        Mockito.doNothing().when(adapter).startJob(anyString());
        String content = "{}";
        Assertions.assertThatThrownBy(
                        () ->
                                mvc.perform(
                                                post(SWITCH_JOB_URL, RESOURCE_ID)
                                                        .header(X_AUTH_TOKE, MOCK_TOKEN)
                                                        .contentType(MediaType.APPLICATION_JSON)
                                                        .content(content))
                                        .andDo(print()))
                .hasCause(
                        new ConstraintViolationException(
                                "switchJob.action: size must be between 1 and 1", Collections.emptySet()));
    }

    /**
     * 用例名称：请求开关任务时，如果action多于1个，则抛出ConstraintViolationException<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：action只能有1个<br/>
     */
    @Test
    public void should_throwConstraintViolationException_when_requestSwitchJob_given_twoAction() throws Exception {
        Mockito.doNothing().when(adapter).startJob(anyString());
        String content = "{\"stop\": null, \"start\": null}";
        Assertions.assertThatThrownBy(
                        () ->
                                mvc.perform(
                                                post(SWITCH_JOB_URL, RESOURCE_ID)
                                                        .header(X_AUTH_TOKE, MOCK_TOKEN)
                                                        .contentType(MediaType.APPLICATION_JSON)
                                                        .content(content))
                                        .andDo(print()))
                .hasCause(
                        new ConstraintViolationException(
                                "switchJob.action: size must be between 1 and 1", Collections.emptySet()));
    }

    /**
     * 用例名称：验证单个备份任务详情成功后，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_queryJob() throws Exception {
        OpenStackBackupJobDto backupJob = new OpenStackBackupJobDto();
        Mockito.when(adapter.queryJob(RESOURCE_ID)).thenReturn(backupJob);
        mvc.perform(get(BASE_URL + "/{id}", RESOURCE_ID).header(X_AUTH_TOKE, MOCK_TOKEN))
            .andDo(print())
            .andExpect(status().isOk());
    }

    /**
     * 用例名称：验证全部备份任务详情成功后，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_queryJobs() throws Exception {
        mvc.perform(get(BASE_URL).requestAttr("projectId", UUIDGenerator.getUUID()))
            .andDo(print())
            .andExpect(status().isOk());
    }

    /**
     * 用例名称：验证删除备份任务成功后，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_deleteJob() throws Exception {
        mvc.perform(delete(BASE_URL + "/{id}", RESOURCE_ID).header(X_AUTH_TOKE, MOCK_TOKEN))
            .andDo(print())
            .andExpect(status().isOk());
    }

    /**
     * 用例名称：验证删除备份任务抛出OpenStackException，且错误码为404时，返回状态码NotFound<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_returnNotFound_when_deleteJob_given_throwOpenStackException() throws Exception {
        mvc = MockMvcBuilders.standaloneSetup(controller).setControllerAdvice(exceptionHandler).build();
        Mockito.doThrow(new OpenStackException(404)).when(adapter).deleteJob(RESOURCE_ID);
        mvc.perform(delete(BASE_URL + "/{id}", RESOURCE_ID).header(X_AUTH_TOKE, MOCK_TOKEN))
            .andDo(print())
            .andExpect(status().isNotFound());
    }

    /**
     * 用例名称：验证创建备份任务时，给定1个OpenStackBackupJobDto，能够请求成功，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_createJob_givenOneBackupJob() throws Exception {
        BatchCreateBackupJobDto req = new BatchCreateBackupJobDto();
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        req.setBackupJobs(Collections.singletonList(backupJob));

        mvc.perform(post(BASE_URL).header(X_AUTH_TOKE, MOCK_TOKEN)
            .contentType(MediaType.APPLICATION_JSON)
            .content(JSONObject.fromObject(req).toString())).andDo(print()).andExpect(status().isCreated());
    }

    /**
     * 用例名称：验证创建备份任务时，给定50个OpenStackBackupJobDto，能够请求成功，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_createJob_given50BackupJobs() throws Exception {
        BatchCreateBackupJobDto req = new BatchCreateBackupJobDto();
        List<OpenStackBackupJobDto> backupJobs = new ArrayList<>();
        IntStream.range(0, 50).forEach(i -> {
            try {
                backupJobs.add(TestDataGenerator.createTimeRetentionDaysScheduleJob());
            } catch (IOException exception) {
                exception.printStackTrace();
            }
        });
        req.setBackupJobs(backupJobs);

        mvc.perform(post(BASE_URL).header(X_AUTH_TOKE, MOCK_TOKEN)
            .contentType(MediaType.APPLICATION_JSON)
            .content(JSONObject.fromObject(req).toString())).andDo(print()).andExpect(status().isCreated());
    }

    /**
     * 用例名称：如果创建备份任务请求体为null，返回状态码400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return400_when_createJob_given_nullBackupJobs() throws Exception {
        mvc.perform(post(BASE_URL).header(X_AUTH_TOKE, MOCK_TOKEN)
            .contentType(MediaType.APPLICATION_JSON)
            .content("{}")).andDo(print()).andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：如果创建备份任务请求体为0，返回状态码400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return400_when_createJob_given_backupJobsOneOpenStackBackupJobDto() throws Exception {
        mvc.perform(post(BASE_URL).header(X_AUTH_TOKE, MOCK_TOKEN)
            .contentType(MediaType.APPLICATION_JSON)
            .content("{\"backup_jobs\": []}")).andDo(print()).andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：如果创建备份任务请求体超过最大值50，返回状态码400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return400_when_createJob_given_backupJobs51OpenStackBackupJobDto() throws Exception {
        BatchCreateBackupJobDto req = new BatchCreateBackupJobDto();
        List<OpenStackBackupJobDto> backupJobs = new ArrayList<>();
        IntStream.range(0, 51).forEach(i -> backupJobs.add(new OpenStackBackupJobDto()));
        req.setBackupJobs(backupJobs);

        mvc.perform(post(BASE_URL).header(X_AUTH_TOKE, MOCK_TOKEN)
            .contentType(MediaType.APPLICATION_JSON)
            .content(JSONObject.fromObject(req).toString())).andDo(print()).andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：更新备份任务请求成功时，返回201<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return201_when_updateJob() throws Exception {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        UpdateBackupJobDto req = new UpdateBackupJobDto();
        req.setBackupJob(backupJob);
        mvc.perform(put(BASE_URL + "/{id}", RESOURCE_ID).contentType(MediaType.APPLICATION_JSON)
            .content(JSONObject.fromObject(req).toString())
            .header(X_AUTH_TOKE, MOCK_TOKEN)).andDo(print()).andExpect(status().isCreated());
    }

    /**
     * 用例名称：更新备份任务时，如果description为空，则返回400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return400_when_updateJob_given_nullDescriptionReq() throws Exception {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        backupJob.setDescription("");
        UpdateBackupJobDto req = new UpdateBackupJobDto();
        req.setBackupJob(backupJob);
        mvc.perform(put(BASE_URL + "/{id}", RESOURCE_ID).contentType(MediaType.APPLICATION_JSON)
            .content(JSONObject.fromObject(req).toString())
            .header(X_AUTH_TOKE, MOCK_TOKEN)).andDo(print()).andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：更新备份任务时，如果daysOfWeek超出范围，则返回400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return400_when_updateJob_given_daysOfWeekOutOfRange() throws Exception {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createWeekScheduleWithoutDaysOfWeekJob();
        backupJob.getJobsSchedule().getPolicy().setDaysOfWeek(Arrays.asList(0, 1));
        UpdateBackupJobDto req = new UpdateBackupJobDto();
        req.setBackupJob(backupJob);
        mvc.perform(put(BASE_URL + "/{id}", RESOURCE_ID).contentType(MediaType.APPLICATION_JSON)
            .content(JSONObject.fromObject(req).toString())
            .header(X_AUTH_TOKE, MOCK_TOKEN)).andDo(print()).andExpect(status().isBadRequest());
    }
}

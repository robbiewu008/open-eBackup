package openbackup.openstack.adapter.controller;

import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import openbackup.openstack.adapter.service.OpenStackRestoreAdapter;
import openbackup.openstack.adapter.testdata.TestDataGenerator;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;

/**
 * {@link OpenStackRestoreController} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-16
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = OpenStackRestoreController.class)
@AutoConfigureWebMvc
@AutoConfigureMockMvc
public class OpenStackRestoreControllerTest {
    private static final String X_AUTH_TOKE = "X-Auth-Token";
    private static final String MOCK_TOKEN = "mock_token";
    private static final String BASE_URL = "/v2/backup_restore";

    @Autowired
    private MockMvc mvc;

    @MockBean
    private OpenStackRestoreAdapter adapter;

    /**
     * 用例名称：验证创建恢复任务成功后，返回状态码202<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return202_when_requestCreateJob() throws Exception {
        OpenStackRestoreJobDto restoreJob = TestDataGenerator.createServerRestoreJob();
        mvc.perform(
                        post(BASE_URL)
                                .contentType(MediaType.APPLICATION_JSON)
                                .content(JSONObject.fromObject(restoreJob).toString()))
                .andDo(print())
                .andExpect(status().isAccepted());
    }

    /**
     * 用例名称：验证创建恢复任务，name为空，返回400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：创建恢复任务请求中，name必须存在<br/>
     */
    @Test
    public void should_return400_when_requestCreateJob_given_emptyNameRequest() throws Exception {
        OpenStackRestoreJobDto restoreJob = TestDataGenerator.createServerRestoreJob();
        restoreJob.setName("");
        mvc.perform(
            post(BASE_URL)
                .contentType(MediaType.APPLICATION_JSON)
                .content(JSONObject.fromObject(restoreJob).toString()))
            .andDo(print())
            .andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：验证创建恢复任务，description为空，返回400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：创建恢复任务请求中，description必须存在<br/>
     */
    @Test
    public void should_return400_when_requestCreateJob_given_nullDescription() throws Exception {
        OpenStackRestoreJobDto restoreJob = TestDataGenerator.createServerRestoreJob();
        restoreJob.setDescription("");
        mvc.perform(
            post(BASE_URL)
                .contentType(MediaType.APPLICATION_JSON)
                .content(JSONObject.fromObject(restoreJob).toString()))
            .andDo(print())
            .andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：验证创建恢复任务，instanceId为空，返回400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：创建恢复任务请求中，instanceId必须存在<br/>
     */
    @Test
    public void should_return400_when_requestCreateJob_given_emptyInstanceId() throws Exception {
        OpenStackRestoreJobDto restoreJob = TestDataGenerator.createServerRestoreJob();
        restoreJob.setInstanceId(null);
        mvc.perform(
            post(BASE_URL)
                .contentType(MediaType.APPLICATION_JSON)
                .content(JSONObject.fromObject(restoreJob).toString()))
            .andDo(print())
            .andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：验证查询恢复任务成功，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_requestQueryJob() throws Exception {
        mvc.perform(get(BASE_URL + "/{id}", UUIDGenerator.getUUID())).andDo(print()).andExpect(status().isOk());
    }

    /**
     * 用例名称：验证查询所有恢复任务成功，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_requestQueryAllJobs() throws Exception {
        mvc.perform(get(BASE_URL, UUIDGenerator.getUUID()).requestAttr("projectId", UUIDGenerator.getUUID()))
            .andDo(print())
            .andExpect(status().isOk());
    }
}

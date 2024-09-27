package openbackup.openstack.adapter.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.get;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import openbackup.openstack.adapter.controller.OpenStackQuotaController;
import openbackup.openstack.adapter.dto.OpenStackQuotaDto;
import openbackup.openstack.adapter.exception.OpenStackException;
import openbackup.openstack.adapter.exception.OpenStackExceptionHandler;
import openbackup.openstack.adapter.service.OpenStackQuotaAdapter;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;

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

/**
 * {@link OpenStackQuotaController} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-28
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = OpenStackQuotaController.class)
@AutoConfigureWebMvc
@AutoConfigureMockMvc
public class OpenStackQuotaControllerTest {
    @Autowired
    private MockMvc mvc;

    @MockBean
    private OpenStackQuotaAdapter adapter;

    @Autowired
    private OpenStackQuotaController controller;

    private final OpenStackExceptionHandler exceptionHandler = new OpenStackExceptionHandler();

    /**
     * 用例名称：验证设置配额成功后，返回状态码201<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return201_when_requestSetQuotaSuccess() throws Exception {
        mvc.perform(post("/v2/backup_quota").requestAttr("projectId", UUIDGenerator.getUUID())
            .contentType(MediaType.APPLICATION_JSON)
            .content("{\"size\": -1}")).andDo(print()).andExpect(status().isCreated());
    }

    /**
     * 用例名称：验证设置配额不在合法范围内，返回状态码400<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return400_when_requestSetQuotaSuccess_given_sizeNegative2() throws Exception {
        mvc.perform(post("/v2/backup_quota").requestAttr("projectId", UUIDGenerator.getUUID())
            .contentType(MediaType.APPLICATION_JSON)
            .content("{\"size\": -2}")).andDo(print()).andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：验证设置配额抛出OpenStackException，且错误码为403时，返回状态码Forbidden<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_returnForbidden_when_deleteJob_given_throwOpenStackException() throws Exception {
        mvc = MockMvcBuilders.standaloneSetup(controller).setControllerAdvice(exceptionHandler).build();

        OpenStackQuotaDto quota = new OpenStackQuotaDto();
        quota.setSize(3);
        Mockito.doThrow(new OpenStackException(403)).when(adapter).setQuota(anyString(), any());
        mvc.perform(post("/v2/backup_quota").requestAttr("projectId", UUIDGenerator.getUUID())
            .contentType(MediaType.APPLICATION_JSON)
            .content(JSONObject.fromObject(quota).toString())).andDo(print()).andExpect(status().isForbidden());
    }

    /**
     * 用例名称：验证查询配额成功后，返回状态码200<br/>
     * 前置条件：spring mvc框架正常运行。<br/>
     * check点：状态码按约定返回<br/>
     */
    @Test
    public void should_return200_when_requestQueryQuotaSuccess() throws Exception {
        mvc.perform(get("/v2/backup_quota").requestAttr("projectId", UUIDGenerator.getUUID()))
            .andDo(print())
            .andExpect(status().isOk());
    }
}

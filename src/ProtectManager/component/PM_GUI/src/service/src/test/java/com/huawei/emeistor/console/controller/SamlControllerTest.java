package com.huawei.emeistor.console.controller;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.emeistor.console.service.SessionService;
import com.huawei.emeistor.console.service.UserService;
import com.huawei.emeistor.console.util.EncryptorRestClient;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.io.IOException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * Smal用户登录接口测试类
 *
 * @author w30042425
 * @since 2023-02-14
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {SamlController.class})
public class SamlControllerTest {

    @MockBean
    private SessionService sessionService;

    @MockBean
    private EncryptorRestClient encryptorRestClient;

    @MockBean
    private UserService userService;

    @Autowired
    private SamlController samlController;

    @Test
    public void test_saml_user_login() throws IOException {
        HttpServletRequest httpServletRequest = PowerMockito.mock(HttpServletRequest.class);
        HttpServletResponse httpServletResponse = PowerMockito.mock(HttpServletResponse.class);
        PowerMockito.doNothing().when(userService).samlLogin(any(), any());
        samlController.samlLogin(httpServletRequest, httpServletResponse);
    }
}

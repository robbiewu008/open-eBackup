package com.huawei.emeistor.console.controller;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.mock.web.MockHttpServletResponse;

import java.io.IOException;

/**
 * 功能描述
 *
 * @author w30042425
 * @since 2023-09-22
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {
    ErrorPageController.class
})
public class ErrorPageControllerTest {
    @InjectMocks
    private ErrorPageController errorPageController;

    @Test
    public void test_not_found() throws IOException {
        MockHttpServletResponse mockHttpServletResponse = new MockHttpServletResponse();
        errorPageController.notFound(mockHttpServletResponse);
    }
}

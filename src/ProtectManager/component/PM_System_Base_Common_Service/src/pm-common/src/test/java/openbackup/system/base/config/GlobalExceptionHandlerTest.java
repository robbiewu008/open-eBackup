/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.config;

import static org.mockito.ArgumentMatchers.anyString;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import com.fasterxml.jackson.databind.exc.InvalidFormatException;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.ErrorResponse;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.config.GlobalExceptionHandler;
import openbackup.system.base.controller.SecretController;
import openbackup.system.base.service.ConfigMapServiceImpl;

import com.fasterxml.jackson.databind.JsonMappingException;

import org.hibernate.validator.internal.engine.path.PathImpl;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureWebMvc;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.MessageSource;
import org.springframework.http.converter.HttpMessageNotReadableException;
import org.springframework.mock.web.MockHttpServletResponse;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.ResultMatcher;
import org.springframework.test.web.servlet.request.MockHttpServletRequestBuilder;
import org.springframework.test.web.servlet.request.MockMvcRequestBuilders;
import org.springframework.test.web.servlet.result.MockMvcResultHandlers;
import org.springframework.test.web.servlet.setup.MockMvcBuilders;

import java.lang.reflect.Field;
import java.util.HashSet;
import java.util.Set;

import javax.validation.ConstraintViolation;
import javax.validation.ConstraintViolationException;

import feign.FeignException;

/**
 * GlobalExceptionHandlerTest
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-29
 */
@RunWith(SpringRunner.class)
@AutoConfigureWebMvc
@AutoConfigureMockMvc
@SpringBootTest(classes = {SecretController.class})
public class GlobalExceptionHandlerTest {
    private static final String URL = "/v1/secret/redis";
    private static final String TEST_MESSAGE = "test message";
    private final GlobalExceptionHandler handler = new GlobalExceptionHandler();

    @Autowired
    private MockMvc mockMvc;

    @Autowired
    private SecretController controller;

    @MockBean
    private ConfigMapServiceImpl configMapService;

    @MockBean
    private MessageSource messageSource;

    @Before
    public void setUp() throws IllegalAccessException {
        mockMvc = MockMvcBuilders.standaloneSetup(controller).setControllerAdvice(handler).build();
        Field messageSourceField = PowerMockito.field(GlobalExceptionHandler.class, "messageSource");
        messageSourceField.set(handler, messageSource);
        PowerMockito.when(messageSource.getMessage(CommonErrorCode.SYSTEM_ERROR + "", null, null)).thenReturn(TEST_MESSAGE);
        PowerMockito.when(messageSource.getMessage(CommonErrorCode.OPERATION_FAILED + "", null, null)).thenReturn(TEST_MESSAGE);
    }

    /**
     * 用例场景：全局捕获异常处理
     * 前置条件：Controller业务抛出LegoCheckedException（已基线的错误码或已知统一错误码）
     * 检查点：切面捕获并处理异常返回ErrorResponse
     */
    @Test
    public void handle_LegoCheckedException_success_when_controller_throw_LegoCheckedException_with_errorCode() throws Exception {
        PowerMockito.doThrow(new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR)).when(configMapService).getValueFromSecretByKey(anyString());
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(URL);
        assetError(requestBuilder, status().is5xxServerError(), CommonErrorCode.SYSTEM_ERROR, TEST_MESSAGE);
    }

    /**
     * 用例场景：全局捕获异常处理
     * 前置条件：Controller业务抛出LegoCheckedException（未知错误码）
     * 检查点：切面捕获并处理异常返回ErrorResponse
     */
    @Test
    public void handle_exception_success_when_controller_throw_LegoCheckedException_with_unknown() throws Exception {
        PowerMockito.doThrow(new LegoCheckedException(-1)).when(configMapService).getValueFromSecretByKey(anyString());
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(URL);
        assetError(requestBuilder, status().is5xxServerError(), CommonErrorCode.OPERATION_FAILED, TEST_MESSAGE);
    }

    /**
     * 用例场景：全局捕获异常处理
     * 前置条件：Controller业务抛出FeignCheckedException
     * 检查点：切面捕获并处理异常返回ErrorResponse
     */
    @Test
    public void handle_exception_success_when_controller_throw_FeignCheckedException() throws Exception {
        FeignException feignException = PowerMockito.mock(FeignException.class);
        PowerMockito.doThrow(feignException).when(configMapService).getValueFromSecretByKey(anyString());
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(URL);
        assetError(requestBuilder, status().is5xxServerError(), CommonErrorCode.OPERATION_FAILED, TEST_MESSAGE);
    }

    /**
     * 用例场景：全局捕获异常处理
     * 前置条件：Controller业务抛出Exception
     * 检查点：切面捕获并处理异常返回ErrorResponse
     */
    @Test
    public void handle_exception_success_when_controller_throw_Exception() throws Exception {
        PowerMockito.doThrow(new RuntimeException()).when(configMapService).getValueFromSecretByKey(anyString());
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(URL);
        assetError(requestBuilder, status().is5xxServerError(), CommonErrorCode.OPERATION_FAILED, TEST_MESSAGE);
    }

    /**
     * 用例场景：全局捕获异常处理
     * 前置条件：Controller业务抛出ConstraintViolationException
     * 检查点：切面捕获并处理异常返回ErrorResponse
     */
    @Test
    public void handle_exception_success_when_controller_throw_ConstraintViolationException() throws Exception {
        ConstraintViolationException violationException = PowerMockito.mock(ConstraintViolationException.class);
        ConstraintViolation<?> violation = PowerMockito.mock(ConstraintViolation.class);
        Set<ConstraintViolation<?>> violations = new HashSet<>();
        String key = "testPath";
        String value = "testPath is invalid";
        PathImpl pathImpl = PathImpl.createPathFromString(key);
        PowerMockito.when(violation.getPropertyPath()).thenReturn(pathImpl);
        PowerMockito.when(violation.getMessage()).thenReturn(value);
        violations.add(violation);
        PowerMockito.when(violationException.getConstraintViolations()).thenReturn(violations);
        PowerMockito.doThrow(violationException).when(configMapService).getValueFromSecretByKey(anyString());
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(URL);
        assetError(requestBuilder, status().isBadRequest(), CommonErrorCode.ERR_PARAM, String.join(": ", key, value));
    }

    /**
     * 用例场景：全局捕获异常处理
     * 前置条件：Controller业务抛出JsonMappingException
     * 检查点：切面捕获并处理异常返回ErrorResponse
     */
    @Test
    public void handle_exception_success_when_controller_throw_JsonMappingException() throws Exception {
        HttpMessageNotReadableException exception = PowerMockito.mock(HttpMessageNotReadableException.class);
        JsonMappingException jsonMappingException = PowerMockito.mock(JsonMappingException.class);
        LegoCheckedException legoCheckedException = PowerMockito.mock(LegoCheckedException.class);
        PowerMockito.doThrow(exception).when(configMapService).getValueFromSecretByKey(anyString());
        PowerMockito.when(exception.getCause()).thenReturn(jsonMappingException);
        PowerMockito.when(jsonMappingException.getCause()).thenReturn(legoCheckedException);
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(URL);
        assetError(requestBuilder, status().is5xxServerError(), CommonErrorCode.ILLEGAL_PARAM, null);
    }

    /**
     * 用例场景：全局捕获异常处理
     * 前置条件：Controller接口层 抛出 HttpMessageNotReadableException（cause为 InvalidFormatException）
     * 检查点：切面捕获并处理异常返回 BAD_REQUEST
     */
    @Test
    public void handle_exception_success_when_controller_throw_HttpMessageNotReadableException() throws Exception {
        HttpMessageNotReadableException exception = PowerMockito.mock(HttpMessageNotReadableException.class);
        InvalidFormatException invalidFormatException = PowerMockito.mock(InvalidFormatException.class);
        PowerMockito.doThrow(exception).when(configMapService).getValueFromSecretByKey(anyString());
        PowerMockito.when(exception.getCause()).thenReturn(invalidFormatException);
        MockHttpServletRequestBuilder requestBuilder = MockMvcRequestBuilders.get(URL);

        MockHttpServletResponse response = mockMvc.perform(requestBuilder)
                .andDo(MockMvcResultHandlers.print()).andReturn().getResponse();
        Assert.assertEquals(400, response.getStatus());
    }

    private void assetError(MockHttpServletRequestBuilder requestBuilder, ResultMatcher matcher, long errorCode, String message) throws Exception {
        MockHttpServletResponse response = mockMvc.perform(requestBuilder)
            .andExpect(matcher).andDo(MockMvcResultHandlers.print()).andReturn().getResponse();
        ErrorResponse errorResponse = JSONObject.cast(response.getContentAsString(), ErrorResponse.class);
        Assert.assertEquals(errorCode + "", errorResponse.getErrorCode());
        Assert.assertEquals(message, errorResponse.getErrorMessage());
    }
}

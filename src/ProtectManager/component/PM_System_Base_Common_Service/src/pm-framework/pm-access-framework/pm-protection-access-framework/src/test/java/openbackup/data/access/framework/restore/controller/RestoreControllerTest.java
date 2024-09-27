/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.access.framework.restore.controller;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;
import static org.springframework.test.web.servlet.request.MockMvcRequestBuilders.post;
import static org.springframework.test.web.servlet.result.MockMvcResultHandlers.print;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.jsonPath;
import static org.springframework.test.web.servlet.result.MockMvcResultMatchers.status;

import openbackup.data.access.framework.protection.controller.v2.BaseControllerTest;
import openbackup.data.access.framework.protection.mocks.CreateRestoreTaskRequestMocker;

import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.access.framework.restore.service.RestoreTaskManager;
import openbackup.data.access.framework.restore.validator.CreateRestoreTaskRequestValidator;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.util.SpringBeanUtils;

import org.apache.commons.lang3.RandomStringUtils;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.http.MediaType;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.test.web.servlet.MockMvc;
import org.springframework.test.web.servlet.ResultActions;

import java.util.UUID;

/**
 * RestoreController 的单元测试用例集合
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/11/30
 **/
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {RestoreController.class, CreateRestoreTaskRequestValidator.class, SpringBeanUtils.class})
@MockBean(classes = {CopyRestApi.class})
public class RestoreControllerTest extends BaseControllerTest {

    public static final String RESTORE_V2_REST_PATH = "/v2/restore/jobs";

    @Autowired
    private MockMvc mockMvc;

    @MockBean
    private RestoreTaskManager restoreTaskManager;

    @Before
    public void init(){
        given(restoreTaskManager.init(any())).willReturn(UUID.randomUUID().toString());
    }

    /**
     * 用例名称：验证创建恢复REST接口调用成功<br/>
     * 前置条件：无<br/>
     * check点：rest接口响应码是200。<br/>
     */
    @Test
    public void should_success_when_create_restore_task_given_request_correct() throws Exception {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        final String requestJson = CreateRestoreTaskRequestMocker.toJsonString(mockRequest);
        // When
        ResultActions resultActions = mockMvc.perform(
            post(RESTORE_V2_REST_PATH).contentType(MediaType.APPLICATION_JSON_VALUE).content(requestJson).characterEncoding("UTF-8")).andDo(print());
        // Then
        resultActions.andExpect(status().isOk()).andExpect(jsonPath("$.uuid").exists());
    }

    /**
     * 用例名称：当请求参数为空时，验证创建恢复REST接口调用失败<br/>
     * 前置条件：无<br/>
     * check点：rest接口响应码是400。<br/>
     */
    @Test
    public void should_bad_request_when_create_restore_task_given_request_empty() throws Exception {
        // Given
        String requestJson = "{\n" + "\n" + "}";
        // When
        ResultActions resultActions = mockMvc.perform(
            post(RESTORE_V2_REST_PATH).contentType(MediaType.APPLICATION_JSON_VALUE).content(requestJson));
        // Then
        resultActions.andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：当请求参数copyId长度为65时，验证创建恢复REST接口调用失败<br/>
     * 前置条件：无<br/>
     * check点：rest接口响应码是400。<br/>
     */
    @Test
    public void should_bad_request_when_create_restore_task_given_param_copyId_length_is_65() throws Exception {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        mockRequest.setCopyId(RandomStringUtils.random(65));
        final String requestJson = CreateRestoreTaskRequestMocker.toJsonString(mockRequest);
        // When
        ResultActions resultActions = mockMvc.perform(
            post(RESTORE_V2_REST_PATH).contentType(MediaType.APPLICATION_JSON_VALUE).content(requestJson));
        // Then
        resultActions.andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：当请求参数copyId长度为31时，验证创建恢复REST接口调用失败<br/>
     * 前置条件：无<br/>
     * check点：rest接口响应码是400。<br/>
     */
    @Test
    public void should_bad_request_when_create_restore_task_given_param_copyId_length_is_31() throws Exception {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        mockRequest.setCopyId(RandomStringUtils.random(31));
        final String requestJson = CreateRestoreTaskRequestMocker.toJsonString(mockRequest);
        // When
        ResultActions resultActions = mockMvc.perform(
            post(RESTORE_V2_REST_PATH).contentType(MediaType.APPLICATION_JSON_VALUE).content(requestJson));
        // Then
        resultActions.andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：当请求参数targetEnv长度为65时，验证创建恢复REST接口调用失败<br/>
     * 前置条件：无<br/>
     * check点：rest接口响应码是400。<br/>
     */
    @Test
    public void should_bad_request_when_create_restore_task_given_param_targetEnv_length_is_65() throws Exception {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        mockRequest.setTargetEnv(RandomStringUtils.random(65));
        final String requestJson = CreateRestoreTaskRequestMocker.toJsonString(mockRequest);
        // When
        ResultActions resultActions = mockMvc.perform(
            post(RESTORE_V2_REST_PATH).contentType(MediaType.APPLICATION_JSON_VALUE).content(requestJson));
        // Then
        resultActions.andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：当请求参数targetEnv长度为31时，验证创建恢复REST接口调用失败<br/>
     * 前置条件：无<br/>
     * check点：rest接口响应码是400。<br/>
     */
    @Test
    public void should_bad_request_when_create_restore_task_given_param_targetEnv_length_is_0() throws Exception {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        mockRequest.setTargetEnv("");
        final String requestJson = CreateRestoreTaskRequestMocker.toJsonString(mockRequest);
        // When
        ResultActions resultActions = mockMvc.perform(
            post(RESTORE_V2_REST_PATH).contentType(MediaType.APPLICATION_JSON_VALUE).content(requestJson));
        // Then
        resultActions.andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：当请求参数targetObject长度为257时，验证创建恢复REST接口调用失败<br/>
     * 前置条件：无<br/>
     * check点：rest接口响应码是400。<br/>
     */
    @Test
    public void should_bad_request_when_create_restore_task_given_param_targetObject_length_is_257() throws Exception {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        mockRequest.setTargetObject(RandomStringUtils.random(257));
        final String requestJson = CreateRestoreTaskRequestMocker.toJsonString(mockRequest);
        // When
        ResultActions resultActions = mockMvc.perform(
            post(RESTORE_V2_REST_PATH).contentType(MediaType.APPLICATION_JSON_VALUE).content(requestJson));
        // Then
        resultActions.andExpect(status().isBadRequest());
    }

    /**
     * 用例名称：当请求参数targetObject长度为1时，验证创建恢复REST接口调用失败<br/>
     * 前置条件：无<br/>
     * check点：rest接口响应码是400。<br/>
     */
    @Test
    public void should_bad_request_when_create_restore_task_given_param_targetObject_length_is_0() throws Exception {
        // Given
        final CreateRestoreTaskRequest mockRequest = CreateRestoreTaskRequestMocker.mockSuccessRequest();
        mockRequest.setTargetObject("");
        final String requestJson = CreateRestoreTaskRequestMocker.toJsonString(mockRequest);
        // When
        ResultActions resultActions = mockMvc.perform(
            post(RESTORE_V2_REST_PATH).contentType(MediaType.APPLICATION_JSON_VALUE).content(requestJson));
        // Then
        resultActions.andExpect(status().isBadRequest());
    }
}
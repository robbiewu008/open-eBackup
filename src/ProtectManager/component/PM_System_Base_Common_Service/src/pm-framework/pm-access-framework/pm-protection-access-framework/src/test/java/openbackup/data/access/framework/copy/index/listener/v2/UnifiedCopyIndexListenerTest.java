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
package openbackup.data.access.framework.copy.index.listener.v2;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.copy.index.listener.v2.UnifiedCopyIndexListener;
import openbackup.data.access.framework.copy.index.service.impl.UnifiedCopyIndexService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * 统一框架副本索引开始监听器测试类
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {UnifiedCopyIndexListener.class})
public class UnifiedCopyIndexListenerTest {
    private static String copyIndexMessage = "{\"msg_id\": \"f0eec040-c0b4-4188-b5c9-795c9902a44a\", \"request_id\": \"9ad5df6f-64ec-41ea-b3e0-3b0f930edd55\", \"default_publish_topic\": \"copy.save.event\", \"response_topic\": \"\", \"copy_id\": \"fb62cbea-dc5d-4903-b409-07d49cb9dd78\", \"gen_index\": \"manual\"}";

    private static String copyIndexCopyIdIsBlank = "{\"msg_id\": \"f0eec040-c0b4-4188-b5c9-795c9902a44a\", \"request_id\": \"9ad5df6f-64ec-41ea-b3e0-3b0f930edd55\", \"default_publish_topic\": \"copy.save.event\", \"response_topic\": \"\", \"copy_id\": \"\", \"gen_index\": \"manual\"}";

    private Acknowledgment acknowledgment;

    @InjectMocks
    @Autowired
    private UnifiedCopyIndexListener unifiedCopyIndexListener;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private UnifiedCopyIndexService unifiedCopyIndexService;

    @Before
    public void setUp() {
        acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
    }

    /**
     * 用例场景：正常执行索引方法
     * 前置条件：接收到索引监听消息
     * 检查点：无异常抛出
     */
    @Test
    public void execute_index_start_success() {
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(new Copy());
        unifiedCopyIndexListener.indexStart(copyIndexMessage, acknowledgment);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(any());
    }

    /**
     * 用例场景：正常执行索引方法
     * 前置条件：副本id为空
     * 检查点：无异常抛出
     */
    @Test
    public void should_return_if_copy_id_is_blank_when_execute_index_start() {
        unifiedCopyIndexListener.indexStart(copyIndexCopyIdIsBlank, acknowledgment);
        Assert.assertNotNull(copyIndexCopyIdIsBlank);
    }

    /**
     * 用例场景：正常执行索引方法
     * 前置条件：副本为空
     * 检查点：无异常抛出
     */
    @Test
    public void should_return_if_copy_is_null_when_execute_index_start() {
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(null);
        unifiedCopyIndexListener.indexStart(copyIndexMessage, acknowledgment);
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(any());
    }
}
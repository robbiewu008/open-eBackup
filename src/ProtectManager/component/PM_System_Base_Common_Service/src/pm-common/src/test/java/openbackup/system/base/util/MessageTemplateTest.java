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
package openbackup.system.base.util;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.util.MessageCallback;
import openbackup.system.base.util.MessageTemplate;

import org.checkerframework.checker.units.qual.K;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.kafka.core.KafkaTemplate;

/**
 * Message Template Test
 *
 */
@RunWith(MockitoJUnitRunner.class)
@SpringBootTest(classes = {MessageTemplate.class, KafkaTemplate.class, MessageCallback.class})
public class MessageTemplateTest {

    private final String topic = "replication.completed";

    @Autowired
    @InjectMocks
    MessageTemplate messageTemplate;

    @Autowired
    @Mock
    KafkaTemplate<K, String> kafkaTemplate;

    @Autowired
    @Mock
    MessageCallback<K, String> callback;

    /**
     * 测试Send方法
     */
    @Test
    public void testSendJ(){
        JSONObject jsonObject = new JSONObject();
        messageTemplate.send(topic, jsonObject);
    }

    /**
     * 测试Send方法
     */
    @Test
    public void testSend(){
        String data = "ok";
        messageTemplate.send(topic, data);
    }

}

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
 * @author twx1009756
 * @since 2021-03-17
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

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.util;

import openbackup.system.base.service.SensitiveDataEliminateService;
import openbackup.system.base.util.MessageCallback;

import org.apache.kafka.clients.producer.ProducerRecord;
import org.apache.kafka.clients.producer.RecordMetadata;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

/**
 * EnumUtil Test
 *
 * @author twx1009756
 * @since 2021-03-17
 */
@RunWith(MockitoJUnitRunner.class)
@SpringBootTest(classes = {
    MessageCallback.class, Exception.class, ProducerRecord.class,
    RecordMetadata.class, SensitiveDataEliminateService.class})
public class MessageCallbackTest {

    @Autowired
    @InjectMocks
    MessageCallback messageCallback;

    @Autowired
    @Mock
    Exception exception;

    @Autowired
    @Mock
    ProducerRecord producerRecord;

    @Autowired
    RecordMetadata recordMetadata;

    @Autowired
    @Mock
    private SensitiveDataEliminateService sensitiveDataEliminateService;

    /**
     * 测试OnSuccess方法
     */
    @Test
    public void testOnSuccess(){
        messageCallback.onSuccess(producerRecord, recordMetadata);
    }

    /**
     * 测试OnError方法
     */
    @Test
    public void testOnError(){
        messageCallback.onError(producerRecord, recordMetadata, new RuntimeException());
    }}

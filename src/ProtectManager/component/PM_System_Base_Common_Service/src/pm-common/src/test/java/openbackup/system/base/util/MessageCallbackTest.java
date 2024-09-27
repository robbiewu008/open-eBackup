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

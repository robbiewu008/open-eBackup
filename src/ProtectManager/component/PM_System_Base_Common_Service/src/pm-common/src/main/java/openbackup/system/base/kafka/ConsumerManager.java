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
package openbackup.system.base.kafka;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.CommandLineRunner;
import org.springframework.kafka.config.KafkaListenerEndpointRegistry;
import org.springframework.kafka.listener.MessageListenerContainer;
import org.springframework.stereotype.Component;

import java.util.Collection;

/**
 * InitConsumerManager
 *
 */
@Slf4j
@Component
public class ConsumerManager implements CommandLineRunner {
    @Autowired
    private KafkaListenerEndpointRegistry kafkaListenerEndpointRegistry;

    /**
     * Callback used to run the bean.
     *
     * @param args incoming main method arguments
     */
    @Override
    public void run(String... args) {
        log.info("ready to start all listener Container");
        Collection<MessageListenerContainer> allListenerContainers
            = kafkaListenerEndpointRegistry.getAllListenerContainers();
        for (MessageListenerContainer allListenerContainer : allListenerContainers) {
            String[] topics = allListenerContainer.getContainerProperties().getTopics();
            log.debug("prepare ready start listener Container topic:{}, status:{}", topics == null ? "" : topics[0],
                allListenerContainer.isRunning());
            if (allListenerContainer.isRunning()) {
                continue;
            }
            allListenerContainer.start();
            allListenerContainer.resume();
        }
        log.info("already start all listener Container");
    }
}

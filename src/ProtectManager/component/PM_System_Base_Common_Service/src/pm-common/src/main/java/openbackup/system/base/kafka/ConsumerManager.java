/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
 * @author swx1010572
 * @since 2022-03-15
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

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

import static openbackup.system.base.kafka.annotations.MessageListener.RETRY_FACTORY;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.util.ProviderRegistry;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

/**
 * Message Listener
 *
 */
@Component
public class MessageTestListener {
    @Autowired
    private ProviderRegistry providerRegistry;

    /**
     * throw lego checked exception
     *
     * @param message message
     * @param acknowledgment acknowledgment
     */
    @MessageListener(
            topics = "xxx",
            containerFactory = "retryFactory",
            log = {"job_log_environment_scan_label", MessageListener.STEP_STATUS},
            terminatedMessage = true)
    public void throwLegoCheckedException(String message, Acknowledgment acknowledgment) {
        throw new LegoCheckedException("mock error. message: " + message);
    }

    /**
     * return topic message
     *
     * @param message message
     * @param acknowledgment acknowledgment
     * @return topic message
     */
    @MessageListener(
            topics = "step_topic",
            containerFactory = RETRY_FACTORY,
            failures = "final_topic",
            log = {"job_log_live_mount_copy_clone_label", "job_status_{payload.job_status|status}_label"},
            unlock = true)
    public JSONObject returnTopicMessage(String message, Acknowledgment acknowledgment) {
        providerRegistry.findProvider(MessageTestApplicable.class, "");
        return new MessageObject("next_topic", new JSONObject());
    }
}

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

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.job.model.JobStatusEnum;

/**
 * Message Object
 *
 */
public class MessageObject extends JSONObject {
    /**
     * TOPIC
     */
    public static final String TOPIC = "topic";

    /**
     * MESSAGE
     */
    public static final String MESSAGE = "message";

    /**
     * SUCCESS
     */
    public static final String SUCCESS = "success";

    /**
     * status
     */
    public static final String STATUS = "status";

    /**
     * job status
     */
    public static final String JOB_STATUS = "job_status";

    /**
     * default constructor
     */
    public MessageObject() {
        super();
    }

    /**
     * Message Object
     *
     * @param topic topic
     * @param message message
     */
    public MessageObject(String topic, JSONObject message) {
        this();
        init(topic, message, null, null);
    }

    /**
     * Message Object
     *
     * @param topic topic
     * @param message message
     * @param status status
     * @param jobStatus job status
     */
    public MessageObject(String topic, JSONObject message, String status, String jobStatus) {
        this();
        init(topic, message, status, jobStatus);
    }

    /**
     * Message Object
     *
     * @param message message
     * @param status status
     * @param jobStatus job status
     */
    public MessageObject(JSONObject message, String status, String jobStatus) {
        this();
        init(null, message, status, jobStatus);
    }

    /**
     * init method
     *
     * @param topic topic
     * @param message message
     * @param status status
     * @param jobStatus job status
     * @return message object
     */
    public final MessageObject init(String topic, JSONObject message, String status, String jobStatus) {
        if (!VerifyUtil.isEmpty(topic)) {
            set(TOPIC, topic);
        }
        if (!VerifyUtil.isEmpty(message)) {
            set(MESSAGE, message);
        }
        if (!VerifyUtil.isEmpty(status)) {
            set(STATUS, status);
        }
        if (!VerifyUtil.isEmpty(jobStatus)) {
            set(JOB_STATUS, jobStatus);
        }
        return this;
    }

    /**
     * status
     *
     * @param statusEnum status
     * @return message object
     */
    public MessageObject status(JobStatusEnum statusEnum) {
        return status(statusEnum.name());
    }

    /**
     * status
     *
     * @param status status
     * @return message object
     */
    public MessageObject status(String status) {
        set(STATUS, status);
        return this;
    }
}

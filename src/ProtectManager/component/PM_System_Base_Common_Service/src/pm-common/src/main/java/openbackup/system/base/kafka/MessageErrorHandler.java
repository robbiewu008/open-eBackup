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

import openbackup.system.base.util.Applicable;

import java.util.Collections;
import java.util.Map;

/**
 * Message Error Handler
 *
 */
public interface MessageErrorHandler extends Applicable<Throwable> {
    /**
     * retryable exceptions
     *
     * @return retryable exceptions
     */
    default Map<Class<? extends Throwable>, Boolean> retryableExceptions() {
        return Collections.singletonMap(Throwable.class, Boolean.TRUE);
    }

    /**
     * handle error message
     *
     * @param topic topic
     * @param message message
     * @param throwable throwable
     */
    void handle(String topic, String message, Throwable throwable);
}

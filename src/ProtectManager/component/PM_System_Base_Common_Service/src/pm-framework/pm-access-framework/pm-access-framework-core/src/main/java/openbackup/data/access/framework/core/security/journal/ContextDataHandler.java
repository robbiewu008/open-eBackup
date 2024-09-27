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
package openbackup.data.access.framework.core.security.journal;

import java.util.List;
import java.util.function.Consumer;

/**
 * Define Cache Callback
 *
 * @author l00272247
 * @since 2021-12-14
 */
public interface ContextDataHandler {
    /**
     * handler method
     *
     * @param loggingContexts logging context
     * @param callback callback
     * @return result
     * @throws Throwable throwable
     */
    Object handle(List<LoggingContext> loggingContexts, Consumer<Object> callback) throws Throwable;
}

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

import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Logging Context
 *
 */
public class LoggingContext {
    /**
     * EVALUATIONS
     */
    public static final String EVALUATIONS = "evaluations";

    /**
     * PARAMS
     */
    public static final String PARAMS = "params";

    /**
     * CONTEXT_REGISTRATIONS
     */
    public static final String CONTEXT_REGISTRATIONS = "contextRegistrations";

    private final List<?> args;
    private final Map<String, Object> data;

    /**
     * constructor
     *
     * @param args args
     */
    public LoggingContext(List<?> args) {
        this.args = args;
        data = new HashMap<>();
    }

    public List<?> getArgs() {
        return args;
    }

    /**
     * set value by field
     *
     * @param field field
     * @param value value
     * @param <T> template type T
     * @return context object
     */
    public <T> LoggingContext set(String field, T value) {
        data.put(field, value);
        return this;
    }

    /**
     * get value by field
     *
     * @param field field
     * @param <T> template type
     * @return value
     */
    public <T> T get(String field) {
        return (T) data.get(field);
    }
}

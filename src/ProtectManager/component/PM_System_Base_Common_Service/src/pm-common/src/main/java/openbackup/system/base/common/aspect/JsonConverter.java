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
package openbackup.system.base.common.aspect;

import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import org.springframework.stereotype.Component;

import java.util.Collection;

/**
 * Json Converter
 *
 */
@Component
public class JsonConverter extends AbstractConverter {
    /**
     * constructor
     */
    public JsonConverter() {
        super("json");
    }

    /**
     * data cast
     *
     * @param data data
     * @return result
     */
    @Override
    protected Object cast(Object data) {
        if (data == null) {
            return null;
        }
        if (data instanceof Collection || data.getClass().isArray()) {
            return JSONArray.fromObject(data);
        }
        return JSONObject.fromObject(data);
    }
}

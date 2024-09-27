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

import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.function.Supplier;

/**
 * Redis Context Service
 *
 * @author l00272247
 * @since 2020-10-09
 */
@Component
public class RedisContextService {
    @Autowired
    private RedissonClient redissonClient;

    private final ThreadLocal<String> local = new ThreadLocal<>();

    /**
     * get value
     *
     * @param key key
     * @return value
     */
    public String get(String key) {
        String requestId = local.get();
        if (requestId == null) {
            local.remove();
            return null;
        }
        RMap<String, String> map = getMap(requestId);
        return map.get(key);
    }

    /**
     * set value
     *
     * @param key   key
     * @param value value
     */
    public void set(String key, String value) {
        String requestId = local.get();
        if (requestId == null) {
            local.remove();
            return;
        }
        RMap<String, String> map = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        if (value != null) {
            map.put(key, value);
        } else {
            map.remove(key);
        }
    }

    /**
     * set value
     *
     * @param key   key
     * @param value value
     */
    public void set(String key, Object value) {
        if (value == null) {
            return;
        }
        if (value instanceof String) {
            set(key, (String) value);
        } else if (canStringify(value)) {
            set(key, value.toString());
        } else {
            set(key, JSONObject.fromObject(value).toString());
        }
    }

    /**
     * set data to context
     *
     * @param data data
     */
    public void set(JSONObject data) {
        String requestId = data.getString("request_id");
        if (requestId != null) {
            run(() -> update(data), requestId);
        }
    }

    private void update(JSONObject data) {
        data.forEach(
                (key, value) -> {
                    if (!"request_id".equals(key)) {
                        set(key.toString(), value);
                    }
                });
    }

    private boolean canStringify(Object value) {
        return value instanceof JSONObject
                || value instanceof JSONArray
                || value.getClass().isPrimitive()
                || value.getClass().getPackage().equals(Object.class.getPackage());
    }

    /**
     * run method
     *
     * @param requestId request id
     * @param supplier  supplier
     * @param data      data
     * @param <T>       template type
     * @return result
     */
    public <T> T run(Supplier<T> supplier, String requestId, List<JSONObject> data) {
        String oldRequestId = local.get();
        local.set(requestId);
        try {
            for (JSONObject json : data) {
                update(json);
            }
            return supplier.get();
        } finally {
            if (oldRequestId != null) {
                local.set(oldRequestId);
            } else {
                local.remove();
            }
        }
    }

    /**
     * update context data
     *
     * @param requestId request id
     * @param data      data
     */
    public void update(String requestId, List<JSONObject> data) {
        run(() -> null, requestId, data);
    }

    /**
     * update context data
     *
     * @param requestId request id
     * @param key       key
     * @param value     value
     */
    public void updateStringValue(String requestId, String key, String value) {
        run(() -> set(key, value), requestId);
    }

    /**
     * update context data
     *
     * @param requestId request id
     * @param key       key
     * @param data      data
     */
    public void update(String requestId, String key, JSONObject data) {
        String value = data != null ? data.toString() : null;
        run(() -> set(key, value), requestId);
    }

    /**
     * run method
     *
     * @param requestId request id
     * @param runnable  runnable
     */
    public void run(Runnable runnable, String requestId) {
        run(runnable, requestId, new ArrayList<>());
    }

    /**
     * run method
     *
     * @param requestId request id
     * @param runnable  runnable
     * @param data      data
     */
    public void run(Runnable runnable, String requestId, List<JSONObject> data) {
        run(
                () -> {
                    runnable.run();
                    return null;
                },
                requestId,
                data);
    }

    /**
     * delete context data by request id.
     *
     * @param requestId request id
     */
    public void delete(String requestId) {
        getMap(requestId).delete();
    }

    private RMap<String, String> getMap(String requestId) {
        return redissonClient.getMap(requestId, StringCodec.INSTANCE);
    }
}

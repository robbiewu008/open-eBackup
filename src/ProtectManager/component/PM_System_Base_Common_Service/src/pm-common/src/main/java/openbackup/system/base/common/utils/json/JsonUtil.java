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
package openbackup.system.base.common.utils.json;

import openbackup.system.base.common.exception.DataMoverCheckedException;
import openbackup.system.base.common.exception.DataMoverErrorCode;

import com.alibaba.fastjson.JSONException;
import com.alibaba.fastjson.JSONObject;
import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.json.JsonSanitizer;

import lombok.extern.slf4j.Slf4j;

import java.io.IOException;
import java.io.InputStream;

/**
 * Json Util
 *
 */
@Slf4j
public class JsonUtil {
    /**
     * Object Mapper
     */
    public static final ObjectMapper MAPPER = new ObjectMapper();

    static {
        MAPPER.setSerializationInclusion(JsonInclude.Include.NON_NULL);
        MAPPER.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
    }

    /**
     * cast data to another type
     *
     * @param data data
     * @param type type
     * @param <T> template type
     * @return result
     */
    public static <T> T cast(Object data, Class<T> type) {
        return read(json(data), type);
    }

    /**
     * read json data as the special type data
     *
     * @param json json data
     * @param type type
     * @param <T> template type
     * @return result
     */
    public static <T> T read(String json, Class<T> type) {
        Object object;
        try {
            object = MAPPER.readValue(JsonSanitizer.sanitize(json), new SimpleTypeReference(type));
        } catch (JsonProcessingException e) {
            throw new DataMoverCheckedException(e, DataMoverErrorCode.OPERATION_FAILED);
        }
        return type.cast(object);
    }

    /**
     * cast data to json
     *
     * @param data data
     * @return json string
     */
    public static String json(Object data) {
        try {
            return MAPPER.writeValueAsString(data);
        } catch (JsonProcessingException e) {
            throw new DataMoverCheckedException(e, DataMoverErrorCode.OPERATION_FAILED);
        }
    }

    /**
     * cast inputStream to jsonNode
     *
     * @param inputStream inputStream
     * @return JsonNode
     */
    public static JsonNode read(InputStream inputStream) {
        try {
            return MAPPER.readTree(inputStream);
        } catch (IOException e) {
            throw new DataMoverCheckedException(e, DataMoverErrorCode.OPERATION_FAILED);
        } finally {
            if (inputStream != null) {
                try {
                    inputStream.close();
                } catch (IOException e) {
                    log.error("close inputStream occurs error.");
                }
            }
        }
    }

    /**
     * read json data as the special typeReference data
     *
     * @param object object
     * @param typeReference typeReference
     * @param <T> template type
     * @return result
     */
    public static <T> T read(Object object, TypeReference<T> typeReference) {
        try {
            if (object instanceof String) {
                return MAPPER.readValue((String) object, typeReference);
            }
            return MAPPER.readValue(json(object), typeReference);
        } catch (JsonProcessingException e) {
            throw new DataMoverCheckedException(e, DataMoverErrorCode.OPERATION_FAILED);
        }
    }

    /**
     * 根据 json Str 获取 JsonObject
     *
     * @param jsonStr jsonStr
     * @return JSONObject
     */
    public static JSONObject getJsonObjectFromStr(String jsonStr) {
        try {
            return JSONObject.parseObject(jsonStr);
        } catch (JSONException e) {
            throw new DataMoverCheckedException(e, DataMoverErrorCode.OPERATION_FAILED);
        }
    }
}

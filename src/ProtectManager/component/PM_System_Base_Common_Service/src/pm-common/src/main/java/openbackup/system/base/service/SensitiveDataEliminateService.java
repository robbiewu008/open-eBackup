/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.service;

import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.core.JsonProcessingException;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * Sensitive Data Eliminate Service
 *
 * @author l00272247
 * @since 2021-03-06
 */
@Component
public class SensitiveDataEliminateService {
    @Value("#{'${security.sensitive.fields}'.split(',')}")
    private List<String> sensitiveFields;

    /**
     * eliminate sensitive data of value，affect source data default is true
     *
     * @param value value
     * @param patterns patterns
     * @param <T> template type
     * @return result
     */
    public <T> T eliminate(T value, String... patterns) {
        return eliminate(value, true,
                Optional.ofNullable(patterns).map(Arrays::asList).orElse(Collections.emptyList()));
    }

    /**
     * eliminate sensitive data of value
     *
     * @param value value
     * @param isAffectSourceData is affect source data
     * @param patterns patterns
     * @param <T> template type
     * @return result
     */
    public <T> T eliminate(T value, boolean isAffectSourceData, String... patterns) {
        return eliminate(value, isAffectSourceData,
                Optional.ofNullable(patterns).map(Arrays::asList).orElse(Collections.emptyList()));
    }

    /**
     * eliminate sensitive data of value，affect source data default is true
     *
     * @param value value
     * @param patterns patterns
     * @param <T> template type
     * @return result
     */
    public <T> T eliminate(T value, List<String> patterns) {
        return eliminate(value, true, patterns);
    }

    /**
     * eliminate sensitive data of value
     *
     * @param value value
     * @param isAffectSourceData is affect source data
     * @param patterns patterns
     * @param <T> template type
     * @return result
     */
    public <T> T eliminate(T value, boolean isAffectSourceData, List<String> patterns) {
        Object result;
        if (value instanceof JSONObject) {
            JSONObject tempValue = isAffectSourceData ? (JSONObject) value : ((JSONObject) value).duplicate();
            eliminateJsonObject(tempValue, isAffectSourceData, patterns);
            result = tempValue;
        } else if (value instanceof JSONArray) {
            JSONArray tempValue = isAffectSourceData ? (JSONArray) value : new JSONArray(new Object[]{value});
            eliminateJsonArray(tempValue, isAffectSourceData, patterns);
            result = tempValue;
        } else if (value instanceof String) {
            result = eliminateString((String) value, isAffectSourceData, patterns);
        } else {
            result = value;
        }
        return (T) result;
    }

    /**
     * eliminate sensitive data of url
     *
     * @param url url
     * @return result
     */
    public String eliminateUrl(String url) {
        return eliminateUrl(url, Collections.emptyList());
    }

    /**
     * eliminate sensitive data of url
     *
     * @param url url
     * @param patterns patterns
     * @return result
     */
    public String eliminateUrl(String url, List<String> patterns) {
        if (StringUtils.isEmpty(url)) {
            return "";
        }
        String[] urlArray = url.split("\\?");
        JSONObject paramJsonObject = new JSONObject();
        JSONObject jsonObject = new JSONObject().set("url", urlArray[0]).set("param", paramJsonObject);
        if (urlArray.length <= 1) {
            return url;
        }
        Arrays.stream(urlArray[1].split("&")).forEach(param -> {
            String[] paramArray = param.split("=");
            paramJsonObject.set(paramArray[0],
                    String.join("=", Arrays.stream(paramArray).skip(1).collect(Collectors.toList())));
        });
        eliminate(jsonObject, patterns);
        StringBuilder stringBuilder = new StringBuilder().append(urlArray[0]).append("?");
        paramJsonObject.forEach((key, value) -> stringBuilder.append("&").append(key).append("=").append(value));
        return stringBuilder.toString().replaceFirst("&", "").replace("&=&", "&&");
    }

    private void eliminateJsonObject(JSONObject data, boolean isAffectSourceData, List<String> fields) {
        List<String> patterns = new ArrayList<>(fields);
        patterns.addAll(Optional.ofNullable(sensitiveFields).orElse(Collections.emptyList()));
        ((Map<?, ?>) data).keySet().removeIf(field -> isSensitiveField(field.toString(), patterns));
        for (Object key : data.keySet()) {
            Object value = data.get(key);
            Object result = eliminate(value, isAffectSourceData, fields);
            if (result != value) {
                data.set(key, result);
            }
        }
    }

    private boolean isSensitiveField(String field, List<String> patterns) {
        return patterns.stream().anyMatch(pattern -> isSensitiveField(field.trim(), pattern));
    }

    private boolean isSensitiveField(String field, String pattern) {
        if (pattern.startsWith("@")) {
            return pattern.substring(1).equalsIgnoreCase(field);
        } else if (pattern.contains("%")) {
            String regexp = pattern.replaceAll("%", ".*");
            return Pattern.compile(regexp, Pattern.CASE_INSENSITIVE).matcher(field).find();
        } else {
            return pattern.equalsIgnoreCase(field);
        }
    }

    private String eliminateString(String content, boolean isAffectSourceData, List<String> fields) {
        Object object = parseAsJson(content);
        if (object == null || object instanceof String) {
            return content;
        }
        return eliminate(object, isAffectSourceData, fields).toString();
    }

    private Object parseAsJson(String content) {
        try {
            Object result = JSONObject.DEFAULT_OBJ_MAPPER.readValue(content, Map.class);
            if (result == null) {
                return null;
            }
            return JSONObject.fromObject(content);
        } catch (JsonProcessingException e) {
            try {
                JSONObject.DEFAULT_OBJ_MAPPER.readValue(content, List.class);
                return JSONArray.fromObject(content);
            } catch (JsonProcessingException ex) {
                return content;
            }
        }
    }

    private void eliminateJsonArray(JSONArray data, boolean isAffectSourceData, List<String> fields) {
        for (int index = 0; index < data.size(); index++) {
            Object value = data.get(index);
            Object result = eliminate(value, isAffectSourceData, fields);
            data.set(index, result);
        }
    }
}

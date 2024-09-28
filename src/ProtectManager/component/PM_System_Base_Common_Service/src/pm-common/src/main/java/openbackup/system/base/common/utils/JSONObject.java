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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;
import openbackup.system.base.common.exception.LegoCheckedException;

import com.fasterxml.jackson.core.JsonParser.Feature;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.ObjectWriter;
import com.fasterxml.jackson.databind.introspect.Annotated;
import com.fasterxml.jackson.databind.introspect.JacksonAnnotationIntrospector;
import com.fasterxml.jackson.databind.ser.FilterProvider;
import com.fasterxml.jackson.databind.ser.impl.SimpleBeanPropertyFilter;
import com.fasterxml.jackson.databind.ser.impl.SimpleFilterProvider;
import com.google.json.JsonSanitizer;

import org.apache.commons.collections.map.ListOrderedMap;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TimeZone;

/**
 * JSONObject封装类 修改内容： 2016.11.26 1、去除序列化时对空串、null的过滤：
 * 2、华为虚拟机业务中会根据rest返回结果的参数格式，判断操作是否成功
 *
 */
public class JSONObject implements Map {
    /**
     * object mapper
     */
    public static final ObjectMapper DEFAULT_OBJ_MAPPER = createObjectMapper();

    /**
     * object mapper
     */
    public static final ObjectMapper RAW_OBJ_MAPPER = createObjectMapper();

    private static final Logger logger = LoggerFactory.getLogger(JSONObject.class);

    static {
        try {
            DEFAULT_OBJ_MAPPER.setSerializerProvider(new CustomSerializerProvider());
        } catch (Exception e) {
            logger.error("Initialize ObjectMapper failed: %s", ExceptionUtil.getErrorMessage(e));
        }
    }

    private final Map properties;

    /**
     * 默认构造函数
     */
    public JSONObject() {
        properties = new ListOrderedMap();
    }

    /**
     * create object mapper
     *
     * @return object mapper
     */
    public static ObjectMapper createObjectMapper() {
        ObjectMapper objectMapper = new ObjectMapper();
        objectMapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
        objectMapper.configure(Feature.ALLOW_SINGLE_QUOTES, true);
        objectMapper.setTimeZone(TimeZone.getDefault());
        return objectMapper;
    }

    /**
     * 转化成JSONObject
     *
     * @param obj Object
     * @return JSONObject
     */
    public static JSONObject fromObject(Object obj) {
        return fromObject(obj, DEFAULT_OBJ_MAPPER);
    }

    /**
     * 转化成JSONObject
     *
     * @param obj Object
     * @param mapper mapper
     * @return JSONObject
     */
    public static JSONObject fromObject(Object obj, ObjectMapper mapper) {
        if (VerifyUtil.isEmpty(obj)) {
            return new JSONObject();
        }
        if (obj instanceof JSONObject) {
            return (JSONObject) obj;
        }
        JSONObject json = new JSONObject();
        try {
            Map ret;
            if (obj instanceof String) {
                String data = obj.toString();
                String content = JsonSanitizer.sanitize(data);
                ret = mapper.readValue(content, Map.class);
            } else {
                String data = mapper.writeValueAsString(obj);
                String content = JsonSanitizer.sanitize(data);
                ret = mapper.readValue(content, Map.class);
            }
            if (ret != null) {
                convertProperty(json, ret, null);
            }
        } catch (Exception e) {
            logger.error("fromObject failed: %s", ExceptionUtil.getErrorMessage(e));
        }

        return json;
    }

    /**
     * clone object
     *
     * @param data data
     * @param <T> template type
     * @return clone object
     */
    public static <T> T clone(T data) {
        if (data == null) {
            return null;
        }
        Class<?> type = data.getClass();
        try {
            String content = RAW_OBJ_MAPPER.writeValueAsString(data);
            return (T) RAW_OBJ_MAPPER.readValue(content, type);
        } catch (JsonProcessingException e) {
            throw new LegoCheckedException("clone failed", e);
        }
    }

    /**
     * check object if empty
     *
     * @param object object
     * @return result
     */
    public static boolean isEmptyObject(Object object) {
        return fromObject(fromObject(object).toString()).isEmpty();
    }

    private static void convertProperty(JSONObject json, Map ret, Set<String> ignoreProperties) {
        for (Object obj : ret.entrySet()) {
            if (!(obj instanceof Entry)) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "value in entrySet error");
            }
            Entry entry = (Entry) obj;
            String key = entry.getKey().toString();
            if (ignoreProperties != null && ignoreProperties.contains(key)) {
                continue;
            }

            Object value = entry.getValue();

            Object tempValue = transValueToJson(value);
            json.properties.put(entry.getKey(), tempValue);
        }
    }

    /**
     * 将Map 转换成jsonObject 将Collection转换成JSONArray 其他类型原值返回
     * value值为经过jackson转换后的值，如果此时仍然为String类型，则该值即为纯字符串， 并不会存在该值为json字符串的情况
     * 因此只需要对Map或者Collection单独再进行转换，而不对String类型进行转换
     *
     * @param value value
     * @return Object Object
     */
    private static Object transValueToJson(Object value) {
        Object tempValue = value;
        if (value instanceof Map && !(value instanceof JSONObject)) {
            tempValue = JSONObject.fromObject(value);
        } else if (value instanceof Collection && !(value instanceof JSONArray)) {
            tempValue = JSONArray.fromObject(value);
        }

        return tempValue;
    }

    /**
     * 转化成JSONObject 注意： 不支持针对非 java bean进行属性过滤，比如obj为一个Map
     *
     * @param obj Object
     * @param ignoreProperties String[]忽略的属性
     * @return JSONObject
     */
    public static JSONObject fromObject(Object obj, String[] ignoreProperties) {
        Set<String> ignoreSet = new HashSet<String>();
        if (ignoreProperties != null) {
            for (String property : ignoreProperties) {
                ignoreSet.add(property);
            }
        }
        return fromObject(obj, ignoreSet);
    }

    /**
     * 转换为JSONObject
     *
     * @param obj Object
     * @param includes 包含属性
     * @return JSONObject
     */
    public static JSONObject fromObject(Object obj, List<String> includes) {
        JSONObject json = fromObject(obj);
        if (includes != null && !includes.isEmpty()) {
            json.entrySet()
                    .removeIf(entry -> !(entry instanceof Entry) || !includes.contains(((Entry) entry).getKey()));
        }
        return json;
    }

    /**
     * 转化成JSONObject 注意： 不支持针对Map进行属性过滤转换，比如obj为一个Map,只支持针对java bean的属性过滤
     *
     * @param obj Object
     * @param ignoreProperties Set String 忽略的属性
     * @return JSONObject
     */
    public static JSONObject fromObject(Object obj, Set<String> ignoreProperties) {
        if (obj == null) {
            logger.error("fromObject param is emtpy");
            throw new EmeiStorDefaultExceptionHandler("fromObject param is emtpy");
        }
        JSONObject json = new JSONObject();
        try {
            Map ret = null;
            ObjectWriter objWriter = DEFAULT_OBJ_MAPPER.writer();

            if (!VerifyUtil.isEmpty(ignoreProperties)) {
                objWriter = getWriterWithFilter(obj, ignoreProperties);
            }
            if (obj instanceof String) {
                ret = DEFAULT_OBJ_MAPPER.readValue(obj.toString(), Map.class);
            } else {
                String jsonString = objWriter.writeValueAsString(obj);
                ret = DEFAULT_OBJ_MAPPER.readValue(jsonString, Map.class);
            }
            if (ret != null) {
                convertProperty(json, ret, ignoreProperties);
            }
        } catch (Exception e) {
            logger.error("fromObject failed: %s", ExceptionUtil.getErrorMessage(e));
        }

        return json;
    }

    private static ObjectWriter getWriterWithFilter(Object obj, Set<String> ignoreProperties) {
        ObjectMapper objMapper = new ObjectMapper();
        objMapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
        objMapper.configure(Feature.ALLOW_SINGLE_QUOTES, true);
        CustomSerializerProvider sp = new CustomSerializerProvider();
        objMapper.setSerializerProvider(sp);
        objMapper.setAnnotationIntrospector(
                new JacksonAnnotationIntrospector() {
                    private static final long serialVersionUID = 7803579234214249536L;

                    @Override
                    public Object findFilterId(Annotated ac) {
                        return ac.getName();
                    }
                });

        FilterProvider filters =
                new SimpleFilterProvider()
                        .addFilter(
                                obj.getClass().getName(),
                                SimpleBeanPropertyFilter.serializeAllExcept(ignoreProperties));
        return objMapper.writer(filters);
    }

    /**
     * 将jsonobject转化成对应的对象
     *
     * @param jsonObject JSONObject
     * @param beanClass Class
     * @param <T> Class
     * @return Object
     */
    public static <T> T toBean(JSONObject jsonObject, Class<T> beanClass) {
        if (jsonObject == null) {
            logger.info("JSON Object is null");
            return null;
        }
        return toBean(jsonObject.toString(), beanClass);
    }

    /**
     * 将jsonobjectString转化成对应的对象
     *
     * @param jsonStr String
     * @param <T> Class
     * @param clazz Class T
     * @return T
     */
    public static <T> T toBean(String jsonStr, Class<T> clazz) {
        return toBean(jsonStr, clazz, false);
    }

    /**
     * 将jsonobjectString转化成对应的对象
     *
     * @param jsonStr String
     * @param <T> Class
     * @param clazz Class T
     * @param isStrict strict mode
     * @return T
     */
    public static <T> T toBean(String jsonStr, Class<T> clazz, boolean isStrict) {
        return toBean(jsonStr, clazz, isStrict, DEFAULT_OBJ_MAPPER);
    }

    /**
     * 将json object字符串转化成对应的对象
     *
     * @param jsonStr String
     * @param <T> Class
     * @param clazz Class T
     * @param mapper mapper
     * @return T
     */
    public static <T> T toBean(String jsonStr, Class<T> clazz, ObjectMapper mapper) {
        return toBean(jsonStr, clazz, true, mapper);
    }

    /**
     * 将json object字符串转化成对应的对象
     *
     * @param jsonStr String
     * @param <T> Class
     * @param clazz Class T
     * @param isStrict strict mode
     * @param mapper mapper
     * @return T
     */
    public static <T> T toBean(String jsonStr, Class<T> clazz, boolean isStrict, ObjectMapper mapper) {
        if (clazz == null) {
            logger.error("parameter error");
            throw new EmeiStorDefaultExceptionHandler("parameter error");
        }

        if (VerifyUtil.isEmpty(jsonStr)) {
            logger.info("Json Str is null");
            return null;
        }

        T body = null;
        try {
            String jsonStrVar = JsonSanitizer.sanitize(jsonStr);
            body = mapper.readValue(jsonStrVar, clazz);
        } catch (Exception e) {
            if (isStrict) {
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "json error", e);
            } else {
                logger.error("json to bean failed: %s", ExceptionUtil.getErrorMessage(e));
            }
        }
        return body;
    }

    /**
     * 将Java Bean转换为json字符串
     *
     * @param data data
     * @return json字符串
     */
    public static String stringify(Object data) {
        return stringify(data, RAW_OBJ_MAPPER);
    }

    /**
     * 将Java Bean转换为json字符串
     *
     * @param data data
     * @param mapper mapper
     * @return json字符串
     */
    public static String stringify(Object data, ObjectMapper mapper) {
        if (data == null) {
            return null;
        }
        try {
            return mapper.writeValueAsString(data);
        } catch (JsonProcessingException e) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "data error", e);
        }
    }

    /**
     * 转换成为map对象
     *
     * @param object json object
     * @param type 类型
     * @param <T> 模板
     * @return map对象
     */
    public static <T> Map<String, T> toMap(JSONObject object, Class<T> type) {
        if (object == null) {
            return Collections.emptyMap();
        }
        return object.toMap(type);
    }

    /**
     * data type cast
     *
     * @param source source data
     * @param targetType target type
     * @param <T> template type
     * @return result
     */
    public static <T> T cast(Object source, Class<T> targetType) {
        return cast(source, targetType, false);
    }

    /**
     * data type cast
     *
     * @param source source data
     * @param targetType target type
     * @param <T> template type
     * @param isStrict strict mode
     * @return result
     */
    public static <T> T cast(Object source, Class<T> targetType, boolean isStrict) {
        if (source == null) {
            return null;
        }
        if (targetType.isInstance(source)) {
            return targetType.cast(source);
        }
        return fromObject(source).toBean(targetType, isStrict);
    }

    /**
     * write Value As String
     *
     * @param object object
     * @return string
     */
    public static String writeValueAsString(Object object) {
        try {
            return DEFAULT_OBJ_MAPPER.writeValueAsString(object);
        } catch (JsonProcessingException e) {
            logger.error("toString failed:" + ExceptionUtil.getErrorMessage(e));
            throw new EmeiStorDefaultExceptionHandler("toString failed:" + e);
        }
    }

    /**
     * 判断是否是合法的json字符串
     *
     * @param json 待判断字符串
     * @return true-是合法json;  false-不是合法json
     */
    public static boolean isValidJson(String json) {
        try {
            RAW_OBJ_MAPPER.readTree(json);
            return Boolean.TRUE;
        } catch (JsonProcessingException e) {
            return Boolean.FALSE;
        }
    }

    /**
     * 获取值
     *
     * @param key String
     * @return Object
     */
    public Object get(String key) {
        return properties.get(key);
    }

    /**
     * Get the boolean value associated with a key.
     *
     * @param key A key string.
     * @return The truth.
     */
    public boolean getBoolean(String key) {
        Object value = get(key);
        if (value != null) {
            if (value instanceof Boolean) {
                return !Boolean.FALSE.equals(value);
            } else if (value instanceof String) {
                if (((String) value).equalsIgnoreCase("false")) {
                    return false;
                } else if (((String) value).equalsIgnoreCase("true")) {
                    return true;
                } else {
                    logger.trace("value error");
                }
            } else {
                logger.trace("value error");
            }
        }
        logger.error("JSONObject getBoolean failed,value is null or doesn't contain this key.key: {}", key);
        throw new EmeiStorDefaultExceptionHandler(
                "JSONObject getBoolean failed,value is null or doesn't contain this key.key" + key);
    }

    /**
     * Get the boolean value associated with a key.
     *
     * @param key A key string.
     * @param defaultValue if the JSONObject cannot contain the key ,return the
     *            defaultValue
     * @return The truth.
     */
    public boolean getBoolean(String key, boolean defaultValue) {
        if (properties.containsKey(key)) {
            return getBoolean(key);
        }
        return defaultValue;
    }

    /**
     * Get the double value associated with a key.
     *
     * @param key A key string.
     * @return The numeric value.
     */
    public double getDouble(String key) {
        Object value = get(key);
        if (value != null) {
            try {
                return value instanceof Number ? ((Number) value).doubleValue() : Double.parseDouble(value.toString());
            } catch (Exception e) {
                logger.error("getDouble failed:" + ExceptionUtil.getErrorMessage(e));
            }
        }
        logger.error("JSONObject getDouble failed, value is null or doesn't contain this key.key: {}" + key);
        throw new EmeiStorDefaultExceptionHandler(
                "JSONObject getBoolean failed,value is null or doesn't contain this key.key" + key);
    }

    /**
     * Get the double value associated with a key.
     *
     * @param key A key string.
     * @param defaultValue if the JSONObject cannot contain the key ,return the
     *            defaultValue
     * @return The numeric value.
     */
    public double getDouble(String key, double defaultValue) {
        double value = defaultValue;
        if (properties.containsKey(key)) {
            value = getDouble(key);
        }
        return value;
    }

    /**
     * Get the int value associated with a key. If the number value is too large for
     * an int, it will be clipped.
     *
     * @param key A key string.
     * @return The integer value.
     */
    public int getInt(String key) {
        Object object = get(key);
        if (object != null) {
            return object instanceof Number ? ((Number) object).intValue() : (int) getDouble(key);
        }
        logger.error("JSONObject getInt failed,value is null or doesn't contain this key.key=" + key);
        throw new EmeiStorDefaultExceptionHandler(
                "JSONObject getBoolean failed,value is null or doesn't contain this key.key" + key);
    }

    /**
     * Get the int value associated with a key. If the number value is too large for
     * an int, it will be clipped.
     *
     * @param key A key string.
     * @param defaultValue if the JSONObject cannot contain the key ,return the
     *            defaultValue
     * @return The integer value.
     */
    public int getInt(String key, int defaultValue) {
        int value = defaultValue;
        if (properties.containsKey(key)) {
            value = getInt(key);
        }
        return value;
    }

    /**
     * Get the long value associated with a key. If the number value is too long for
     * a long, it will be clipped.
     *
     * @param key A key string.
     * @return The long value.
     */
    public long getLong(String key) {
        Object object = get(key);
        if (object != null) {
            return object instanceof Number ? ((Number) object).longValue() : (int) getDouble(key);
        }
        logger.error("JSONObject getLong failed,value is null or doesn't contain this key.key=" + key);
        throw new EmeiStorDefaultExceptionHandler(
                "JSONObject getBoolean failed,value is null or doesn't contain this key.key" + key);
    }

    /**
     * Get the long value associated with a key. If the number value is too long for
     * a long, it will be clipped.
     *
     * @param key A key string.
     * @param defaultValue if the JSONObject cannot contain the key ,return the
     *            defaultValue
     * @return The long value.
     */
    public long getLong(String key, long defaultValue) {
        long value = defaultValue;
        if (properties.containsKey(key)) {
            value = getLong(key);
        }
        return value;
    }

    /**
     * Get the string associated with a key.
     *
     * @param key A key string.
     * @return A string which is the value.
     */
    public String getString(String key) {
        Object object = get(key);
        if (object != null) {
            return object.toString();
        }
        return null;
    }

    /**
     * Get the string associated with a key.
     *
     * @param key A key string.
     * @param defaultValue if the JSONObject cannot contain the key ,return the
     *            defaultValue
     * @return A string which is the value.
     */
    public String getString(String key, String defaultValue) {
        String value = defaultValue;
        if (properties.containsKey(key)) {
            value = this.getString(key);
        }
        return value;
    }

    /**
     * toString
     *
     * @return String
     */
    public String toString() {
        return writeValueAsString(this.properties);
    }

    /**
     * 获取key对应的JSonOBject
     *
     * @param key String
     * @return JSONObject
     */
    public JSONObject getJSONObject(String key) {
        Object object = get(key);
        if (object != null) {
            JSONObject json = null;
            if (!(object instanceof JSONObject)) {
                json = fromObject(object);
            } else {
                json = (JSONObject) object;
            }
            return json;
        }
        return null;
    }

    /**
     * 获取key对应的JSONArray
     *
     * @param key String
     * @return JSONArray
     */
    public JSONArray getJSONArray(String key) {
        Object object = get(key);
        if (object != null) {
            JSONArray array = null;
            if (!(object instanceof JSONArray)) {
                array = JSONArray.fromObject(object);
            } else {
                array = (JSONArray) object;
            }
            return array;
        }
        return null;
    }

    /**
     * 清空属性
     */
    @Override
    public void clear() {
        this.properties.clear();
    }

    /**
     * 包含属性
     *
     * @param key Object
     * @return boolean
     */
    @Override
    public boolean containsKey(Object key) {
        return this.properties.containsKey(key);
    }

    /**
     * 包含某个值
     *
     * @param value Object
     * @return boolean
     */
    @Override
    public boolean containsValue(Object value) {
        return this.properties.containsValue(value);
    }

    /**
     * 转化成set
     *
     * @return Set
     */
    @Override
    public Set entrySet() {
        return this.properties.entrySet();
    }

    /**
     * 获取值
     *
     * @param key Object
     * @return Object
     */
    @Override
    public Object get(Object key) {
        return this.properties.get(key);
    }

    /**
     * 判空
     *
     * @return isEmpty
     */
    @Override
    public boolean isEmpty() {
        return this.properties.isEmpty();
    }

    /**
     * 获取key值的迭代器
     *
     * @return Iterator
     */
    public Iterator keys() {
        return properties.keySet().iterator();
    }

    /**
     * 获取key的set
     *
     * @return Set
     */
    @Override
    public Set keySet() {
        return this.properties.keySet();
    }

    /**
     * 加入元素
     *
     * @param key Object
     * @param value Object
     * @return Object
     */
    @Override
    public Object put(Object key, Object value) {
        if (key == null) {
            logger.error("the key is empty");
            throw new EmeiStorDefaultExceptionHandler(
                    "JSONObject getBoolean failed,value is null or doesn't contain this key.key" + key);
        }
        if (value == null) {
            logger.debug("the value will put is null");
            return this;
        }
        Object tempValue = value;
        if (value instanceof String) {
            tempValue = convertStr2Json(value);
        } else if (value instanceof Map && !(value instanceof JSONObject)) {
            tempValue = JSONObject.fromObject(value);
        } else if (CollectionUtils.isArrayObject(value) && !(value instanceof JSONArray)) {
            tempValue = JSONArray.fromObject(value);
        } else {
            logger.trace("value error");
        }
        return this.properties.put(key, tempValue);
    }

    /**
     * 链式设值函数
     *
     * @param key key
     * @param value value
     * @return this json object
     */
    public JSONObject set(Object key, Object value) {
        put(key, value);
        return this;
    }

    /**
     * 仅用于put操作，当put String时，如果无法转换成json 对象，则直接将该字符串放入内部集合
     * <p>
     * 适配：value值为“[deviceID=2102350BSG10F7000010, moType=LUN, id=14]” 场景
     * 虽然其以中括号开头结尾，但其并不是jsonarray格式字符串
     *
     * @param value value
     * @return Object Object
     */
    private Object convertStr2Json(Object value) {
        Object tempValue = value;
        try {
            assert value instanceof String;
            if (((String) value).startsWith("{") && ((String) value).endsWith("}")) {
                tempValue = JSONObject.fromObject(value);
            } else if (((String) value).startsWith("[") && ((String) value).endsWith("]")) {
                tempValue = JSONArray.fromObject(value);
            } else {
                logger.trace("value error");
            }
        } catch (Exception e) {
            // 尝试对字符串进行JSON对象转换失败的情况太多，对日志冲刷严重，此处在异常分支打印debug日志属于特例。
            logger.debug("can't convert string to JSON Object:str= {}", value);
        }
        return tempValue;
    }

    /**
     * 将map全部放入jsonobject中
     *
     * @param map Map
     */
    @Override
    public void putAll(Map map) {
        for (Object obj : map.entrySet()) {
            if (!(obj instanceof Entry)) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "obj is not instance of Entry");
            }
            Entry entry = (Entry) obj;
            Object key = entry.getKey();
            Object value = entry.getValue();
            put(key, value);
        }
    }

    /**
     * update
     *
     * @param map map
     * @return json object
     */
    public JSONObject update(Map map) {
        putAll(map);
        return this;
    }

    /**
     * 删除Object
     *
     * @param key Object
     * @return Object
     */
    @Override
    public Object remove(Object key) {
        return this.properties.remove(key);
    }

    /**
     * 获取大小
     *
     * @return int
     */
    @Override
    public int size() {
        return this.properties.size();
    }

    /**
     * 获取所有的value
     *
     * @return Collection
     */
    @Override
    public Collection values() {
        return this.properties.values();
    }

    /**
     * 转换成为java bean
     *
     * @param type 类型
     * @param <T> 模板
     * @return java bean
     */
    public <T> T toBean(Class<T> type) {
        return toBean(this, type);
    }

    /**
     * 转换成为java bean
     *
     * @param type 类型
     * @param <T> 模板
     * @param isStrict 是否是严格模式，转换不成功，严格模式会抛出异常
     * @return java bean
     */
    public <T> T toBean(Class<T> type, boolean isStrict) {
        return toBean(this.toString(), type, isStrict);
    }

    /**
     * get bean
     *
     * @param key key
     * @param type type
     * @param <T> template type
     * @return bean
     */
    public <T> T getBean(String key, Class<T> type) {
        JSONObject json = getJSONObject(key);
        if (json == null) {
            return null;
        }
        return json.toBean(type);
    }

    /**
     * 转换成为map对象
     *
     * @param type 类型
     * @param <T> 模板
     * @return map对象
     */
    public <T> Map<String, T> toMap(Class<T> type) {
        Map<String, T> map = new HashMap<>();
        if (type != null) {
            properties.forEach(
                    (key, value) -> {
                        if (value == null || type.isInstance(value)) {
                            map.put(key.toString(), type.cast(value));
                        }
                    });
        }
        return map;
    }

    /**
     * pick field from data map object.
     *
     * @param fields which fields to pick
     * @return this json object
     */
    public JSONObject pick(String... fields) {
        JSONObject json = new JSONObject();
        if (fields != null) {
            for (String field : fields) {
                if (containsKey(field)) {
                    json.put(field, get(field));
                }
            }
        }
        return json;
    }

    /**
     * rename source field to target field
     *
     * @param source source field name
     * @param target target field name
     * @return this json object
     */
    public JSONObject rename(String source, String target) {
        if (!VerifyUtil.isEmpty(source) && !VerifyUtil.isEmpty(target) && containsKey(source)) {
            properties.put(target, properties.remove(source));
        }
        return this;
    }

    /**
     * copy source field as target field
     *
     * @param source source field name
     * @param target target field name
     * @return this json object
     */
    public JSONObject copyAs(String source, String target) {
        if (!VerifyUtil.isEmpty(source) && !VerifyUtil.isEmpty(target) && containsKey(source)) {
            properties.put(target, properties.remove(source));
        }
        return this;
    }

    /**
     * clone json object
     *
     * @return clone json object
     */
    public JSONObject duplicate() {
        return fromObject(toString());
    }
}

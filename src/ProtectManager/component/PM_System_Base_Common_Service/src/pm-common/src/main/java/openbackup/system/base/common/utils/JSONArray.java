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

import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;

import com.fasterxml.jackson.core.JsonParser.Feature;
import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.core.type.TypeReference;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.google.json.JsonSanitizer;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.text.Normalizer;
import java.text.Normalizer.Form;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.ListIterator;
import java.util.Map;
import java.util.function.Function;

/**
 * JSONArray封装类
 *
 * @author z00398217
 * @version [OceanStor ReplicationDirector V200R001C10, 2016年11月12日]
 * @since 2019-11-01
 */
public class JSONArray extends ArrayList {
    private static final long serialVersionUID = 5871970967397101346L;

    private static final Logger logger = LoggerFactory.getLogger(JSONArray.class);

    private static final ObjectMapper DEFAULT_OBJ_MAPPER = new ObjectMapper();

    static {
        try {
            DEFAULT_OBJ_MAPPER.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false);
            DEFAULT_OBJ_MAPPER.configure(Feature.ALLOW_SINGLE_QUOTES, true);
            CustomSerializerProvider sp = new CustomSerializerProvider();
            DEFAULT_OBJ_MAPPER.setSerializerProvider(sp);
        } catch (Exception e) {
            logger.error("Initialize ObjectMapper failed: %s", ExceptionUtil.getErrorMessage(e));
        }
    }

    private List elements = new ArrayList();

    /**
     * 构造函数
     */
    public JSONArray() {
        elements.clear();
        elements = new ArrayList();
    }

    /**
     * 构造函数
     *
     * @param items 元素列表
     */
    public JSONArray(Collection items) {
        this();
        this.addCollection(items);
    }

    /**
     * 构造函数
     *
     * @param items 元素
     */
    public JSONArray(Object[] items) {
        this(Arrays.asList(items));
    }

    /**
     * 将对象转化为JSONArray
     *
     * @param object 对象
     * @return JSONArray
     */
    public static JSONArray fromObject(Object object) {
        if (VerifyUtil.isEmpty(object)) {
            return new JSONArray();
        }
        if (object instanceof JSONArray) {
            return (JSONArray) object;
        }
        JSONArray jsonArray = new JSONArray();

        try {
            if (object instanceof String) {
                String objectString = object.toString();
                objectString = JsonSanitizer.sanitize(objectString);
                List<Object> list = DEFAULT_OBJ_MAPPER.readValue(objectString, new TypeReference<List<Object>>() {
                });
                if (list != null) {
                    convertElement(jsonArray, list);
                }
            } else if (CollectionUtils.isArrayObject(object)) {
                String arrayStr = DEFAULT_OBJ_MAPPER.writeValueAsString(object);
                return fromObject(arrayStr);
            } else {
                return jsonArray;
            }
        } catch (Exception e) {
            logger.error("fromObject failed: %s", ExceptionUtil.getErrorMessage(e));
        }

        return jsonArray;
    }

    private static void convertElement(JSONArray array, List<Object> list) {
        Object tempEle = null;
        for (Object ele : list) {
            tempEle = ele;
            if (ele instanceof Map && !(ele instanceof JSONObject)) {
                tempEle = JSONObject.fromObject(ele);
            } else if (ele instanceof Collection && !(ele instanceof JSONArray)) {
                tempEle = JSONArray.fromObject(ele);
            }
            array.elements.add(tempEle);
        }
    }

    /**
     * toString方法
     *
     * @return String
     */
    public String toString() {
        try {
            return DEFAULT_OBJ_MAPPER.writeValueAsString(this.elements);
        } catch (JsonProcessingException e) {
            logger.error("toString failed: %s", ExceptionUtil.getErrorMessage(e));
            throw new EmeiStorDefaultExceptionHandler("toString failed:" + e);
        }
    }

    /**
     * 转换成为前台能够识别的国际化字符串
     *
     * @return 国际化字符串
     */
    public String toI18nString() {
        return "i18n:" + this;
    }

    /**
     * 见json数据转换成list
     *
     * @param type 元素类型
     * @param <T> 模板类型
     * @return list列表
     */
    public <T> List<T> toBean(Class<T> type) {
        return toCollection(this, type);
    }

    /**
     * 转换成Collection类型
     *
     * @param array JSONArray
     * @param objectClass 仅支持自定义类
     * @param <T> 类型
     * @return List T 对应objectClass集合
     */
    public static <T> List<T> toCollection(JSONArray array, Class<T> objectClass) {
        if (array == null) {
            logger.error("toCollection param is null");
            throw new EmeiStorDefaultExceptionHandler("toCollection param is null");
        }

        try {
            return DEFAULT_OBJ_MAPPER.readValue(array.toString(),
                    DEFAULT_OBJ_MAPPER.getTypeFactory().constructCollectionType(List.class, objectClass));
        } catch (Exception e) {
            logger.error("toCollection failed: %s", ExceptionUtil.getErrorMessage(e));
            throw new EmeiStorDefaultExceptionHandler("toCollection failed:" + e.getMessage());
        }
    }

    /**
     * 获取指定索引位置的JSONObject
     *
     * @param index 索引
     * @return JSONObject
     */
    public JSONObject getJSONObject(int index) {
        if (index < 0 || index >= elements.size()) {
            logger.error("index is out of boundary");
            throw new EmeiStorDefaultExceptionHandler("index is out of boundary");
        }
        Object value = elements.get(index);
        if (value != null) {
            if (value instanceof JSONObject) {
                return (JSONObject) value;
            }
        }
        logger.error("the element: {} is null", index);
        throw new EmeiStorDefaultExceptionHandler("the element[ %s ] is null" + index);
    }

    /**
     * 获取String值
     *
     * @param index 索引
     * @return String
     */
    public String getString(int index) {
        if (index < 0 || index >= elements.size()) {
            logger.error("index is out of boundary");
            throw new EmeiStorDefaultExceptionHandler("index is out of boundary");
        }
        Object value = elements.get(index);

        return value.toString();
    }

    /**
     * 获取Boolean值
     *
     * @param index 索引
     * @return Boolean
     */
    public Boolean getBoolean(int index) {
        if (index < 0 || index >= elements.size()) {
            logger.error("index is out of boundary");
            throw new EmeiStorDefaultExceptionHandler("index is out of boundary");
        }
        Object value = elements.get(index);
        if (value != null) {
            if (Boolean.FALSE.equals(value)
                    || (value instanceof String && ((String) value).equalsIgnoreCase("false"))) {
                return false;
            } else if (Boolean.TRUE.equals(value)
                    || (value instanceof String && ((String) value).equalsIgnoreCase("true"))) {
                return true;
            } else {
                logger.error("the element: {} is null", index);
                throw new EmeiStorDefaultExceptionHandler("the element[ %s ] is null" + index);
            }
        }
        logger.error("the element: {} is null", index);
        throw new EmeiStorDefaultExceptionHandler("the element[ %s ] is null" + index);
    }

    /**
     * isArray
     *
     * @return boolean
     */
    public boolean isArray() {
        return true;
    }

    /**
     * 向array中添加元素
     *
     * @param object Object
     * @return boolean
     */
    @Override
    public boolean add(Object object) {
        if (object == null) {
            return false;
        }
        Object tempValue = convertValue(object);
        return this.elements.add(tempValue);
    }

    private Object convertValue(Object object) {
        Object tempValue = object;
        if (object instanceof String) {
            tempValue = convertStr2Json((String) object);
        } else if (object instanceof Map && !(object instanceof JSONObject)) {
            tempValue = JSONObject.fromObject(object);
        } else if (object instanceof Collection && !(object instanceof JSONArray)) {
            tempValue = JSONArray.fromObject(object);
        } else {
            return tempValue;
        }
        return tempValue;
    }

    /**
     * 仅用于add元素操作,当转换操作失败时，直接以字符串形式添加到内部集合
     * 适配：value值为“[deviceID=2102350BSG10F7000010, moType=LUN, id=14]” 场景
     * 虽然其以中括号开头结尾，但其并不是jsonarray格式字符串
     *
     * @param value 目标
     * @return Object Object
     */
    private Object convertStr2Json(String value) {
        Object tempValue = value;
        try {
            String normalValue = Normalizer.normalize(value, Form.NFKC);
            if (normalValue.startsWith("{") && normalValue.endsWith("}")) {
                tempValue = JSONObject.fromObject(normalValue);
            } else if (normalValue.startsWith("[") && normalValue.endsWith("]")) {
                tempValue = JSONArray.fromObject(normalValue);
            } else {
                return tempValue;
            }
        } catch (Exception e) {
            logger.error("Can't convert string to JSONObject str:{}", tempValue);
        }
        return tempValue;
    }

    /**
     * 添加对象到指定位置
     *
     * @param index 索引
     * @param object 对象
     */
    @Override
    public void add(int index, Object object) {
        if (object == null) {
            logger.info("can't add null value to array");
            return;
        }
        Object tempValue = convertValue(object);
        this.elements.add(index, tempValue);
    }

    private void addCollection(Collection objects) {
        for (Object obj : objects) {
            add(obj);
        }
    }

    /**
     * 将collectino加入到数组
     *
     * @param objects collection
     * @return boolean
     */
    @Override
    public boolean addAll(Collection objects) {
        if (objects == null || objects.isEmpty()) {
            return false;
        }
        this.addCollection(objects);
        return true;
    }

    /**
     * 将collectino加入到数组的指定位置
     *
     * @param index 索引
     * @param objects collection
     * @return boolean
     */
    @Override
    public boolean addAll(int index, Collection objects) {
        if (objects == null || objects.isEmpty()) {
            return false;
        }
        for (Object obj : objects) {
            add(index, obj);
        }
        return true;
    }

    /**
     * 清空数组
     */
    @Override
    public void clear() {
        this.elements.clear();
    }

    /**
     * 判断是否包含对象
     *
     * @param object Object
     * @return boolean
     */
    @Override
    public boolean contains(Object object) {
        return this.elements.contains(object);
    }

    /**
     * 判断是否包含Collection
     *
     * @param objects Collection
     * @return boolean
     */
    @Override
    public boolean containsAll(Collection objects) {
        return this.elements.containsAll(objects);
    }

    /**
     * 获取指定位置的对象
     *
     * @param index 索引
     * @return Object
     */
    @Override
    public Object get(int index) {
        return this.elements.get(index);
    }

    /**
     * 获取对象的索引
     *
     * @param object Object
     * @return int
     */
    @Override
    public int indexOf(Object object) {
        return this.elements.indexOf(object);
    }

    /**
     * 判断是否为空数组
     *
     * @return boolean
     */
    @Override
    public boolean isEmpty() {
        return this.elements.isEmpty();
    }

    /**
     * 转化为迭代器
     *
     * @return Iterator
     */
    @Override
    public Iterator iterator() {
        return this.elements.iterator();
    }

    /**
     * 获取object的最后一个索引
     *
     * @param object Object
     * @return int
     */
    @Override
    public int lastIndexOf(Object object) {
        return this.elements.lastIndexOf(object);
    }

    /**
     * 转化为ListIterator
     *
     * @return ListIterator
     */
    @Override
    public ListIterator listIterator() {
        return this.elements.listIterator();
    }

    /**
     * 返回指定位置的 ListIterator
     *
     * @param index 索引
     * @return ListIterator
     */
    @Override
    public ListIterator listIterator(int index) {
        return this.elements.listIterator(index);
    }

    /**
     * 删除对象
     *
     * @param object Object
     * @return boolean
     */
    @Override
    public boolean remove(Object object) {
        return this.elements.remove(object);
    }

    /**
     * 删除指定位置的对象
     *
     * @param index 索引
     * @return Object
     */
    @Override
    public Object remove(int index) {
        return this.elements.remove(index);
    }

    /**
     * 移除
     *
     * @param objects Collection
     * @return boolean
     */
    @Override
    public boolean removeAll(Collection objects) {
        return this.elements.removeAll(objects);
    }

    /**
     * 只包含Collection中的元素
     *
     * @param objects Collection
     * @return boolean
     */
    @Override
    public boolean retainAll(Collection objects) {
        return this.elements.retainAll(objects);
    }

    /**
     * 设置指定索引下的值
     *
     * @param index 索引
     * @param object Object
     * @return Object
     */
    @Override
    public Object set(int index, Object object) {
        return this.elements.set(index, object);
    }

    /**
     * 判断数组长度
     *
     * @return int
     */
    @Override
    public int size() {
        return this.elements.size();
    }

    /**
     * 截断数组
     *
     * @param begin 开始位置
     * @param end 介绍位置
     * @return List
     */
    @Override
    public List subList(int begin, int end) {
        return this.elements.subList(begin, end);
    }

    /**
     * 将List转化为数组
     *
     * @return Object[]
     */
    @Override
    public Object[] toArray() {
        return this.elements.toArray();
    }

    /**
     * 返回一个包含这些所有元素的数组
     *
     * @param objects Object[]
     * @return Object[]
     */
    @Override
    public Object[] toArray(Object[] objects) {
        return this.elements.toArray(objects);
    }

    /**
     * 将jsonarray转换成jsonobject 针对jsonarray中的每个元素，使用其每个元素的值作为key
     *
     * @param array array
     * @return JSONObject 返回值
     */
    public JSONObject toJSONObject(JSONArray array) {
        if (array == null || array.isEmpty()) {
            return null;
        }
        JSONObject json = new JSONObject();
        for (Object obj : array.elements) {
            json.put(obj.toString(), obj);
        }
        return json;
    }

    /**
     * 过滤指定类型的元素
     *
     * @param clazz 类型
     * @param <T> return type
     * @return 过滤结果
     */
    public <T> Collection<T> filter(Class<T> clazz) {
        List<T> list = new ArrayList<>();
        for (int i = 0, n = this.size(); i < n; i++) {
            Object item = this.get(i);
            if (clazz.isInstance(item)) {
                list.add(clazz.cast(item));
            }
        }
        return list;
    }

    /**
     * 元素转换
     *
     * @param converter converter
     * @param <T> template type
     * @return result
     */
    public <T> List<T> map(Function<Object, T> converter) {
        List<T> list = new ArrayList<>();
        for (int i = 0, n = this.size(); i < n; i++) {
            Object item = this.get(i);
            list.add(converter.apply(item));
        }
        return list;
    }
}

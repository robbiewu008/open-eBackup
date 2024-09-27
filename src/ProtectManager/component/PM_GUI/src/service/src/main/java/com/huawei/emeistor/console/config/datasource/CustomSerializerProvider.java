/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.config.datasource;

import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.databind.BeanProperty;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.fasterxml.jackson.databind.JsonSerializer;
import com.fasterxml.jackson.databind.SerializationConfig;
import com.fasterxml.jackson.databind.SerializerProvider;
import com.fasterxml.jackson.databind.ser.DefaultSerializerProvider;
import com.fasterxml.jackson.databind.ser.SerializerFactory;

import java.io.IOException;
import java.util.Collection;

/**
 * 自定义序列化器提供类
 * <p>
 * jackson在序列化时，null值会直接序列化成null，而json-lib会根据属性的类型自动赋予默认值，
 * 为了和json-lib特性保持一致，因此实现自定义序列化提供类
 * String 类型 null 值 序列化为  ""
 * 集合 序列化为[]
 * 数字初始化为0 或者 0.0
 * <p>
 * 其他仍然为null
 *
 * @author z00398217
 * @version [OceanStor ReplicationDirector V200R001C10, 2016年12月7日]
 * @since 2019-11-01
 */
public class CustomSerializerProvider extends DefaultSerializerProvider {
    // <变量的意义、目的、功能和可能被用到的地方>
    private static final long serialVersionUID = 8134089332517831285L;

    /**
     * 默认构造函数
     */
    public CustomSerializerProvider() {
        super();
    }

    /**
     * 默认构造函数
     *
     * @param provider provider
     * @param cfg      cfg
     * @param sf       sf
     */
    public CustomSerializerProvider(CustomSerializerProvider provider, SerializationConfig cfg, SerializerFactory sf) {
        super(provider, cfg, sf);
    }

    /**
     * 创建实例
     *
     * @param cfg cfg
     * @param sf  sf
     * @return DefaultSerializerProvider
     */
    @Override
    public DefaultSerializerProvider createInstance(SerializationConfig cfg, SerializerFactory sf) {
        return new CustomSerializerProvider(this, cfg, sf);
    }

    /**
     * 空值序列化器
     *
     * @param property property
     * @return 序列化器
     * @throws JsonMappingException JsonMappingException
     */
    @SuppressWarnings("rawtypes")
    @Override
    public JsonSerializer<Object> findNullValueSerializer(BeanProperty property) throws JsonMappingException {
        Class clazz = property.getType().getRawClass();
        if (isString(clazz)) {
            return getEmptyStringSerializer();
        } else if (isArray(clazz)) {
            return getEmptyArraySerializer();
        } else if (isNumber(clazz)) {
            if (isDouble(clazz)) {
                return getEmptyDoubleSerializer();
            } else {
                return getEmptyIntSerializer();
            }
        } else if (isBoolean(clazz)) {
            return getEmptyBooleanSerializer();
        } else {
            return super.findNullValueSerializer(property);
        }
    }

    @SuppressWarnings("rawtypes")
    private boolean isArray(Class clazz) {
        return clazz != null && (clazz.isArray() || Collection.class.isAssignableFrom(clazz)
            || (JSONArray.class.isAssignableFrom(clazz)));
    }

    @SuppressWarnings("rawtypes")
    private boolean isString(Class clazz) {
        return clazz != null && (String.class.isAssignableFrom(clazz) || (Character.TYPE.isAssignableFrom(clazz)
            || Character.class.isAssignableFrom(clazz)));
    }

    @SuppressWarnings("rawtypes")
    private boolean isNumber(Class clazz) {
        if (clazz == null) {
            return false;
        }
        if (Byte.TYPE.isAssignableFrom(clazz) || Short.TYPE.isAssignableFrom(clazz) || Integer.TYPE.isAssignableFrom(
            clazz)) {
            return true;
        }
        return Long.TYPE.isAssignableFrom(clazz) || Float.TYPE.isAssignableFrom(clazz) || Double.TYPE.isAssignableFrom(
            clazz) || Number.class.isAssignableFrom(clazz);
    }

    @SuppressWarnings("rawtypes")
    private boolean isDouble(Class clazz) {
        return clazz != null && (Double.TYPE.isAssignableFrom(clazz) || Double.class.isAssignableFrom(clazz));
    }

    @SuppressWarnings("rawtypes")
    private boolean isBoolean(Class clazz) {
        return clazz != null && (Boolean.TYPE.isAssignableFrom(clazz) || Boolean.class.isAssignableFrom(clazz));
    }

    private JsonSerializer<Object> getEmptyStringSerializer() {
        return new JsonSerializer<Object>() {
            /**
             * 序列化函数
             *
             * @param obj obj
             * @param jgen jgen
             * @param provider provider
             * @throws IOException IOException
             */
            @Override
            public void serialize(Object obj, JsonGenerator jgen, SerializerProvider provider) throws IOException {
                jgen.writeString("");
            }
        };
    }

    private JsonSerializer<Object> getEmptyBooleanSerializer() {
        return new JsonSerializer<Object>() {
            /**
             * 序列化函数
             *
             * @param obj obj
             * @param jgen jgen
             * @param provider provider
             * @throws IOException IOException
             */
            @Override
            public void serialize(Object obj, JsonGenerator jgen, SerializerProvider provider) throws IOException {
                jgen.writeBoolean(false);
            }
        };
    }

    private JsonSerializer<Object> getEmptyArraySerializer() {
        return new JsonSerializer<Object>() {
            /**
             * 序列化函数
             *
             * @param value 需要序列化的值
             * @param gen json生成器接口
             * @param provider 序列化provider
             * @throws IOException IOException
             */
            @Override
            public void serialize(Object value, JsonGenerator gen, SerializerProvider provider) throws IOException {
                gen.writeObject(new JSONArray());
            }
        };
    }

    private JsonSerializer<Object> getEmptyIntSerializer() {
        return new JsonSerializer<Object>() {
            /**
             * 序列化函数
             *
             * @param value 需要序列化的值
             * @param gen json生成器接口
             * @param provider 序列化provider
             * @throws IOException IOException
             */
            @Override
            public void serialize(Object value, JsonGenerator gen, SerializerProvider provider) throws IOException {
                gen.writeNumber(0);
            }
        };
    }

    private JsonSerializer<Object> getEmptyDoubleSerializer() {
        return new JsonSerializer<Object>() {
            /**
             * 序列化函数
             *
             * @param value 需要序列化的值
             * @param gen json生成器接口
             * @param provider 序列化provider
             * @throws IOException IOException
             */
            @Override
            public void serialize(Object value, JsonGenerator gen, SerializerProvider provider) throws IOException {
                gen.writeNumber(0d);
            }
        };
    }
}

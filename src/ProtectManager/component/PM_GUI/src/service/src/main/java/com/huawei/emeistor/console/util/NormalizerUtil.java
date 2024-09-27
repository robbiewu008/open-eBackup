/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.emeistor.console.util;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.alibaba.fastjson.serializer.SerializerFeature;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.util.StringUtils;

import java.text.Normalizer;
import java.util.Objects;

/**
 * 处理服务器注入风险Server-Side Request Forgery工具类
 *
 * @author l00422407
 * @since 2021-01-23
 */
public class NormalizerUtil {
    private static final Logger LOGGER = LoggerFactory.getLogger(NormalizerUtil.class);

    /**
     * 过滤不安全的特殊字符
     *
     * @param item item
     * @return String
     */
    public static String normalizeForString(String item) {
        if (StringUtils.isEmpty(item)) {
            return "";
        }
        return Normalizer.normalize(item, Normalizer.Form.NFKC);
    }

    /**
     * 校验对象
     *
     * @param obj 待检验对象
     * @param clazz 反序列化后正确类
     * @return T 检验后对象
     */

    public static <T> T normalizeForBean(T obj, Class<T> clazz) {
        T result = null;
        try {
            if (Objects.isNull(obj)) {
                return result;
            }
            String temp =
                Normalizer.normalize(JSON.toJSONString(obj, SerializerFeature.WriteMapNullValue), Normalizer.Form.NFKC);
            result = JSONObject.toJavaObject(JSON.parseObject(temp), clazz);
        } catch (ClassCastException e) {
            LOGGER.error("Be Care For SSRF!!!");
        }
        return result;
    }
}

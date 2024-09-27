/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.rest;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import feign.codec.Encoder;

import org.springframework.beans.factory.FactoryBean;
import org.springframework.beans.factory.annotation.Autowired;

import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;

/**
 * Feign Client Configuration
 *
 * @param <T> template param
 * @author l00272247
 * @since 2020-12-11
 */
public abstract class FeignClientFactoryBean<T> implements FactoryBean<T> {
    private final Class<T> type;

    @Autowired(required = false)
    private Encoder encoder;

    /**
     * constructor
     */
    public FeignClientFactoryBean() {
        Type superClass = this.getClass().getGenericSuperclass();
        if (superClass instanceof Class) {
            throw new LegoCheckedException("Internal error: TypeReference constructed without actual type information");
        }
        if (!(superClass instanceof ParameterizedType)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "superClass is not instance of ParameterizedType");
        }
        final Type actualTypeArgument = ((ParameterizedType) superClass).getActualTypeArguments()[0];
        if (!(actualTypeArgument instanceof Class)) {
            throw new IllegalArgumentException("actualTypeArgument is not instance of Class");
        }
        this.type = (Class<T>) actualTypeArgument;
    }

    @Override
    public T getObject() {
        return FeignBuilder.buildConfigWithDefaultConfig(type, encoder);
    }

    @Override
    public Class<?> getObjectType() {
        return type;
    }
}

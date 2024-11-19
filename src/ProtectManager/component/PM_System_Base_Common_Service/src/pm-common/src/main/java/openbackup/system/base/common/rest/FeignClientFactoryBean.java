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
package openbackup.system.base.common.rest;

import feign.codec.Encoder;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.springframework.beans.factory.FactoryBean;
import org.springframework.beans.factory.annotation.Autowired;

import java.lang.reflect.ParameterizedType;
import java.lang.reflect.Type;

/**
 * Feign Client Configuration
 *
 * @param <T> template param
 *
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

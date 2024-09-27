/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.plugin;

import com.fasterxml.jackson.core.type.TypeReference;

/**
 * 资源扩展属性handler
 *
 * @author h30027154
 * @since 2022-05-30
 */
public abstract class ResourceExtensionHandler<T, R> {
    /**
     * 默认构造
     */
    protected ResourceExtensionHandler() {
        new TypeReference<T>() {};
        new TypeReference<R>() {};
    }

    /**
     * 配置的路径
     *
     * @return namePath
     */
    public abstract String getNamePath();

    /**
     * 对扩展资源的处理
     *
     * @param configObj 配置对象
     * @param params params
     * @return R
     */
    public abstract R handle(Object configObj, T params);
}

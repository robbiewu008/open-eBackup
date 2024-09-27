/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.plugin;

/**
 * 资源扩展invoke管理
 *
 * @author h30027154
 * @since 2022-05-30
 */
public interface ResourceExtensionManager {
    /**
     * invoke
     *
     * @param subType subType
     * @param namePath 配置的路径
     * @param <T> 输入类型
     * @param <R> 输出类型
     * @param context context
     * @return R
     */
    <T, R> R invoke(String subType, String namePath, PluginExtensionInvokeContext<T, R> context);
}

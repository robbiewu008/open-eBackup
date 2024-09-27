/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 受保护资源发送告警资源名称填充Provider
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-3-31
 */
public interface ResourceAlarmProvider extends DataProtectionProvider<ProtectedResource> {
    /**
     * 获取资源名称
     *
     * @param resource 资源
     * @return String 默认发资源名称，插件自定义资源名称的返回值
     */
    default String getAlarmResourceName(ProtectedResource resource) {
        return resource.getName();
    }
}

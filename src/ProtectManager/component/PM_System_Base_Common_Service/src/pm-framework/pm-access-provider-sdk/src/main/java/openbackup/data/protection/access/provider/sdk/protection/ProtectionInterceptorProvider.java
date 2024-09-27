/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.protection;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.resourcegroup.req.ProtectedObjectRequest;

/**
 * 保护拦截器
 *
 * @author c30062305
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-07-30
 */
public interface ProtectionInterceptorProvider extends DataProtectionProvider<String> {
    /**
     * 保护前校验
     *
     * @param request 保护对象request
     */
    void preCheck(ProtectedObjectRequest request);
}
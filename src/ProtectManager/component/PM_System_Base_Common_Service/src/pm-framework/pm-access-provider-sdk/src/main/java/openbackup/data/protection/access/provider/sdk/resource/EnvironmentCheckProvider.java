/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 功能描述: 注册环境时环境检查check接口定义
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-16
 */
public interface EnvironmentCheckProvider extends DataProtectionProvider<ProtectedEnvironment> {
    /**
     * 对环境信息进行检查，该接口用于注册受保护环境，修改受保护环境时对环境信息进行验证
     * 比如检查受保护环境与ProtectManager之间的连通性，认证信息是否合法。环境参数是否合法等等
     * 检查不通过抛出com.huawei.oceanprotect.data.protection.access.provider.sdk.exception.DataProtectionAccessException
     *
     * @param environment 受保护环境
     */
    void check(ProtectedEnvironment environment);
}
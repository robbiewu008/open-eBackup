/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 功能描述: EnvironmentParamProvider
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-29
 */
public interface EnvironmentParamProvider extends DataProtectionProvider<ProtectedEnvironment> {
    /**
     * 注册、修改受保护环境时，在下发agent插件之前，先校验、填充参数
     *
     * @param environment 受保护环境
     */
    void checkAndPrepareParam(ProtectedEnvironment environment);

    /**
     * 连通性检查之后，某些特性需要从agent查询信息并回填到environment中
     *
     * @param environment 受保护环境
     */
    void updateEnvironment(ProtectedEnvironment environment);

    /**
     * 校验受保护环境是否被重复注册
     *
     * @param environment 受保护环境
     */
    void checkEnvironmentRepeat(ProtectedEnvironment environment);
}
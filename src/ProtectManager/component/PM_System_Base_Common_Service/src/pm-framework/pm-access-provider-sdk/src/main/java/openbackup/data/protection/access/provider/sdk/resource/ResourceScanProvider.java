/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

import java.util.Collections;
import java.util.List;

/**
 * 功能描述: 资源扫描 Provider
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-08
 */
public interface ResourceScanProvider extends DataProtectionProvider<ProtectedEnvironment> {
    /**
     * 扫描受保护环境的资源
     *
     * @param environment 受保护环境
     * @return 受保护环境中的资源列表
     */
    List<ProtectedResource> scan(ProtectedEnvironment environment);

    /**
     * 当出现异常时，查询资源
     * <p>
     * DataProtectionAccessException或LegoCheckedException时触发
     *
     * @param uuid uuid
     * @return 受保护环境中的资源列表
     */
    default List<ProtectedResource> queryResourceWhenException(String uuid) {
        return Collections.emptyList();
    }
}
/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 *
 */

package openbackup.system.base.sdk.system;

import openbackup.system.base.sdk.system.model.StorageAuth;

/**
 * 本地系统接口
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/3/16
 */
public interface SystemNativeApi {
    /**
     * 认证登录信息
     *
     * @param authInfo 登录信息
     * @return 设备id
     */
    String checkAuth(StorageAuth authInfo);
}

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

/**
 * 受保护环境检查结果
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-12
 */
public enum EnvironmentCheckResult {
    /**
     * 正常
     */
    NORMAL,
    /**
     * 网络连接超时
     */
    CONN_TIMEOUT,
    /**
     * 认证失败
     */
    AUTH_FAILED,

    /**
     * 参数错误
     */
    PARAM_ERROR
}

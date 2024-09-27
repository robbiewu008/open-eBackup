/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.constants;

/**
 * 池化HttpClient常量
 *
 * @author y30037959
 * @since 2023-03-29
 */
public final class PoolingHttpClientConstant {
    /**
     * 每个路由分配的最大连接数
     */
    public static final int MAX_CONN_PER_ROUTE = 10;

    /**
     * 最大总连接数
     */
    public static final int MAX_CONN_TOTAL = 50;

    /**
     * 连接存活时间
     */
    public static final int TIME_TO_LIVE = 10 * 60 * 1000;

    /**
     * 失活检测时间（失活后多久检查连接是否有效）
     */
    public static final int VALIDATE_AFTER_INACTIVITY = 5 * 60 * 1000;

    private PoolingHttpClientConstant() {
    }
}

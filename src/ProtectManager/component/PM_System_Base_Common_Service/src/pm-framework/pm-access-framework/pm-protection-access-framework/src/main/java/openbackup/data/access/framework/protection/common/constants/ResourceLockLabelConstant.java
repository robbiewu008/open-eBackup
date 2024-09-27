/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.protection.common.constants;

/**
 * 资源锁任务步骤国际化key定义
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/27
 **/
public final class ResourceLockLabelConstant {
    /**
     * 对资源加锁
     */
    public static final String LOCK_START_KEY = "lock_running_label";

    /**
     * 对资源加锁成功
     */
    public static final String LOCK_SUCCESS_KEY = "lock_succeeded_label";

    /**
     * 对资源加失败
     */
    public static final String LOCK_FAILED_KEY = "lock_failed_label";

    /**
     * 对资源解锁
     */
    public static final String UNLOCK_START_KEY = "unlock_running_label";

    /**
     * 对资源解锁成功
     */
    public static final String UNLOCK_SUCCESS_KEY = "unlock_succeeded_label";

    /**
     * 对资源解锁失败
     */
    public static final String UNLOCK_FAILED_KEY = "unlock_failed_label";
}

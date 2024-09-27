/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.constants;

/**
 * rest接口切面的执行顺序常量
 *
 * @author t30028453
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-31
 */
public class AspectOrderConstant {
    /**
     * 日志切面
     */
    public static final int LOGGING_ASPECT_ORDER = 0;

    /**
     * 操作日志切面
     */
    public static final int OPERATION_LOG_ASPECT_ORDER = 1000;

    /**
     * 权限切面
     */
    public static final int PERMISSION_ASPECT_ORDER = 2000;

    /**
     * 文件校验切面
     */
    public static final int FILE_CHECK_ASPECT_ORDER = 3000;

    /**
     * 部署方式校验切面
     */
    public static final int DEPLOY_TYPE_ASPECT_ORDER = 4000;
}

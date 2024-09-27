/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.handler.v2;

import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;

/**
 * 统一任务完成处理器
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-07
 */
public abstract class UnifiedTaskCompleteHandler implements TaskCompleteHandler {
    /**
     * 处理器版本
     */
    public static final String V2 = "v2";

    /**
     * 处理器的版本
     *
     * @return 返回处理器的版本
     */
    protected String version() {
        return V2;
    }
}

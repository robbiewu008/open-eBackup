/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.copy.verify.constant;

/**
 * 副本校验任务步骤国际化标签常量类
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/30
 **/
public final class CopyVerifyJobLabelConstant {
    /**
     * 恢复任务初始化
     */
    public static final String COPY_CHECK_INIT = "job_log_copy_verify_init_label";

    /**
     * 恢复任务执行
     */
    public static final String COPY_CHECK_START = "job_log_copy_verify_execute_label";

    /**
     * 恢复任务完成
     */
    public static final String COPY_CHECK_COMPLETE = "job_log_copy_verify_complete_label";

    private CopyVerifyJobLabelConstant() {
    }
}

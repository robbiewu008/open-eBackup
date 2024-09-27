/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.copy.verify.constant;

/**
 * 副本校验任务错误码常量类
 *
 * @author: y00559272
 * @version: [OceanProtect X8000 1.2.1]
 * @since: 2022/8/6
 **/
public abstract class CopyVerifyTaskErrorCode {
    /**
     * 错误场景：用户执行副本校验任务时，副本校验文件不存在，不允许执行校验。
     */
    public static final long COPY_VERIFY_FILE_NOT_EXISTED = 1677933334L;

    /**
     * 错误场景：用户执行副本校验任务状态时，副本不是正常状态
     */
    public static final long COPY_STATUS_IS_NOT_INVALID = 1677933335L;

    /**
     * 错误场景：用户执行副本校验任务状态时，归档副本不支持校验
     */
    public static final long COPY_IS_GENERATE_BY_ARCHIVE = 1677933336L;

    /**
     * 错误场景：用户执行副本校验任务状态时，复制副本不支持校验
     */
    public static final long COPY_IS_GENERATE_BY_REPLICATION = 1677933337L;
}

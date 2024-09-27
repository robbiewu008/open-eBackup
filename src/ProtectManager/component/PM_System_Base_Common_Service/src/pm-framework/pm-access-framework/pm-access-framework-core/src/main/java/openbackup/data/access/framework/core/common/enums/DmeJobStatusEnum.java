/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.common.enums;

import com.google.common.collect.ImmutableList;

import java.util.List;

/**
 * 功能描述
 *
 * @author y30000858
 * @since 2020-09-21
 */
public enum DmeJobStatusEnum {
    /**
     * 运行中
     */
    RUNNING(1),

    /**
     * 中止中
     */
    ABORTING(2),

    /**
     * 成功
     */
    SUCCESS(3),

    /**
     * 停止
     */
    ABORTED(4),

    /**
     * 中止失败
     */
    ABORTED_FAILED(5),

    /**
     * 失败
     */
    FAIL(6),

    /**
     * 部分成功
     */
    PARTIAL_SUCCESS(13);

    private final int status;

    DmeJobStatusEnum(int status) {
        this.status = status;
    }

    /**
     * 根据DME状态码获取枚举值
     *
     * @param status 状态码
     * @return enum
     */
    public static DmeJobStatusEnum fromStatus(int status) {
        for (DmeJobStatusEnum type : DmeJobStatusEnum.values()) {
            if (type.getTypeName() == status) {
                return type;
            }
        }
        return FAIL;
    }

    /**
     * 完成状态列表
     */
    public static final List<DmeJobStatusEnum> FINISHED_STATUS_LIST =
            ImmutableList.of(DmeJobStatusEnum.FAIL, DmeJobStatusEnum.SUCCESS, DmeJobStatusEnum.ABORTED,
                    DmeJobStatusEnum.PARTIAL_SUCCESS, DmeJobStatusEnum.ABORTED_FAILED);

    /**
     * dme组件任务处理失败状态列表
     */
    public static final List<DmeJobStatusEnum> FAILED_STATUS_LIST =
            ImmutableList.of(DmeJobStatusEnum.FAIL, DmeJobStatusEnum.ABORTED, DmeJobStatusEnum.ABORTED_FAILED);

    /**
     * 根据type类型获取状态
     *
     * @return value
     */
    public int getTypeName() {
        return this.status;
    }
}

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.taskflow;

import lombok.Data;

import java.util.List;

/**
 * 任务流基础上下文
 *
 * @author w00607005
 * @since 2023-05-18
 */
@Data
public class BaseContext {
    /**
     * 任务ID
     */
    private String jobId;

    /**
     * 任务类型
     */
    private String type;

    /**
     * 任务状态
     */
    private int status;

    private List<String> jobLogs;

    private String detailParas;
}

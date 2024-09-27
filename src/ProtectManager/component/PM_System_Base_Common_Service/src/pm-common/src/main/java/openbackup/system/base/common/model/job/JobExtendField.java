/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.model.job;

import openbackup.system.base.query.PageQueryConfig;

import lombok.Data;

/**
 * 功能描述
 *
 * @author w30044259
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-18
 */
@Data
@PageQueryConfig(conditions = {"%cluster_name%", "%unit_name%"})
public class JobExtendField extends Job {
    /**
     * 任务所在节点名称
     */
    private String clusterName;

    /**
     * 任务所在节点名称
     */
    private String unitName;
}

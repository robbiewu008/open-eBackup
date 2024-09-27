/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.alarm.model;

import lombok.Data;

/**
 * 目标集群告警信息查询数据返回模型
 *
 * @author dWX1009286
 * @since 2021-07-20
 */
@Data
public class ClusterAlarmsInfo {
    // 关键告警
    private int critical;

    // 警告告警
    private int warning;

    // 主要告警
    private int major;

    // 次要告警
    private int minor;
}

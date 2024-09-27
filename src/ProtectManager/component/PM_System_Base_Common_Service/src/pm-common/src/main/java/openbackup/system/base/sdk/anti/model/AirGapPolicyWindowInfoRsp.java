/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.anti.model;

import lombok.Data;

/**
 * AirGap策略信息时间窗对象返回
 *
 * @author q00654632
 * @since 2023-07-12
 */
@Data
public class AirGapPolicyWindowInfoRsp {
    /**
     * 唯一标识符
     */
    private String id;

    /**
     * 开始时间
     */
    private String startTime;

    /**
     * 结束时间
     */
    private String endTime;
}

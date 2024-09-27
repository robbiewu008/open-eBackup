/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dme;

import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;

/**
 * Dme Mount Qos
 *
 * @author l00272247
 * @since 2022-01-07
 */
@Data
@JsonInclude(JsonInclude.Include.NON_DEFAULT)
public class DmeMountQos {
    private int bandwidthMin;
    private int bandwidthMax;
    private int bandwidthBurst;
    private int iopsMin;
    private int iopsMax;
    private int iopsBurst;
    private int burstTime;
    private int latency;
}

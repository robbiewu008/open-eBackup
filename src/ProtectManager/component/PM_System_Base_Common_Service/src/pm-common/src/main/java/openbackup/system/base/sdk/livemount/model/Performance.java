/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.system.base.sdk.livemount.model;

import openbackup.system.base.common.constants.IsmNumberConstant;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.extern.slf4j.Slf4j;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Arrays;
import java.util.List;
import java.util.function.BooleanSupplier;
import java.util.stream.Stream;

import javax.validation.constraints.Max;
import javax.validation.constraints.Min;

/**
 * Smart Qos Policy
 *
 */
@Data
@JsonInclude(JsonInclude.Include.NON_DEFAULT)
@Slf4j
public class Performance {
    /**
     * LATENCY_VALUES
     */
    public static final List<Integer> LATENCY_VALUES;

    private static final Logger LOGGER = LoggerFactory.getLogger(Performance.class);

    private static final int LATENCY_VALUE1 = 500;

    private static final int LATENCY_VALUE2 = 1500;

    private static final int MAX = 999999999;

    static {
        LATENCY_VALUES = Arrays.asList(LATENCY_VALUE1, LATENCY_VALUE2);
    }

    /**
     * 1~999999999
     */
    @JsonProperty("min_bandwidth")
    @JsonInclude(JsonInclude.Include.NON_DEFAULT)
    @Min(0)
    @Max(MAX)
    private int minBandwidth;

    /**
     * 1~999999999
     */
    @JsonProperty("max_bandwidth")
    @JsonInclude(JsonInclude.Include.NON_DEFAULT)
    @Min(0)
    @Max(MAX)
    private int maxBandwidth;

    /**
     * 1~999999999
     */
    @JsonProperty("burst_bandwidth")
    @JsonInclude(JsonInclude.Include.NON_DEFAULT)
    @Min(0)
    @Max(MAX)
    private int burstBandwidth;

    /**
     * 100~999999999
     */
    @JsonProperty("min_iops")
    @JsonInclude(JsonInclude.Include.NON_DEFAULT)
    @Min(0)
    @Max(MAX)
    private int minIops;

    /**
     * 100~999999999
     */
    @JsonProperty("max_iops")
    @JsonInclude(JsonInclude.Include.NON_DEFAULT)
    @Min(0)
    @Max(MAX)
    private int maxIops;

    /**
     * 100~999999999
     */
    @JsonProperty("burst_iops")
    @JsonInclude(JsonInclude.Include.NON_DEFAULT)
    @Min(0)
    @Max(MAX)
    private int burstIops;

    /**
     * 1~999999999
     */
    @JsonProperty("burst_time")
    @JsonInclude(JsonInclude.Include.NON_DEFAULT)
    @Min(0)
    @Max(MAX)
    private int burstTime;

    /**
     * 500、1500
     */
    @JsonProperty("latency")
    @JsonInclude(JsonInclude.Include.NON_DEFAULT)
    private int latency;

    @JsonInclude(JsonInclude.Include.NON_DEFAULT)
    private int fileSystemKeepTime;

    /**
     * validate
     *
     * @return validate result
     */
    public boolean validate() {
        return Stream.<BooleanSupplier>of(this::validateBandwidth, this::validateIops, this::validateLatency)
            .allMatch(BooleanSupplier::getAsBoolean);
    }

    private boolean validate(int min, int max, int minLimit) {
        return min >= minLimit && max >= minLimit && max >= min && min <= MAX && max <= MAX;
    }

    private boolean validateBandwidth() {
        if (minBandwidth != 0 && maxBandwidth != 0) {
            if (!validate(minBandwidth, maxBandwidth, 1)) {
                return false;
            }
        }

        if (maxBandwidth == 0) {
            if (burstBandwidth != 0) {
                log.error("burstBandwidth is not allowed");
                return false;
            }
        } else if (burstBandwidth > 0) {
            if (burstBandwidth <= maxBandwidth) {
                log.error("burstBandwidth({}) <= maxBandwidth({})", burstBandwidth, maxBandwidth);
                return false;
            } else {
                // burstTime为0，表示已设置，该场景下burstTime为必选参数
                log.info("burstTime({})", burstTime);
                return burstTime > 0;
            }
        } else {
            LOGGER.info("burstBandwidth is valid");
        }
        return true;
    }

    private boolean validateIops() {
        if (minIops != 0 && maxIops != 0) {
            if (!validate(minIops, maxIops, IsmNumberConstant.HUNDRED)) {
                return false;
            }
        }
        if (maxIops == 0) {
            if (burstIops != 0) {
                log.error("burstIops is not allowed");
                return false;
            }
        } else if (burstIops > 0) {
            if (burstIops < IsmNumberConstant.HUNDRED) {
                log.error("burstIops({}) < 100", burstIops);
                return false;
            }
            if (burstIops <= maxIops) {
                log.error("burstIops({}) < maxIops({})", burstIops, maxIops);
                return false;
            }
            // burstTime为0，表示已设置，该场景下burstTime为必选参数
            log.info("burstTime({})", burstTime);
            return burstTime > 0;
        } else {
            LOGGER.info("burstIops is valid");
        }
        return true;
    }

    private boolean validateLatency() {
        // latency为0，表示没有设置
        if (latency > 0) {
            return Performance.LATENCY_VALUES.contains(latency);
        }
        return true;
    }
}

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
package openbackup.system.base.common.model.repository;

import openbackup.system.base.common.utils.JSONObject;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.math.BigDecimal;

/**
 * 存储池
 *
 * @author w00493811
 * @since 2020-07-01
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
public class StoragePool {
    /**
     * 存储池ID
     */
    @JsonProperty("ID")
    private String id;

    /**
     * 存储池名称
     */
    @JsonProperty("NAME")
    private String name;

    /**
     * 运行状态
     */
    @JsonProperty("RUNNINGSTATUS")
    private String status;

    /**
     * 总容量。
     * 单位：sectors
     */
    @JsonProperty("USERTOTALCAPACITY")
    private BigDecimal userTotalCapacity;

    /**
     * 空闲容量。
     * 单位：sectors
     */
    @JsonProperty("USERFREECAPACITY")
    private BigDecimal userFreeCapacity;

    /**
     * 已用容量。
     * 单位：sectors
     */
    @JsonProperty("USERCONSUMEDCAPACITY")
    private BigDecimal userConsumedCapacity;

    /**
     * 父对象ID
     */
    @JsonProperty("PARENTID")
    private String parentId;

    /**
     * 父对象名称
     */
    @JsonProperty("PARENTNAME")
    private String parentName;

    /**
     * 健康状态
     */
    @JsonProperty("HEALTHSTATUS")
    private String storagePoolHealthStatus;

    /**
     * 已用容量百分比
     */
    @JsonProperty("USERCONSUMEDCAPACITYPERCENTAGE")
    private String useRconsumedCapacityPercentage;

    /**
     * 已用容量阈值
     */
    @JsonProperty("USERCONSUMEDCAPACITYTHRESHOLD")
    private String useRconsumedCapacityThreshold;

    /**
     * 数据缩减比
     */
    @JsonProperty("SPACEREDUCTIONRATE")
    private String spaceReductionRate;

    /**
     * 查询存储池数据缩减比
     *
     * @return SpaceReductionRatio 数据缩减比
     */
    public double getSpaceReductionRatio() {
        JSONObject jsonObject = JSONObject.fromObject(this.spaceReductionRate);
        double numerator = jsonObject.getDouble("numerator", 1.0);
        double denominator = jsonObject.getDouble("denominator", 1.0);
        return numerator / denominator;
    }
}
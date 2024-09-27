/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.system.model;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.LinkedList;
import java.util.List;

/**
 * 配置状态
 *
 * @author l00347293
 * @since 2020-12-19
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class ConfigStatus {
    /**
     * 状态
     */
    private int status;

    /**
     * 进度错误编码
     */
    private String code;

    /**
     * 进度描述
     */
    private String desc;

    /**
     * 进度比率
     */
    private int rate;

    /**
     * 返回参数
     */
    private List<String> params = new LinkedList<>();
}
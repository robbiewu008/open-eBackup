/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package com.huawei.emeistor.console.controller.response;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

import java.util.List;

/**
 * 分页模板类
 *
 * @param <T> the body type
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2019-11-12
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class PageListResponse<T> {
    private int total;

    private Long currentCount;

    private Integer startIndex;

    private Integer pageSize;

    private List<T> userList;
}

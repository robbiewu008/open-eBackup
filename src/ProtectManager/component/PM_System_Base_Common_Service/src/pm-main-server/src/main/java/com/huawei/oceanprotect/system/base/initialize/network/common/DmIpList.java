/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import org.springframework.util.StringUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * 转换方法
 *
 * @author swx1010572
 * @since 2021-01-22
 */
public class DmIpList extends ArrayList<String> {
    /**
     * DmIpList
     *
     * @param list list
     */
    public DmIpList(List<String> list) {
        super();
        list.stream().forEach(str -> {
            if (!StringUtils.isEmpty(str)) {
                this.add("\"" + str + "\"");
            }
        });
    }
}

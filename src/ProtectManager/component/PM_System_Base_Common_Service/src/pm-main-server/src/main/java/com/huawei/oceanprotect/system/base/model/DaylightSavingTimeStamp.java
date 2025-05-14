/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 冬夏令时时间戳
 *
 * @author h00889002
 * @since 2024-12-08
 */
@Getter
@Setter
public class DaylightSavingTimeStamp {
    private List<String> timeStr;

    private List<String> timeStamp;
}

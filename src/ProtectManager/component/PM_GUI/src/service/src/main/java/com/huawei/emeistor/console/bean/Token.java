/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.emeistor.console.bean;

import com.huawei.emeistor.console.contant.NumberConstant;

import lombok.Getter;
import lombok.Setter;

import java.io.Serializable;

/**
 * 必须实现序列化才能写入redis
 *
 * @author lwx544155
 * @version [OceanStor DJ V100R003C00, 2020年03月05日]
 * @since 2020-11-30
 */
@Getter
@Setter
public class Token implements Serializable {
    private String token;

    private boolean modifyPassword;

    private long expireDay = NumberConstant.DEFAULT_VALUE;

    private String userId;

    private String timeZone;

    private String serviceProduct;
}

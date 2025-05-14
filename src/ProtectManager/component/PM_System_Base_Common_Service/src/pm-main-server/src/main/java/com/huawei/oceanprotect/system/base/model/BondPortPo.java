/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.model;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import java.util.List;

/**
 * 业务端口存放的绑定端口信息
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/22
 */
@Getter
@Setter
public class BondPortPo {
    /**
     * 绑定端口id
     */
    private String id;

    /**
     * 绑定端口名称
     */
    @Length(max = 31, message = "The length of port is 1 ~ 31.")
    private String name;

    /**
     * 以太网端口的名称列表
     */
    private List<String> portNameList;

    /**
     * 最大传输单元
     */
    private String mtu;
}

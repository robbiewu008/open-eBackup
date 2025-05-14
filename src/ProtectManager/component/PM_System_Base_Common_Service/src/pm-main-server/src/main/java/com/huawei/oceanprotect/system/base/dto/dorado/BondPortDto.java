/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.dto.dorado;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HealthStatus;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.RunningStatus;

import lombok.Data;

import java.util.List;

/**
 * 对外体现绑定端口
 *
 * @author swx1010572
 * @since 2022-12-13
 */
@Data
public class BondPortDto {
    /**
     * id
     */
    private String id;

    /**
     * 名称
     */
    private String name;

    /**
     * 端口归属控制器
     */
    private String ownIngController;

    /**
     * 健康状态
     */
    private HealthStatus healthStatus;

    /**
     * 运行状态
     */
    private RunningStatus runningStatus;

    /**
     * 端口id列表
     */
    private List<String> portIdList;

    /**
     * 端口名称str
     */
    private String bondInfo;

    /**
     * 端口mtu
     */
    private String mtu;
}

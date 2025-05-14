/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.model;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;

import lombok.Getter;
import lombok.Setter;

/**
 * 映射初始化表中存放的业务端口
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/22
 */

@Getter
@Setter
public class ServicePortPo {
    /**
     * 业务端口名称
     */
    private String name;

    /**
     * 业务端口id
     */
    private String id;

    /**
     * 端口类型:1-以太网,7-绑定端口,8-Vlan
     */
    private HomePortType homePortType;

    /**
     * 业务端口所属控制器
     */
    private String currentControllerId;

    /**
     * 业务端口配置的Vlan信息
     */
    private VlanPo vlan;

    /**
     * 业务端口配置的绑定端口
     */
    private BondPortPo bondPort;

    /**
     * 业务端口界面展示的角色，备份：2、复制：4、归档：11
     */
    private PortRole role;

    /**
     * 底座真正创建的角色类型
     */
    private PortRole dmRole;
}

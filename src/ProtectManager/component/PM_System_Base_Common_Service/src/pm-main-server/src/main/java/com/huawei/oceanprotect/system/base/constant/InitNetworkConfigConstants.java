/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.constant;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.HomePortType;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.PortRole;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.enums.VlanPortType;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * 系统初始化常量类
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/12/30
 */
public class InitNetworkConfigConstants {
    /**
     * 查询端口的id的条件：端口类型
     */
    public static final String HOME_PORT_TYPE = "homePortType";

    /**
     * 查询端口的id的条件：端口类型
     */
    public static final String ENABLE_ROUTE_SRC = "1";

    /**
     * 查询端口id的条件：端口名称
     */
    public static final String HOME_PORT_NAME = "homePortName";

    /**
     * 查询端口名称的条件：端口id
     */
    public static final String HOME_PORT_ID = "homePortId";

    /**
     * op创建绑定端口前缀
     */
    public static final String BOND_PORT_PREFIX = "op_bond_port";

    /**
     * 初始化支持的逻辑端口角色
     */
    public static final List<PortRole> serviceRoles = Arrays.asList(PortRole.SERVICE, PortRole.TRANSLATE,
            PortRole.ARCHIVE);

    /**
     * x9000可以修改控制器的逻辑端口角色
     */
    public static final List<PortRole> modifyControllerRoles = Arrays.asList(PortRole.SERVICE, PortRole.ARCHIVE);

    /**
     * 可复用物理端口类型
     */
    public static final List<HomePortType> MULTIPLEXING_PHYSICAL_PORT_TYPES = Collections
            .unmodifiableList(Arrays.asList(HomePortType.BINDING, HomePortType.VLAN, HomePortType.ETHERNETPORT));

    /**
     * 可复用物理端口类型
     */
    public static final List<VlanPortType> VLAN_PORT_TYPES = Collections
            .unmodifiableList(Arrays.asList(VlanPortType.ETH, VlanPortType.BOND));

    /**
     * 逻辑端口支持的端口类型：以太网口-1，绑定端口-7，vlan端口-8
     */

    public static final String ETH_PORT_TYPE = "1";

    /**
     * 逻辑端口支持的端口类型：以太网口-1，绑定端口-7，vlan端口-8
     */
    public static final String BOND_PORT_TYPE = "7";

    /**
     * 逻辑端口支持的端口类型：以太网口-1，绑定端口-7，vlan端口-8
     */
    public static final String VLAN_PORT_TYPE = "8";

    /**
     * X9000 0A控的以太网端口
     */
    public static final String CONTRONLLER_0A = "0A";

    /**
     * X9000 0B控的以太网端口
     */
    public static final String CONTRONLLER_0B = "0B";

    /**
     * X9000 0C控的以太网端口
     */
    public static final String CONTRONLLER_0C = "0C";

    /**
     * X9000 0D控的以太网端口
     */
    public static final String CONTRONLLER_0D = "0D";

    /**
     * 初始化超时时间：默认30分钟
     */
    public static final long INIT_OVER_TIME = 30 * 60 * 1000L;

    /**
     * 修改超时时间：默认30分钟
     */
    public static final long MODIFY_OVER_TIME = 30 * 60 * 1000L;

    /**
     * 单控最多创建业务端口数量(备份、归档)
     */
    public static final int SINGLE_CONTROLLER_MAX_LOGIC_PORTS_NUM = 32;

    /**
     * 单控最多创建业务端口数量(复制)
     */
    public static final int SINGLE_CONTROLLER_MAX_REPLICATION_LOGIC_PORTS_NUM = 8;

    /**
     * 备份业务端口
     */
    public static final String BACKUP_SERVICE_PORT_LABEL = "backup_service_port_label";

    /**
     * 复制业务端口
     */
    public static final String REPLICATION_SERVICE_PORT_LABEL = "replication_service_port_label";

    /**
     * 归档业务端口
     */
    public static final String ARCHIVE_SERVICE_PORT_LABEL = "archive_service_port_label";

    /**
     * 数据库保存漂移组的名称前缀
     */
    public static final String FAIL_OVER_GROUP = "FG";

    /**
     * 创建漂移组的名称前缀
     */
    public static final String FAIL_OVER_GROUP_PREFIX = "OP";

    /**
     * 查询漂移组的类型
     */
    public static final String FAIL_OVER_GROUP_TYPE = "289";
}

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.enums;

/**
 * 归档IP类型
 *
 * @author l00347293
 * @since 2020-12-19
 */
public enum DeviceType {
    /**
     * IPV4
     */
    DEVICE_TYPE("device_type_init_system_label"),
    MANAGER_IP("manager_ip_init_system_label"),
    IS_CREATE_NET_PLANE("is_create_net_plane_init_system_label"),
    NET_PLANE_NAME("net_plane_name_init_system_label"),
    CONFIG_PORT_INFORMATION("config_port_information_init_system_label"),
    CONTAINER_PORT_INFORMATION("container_port_information_init_system_label"),
    BUSINESS_IP("business_ip_init_system_label"),
    CONTAINER_IP("container_ip_init_system_label"),
    VLAN_ID("vlan_id_init_system_label"),
    SUB_NET_MASK("sub_net_mask_init_system_label"),
    GET_WAY("get_way_init_system_label"),
    ROUTE_INFORMATION("route_information_init_system_label"),
    /**
     * IPV6
     */
    MTU("mtu_init_system_label");

    private final String value;

    /**
     * 默认构造函数
     *
     * @param type 类型
     */
    DeviceType(String type) {
        value = type;
    }

    /**
     * 获取IP类型时ipv4还是ipv6
     *
     * @return ipType
     */
    public String getType() {
        return value;
    }
}
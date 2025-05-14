/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.constant;

/**
 * 系统初始化错误码
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/1/3
 */
public class InitConfigErrorCode {
    /**
     * 执行初始化操作时，由于控制器（{0}）未配置备份逻辑端口，初始化失败。
     */
    public static final long INITALIZATION_NOT_CONFIGURE_LOGIC_PORT_EXCEPTION = 1677935664L;

    /**
     * 执行系统初始化过程中，由于选择的逻辑端口（{0}）关联的绑定端口或者以太网端口状态为未连接，系统初始化失败。
     */
    public static final long INITALIZATION_LOGIC_PORT_STATUS_EXCEPTION = 1677935665L;

    /**
     * 执行系统初始化时，由于控制器（{0}）未配置备份网络，系统初始化失败。
     */
    public static final long INITALIZATION_NOT_CONFIGURE_BACKUP_NETWORK_EXCEPTION = 1677935666L;

    /**
     * 执行系统初始化操作时，由于逻辑端口（{0}）选择的数据协议有误，系统初始化失败。
     */
    public static final long INITALIZATION_LOGIC_PORT_SELECT_DATA_PROTOCOL_EXCEPTION = 1677935667L;

    /**
     * 执行系统初始化操作时，由于逻辑端口（{0}）角色错误，系统初始化失败。
     */
    public static final long INITALIZATION_LOGIC_PORT_ROLE_ERROR_EXCEPTION = 1677935668L;

    /**
     * 执行系统初始化操作时，由于控制器（{0}）配置的复制网络错误，系统初始化失败。
     */
    public static final long INITALIZATION_COPY_REPLICATION_NETWORK_ERROR_EXCEPTION = 1677935669L;

    /**
     * 执行系统初始化操作时，由于控制器（{0}）配置的备份网络错误，系统初始化失败。
     */
    public static final long INITALIZATION_COPY_BACKUP_NETWORK_ERROR_EXCEPTION = 1677935665L;

    /**
     * 执行系统初始化操作时，由于控制器（{0}）配置的归档网络错误，系统初始化失败。
     */
    public static final long INITALIZATION_COPY_ARCHIVE_NETWORK_ERROR_EXCEPTION = 1677935667L;

    /**
     * 执行系统初始化时，由于配置备份、归档、复制网络时配置的逻辑端口（{0}）名称或者ip重复，系统初始化失败。
     */
    public static final long INITALIZATION_IP_NAME_OF_LOGIC_PORT_REPEATABLE_EXCEPTION = 1677935670L;

    /**
     * 执行系统初始化操作时，由于逻辑端口（{0}）不存在，系统初始化失败。
     */
    public static final long INITALIZATION_LOGIC_PORT_NOT_EXIST_EXCEPTION = 1677935671L;

    /**
     * 原因：DeviceManager系统繁忙。
     * 建议：请稍后在DeviceManager查看逻辑端口创建结果，若创建成功请手动清除逻辑端口后重试该操作，若创建失败请直接重试。
     */
    public static final long PORT_ADD_FAILED_CASE_DEVICE_BUSY = 1677935127L;

    /**
     * 错误场景：创建相同业务端口类型时,由于不支持配置同网段VLAN类型和非VLAN类型的业务端口，操作失败。
     * 原因：不支持配置同网段VLAN类型和非VLAN类型的业务端口。
     * 建议：创建相同业务端口类型时，配置同网段内均为VLAN或均为非VLAN类型的业务端口后重试。
     */
    public static final long VLAN_AND_NON_VLAN_COEXIST_ERROR = 1677935675L;

    /**
     * 错误场景：创建VLAN类型的业务端口时，由于不支持配置同网段内不同VLAN ID的业务端口，操作失败。
     * 原因：不支持配置同网段内不同VLAN ID的业务端口。
     * 建议：创建VLAN类型的业务端口时，保持同网段内业务端口的VLAN ID相同后重试。
     */
    public static final long VLAN_ID_DIFFERENT_ERROR = 1677935672L;

    /**
     * 错误场景：删除、修改对象时检测到对象不存在
     * 原因：指定的对象不存在。
     * 建议：无。
     */
    public static final long RETURN_OBJ_NOT_EXIST = 1077948996L;
}

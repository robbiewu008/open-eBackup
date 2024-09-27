/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.dto;

import lombok.Getter;
import lombok.Setter;

/**
 *  磁盘信息
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022/12/24 17:54
 */
@Getter
@Setter
public class VolInfo {
    /**
     * 磁盘id
     */
    private String id;

    /**
     * 磁盘名称
     */
    private String name;

    /**
     * 盘类型：true-系统盘 false-数据盘
     */
    private String bootable;

    /**
     * 磁盘大小
     */
    private String size;

    /**
     * 是否共享盘
     */
    private String shareable;

    /**
     * 盘架构：arm/x86_64
     */
    private String architecture;

    /**
     * 是否为克隆卷
     */
    private String fullClone;

    /**
     * 卷类型
     */
    private String volumeType;

    /**
     * 挂载路径
     */
    private String device;
}
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.dto;

import lombok.Getter;
import lombok.Setter;

/**
 * OpenStack环境所需扩展字段
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-16
 */
@Getter
@Setter
public class OpenstackEnvExtendInfo {
    /**
     * service id
     */
    private String serviceId;

    /**
     * 是否注册service到OpenStack
     */
    private String registerService;

    /**
     * 注册service的代理ip
     */
    private String cpsIp;
}

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.oceanbase.common.dto;

import lombok.Data;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-13
 */
@Data
public class OBTenantInfo {
    /**
     * 租户名称
     */
    private String name;

    /**
     * 节点状态
     */
    private String linkStatus;

    public OBTenantInfo() {
        super();
    }

    public OBTenantInfo(String name) {
        this.name = name;
    }
}

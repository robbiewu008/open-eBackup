/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdbt.protection.access.provider.constant;

/**
 * GaussDBT集群节点状态枚举
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-23
 */
public enum GaussDBTNodeStatusEnum {
    /**
     * 在线状态
     */
    ONLINE("ONLINE"),

    /**
     * 离线状态
     */
    OFFLINE("OFFLINE"),

    /**
     * 停止状态
     */
    STOPPED("STOPPED");

    private final String status;

    /**
     * Constructor
     *
     * @param status 状态
     */
    GaussDBTNodeStatusEnum(String status) {
        this.status = status;
    }

    /**
     * getter
     *
     * @return status
     */
    public String getStatus() {
        return status;
    }
}

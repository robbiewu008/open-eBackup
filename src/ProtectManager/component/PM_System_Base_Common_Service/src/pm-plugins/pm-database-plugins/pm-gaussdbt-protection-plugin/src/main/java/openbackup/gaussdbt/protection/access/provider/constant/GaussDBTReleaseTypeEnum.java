/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.gaussdbt.protection.access.provider.constant;

/**
 * GaussDBT产品发行类型枚举
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/14
 */
public enum GaussDBTReleaseTypeEnum {
    /**
     * 集群
     */
    CLUSTER("Cluster"),

    /**
     * 单机
     */
    STAND_ALONE("Stand_alone");

    private final String type;

    /**
     * Constructor
     *
     * @param type 类型
     */
    GaussDBTReleaseTypeEnum(String type) {
        this.type = type;
    }

    /**
     * getter
     *
     * @return type
     */
    public String getType() {
        return type;
    }
}

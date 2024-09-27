/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.enums;

/**
 * HA证书操作类型
 *
 * @author fwx1022842
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/9/13
 */
public enum HaCertOperatorEnum {
    /**
     * 更新证书
     */
    UPDATE("update"),

    /**
     * 回滚证书
     */
    ROLLBACK("rollback");

    /**
     * 操作业务类型
     */
    private final String type;

    HaCertOperatorEnum(String type) {
        this.type = type;
    }

    public String getType() {
        return type;
    }
}
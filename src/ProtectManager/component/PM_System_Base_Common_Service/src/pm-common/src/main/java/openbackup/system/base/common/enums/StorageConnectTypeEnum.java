/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.common.enums;

/**
 * 存储库连接类型
 *
 * @author c30057247
 * @since 2024-05-17
 */
public enum StorageConnectTypeEnum {
    /**
     * 标准模式
     */
    STANDARD(0),

    /**
     * 连接字符串模式
     */
    CONNECT_INFO(1);

    private final int connectType;

    StorageConnectTypeEnum(int connectType) {
        this.connectType = connectType;
    }

    /**
     * 获取connect type
     *
     * @return connect type
     */
    public int getConnectType() {
        return connectType;
    }
}
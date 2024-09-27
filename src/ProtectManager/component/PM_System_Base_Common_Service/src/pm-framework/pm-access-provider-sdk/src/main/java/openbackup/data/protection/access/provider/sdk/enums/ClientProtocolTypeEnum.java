/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.enums;

/**
 * 客户端协议类型枚举类
 *
 * @author t30028453
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-02-20
 */
public enum ClientProtocolTypeEnum {
    /**
     * IP协议
     */
    IP(0),

    /**
     * DATA_TURBO协议
     */
    DATA_TURBO(1);

    /**
     * 客户端协议类型
     */
    private Integer clientProtocolType;

    ClientProtocolTypeEnum(int clientProtocolType) {
        this.clientProtocolType = clientProtocolType;
    }

    /**
     * 获取枚举值
     *
     * @return int
     */
    public int getClientProtocolType() {
        return clientProtocolType;
    }
}

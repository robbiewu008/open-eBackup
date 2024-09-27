/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.common.enums;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.HashMap;
import java.util.Map;

/**
 * 互联网协议版本
 *
 * @author swx1010572
 * @since 2021-01-03
 */
public enum AddressFamily {
    /**
     * v4
     */
    IPV4(0),
    /**
     * v6
     */
    IPV6(1);

    /**
     * 枚举常量图
     */
    private static final Map<String, AddressFamily> VALUE_ENUM_MAP = new HashMap<>(AddressFamily.values().length);

    /**
     * 协议版本
     */
    private final int addressFamily;

    static {
        for (AddressFamily each : AddressFamily.values()) {
            VALUE_ENUM_MAP.put(String.valueOf(each.getAddressFamily()), each);
        }
    }

    AddressFamily(int addressFamily) {
        this.addressFamily = addressFamily;
    }

    public int getAddressFamily() {
        return this.addressFamily;
    }

    /**
     * 获取值
     *
     * @return 协议版本
     */
    @JsonValue
    public int getRole() {
        return addressFamily;
    }

    @Override
    public String toString() {
        return String.valueOf(getAddressFamily());
    }

    /**
     * 创建
     *
     * @param addressFamily 协议版本值
     * @return 对应协议版本
     */
    @JsonCreator(mode = JsonCreator.Mode.DELEGATING)
    public static AddressFamily forValues(@JsonProperty("ADDRESSFAMILY") String addressFamily) {
        return VALUE_ENUM_MAP.get(addressFamily);
    }

    /**
     * 获取指定值
     *
     * @param ipType 需要的IP类型
     * @return 协议版本
     */
    public static AddressFamily fromString(String ipType) {
        if (ipType != null) {
            for (AddressFamily address : AddressFamily.values()) {
                if (ipType.equalsIgnoreCase(address.name())) {
                    return address;
                }
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
            "The parameter is not in the enumeration class.");
    }
}

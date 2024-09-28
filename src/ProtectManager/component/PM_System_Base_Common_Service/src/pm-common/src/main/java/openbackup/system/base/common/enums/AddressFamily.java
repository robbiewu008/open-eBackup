/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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

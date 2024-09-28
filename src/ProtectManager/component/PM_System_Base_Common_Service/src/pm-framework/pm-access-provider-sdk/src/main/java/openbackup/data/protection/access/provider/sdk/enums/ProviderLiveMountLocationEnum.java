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
package openbackup.data.protection.access.provider.sdk.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.Arrays;

/**
 * SDK中使用的LiveMountLocation枚举类
 *
 */
public enum ProviderLiveMountLocationEnum {
    ORIGINAL("original"),
    OTHERS("others");

    private String value;

    ProviderLiveMountLocationEnum(String value) {
        this.value = value;
    }

    /**
     * get enum by value
     *
     * @param value value
     * @return enum
     */
    @JsonCreator
    public static ProviderLiveMountLocationEnum get(String value) {
        return Arrays.stream(ProviderLiveMountLocationEnum.values())
            .filter(location -> location.value.equals(value))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }

    @JsonValue
    public String getValue() {
        return value;
    }
}

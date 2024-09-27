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
package openbackup.system.base.sdk.resource.enums;

/**
 * 桶类型枚举
 *
 * @author w00607005
 * @since 2023-11-15
 */
public enum ObjectStorageTypeEnum {
    OTHER(0, "OTHER"),
    OCEAN_STOR_PACIFIC(1, "OCEAN_STOR_PACIFIC"),
    HCS_OBS(2, "HCS_OBS"),
    ALI_CLOUD_OSS(3, "ALI_CLOUD_OSS");

    private final Integer order;

    private final String type;

    ObjectStorageTypeEnum(int order, String type) {
        this.order = order;
        this.type = type;
    }

    /**
     * 获取type
     *
     * @return type
     */
    public String getType() {
        return type;
    }

    /**
     * 获取order
     *
     * @return order
     */
    public Integer getOrder() {
        return order;
    }

    /**
     * 是否是合法的类型
     *
     * @param order order
     * @return true-合法， false-非法
     */
    public static boolean isObjectStorageType(int order) {
        for (ObjectStorageTypeEnum objectStorageTypeEnum : ObjectStorageTypeEnum.values()) {
            if (objectStorageTypeEnum.getOrder().equals(order)) {
                return true;
            }
        }
        return false;
    }
}

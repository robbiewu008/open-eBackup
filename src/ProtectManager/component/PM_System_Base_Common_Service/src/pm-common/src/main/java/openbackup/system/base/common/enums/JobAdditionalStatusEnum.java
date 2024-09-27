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

import openbackup.system.base.util.EnumUtil;

/**
 * 任务附加状态枚举
 *
 * @author w00616953
 * @since 2022-03-07
 */
public enum JobAdditionalStatusEnum {
    /**
     * 数据库可用
     */
    DATABASE_AVAILABLE("Database Available"),

    /**
     * 虚拟机可用
     */
    VIRTUAL_MACHINE_AVAILABLE("Virtual Machine Available");

    private final String value;

    JobAdditionalStatusEnum(String value) {
        this.value = value;
    }

    /**
     * 获取枚举对应的值
     *
     * @return 枚举对应的值
     */
    public String getValue() {
        return value;
    }

    /**
     * 根据值获取枚举实例
     *
     * @param additionalStatus 附加状态值
     * @return 附加状态枚举实例
     */
    public static JobAdditionalStatusEnum getEnum(String additionalStatus) {
        return EnumUtil.get(
                JobAdditionalStatusEnum.class, JobAdditionalStatusEnum::getValue, additionalStatus, false, true);
    }
}

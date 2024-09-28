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

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 小文件聚合枚举类
 *
 */
@Getter
@AllArgsConstructor
public enum SmallFileAggregationEnum {
    /**
     * 禁用的
     */
    DISABLE(1),

    /**
     * 启用的
     */
    ENABLE(2);

    private final int value;

    /**
     * 通过value获取SmallFileAggregationEnum
     *
     * @param value 值
     * @return 小文件聚合开启状态
     */
    public static SmallFileAggregationEnum getTapeLibraryStatus(int value) {
        for (SmallFileAggregationEnum smallFileAggregationEnum : values()) {
            if (value == smallFileAggregationEnum.getValue()) {
                return smallFileAggregationEnum;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "small file aggregation status not exists.");
    }
}

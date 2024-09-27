/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.common.enums;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 小文件聚合枚举类
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-26
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

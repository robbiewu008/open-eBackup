/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 */

package openbackup.system.base.common.utils.unit;

/**
 * 度量单位接口
 *
 * @author l90002863 李溜
 * @version V100R001C00
 * @since 2019-10-25
 */
public interface IUnitType {
    /**
     * 获取度量单位值
     *
     * @return long 度量单位值
     */
    long getUnit();

    /**
     * 是否需要点分显示
     *
     * @return boolean [返回类型说明]
     */
    boolean isGroupingUsed();
}

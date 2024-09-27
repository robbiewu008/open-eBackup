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

import lombok.Getter;

import java.util.Arrays;
import java.util.List;

/**
 * 副本支持的特性枚举类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.0.0]
 * @since: 2020/10/19
 **/
@Getter
public enum CopyFeatureEnum {
    /**
     * 索引
     */
    INDEX(0),
    /**
     * 恢复
     */
    RESTORE(1),
    /**
     * 及时恢复
     */
    INSTANT_RESTORE(2),
    /**
     * 挂载
     */
    MOUNT(3),

    /**
     * 防篡改
     */
    WORM(4);

    private Integer feature;

    CopyFeatureEnum(Integer feature) {
        this.feature = feature;
    }

    /**
     * 设置副本支持的特性，并按位运算获取最终的十进制结果
     *
     * @param features 支持特性列表
     * @return 十进制特性
     */
    public static Integer setAndGetFeatures(List<CopyFeatureEnum> features) {
        return features.stream()
            .mapToInt(CopyFeatureEnum::getFeature)
            .reduce(0, (current, next) -> current | 1 << next);
    }

    /**
     * 设置副本支持的特性，并按位运算获取最终的十进制结果
     *
     * @param features features
     * @return cal fetures val
     */
    public static Integer setAndGetFeatures(CopyFeatureEnum... features) {
        return setAndGetFeatures(Arrays.asList(features));
    }
}

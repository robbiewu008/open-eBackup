/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

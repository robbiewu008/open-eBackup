/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.opengauss.resources.access.enums;

import com.google.common.collect.ImmutableSet;

import java.util.Set;
import java.util.stream.Collectors;

/**
 * OpenGauss集群状态枚举
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-13
 */
public enum OpenGaussClusterStateEnum {
    /**
     * 正常
     */
    NORMAL("Normal"),

    /**
     * 不可用
     */
    UNAVAILABLE("Unavailable"),

    /**
     * 降级
     */
    DEGRADED("Degraded"),

    /**
     * 异常
     */
    ABNORMAL("Abnormal");

    private final String state;

    /**
     * GaussDBTClusterStateEnum
     *
     * @param state 类型
     */
    OpenGaussClusterStateEnum(String state) {
        this.state = state;
    }

    /**
     * getter
     *
     * @return 类型
     */
    public String getState() {
        return state;
    }

    /**
     * NORMAL、DEGRADED代表集群可用
     *
     * @return state
     */
    public static Set<String> getOnlineClusterState() {
        return ImmutableSet.of(NORMAL, DEGRADED)
            .stream()
            .map(OpenGaussClusterStateEnum::getState)
            .collect(Collectors.toSet());
    }
}

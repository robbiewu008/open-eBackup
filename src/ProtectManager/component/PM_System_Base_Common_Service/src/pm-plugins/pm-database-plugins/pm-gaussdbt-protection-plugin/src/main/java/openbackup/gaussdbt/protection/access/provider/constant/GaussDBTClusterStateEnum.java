/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdbt.protection.access.provider.constant;

import com.google.common.collect.ImmutableSet;

import java.util.Set;
import java.util.stream.Collectors;

/**
 * GaussDBT集群状态枚举
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-06
 */
public enum GaussDBTClusterStateEnum {
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
    GaussDBTClusterStateEnum(String state) {
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
     * 获取在线状态的集群集合
     *
     * @return state
     */
    public static Set<String> getOnlineClusterState() {
        return ImmutableSet.of(NORMAL, DEGRADED)
            .stream()
            .map(GaussDBTClusterStateEnum::getState)
            .collect(Collectors.toSet());
    }
}

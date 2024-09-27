/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.enums;

import org.apache.commons.lang3.StringUtils;

/**
 * 版本
 *
 * @author w00607005
 * @since 2023-07-18
 */
public enum PointReleaseEnum {
    /**
     * 1.2.1
     */
    V_1_2_1("1.2.1"),

    /**
     * 1.3.0
     */
    V_1_3_0("1.3.0"),

    /**
     * 1.5.0
     */
    V_1_5_0("1.5.0");

    private final String version;

    PointReleaseEnum(String version) {
        this.version = version;
    }

    public String getVersion() {
        return version;
    }

    /**
     * 是否是1.2.1版本
     *
     * @param version version
     * @return true-是，false-否
     */
    public static boolean isV121(String version) {
        return StringUtils.equals(version, V_1_2_1.getVersion());
    }

    /**
     * 是否是1.3.0版本
     *
     * @param version version
     * @return true-是，false-否
     */
    public static boolean isV130(String version) {
        return StringUtils.equals(version, V_1_3_0.getVersion());
    }
}
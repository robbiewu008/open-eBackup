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
package openbackup.system.base.common.model;

import openbackup.system.base.common.exception.LegoCheckedException;

/**
 * 比较版本信息用实体
 *
 */
public class Version implements Comparable<Version> {
    /**
     * -1
     */
    public static final int NEGATIVE_ONE = -1;

    /**
     * 0
     */
    public static final int ZERO = 0;

    /**
     * 1
     */
    public static final int ONE = 1;

    /**
     * 2
     */
    public static final int TWO = 2;

    /**
     * 3
     */
    public static final int THREE = 3;

    /**
     * RC
     */
    public static final String RC = "RC";

    /**
     * SPC
     */
    public static final String SPC = "SPC";

    /**
     * 分隔符：.
     */
    public static final String PERIOD_SEPARATOR = ".";

    // Generation数字
    private final int generation;

    // 主版本号
    private final int major;

    // 次版本号数值（可以是数字，也可以是 RC 标签的后缀）
    private int minor;

    // 是否是 RC 版本
    private boolean isRc;

    // 是否是 SPC 版本
    private boolean isSpc;

    // SPC(x) 中的数字部分，如果不是SPC则为-1
    private int spcNumber;

    // minor 次版本号字符串
    private String rcVersion;

    /**
     * 构造版本信息
     *
     * @param version 版本号字符串
     */
    public Version(String version) {
        String[] versions = version.split("\\.");
        if (versions.length < TWO) {
            throw new LegoCheckedException("Invalid version format: " + version);
        }
        this.generation = Integer.parseInt(versions[ZERO]);
        this.major = Integer.parseInt(versions[ONE]);

        // 判断 是否有minor 部分，可能是数字或者 RC(x)
        if (versions.length == TWO) {
            return;
        }
        this.rcVersion = versions[TWO]; //
        if (rcVersion.startsWith(RC)) {
            // 记录 RC 标签（如 RC1, RC2）
            this.isRc = true;
            this.minor = Integer.parseInt(rcVersion.substring(TWO));
        } else {
            this.minor = Integer.parseInt(rcVersion);
        }

        // 判断是否有 SPC 部分；默认设置为 -1
        this.spcNumber = NEGATIVE_ONE;
        if (versions.length > THREE && versions[THREE].startsWith(SPC)) {
            this.isSpc = true;
            this.spcNumber = Integer.parseInt(versions[THREE].substring(THREE));
        }
    }

    @Override
    public int compareTo(Version other) {
        // 默认情况下，比较时包括 SPC
        return compareTo(other, true, true);
    }

    // 提供一个灵活的比较方法，允许控制是否比较Minor 和 SPC
    private int compareTo(Version other, boolean isCompareWithMinor, boolean isCompareWithSpc) {
        // 按 generation 比较
        if (this.generation != other.generation) {
            return Integer.compare(this.generation, other.generation);
        }

        // 按 major 比较
        if (!isCompareWithMinor || (this.major != other.major)) {
            return Integer.compare(this.major, other.major);
        }

        // 按 minor 比较：普通数字 > RC
        if (this.isRc && !other.isRc) {
            return NEGATIVE_ONE;
        }
        if (!this.isRc && other.isRc) {
            // 普通数字大于 RC
            return ONE;
        }

        // minor 同为RC 或 同为数字时 比较值
        if (this.minor != other.minor) {
            return Integer.compare(this.minor, other.minor);
        }

        // 如果需要比较 SPC（补丁版本），则进入该部分
        if (!isCompareWithSpc) {
            return ZERO;
        }
        if (this.isSpc && other.isSpc) {
            return Integer.compare(this.spcNumber, other.spcNumber);
        }
        if (this.isSpc) {
            // SPC 比没有 SPC 的版本高
            return ONE;
        }
        return NEGATIVE_ONE;
    }

    @Override
    public String toString() {
        String versionStr = generation + "." + major;
        if (this.rcVersion != null) {
            versionStr += PERIOD_SEPARATOR + rcVersion;
        }
        if (this.isSpc) {
            versionStr += PERIOD_SEPARATOR + SPC + spcNumber;
        }
        return versionStr;
    }

    /**
     * 判断版本是否高于目标版本
     *
     * @param other 目标版本
     * @param isCompareWithMinor 是否比较Minor版本号
     * @param isCompareWithSpc 是否比较补丁版本号
     * @return 比较结果
     */
    public boolean isGreaterThan(Version other, boolean isCompareWithMinor, boolean isCompareWithSpc) {
        return compareTo(other, isCompareWithMinor, isCompareWithSpc) > ZERO;
    }
}
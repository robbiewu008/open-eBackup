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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.model.Version;

import org.apache.commons.lang3.StringUtils;

import java.util.HashMap;
import java.util.Map;

/**
 * 检查版本是否支持特性功能工具类
 *
 */
public class VersionRangeChecker {
    /**
     * vm恢复指定生成快照特性
     */
    public static final String FEATURE_VMWARE_RESTORE_SNAP_GEN = "vmware_restore_snap_gen";

    /**
     * FC副本校验状态为无效恢复特性
     */
    public static final String FEATURE_FUSION_COMPUTE_RESTORE_INVALID = "fusion_compute_restore_invalid";

    /**
     * FC扫描调用异步扫描接口
     */
    public static final String FEATURE_FUSION_COMPUTE_SCAN_ASYNC = "fusion_compute_scan_async";

    /**
     * Hyper-V扫描调用异步扫描接口
     */
    public static final String FEATURE_HYPER_V_SCAN_ASYNC = "hyper_v_scan_async";

    /**
     * Kubernetes FlexVolumn扫描调用异步扫描接口
     */
    public static final String FEATURE_KUBERNETS_FLEXVOLUMN_SCAN_ASYNC = "kubernets_flexvolumn_scan_async";

    /**
     * OP整机恢复特性
     */
    public static final String FEATURE_OPENSTACK_RESTORE_VM = "openstack_restore_vm";

    /**
     * 1.3版本
     */
    public static final String VERSION_ONE_POINT_THREE = "1.3";

    /**
     * 1.6.RC1版本
     */
    public static final String VERSION_ONE_POINT_SIX_RC_ONE = "1.6.RC1";

    private static final Map<String, Version> FEATURE_VERSION_RANGE = new HashMap<>();

    static {
        FEATURE_VERSION_RANGE.put(FEATURE_VMWARE_RESTORE_SNAP_GEN, new Version(VERSION_ONE_POINT_THREE));
        FEATURE_VERSION_RANGE.put(FEATURE_FUSION_COMPUTE_RESTORE_INVALID, new Version(VERSION_ONE_POINT_THREE));
        FEATURE_VERSION_RANGE.put(FEATURE_FUSION_COMPUTE_SCAN_ASYNC, new Version(VERSION_ONE_POINT_SIX_RC_ONE));
        FEATURE_VERSION_RANGE.put(FEATURE_OPENSTACK_RESTORE_VM, new Version(VERSION_ONE_POINT_THREE));
        FEATURE_VERSION_RANGE.put(FEATURE_HYPER_V_SCAN_ASYNC, new Version(VERSION_ONE_POINT_SIX_RC_ONE));
        FEATURE_VERSION_RANGE.put(FEATURE_KUBERNETS_FLEXVOLUMN_SCAN_ASYNC, new Version(VERSION_ONE_POINT_SIX_RC_ONE));
    }

    /**
     * 校验版本是否支持该特性,以1.6.0.SPC2为例:minor版本号为0,SPC版本号为2
     *
     * @param featureName 特性
     * @param versionStr 版本号
     * @param isCompareWithMinor 是否比较minor版本
     * @return 校验结果
     */
    public static boolean isFeatureSupportedCompareWithMinor(String featureName, String versionStr,
        boolean isCompareWithMinor) {
        if (!FEATURE_VERSION_RANGE.containsKey(featureName) || StringUtils.isBlank(versionStr)) {
            return true;
        }
        return new Version(versionStr).isGreaterThan(FEATURE_VERSION_RANGE.get(featureName), isCompareWithMinor, false);
    }

    /**
     * 校验版本是否支持该特性,暂时没有使用，示例扩展手段
     *
     * @param featureName 特性
     * @param versionStr 版本号
     * @param isCompareWithSpc 是否比较补丁版本
     * @return 校验结果
     */
    public static boolean isFeatureSupportedCompareWithSpc(String featureName, String versionStr,
        boolean isCompareWithSpc) {
        if (!FEATURE_VERSION_RANGE.containsKey(featureName) || StringUtils.isBlank(versionStr)) {
            return true;
        }
        return new Version(versionStr).isGreaterThan(FEATURE_VERSION_RANGE.get(featureName), true, isCompareWithSpc);
    }
}

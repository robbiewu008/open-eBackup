/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.util;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.model.beans.netplane.NetPlane;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import java.util.List;

/**
 * DM 返回值处理公共类
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-09
 */
public class DeviceManagerResponseUtils {
    /**
     * 平面网络获取公共方法
     *
     * @param name 平面网络名称
     * @param netPlaneList 全部平面网络列表
     * @return 获取的及时平面网络信息
     */
    public static NetPlane pourNetPlaneList(String name, List<NetPlane> netPlaneList) {
        return netPlaneList.stream()
            .filter(netPlane -> name.equals(netPlane.getName()))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OPERATION_FAILED));
    }
}

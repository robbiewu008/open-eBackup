/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdb.protection.access.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.gaussdb.protection.access.constant.GaussDBConstant;
import openbackup.gaussdb.protection.access.constant.GaussDBErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import java.util.List;

/**
 * 功能描述: 校验 GaussDb各种参数的基本格式等信息
 *
 * @author t30021437
 * @version [OceanProtect x8000 1.3.0]
 * @since 2022-02-06
 */
@Slf4j
public class GaussDBValidator {
    /**
     * 检查已经接入的 GaussDb cluster是否已经超过规格
     *
     * @param existingEnvironments 已接入的GaussDb环境列表
     */
    public static void checkGaussDbCount(List<ProtectedEnvironment> existingEnvironments) {
        if (existingEnvironments.size() >= GaussDBConstant.GAUSSDB_CLUSTER_MAX_COUNT) {
            throw new LegoCheckedException(GaussDBErrorCode.GAUSSDB_RESOURCE_REACHED_THE_UPPER_LIMIT,
                new String[] {String.valueOf(GaussDBConstant.GAUSSDB_CLUSTER_MAX_COUNT)});
        }
    }
}
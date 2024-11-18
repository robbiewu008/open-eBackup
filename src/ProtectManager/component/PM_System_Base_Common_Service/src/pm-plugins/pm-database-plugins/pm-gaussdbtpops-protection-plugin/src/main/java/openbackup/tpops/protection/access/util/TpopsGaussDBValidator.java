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
package openbackup.tpops.protection.access.util;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.tpops.protection.access.constant.TpopsGaussDBConstant;
import openbackup.tpops.protection.access.constant.TpopsGaussDBErrorCode;

import lombok.extern.slf4j.Slf4j;

import java.util.List;

/**
 * 功能描述: 校验 GaussDb各种参数的基本格式等信息
 *
 */
@Slf4j
public class TpopsGaussDBValidator {
    /**
     * 检查已经接入的 GaussDb cluster是否已经超过规格
     *
     * @param existingEnvironments 已接入的GaussDb环境列表
     */
    public static void checkGaussDbCount(List<ProtectedEnvironment> existingEnvironments) {
        if (existingEnvironments.size() >= TpopsGaussDBConstant.GAUSSDB_CLUSTER_MAX_COUNT) {
            throw new LegoCheckedException(TpopsGaussDBErrorCode.GAUSSDB_RESOURCE_REACHED_THE_UPPER_LIMIT,
                new String[] {String.valueOf(TpopsGaussDBConstant.GAUSSDB_CLUSTER_MAX_COUNT)});
        }
    }
}
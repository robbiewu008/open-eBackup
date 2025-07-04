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
package openbackup.gaussdbt.protection.access.provider.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ValidateUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import java.util.Objects;

/**
 * GaussDBT校验工具类
 *
 */
@Slf4j
public class GaussDBTValidator {
    /**
     * 验证GaussDBT资源名称方法
     *
     * @param name 资源名称
     */
    public static void verifyName(String name) {
        if (!ValidateUtil.match(RegexpConstants.NAME_STR, name)) {
            log.error("gaussdbt name: {} is incorrect.", name);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The format of name is not correct.");
        }
    }

    /**
     * 验证GaussDBT资源数量方法
     *
     * @param count 资源数量
     */
    public static void verifyCount(int count) {
        if (count >= GaussDBTConstant.GAUSSDBT_CLUSTER_MAX_COUNT) {
            log.error("gaussdbt resource register exceed max count: {}.", GaussDBTConstant.GAUSSDBT_CLUSTER_MAX_COUNT);
            throw new LegoCheckedException(DatabaseErrorCode.RESOURCE_REACHED_THE_UPPER_LIMIT,
                new String[] {String.valueOf(GaussDBTConstant.GAUSSDBT_CLUSTER_MAX_COUNT)},
                "gaussdbt resource register exceed max count.");
        }
    }

    /**
     * 检测deployType
     *
     * @param deployType 前端传入的deployType
     * @param realDeployType 真实的deployType
     */
    public static void verifyDeployType(String deployType, String realDeployType) {
        if (!Objects.equals(deployType, realDeployType)) {
            log.error("GaussDBT deployType error.deployType: {}, realType:{}", deployType, realDeployType);
            throw new LegoCheckedException(DatabaseErrorCode.RESOURCE_DEPLOY_TYPE_ERROR,
                new String[] {DatabaseDeployTypeEnum.getLabel(deployType),
                        DatabaseDeployTypeEnum.getLabel(realDeployType)}, "GaussDBT deployType error.");
        }
    }

    /**
     * 检查集群节点是否不完全
     *
     * @param count 前端传入的数量
     * @param realCount 实际数量
     */
    public static void verityNodesCount(int count, int realCount) {
        if (count < realCount) {
            throw new LegoCheckedException(CommonErrorCode.NOT_INCLUDE_ALL_CLUSTER_INSTANCES, "nodes is partial.");
        }
    }
}

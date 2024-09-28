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
package openbackup.tdsql.resources.access.util;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.MapUtils;

import java.util.Map;

/**
 * 功能描述
 *
 */
@Slf4j
public class TdsqlValidator {
    private TdsqlValidator() {
    }

    /**
     * 创建/更新Tdsql集群时，校验参数
     *
     * @param protectedEnvironment 集群环境
     */
    public static void checkTdsql(ProtectedEnvironment protectedEnvironment) {
        verifyEnvName(protectedEnvironment.getName());
        Map<String, String> extendInfo = protectedEnvironment.getExtendInfo();
        if (MapUtils.isEmpty(extendInfo)) {
            log.error("TDSQL cluster extendInfo is null.");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "TDSQL cluster extendInfo is null.");
        }
    }

    private static void verifyEnvName(String name) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(name);
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(name);
    }
}

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

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.instance.TdsqlInstance;

import lombok.extern.slf4j.Slf4j;

/**
 * 功能描述TdsqlInstanceValidator参数校验
 *
 * @author z30047175
 * @since 2023-05-25
 */
@Slf4j
public class TdsqlInstanceValidator {
    private TdsqlInstanceValidator() {
    }

    /**
     * 创建/更新TDSQL实例时，校验参数
     *
     * @param resource 实例
     */
    public static void checkTdsqlInstance(ProtectedResource resource) {
        log.info("begin to checkTdsqlInstance params");
        TdsqlInstance instance = getInstance(resource);
        checkGroupNode(instance);
    }

    private static void checkGroupNode(TdsqlInstance instance) {
        if (instance.getGroups() == null || instance.getGroups().size() == 0) {
            log.warn("group size is zero");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "group size is zero");
        }
        if (instance.getGroups().get(0).getDataNodes() == null
            || instance.getGroups().get(0).getDataNodes().size() == 0) {
            log.warn("data node size is zero");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "data node size is zero");
        }
        if (instance.getGroups().get(0).getSetId() == null
            || instance.getGroups().get(0).getSetId().length() == 0) {
            log.warn("Tdsql instance setID is empty");
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Tdsql instance setID is empty");
        }
    }

    private static TdsqlInstance getInstance(ProtectedResource resource) {
        log.info("begin to getInstance in TdsqlInstanceValidator");
        String instanceJson = resource.getExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO);
        return JsonUtil.read(instanceJson, TdsqlInstance.class);
    }
}

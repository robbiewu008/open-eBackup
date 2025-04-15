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
package openbackup.goldendb.protection.access.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;
import openbackup.system.base.common.utils.json.JsonUtil;

/**
 * 功能描述 GoldenDbValidator参数校验
 *
 */
@Slf4j
public class GoldenDbInstanceValidator {
    private GoldenDbInstanceValidator() {
    }

    /**
     * 创建/更新GoldenDb实例时，校验参数
     *
     * @param resource 实例
     */
    public static void checkGoldenDbInstance(ProtectedResource resource) {
        log.info("begin to checkGoldenDbInstance params");
        GoldenInstance instance = getInstance(resource);
        checkGtmNode(instance);
    }

    private static void checkGtmNode(GoldenInstance instance) {
        if (instance.getGtm() == null || instance.getGtm().size() == 0) {
            // 根据业务需求，现在不需要gtm节点也可以进行注册
            log.warn("gtmNode size is zero");
        }
    }

    private static GoldenInstance getInstance(ProtectedResource resource) {
        String instanceJson = resource.getExtendInfoByKey(GoldenDbConstant.CLUSTER_INFO);
        return JsonUtil.read(instanceJson, GoldenInstance.class);
    }
}

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
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;

import com.alibaba.fastjson.JSON;

import java.util.HashMap;

/**
 * 功能描述
 *
 */
public class TdsqlUtils {
    /**
     * 获取连通性检查的resource对象
     *
     * @param resource 受保护资源
     * @param singleNode 节点信息
     * @param subType 资源类型
     * @return ProtectedResource
     */
    public static ProtectedResource getProtectedResource(ProtectedResource resource, Object singleNode,
        String subType) {
        HashMap<String, String> extendInfo = new HashMap<>();
        extendInfo.put(TdsqlConstant.CHECK_TYPE, TdsqlConstant.CHECK_NODE);
        extendInfo.put(TdsqlConstant.SINGLENODE, JSON.toJSONString(singleNode));

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(resource.getUuid());
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setSubType(subType);
        protectedResource.setExtendInfo(extendInfo);
        protectedResource.setAuth(resource.getAuth());
        return protectedResource;
    }
}

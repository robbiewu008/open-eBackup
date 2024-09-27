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
package openbackup.tpops.protection.access.provider;

import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Objects;

/**
 * GaussDBTPOPS擦除不可更新字段provider
 *
 * @author s30031954
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-06-13
 */
@Component
@Slf4j
public class GaussDBTpopsAlwaysOnProvider extends DefaultResourceProvider {
    /**
     * 将指定资源特殊字段擦除，避免更新。
     *
     * @param resource 资源
     */
    @Override
    public void cleanUnmodifiableFieldsWhenUpdate(ProtectedResource resource) {
        // GaussDBTPOPS场景下parentName表示实例所属项目名，应设置为可更新
        resource.setRootUuid(null);
        resource.setSubType(null);
        resource.setUserId(null);
        resource.setAuthorizedUser(null);
        resource.setCreatedTime(null);
        resource.setProtectionStatus(null);
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param object object 受保护资源
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && Objects.equals(object.getSubType(),
            ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType());
    }
}

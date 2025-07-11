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
package openbackup.exchange.protection.access.provider;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.persistence.model.ResourceRepositoryQueryParams;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceAlarmProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * 功能描述
 *
 */
@Component
@Slf4j
public class ExchangeDatabaseAlarmProvider implements ResourceAlarmProvider {
    @Autowired
    ProtectedResourceRepository repository;

    /**
     * 获取资源名称
     *
     * @param resource 资源
     * @return String 默认发资源名称，插件自定义资源名称的返回值
     */
    @Override
    public String getAlarmResourceName(ProtectedResource resource) {
        List<ProtectedResource> protectedResources = repository.query(new ResourceRepositoryQueryParams(true, 0, 20,
                Collections.singletonMap("uuid", Collections.singletonList(resource.getUuid())), new String[0]))
            .map(ProtectedResourcePo::toProtectedResource)
            .getItems();
        if (!protectedResources.isEmpty()) {
            ProtectedResource protectedResource = protectedResources.get(0);
            return protectedResource.getParentName() + "+" + protectedResource.getName();
        }
        log.info("uuid: {}, no data was found in the database", resource.getUuid());
        return resource.getParentName() + "+" + resource.getName();
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.EXCHANGE_DATABASE.getType().equals(object.getSubType());
    }
}

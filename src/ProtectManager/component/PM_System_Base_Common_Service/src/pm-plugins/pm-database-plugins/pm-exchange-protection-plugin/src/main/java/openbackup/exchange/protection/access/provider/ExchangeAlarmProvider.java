/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.exchange.protection.access.provider;

import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.persistence.model.ResourceRepositoryQueryParams;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceAlarmProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Objects;

/**
 * 功能描述
 *
 * @author c30058517
 * @since 2024-04-02
 */
@Component
@Slf4j
public class ExchangeAlarmProvider implements ResourceAlarmProvider {
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
        List<ProtectedResource> protectedResources = repository.query(
            new ResourceRepositoryQueryParams(true, 0, 20,
                Collections.singletonMap("uuid", Collections.singletonList(resource.getUuid())), new String[0])).map(
            ProtectedResourcePo::toProtectedResource).getItems();
        if (!protectedResources.isEmpty()) {
            ProtectedResource protectedResource = protectedResources.get(0);
            return protectedResource.getParentName()
                + "+" + protectedResource.getExtendInfo().get("DatabaseName")
                + "+" + protectedResource.getName();
        }
        log.info("uuid: {}, no data was found in the database", resource.getUuid());
        return resource.getParentName() + "+" + resource.getExtendInfo().get("DatabaseName") + "+" + resource.getName();
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.EXCHANGE_MAILBOX.getType().equals(object.getSubType());
    }
}

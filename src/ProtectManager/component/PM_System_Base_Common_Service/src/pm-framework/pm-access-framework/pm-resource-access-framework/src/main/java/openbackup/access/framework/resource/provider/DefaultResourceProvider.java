/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.access.framework.resource.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 默认ResourceProvider
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-18
 */
@Component("defaultResourceProvider")
@Slf4j
public class DefaultResourceProvider implements ResourceProvider {
    @Override
    public boolean applicable(ProtectedResource object) {
        return false;
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        log.debug("resource provider default. no need execute before create");
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.debug("resource provider default. no need execute before update");
    }
}

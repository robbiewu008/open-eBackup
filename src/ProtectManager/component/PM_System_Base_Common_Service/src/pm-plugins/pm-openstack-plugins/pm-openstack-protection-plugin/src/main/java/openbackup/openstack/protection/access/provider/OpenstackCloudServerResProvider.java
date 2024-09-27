/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.provider;

import openbackup.access.framework.resource.provider.DefaultResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 云主机资源Provider
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-2-22
 */
@Slf4j
@Component
public class OpenstackCloudServerResProvider extends DefaultResourceProvider {
    @Override
    public ResourceFeature getResourceFeature() {
        log.info("get openstack resource features.");
        ResourceFeature resourceFeature = ResourceFeature.defaultValue();
        resourceFeature.setSupportedLanFree(false);
        return resourceFeature;
    }

    @Override
    public boolean applicable(ProtectedResource resource) {
        return ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.equalsSubType(resource.getSubType());
    }
}

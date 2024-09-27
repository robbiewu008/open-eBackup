/*
 *
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

package openbackup.database.base.plugin.provider;

import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Objects;

/**
 * GeneralDb的EnvironmentProvider实现
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-26
 */
@Component
public class GeneralDbEnvironmentProvider implements EnvironmentProvider {
    private final UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;

    public GeneralDbEnvironmentProvider(UnifiedConnectionCheckProvider unifiedConnectionCheckProvider) {
        this.unifiedConnectionCheckProvider = unifiedConnectionCheckProvider;
    }

    @Override
    public boolean applicable(String object) {
        return Objects.equals(object, ResourceSubTypeEnum.GENERAL_DB.getType());
    }

    @Override
    public void register(ProtectedEnvironment environment) {
    }

    @Override
    public void validate(ProtectedEnvironment environment) {
        unifiedConnectionCheckProvider.checkConnection(environment);
    }
}

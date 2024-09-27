/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.mysql.resources.access.provider;

import openbackup.data.protection.access.provider.sdk.copy.CapabilityProvider;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * Mysql副本支持操作
 *
 * @author w30042425
 * @since 2023-12-28
 */
@Component
@Slf4j
public class MysqlCapabilityProvider implements CapabilityProvider {
    private static final List<CopyFeatureEnum> SUPPORTED_FEATURES = Arrays.asList(CopyFeatureEnum.RESTORE,
        CopyFeatureEnum.MOUNT);

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType().equals(object);
    }

    @Override
    public List<CopyFeatureEnum> supportFeatures() {
        return SUPPORTED_FEATURES;
    }
}

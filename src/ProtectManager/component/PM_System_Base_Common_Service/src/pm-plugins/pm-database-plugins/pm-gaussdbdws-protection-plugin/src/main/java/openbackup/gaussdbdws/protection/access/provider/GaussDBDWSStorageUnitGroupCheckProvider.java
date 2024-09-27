/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.gaussdbdws.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.repository.StorageUnitGroupCheckProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableList;

import org.springframework.stereotype.Component;

/**
 * GaussDBDWSStorageUnitGroupCheckProvider
 *
 * @author l00853347
 * @version [DataBackup 1.6.0]
 * @since 2024-04-02
 */
@Component
public class GaussDBDWSStorageUnitGroupCheckProvider implements StorageUnitGroupCheckProvider {
    @Override
    public boolean applicable(String subType) {
        return ImmutableList.of(ResourceSubTypeEnum.GAUSSDB_DWS.getType(),
                ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType(), ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType(),
                ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType()).contains(subType);
    }

    @Override
    public boolean isSupportParallelStorage() {
        return Boolean.TRUE;
    }
}

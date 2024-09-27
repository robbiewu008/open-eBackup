/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.livemount;

import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.provider.DefaultLiveMountServiceProvider;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-06-01
 */
@Slf4j
@Component
public class TdsqlLiveMountServiceProvider extends DefaultLiveMountServiceProvider {
    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType().equals(subType);
    }

    @Override
    public LiveMountEntity buildLiveMountEntity(LiveMountObject liveMountObject, ResourceEntity sourceResourceEntity,
        ResourceEntity targetResourceEntity) {
        LiveMountEntity entity = super.buildLiveMountEntity(liveMountObject, sourceResourceEntity,
            targetResourceEntity);
        List<LiveMountFileSystemShareInfo> fileSystemShareInfoList = liveMountObject.getFileSystemShareInfoList();
        if (VerifyUtil.isEmpty(fileSystemShareInfoList)) {
            return entity;
        }
        for (LiveMountFileSystemShareInfo shareInfo : fileSystemShareInfoList) {
            shareInfo.setFileSystemName(shareInfo.getFileSystemName() + targetResourceEntity.getUuid());
        }
        entity.setTargetResourceName(targetResourceEntity.getEnvironmentName());
        entity.setTargetResourcePath(StringUtils.isEmpty(targetResourceEntity.getPath())
            ? targetResourceEntity.getEnvironmentEndPoint()
            : targetResourceEntity.getPath());
        entity.setFileSystemShareInfo(JSONArray.fromObject(fileSystemShareInfoList).toString());
        return entity;
    }
}

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 */

package openbackup.mysql.resources.access.livemount;

import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.provider.DefaultLiveMountServiceProvider;
import openbackup.mysql.resources.access.enums.MysqlResourceSubTypeEnum;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 功能描述: MysqlLiveMountServiceProvider
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-09
 */
@Slf4j
@Component
public class MysqlLiveMountServiceProvider extends DefaultLiveMountServiceProvider {
    @Override
    public boolean applicable(String subType) {
        return MysqlResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType().equals(subType);
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
        entity.setFileSystemShareInfo(JSONArray.fromObject(fileSystemShareInfoList).toString());
        return entity;
    }
}
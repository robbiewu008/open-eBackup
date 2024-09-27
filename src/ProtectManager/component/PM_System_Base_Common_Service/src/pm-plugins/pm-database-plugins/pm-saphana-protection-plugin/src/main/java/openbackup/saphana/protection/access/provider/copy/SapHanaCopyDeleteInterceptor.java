/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.saphana.protection.access.provider.copy;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * SapHana副本删除Provider
 *
 * @author l30061432
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-09-03
 */
@Slf4j
@Component
public class SapHanaCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    public SapHanaCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.SAPHANA_DATABASE.equalsSubType(resourceSubType);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsCumulativeIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 删除下一个全量副本前所有副本
        return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
    }
}

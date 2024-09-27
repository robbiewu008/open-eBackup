/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.data.protection.access.provider.sdk.backup;

import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.NextBackupParams;

import lombok.Getter;
import lombok.Setter;

import java.util.Collections;
import java.util.List;
import java.util.Locale;

/**
 * 更改下次备份参数
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-13
 */
@Setter
@Getter
public class NextBackupModifyReq {
    /**
     * 资源id集合
     */
    private List<String> resourceIds;

    /**
     * 下次备份参数
     */
    private NextBackupParams nextBackupParams;

    /**
     * 构建下次增量转全量的label信息
     *
     * @param resourceId 资源id
     * @param causeEnum 下次备份的原因
     * @return nextBackupModifyReq 下次备份的信息
     */
    public static NextBackupModifyReq build(String resourceId, NextBackupChangeCauseEnum causeEnum) {
        return build(Collections.singletonList(resourceId), causeEnum);
    }

    /**
     * 构建下次增量转全量的label信息
     *
     * @param resourceIds 资源id集合
     * @param causeEnum 下次备份的原因
     * @return nextBackupModifyReq 下次备份的信息
     */
    public static NextBackupModifyReq build(List<String> resourceIds, NextBackupChangeCauseEnum causeEnum) {
        return build(resourceIds, BackupTypeEnum.FULL.name().toLowerCase(Locale.ROOT), causeEnum);
    }

    /**
     * 构建下次增量转全量的label信息
     *
     * @param resourceIds 资源id集合
     * @param nextBackupType 下次备份类型
     * @param causeEnum 下次备份的原因
     * @return nextBackupModifyReq 下次备份的信息
     */
    public static NextBackupModifyReq build(
            List<String> resourceIds, String nextBackupType, NextBackupChangeCauseEnum causeEnum) {
        NextBackupModifyReq nextBackupModifyReq = new NextBackupModifyReq();
        nextBackupModifyReq.setResourceIds(resourceIds);
        nextBackupModifyReq.setNextBackupParams(new NextBackupParams(nextBackupType, causeEnum.getLabel()));
        return nextBackupModifyReq;
    }
}

/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.postgre.protection.access.provider.resource;

import static openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants.LOG;

import com.alibaba.fastjson.JSONObject;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.Optional;

/**
 * Postgre副本删除provider
 *
 */
@Slf4j
@Component
public class PostgreCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    /**
     * Constructor
     *
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     */
    public PostgreCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.equalsSubType(subType)
            || ResourceSubTypeEnum.POSTGRE_INSTANCE.equalsSubType(subType);
    }

    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return false;
    }

    /**
     * 删除全量副本时，如果当前副本之前有日志副本，则只删除当前副本
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        Optional<Copy> latestLogBackupCopy = copyRestApi.queryLatestFullBackupCopies(thisCopy.getResourceId(),
            thisCopy.getGn(), BackupTypeEnum.LOG.getAbbreviation());
        if (latestLogBackupCopy.isPresent()) {
            Optional<Copy> nextLogBackupCopy = Optional.ofNullable(
                CopyUtil.getNextCopyByType(copies, BackupTypeConstants.LOG, thisCopy.getGn()));
            if (nextLogBackupCopy.isPresent()) {
                JSONObject resourcePropertiesOne = JSONObject.parseObject(latestLogBackupCopy.get().getProperties());
                String endtime = resourcePropertiesOne.getString("endTime");
                log.info("latestLogBackupCopy endtime: {}, uuid is {}", endtime, latestLogBackupCopy.get().getUuid());
                JSONObject resourcePropertiesTwo = JSONObject.parseObject(nextLogBackupCopy.get().getProperties());
                String begintime = resourcePropertiesTwo.getString("beginTime");
                log.info("nextLogBackupCopy begintime: {}, uuid is {}", begintime, nextLogBackupCopy.get().getUuid());
                if (endtime.equals(begintime)) {
                    log.warn("Logs should be continuous; only the current full copy should be deleted.");
                    return Collections.emptyList();
                } else {
                    log.warn("Delete the current full copy and the associated log copies.");
                    return getCopy(copies, thisCopy, nextFullCopy);
                }
            } else {
                log.warn("only the current full copy should be deleted.");
                return Collections.emptyList();
            }
        }

        return getCopy(copies, thisCopy, nextFullCopy);
    }

    private List<String> getCopy(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        int format = CopyUtil.getFormat(thisCopy).orElse(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        if (Objects.equals(format, CopyFormatEnum.INNER_DIRECTORY.getCopyFormat())) {
            return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        }
        if (Objects.equals(format, CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat())) {
            return getAssociatedTypeCopiesByBackup(copies, thisCopy, nextFullCopy, Collections.singletonList(LOG));
        }
        return Collections.emptyList();
    }
}
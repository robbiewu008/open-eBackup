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
package openbackup.opengauss.resources.access.interceptor;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.interceptor.AbstractDbCopyDeleteInterceptor;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.ArrayList;

/**
 * OpenGauss副本删除Provider
 *
 */
@Slf4j
@Component
public class OpenGaussCopyDeleteInterceptor extends AbstractDbCopyDeleteInterceptor {
    /**
     * 构造器注入
     *
     * @param copyRestApi 副本查询接口
     * @param resourceService resourceService
     */
    public OpenGaussCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService) {
        super(copyRestApi, resourceService);
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType(),
            ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType()).contains(subType);
    }

    @Override
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        List<Copy> differenceCopies = CopyUtil.getCopiesByCopyType(copies, BackupTypeConstants.DIFFERENCE_INCREMENT);
        return CopyUtil.getCopyUuidsBetweenTwoCopy(differenceCopies, thisCopy, nextFullCopy);
    }

    /**
     * 删除全量副本时，要删除此副本到下一个全量副本之间的副本
     *
     * @param copies 此副本之后的所有副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        // 前一个日志副本
        Copy previousLogBackupCopy = copyRestApi.queryLatestFullBackupCopies(thisCopy.getResourceId(),
                thisCopy.getGn(), BackupTypeEnum.LOG.getAbbreviation()).orElse(null);
        // 后一个日志副本
        Copy nextLogBackupCopy = copies.stream()
                .filter(copy -> copy.getBackupType() == BackupTypeConstants.LOG.getAbBackupType())
                .findFirst().orElse(null);
        // 如果之前没有日志副本或者之后没有日志副本
        if (previousLogBackupCopy == null || nextLogBackupCopy == null) {
            return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        }
        long previousLogCopyEndTime = Long.parseLong(JSONObject.fromObject(previousLogBackupCopy.getProperties())
                .getString("end_time"));
        long nextLogCopyStartTime = Long.parseLong(JSONObject.fromObject(nextLogBackupCopy.getProperties())
                .getString("begin_time"));
        // 如果后一个日志副本连不上前一个日志副本
        if (previousLogCopyEndTime < nextLogCopyStartTime) {
            return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
        }
        List<BackupTypeConstants> associatedTypes = new ArrayList<>(Arrays.asList(
                BackupTypeConstants.DIFFERENCE_INCREMENT, BackupTypeConstants.CUMULATIVE_INCREMENT));
        return getAssociatedTypeCopiesByBackup(copies, thisCopy, nextFullCopy, associatedTypes);
    }

    @Override
    protected void handleTask(DeleteCopyTask task, CopyInfoBo copy) {
        // 不需要UBC后置任务删除日志备份副本，设置为false
        Map<String, String> map = new HashMap<>(1);
        map.put(OpenGaussConstants.IS_DELETE_RELATIVE_COPIES, String.valueOf(false));
        task.addParameters(map);
    }
}

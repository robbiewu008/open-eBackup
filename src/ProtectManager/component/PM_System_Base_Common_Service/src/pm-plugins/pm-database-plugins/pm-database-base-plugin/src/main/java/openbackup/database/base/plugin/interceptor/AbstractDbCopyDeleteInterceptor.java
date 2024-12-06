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
package openbackup.database.base.plugin.interceptor;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.provider.BaseCopyDeleteInterceptor;
import openbackup.data.access.framework.copy.mng.util.CopyUtil;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.DeleteCopyTask;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * AbstractDbCopyDeleteInterceptor:级联删除与是否下发agent公共
 *
 */
@Slf4j
@Component
public abstract class AbstractDbCopyDeleteInterceptor extends BaseCopyDeleteInterceptor {
    /**
     * 副本rest api
     */
    protected final CopyRestApi copyRestApi;

    private final ResourceService resourceService;

    /**
     * Constructor
     *
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     */
    public AbstractDbCopyDeleteInterceptor(CopyRestApi copyRestApi, ResourceService resourceService) {
        this.copyRestApi = copyRestApi;
        this.resourceService = resourceService;
    }

    /**
     * 非原生格式副本，需要下发agents, 任务资源不存在不下发agent
     *
     * @param copy copy
     * @param task task
     * @return true
     */
    @Override
    protected boolean shouldSupplyAgent(DeleteCopyTask task, CopyInfoBo copy) {
        return CopyUtil.isGeneratedByBackup(copyRestApi, copy) && isResourceExists(task);
    }

    /**
     * 获取与此相关联的资源，用于副本删除成功后 下次备份转全量
     *
     * @param copy 副本信息
     * @param requestId 下发id信息
     * @return 关联资源
     */
    protected List<String> addResourcesToSetNextFull(Copy copy, String requestId) {
        if (!CopyGeneratedByEnum.BY_BACKUP.value().equals(copy.getGeneratedBy())) {
            return Collections.emptyList();
        }
        Map<String, Object> fullCopyFilter = new HashMap<>();
        fullCopyFilter.put("backup_type", BackupTypeEnum.FULL.getAbbreviation());
        Copy latestFullCopy = copyRestApi.queryLatestBackupCopy(copy.getResourceId(), null, fullCopyFilter);
        Copy latestCopy = copyRestApi.queryLatestBackupCopy(copy.getResourceId(), null, null);
        // 如果被删除的副本不是备份副本（归档，复制等副本） 没有最新全量副本，没有副本场景， 退出
        if (VerifyUtil.isEmpty(latestFullCopy) || VerifyUtil.isEmpty(latestCopy)) {
            log.info("[SQL Server]latestFullCopy or latestCopy is empty. request id: {}", requestId);
            return Collections.emptyList();
        }
        switch (BackupTypeConstants.getBackupTypeByAbBackupType(copy.getBackupType())) {
            case FULL:
                // 当被删除的副本是全量副本时，只需要校验该副本是否与最新全量副本uuid相同，相同则添加资源，不同则退出
                // 针对最新的副本就是全量时，被删除的副本是最新全量副本则需要添加资源
                if (!Objects.equals(copy.getUuid(), latestFullCopy.getUuid())) {
                    log.info("[SQL Server]copy is not latest full copy, latestFullCopy: {}, copy id: {}, request id: "
                        + "{}", latestFullCopy.getUuid(), copy.getUuid(), requestId);
                    return Collections.emptyList();
                }
                // 针对最新的副本就是全量时，被删除的副本不是最新全量副本则退出
                return Collections.singletonList(copy.getResourceId());
            case CUMULATIVE_INCREMENT:
                // 当被删除的副本是差异副本时，只需要校验该副本是否与最新全量下的最新差异副本相同
                // （差异副本是否在最新全量后面，在则与被删除副本对比uuid，不在则直接退出）
                Map<String, Object> cumulativeIncrementCopyFilter = new HashMap<>();
                cumulativeIncrementCopyFilter.put("backup_type", BackupTypeEnum.CUMULATIVE_INCREMENT.getAbbreviation());
                Copy latestCumulativeIncrementCopy = copyRestApi.queryLatestBackupCopy(copy.getResourceId(), null,
                    cumulativeIncrementCopyFilter);
                if (VerifyUtil.isEmpty(latestCumulativeIncrementCopy)) {
                    log.error(
                        "[SQL Server] Can not find latest cumulative increment copy from resource id: {}, copy id: {}",
                        copy.getResourceId(), copy.getUuid());
                    return Collections.emptyList();
                }
                if (latestCumulativeIncrementCopy.getGn() < latestFullCopy.getGn() || !Objects.equals(copy.getUuid(),
                    latestCumulativeIncrementCopy.getUuid())) {
                    log.info(
                        "[SQL Server]copy is not latest cumulative increment copy, latestCumulativeIncrementCopy id: {}"
                            + ", copy id: {}, request id: {}", latestCumulativeIncrementCopy.getUuid(), copy.getUuid(),
                        requestId);
                    return Collections.emptyList();
                }
                return Collections.singletonList(copy.getResourceId());
            default:
                log.info("[SQL Server]copy type is wrong. request id: {}", requestId);
                return Collections.emptyList();
        }
    }


    /**
     * 判断资源是否存在：如果不存在不组装nodes/agents等参数
     *
     * @param task 删除任务
     * @return true or false
     */
    public boolean isResourceExists(DeleteCopyTask task) {
        if (task.getProtectEnv() == null || task.getProtectObject() == null) {
            return false;
        }
        String uuid = task.getProtectObject().getUuid();
        if (uuid == null) {
            return false;
        }
        return resourceService.getBasicResourceById(uuid)
            .filter(resource -> resourceService.getBasicResourceById(resource.getRootUuid()).isPresent())
            .isPresent();
    }

    /**
     * 判断主机是否离线：如果离线不组装nodes/agents等参数
     *
     * @param task 删除任务
     * @return true or false
     */
    public boolean isEnvironmentOffline(DeleteCopyTask task) {
        return task.getProtectEnv() == null
                || LinkStatusEnum.OFFLINE.getStatus().toString().equals(task.getProtectEnv().getLinkStatus());
    }

    /**
     * 收集与该副本相关联的需要删除的副本（不返回此副本本身），按删除顺序返回
     *
     * @param copyId 副本ID
     * @return 需要删除的关联的副本
     */
    @Override
    public List<String> getAssociatedCopy(String copyId) {
        return super.getAssociatedCopy(copyId);
    }

    /**
     * getShouldDeleteCopies
     *
     * @param copies copies
     * @param thisCopy thisCopy
     * @return List<String>
     */
    @Override
    protected List<String> getShouldDeleteCopies(List<Copy> copies, Copy thisCopy) {
        return super.getShouldDeleteCopies(copies, thisCopy);
    }

    /**
     * 全量副本时逻辑处理
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsFull(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        return CopyUtil.getCopyUuidsBetweenTwoCopy(copies, thisCopy, nextFullCopy);
    }

    /**
     * 增量副本时逻辑处理
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsDifferenceIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        return Collections.emptyList();
    }

    /**
     * 差异副本时逻辑处理
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsCumulativeIncrement(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        return Collections.emptyList();
    }

    /**
     * 日志副本时逻辑处理
     *
     * @param copies 本个副本之后的所有备份副本
     * @param thisCopy 本个副本
     * @param nextFullCopy 下个全量副本
     * @return 需要删除的集合
     */
    @Override
    protected List<String> getCopiesCopyTypeIsLog(List<Copy> copies, Copy thisCopy, Copy nextFullCopy) {
        return Collections.emptyList();
    }
}


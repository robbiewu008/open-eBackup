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
package openbackup.data.access.framework.copy.mng.service;

import openbackup.data.access.client.sdk.api.framework.dme.AvailableTimeRanges;
import openbackup.data.access.framework.copy.controller.req.CatalogQueryNoReq;
import openbackup.data.access.framework.copy.controller.req.CatalogQueryReq;
import openbackup.data.access.framework.core.model.CopySummaryResource;
import openbackup.system.base.bean.CopiesEntity;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.copy.model.StorageInfo;
import openbackup.system.base.sdk.dee.model.FineGrainedRestore;

import java.util.List;

/**
 * 副本相关的信息
 *
 */
public interface CopyService {
    /**
     * 浏览副本文件和目录信息
     *
     * @param copyId     副本id
     * @param catalogQueryReq 查询参数
     * @return 副本文件和目录信息
     */
    PageListResponse<FineGrainedRestore> listCopyCatalogs(String copyId, CatalogQueryNoReq catalogQueryReq);

    /**
     * 查询指定时间范围可用于恢复的时间段
     *
     * @param resourceId 资源id
     * @param startTime 开始时间
     * @param endTime 结束时间
     * @param pageSize 分页大小
     * @param pageNo 开始页
     * @return 可用于恢复的时间段
     */
    PageListResponse<AvailableTimeRanges> listAvailableTimeRanges(String resourceId, long startTime, long endTime,
        int pageSize, int pageNo);

    /**
     * 下载副本中的文件
     *
     * @param copyId 副本id
     * @param paths 恢复路径
     * @param recordId 导出记录ID
     * @return 请求id
     */
    String downloadFiles(String copyId, List<String> paths, String recordId);

    /**
     * 浏览副本文件和目录信息
     *
     * @param copyId     副本id
     * @param catalogQueryReq 查询参数
     * @return 副本文件和目录信息
     */
    PageListResponse<FineGrainedRestore> listCopyCatalogsByName(String copyId, CatalogQueryReq catalogQueryReq);

    /**
     * 查询副本的数量
     *
     * @param pageNo pageNo
     * @param pageSize pageSize
     * @param conditions conditions
     * @param orders orders
     * @return CopyResourceSummaryHump
     */
    PageListResponse<CopySummaryResource> listCopyResourceSummary(int pageNo, int pageSize, String conditions,
        String[] orders);

    /**
     * 查询副本
     *
     * @param resourceId resourceId
     * @return 查询副本
     */
    CopyResourceSummary queryCopyResourceSummary(String resourceId);

    /**
     * 根据资源 id、副本状态、拓展字段查询副本
     *
     * @param resourceId 资源id
     * @param status 副本状态
     * @param extendType 拓展字段
     * @param retryNum 重试次数（副本数量）
     * @return 查询副本
     */
    BasePage<Copy> queryCopiesByResourceIdAndStatusAndExtendType(
            String resourceId, String status, String extendType, int retryNum);

    /**
     * 查询副本存储设备信息
     *
     * @param copyId 副本ID
     * @return 副本信息
     */
    StorageInfo getStorageInfo(String copyId);

    /**
     * 开启副本guest system
     *
     * @param copyId copyId
     */
    void openCopyGuestSystem(String copyId);

    /**
     * 关闭副本guest system
     *
     * @param copyId copyId
     */
    void closeCopyGuestSystem(String copyId);

    /**
     * 删除关联无效副本（任务重试的）
     *
     * @param sourceId 资源ID
     * @param limit 上限
     * @param excludeCopies 不删除的副本
     */
    void deleteInvalidCopies(String sourceId, int limit, List<String> excludeCopies);

    /**
     * 根据复制副本ID查询副本实体
     *
     * @param copyId 复制副本ID
     * @return 返回查询到的副本实体
     */
    CopiesEntity queryOriginCopyIdById(String copyId);
}

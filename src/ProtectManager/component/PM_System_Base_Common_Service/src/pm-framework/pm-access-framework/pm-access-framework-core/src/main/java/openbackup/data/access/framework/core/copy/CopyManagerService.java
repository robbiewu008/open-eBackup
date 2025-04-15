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
package openbackup.data.access.framework.core.copy;

import openbackup.data.access.framework.core.model.CopySummaryCount;
import openbackup.data.access.framework.core.model.CopySummaryResource;
import openbackup.data.access.framework.core.model.CopySummaryResourceCondition;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.system.base.bean.CopiesEntity;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.model.Copy;

import java.util.Collections;
import java.util.List;

/**
 * copy service
 *
 */
public interface CopyManagerService {
    /**
     * 通过任务ID查询副本
     *
     * @param jobId jobId
     * @return copy
     */
    Copy queryCopyFromJobId(String jobId);

    /**
     * 副本删除后通知复制微服务
     *
     * @param jobId jobId
     * @param copy copy
     */
    void notifyWhenCopyDeleted(String jobId, Copy copy);

    /**
     * 构造副本资源
     *
     * @param copy copy
     * @return taskResource taskResource
     */
    TaskResource buildTaskResource(Copy copy);

    /**
     * 构建任务环境
     *
     * @param rootUuid rootUuid
     * @return TaskEnvironment 环境
     */
    TaskEnvironment buildTaskEnvironment(String rootUuid);

    /**
     * 查询关联的副本
     *
     * @param copyId 副本ID
     * @return 关联的副本ID
     */
    List<String> getAssociatedCopies(String copyId);

    /**
     * 分页查询副本资源
     *
     * @param pageSize 分页大小
     * @param pageNo 分页编码
     * @param orders 排序，字段前+表示增序，-表示减序
     * @param condition 查询条件
     * @return 副本资源列表和总数
     */
    PageListResponse<CopySummaryResource> queryCopySummaryResource(int pageSize, int pageNo, String[] orders,
        CopySummaryResourceCondition condition);

    /**
     * 通过副本id检查是否是san副本
     *
     * @param copyId 副本id
     * @return 是否是san副本
     */
    boolean checkSanCopy(String copyId);

    /**
     * 查询副本数量，按资源分类
     *
     * @param domainId 用户所在域id
     * @return 副本统计
     */
    List<CopySummaryCount> queryCopyCount(String domainId);

    /**
     * 更新副本状态
     *
     * @param copyId 副本ID
     * @param status 副本状态
     */
    default void updateCopyStatus(String copyId, String status) {
        if (VerifyUtil.isEmpty(copyId) || VerifyUtil.isEmpty(status)) {
            return;
        }
        updateCopyStatus(Collections.singletonList(copyId), status);
    }

    /**
     * 更新副本状态
     *
     * @param copyIdList 副本ID列表
     * @param status 副本状态
     */
    void updateCopyStatus(List<String> copyIdList, String status);


    /**
     * 更新副本存储单元状态
     *
     * @param copyIdList 副本ID列表
     * @param copyStorageUnitStatus 副本状态
     */
    void updateCopyStorageUnitStatus(List<String> copyIdList, int copyStorageUnitStatus);

    /**
     * 更新副本状态
     *
     * @param copyId 副本ID
     * @return 副本信息
     */
    CopiesEntity queryCopyById(String copyId);

    /**
     * 查询副本数量
     *
     * @param resourceId    @NotNull 资源ID
     * @param storageUnitId 存储单元id
     * @param backupTypes   备份类型
     * @return 副本数量
     */
    Long queryCopyCounts(String resourceId, String storageUnitId, List<Integer> backupTypes);

    /**
     * 归档副本，根据storage_id查询副本
     *
     * @param generatedBy 副本ID列表
     * @param storageId 副本状态
     * @return 副本id列表
     */
    List<String> queryCopyIdByStorageId(String generatedBy, String storageId);

    /**
     * 批量更新副本用户Id
     *
     * @param copyIdList    副本Idlist
     * @param userId 需要修改成的用户id
     */
    void updateCopiesUserId(List<String> copyIdList, String userId);
}

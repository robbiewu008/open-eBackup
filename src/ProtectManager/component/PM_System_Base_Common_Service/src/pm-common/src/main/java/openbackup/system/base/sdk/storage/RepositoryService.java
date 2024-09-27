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
package openbackup.system.base.sdk.storage;

import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.repository.RepositoryBase;
import openbackup.system.base.common.model.repository.RepositoryType;
import openbackup.system.base.common.model.repository.StorageInfo;
import openbackup.system.base.common.model.storage.CompleteStorageInfoRes;
import openbackup.system.base.common.model.storage.DeleteStorageRes;
import openbackup.system.base.common.model.storage.FullStorageInfo;
import openbackup.system.base.common.model.storage.StorageRequest;
import openbackup.system.base.common.model.storage.UpdateArchiveStorage;
import openbackup.system.base.sdk.storage.model.UpdateImportJobRequest;

import java.util.List;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author y00413474
 * @author w00493811
 * @version [BCManager 8.0.0]
 * @since 2020-06-19
 */
public interface RepositoryService {
    /**
     * 分隔存储编号获得存储池编号
     *
     * @param repositoryId 存储编号
     * @return 存储池编号
     */
    String getStoragePoolIdFromRepositoryId(String repositoryId);

    /**
     * 分页查询存储库
     *
     * @param repositoryType 类型，可为null
     * @param paging 分页条件
     * @param isReversed true:去除该类型，false:包括该类型
     * @return 分页数据
     */
    PageListResponse<RepositoryBase> pageQueryRepository(RepositoryType repositoryType, PagingParamRequest paging,
        boolean isReversed);

    /**
     * 按存储库id查询
     *
     * @param id 存储库id
     * @return 存储库数据
     */
    RepositoryBase queryRepository(String id);

    /**
     * /**
     * 按存储库name查询
     *
     * @param name 存储库name
     * @return 存储库数据
     */
    Optional<RepositoryBase> queryRepositoryByName(String name);

    /**
     * 查询存储信息
     *
     * @return StorageInfo
     */
    List<StorageInfo> storageInfo();

    /**
     * 添加存储库
     *
     * @param request 存储库请求
     * @return Integer 操作码
     */
    Integer createRepository(StorageRequest request);

    /**
     * 更新存储库
     *
     * @param request 更新存储库请求
     * @return Integer 操作码
     */
    Integer updateRepository(UpdateArchiveStorage request);

    /**
     * 删除存储库
     *
     * @param id 存储库id
     * @return DeleteStorageRes 回复
     */
    DeleteStorageRes deleteRepository(String id);

    /**
     * 获取完整的存储库信息
     *
     * @param id 存储库id
     * @return CompleteStorageInfoRes 存储库完整信息
     */
    CompleteStorageInfoRes queryCompleteStorage(String id);

    /**
     * 获取全部存储库信息
     *
     * @param id 存储库id
     * @return CompleteStorageInfoRes 存储库全部信息
     */
    FullStorageInfo queryFullStorage(String id);

    /**
     * 归档副本导入，创建调度
     *
     * @param storageId 存储库id
     */
    void importArchiveCopies(String storageId);

    /**
     * 更新导入副本任务
     *
     * @param updateImportJobRequest 更新job参数
     */
    void updateImportJob(UpdateImportJobRequest updateImportJobRequest);
}

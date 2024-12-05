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
package openbackup.data.access.framework.core.dao;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import openbackup.data.access.framework.core.entity.CopiesEntity;
import openbackup.data.access.framework.core.model.CopySummaryCount;
import openbackup.data.access.framework.core.model.CopySummaryResource;
import openbackup.data.access.framework.core.model.CopySummaryResourceQuery;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.springframework.stereotype.Component;

import java.util.List;

/**
 * 副本mapper
 *
 */
@Mapper
@Component
public interface CopyMapper extends BaseMapper<CopiesEntity> {
    /**
     * 分页查询副本资源列表
     *
     * @param query 查询条件
     * @return 副本资源列表
     */
    List<CopySummaryResource> selectCopySummaryResourceList(@Param("query") CopySummaryResourceQuery query);

    /**
     * 根据条件查询副本资源的总数
     *
     * @param query 查询条件
     * @return 总数
     */
    int selectCopySummaryResourceCount(@Param("query") CopySummaryResourceQuery query);

    /**
     * 根据resourceId查询当前资源下副本所有的deviceEsn
     *
     * @param resourceId resourceId 资源ID
     * @return List<String>  deviceEsnList</>
     */
    List<String> selectCopyDeviceEsnByResourceId(@Param("resource_id") String resourceId);

    /**
     * 统计副本
     *
     * @param domainId 用户所在域id，为空则统计所有
     * @return 副本统计计数
     */
    List<CopySummaryCount> queryCopyCount(@Param("domain_id") String domainId);

    /**
     * 更新副本状态
     *
     * @param copyIdList 副本ID列表
     * @param status 副本状态
     */
    void updateCopyStatus(@Param("copy_id_list") List<String> copyIdList, @Param("status") String status);

    /**
     * 更新副本资源名称
     *
     * @param newResourceName 资源新名称
     * @param resourceId 资源id
     */
    void updateCopyResourceName(@Param("new_resource_name") String newResourceName,
        @Param("resource_id") String resourceId);

    /**
     * 定时更新副本表中worm状态：将已worm且过期时间大于当前时间的副本worm状态更新为已过期
     *
     * @param currentTime 当前时间
     */
    void updateWormCopyExpiredStatus(@Param("current_time") String currentTime);

    /**
     * 更新副本所属用户
     *
     * @param copyIdList 副本ID列表
     * @param userId 用户信息
     */
    void updateCopyUserId(@Param("copy_id_list") List<String> copyIdList, @Param("userId") String userId);
}

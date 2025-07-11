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
package openbackup.access.framework.resource.persistence.dao;

import com.huawei.emeistor.kms.kmc.util.security.exterattack.ExterAttack;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import openbackup.access.framework.resource.persistence.model.ResourceGroupMemberPo;

import org.springframework.data.repository.query.Param;
import org.springframework.stereotype.Repository;

import java.util.List;

/**
 * 资源组成员dao
 *
 */
@Repository
public interface ResourceGroupMemberMapper extends BaseMapper<ResourceGroupMemberPo> {
    /**
     * selectByResourceGroupId
     *
     * @param resourceGroupId resourceGroupId
     * @return List<ResourceGroupMemberPo>
     */
    @ExterAttack
    List<ResourceGroupMemberPo> selectByResourceGroupId(@Param("resourceGroupId") String resourceGroupId);

    /**
     * selectByResourceId
     *
     * @param resourceId resourceId
     * @return List<ResourceGroupMemberPo>
     */
    @ExterAttack
    List<ResourceGroupMemberPo> selectByResourceId(@Param("resourceId") String resourceId);
}
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

import com.baomidou.mybatisplus.core.conditions.Wrapper;
import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.baomidou.mybatisplus.core.metadata.IPage;

import openbackup.access.framework.resource.persistence.model.ResourceGroupExtendField;
import openbackup.access.framework.resource.persistence.model.ResourceGroupPo;

import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;
import org.springframework.stereotype.Repository;

import java.util.List;

/**
 * 资源组dao
 *
 */
@Repository
public interface ResourceGroupMapper extends BaseMapper<ResourceGroupPo> {
    /**
     * 联表查询语句
     */
    String JOIN_SQL = "select g.*, p.sla_name, p.sla_compliance as is_sla_compliance "
            + "from t_resource_group as g "
            + "left join protected_object as p on g.uuid = p.uuid ";

    /**
     * 分页查询语句
     */
    String PAGE_SQL = "select * from ( " + JOIN_SQL + " ) as q ${ew.customSqlSegment}";

    /**
     * selectByName
     *
     * @param scopeResourceId scopeResourceId
     * @param name name
     * @return List<ResourceGroupPo>
     */
    @ExterAttack
    List<ResourceGroupPo> selectByScopeResourceIdAndName(
            @Param("scopeResourceId") String scopeResourceId, @Param("name") String name);

    /**
     * page
     *
     * @param page page
     * @param queryWrapper queryWrapper
     * @return IPage<ResourceGroupExtendField>
     */
    @Select(PAGE_SQL)
    IPage<ResourceGroupExtendField> page(IPage<ResourceGroupExtendField> page,
                                         @Param("ew") Wrapper<ResourceGroupExtendField> queryWrapper);
}
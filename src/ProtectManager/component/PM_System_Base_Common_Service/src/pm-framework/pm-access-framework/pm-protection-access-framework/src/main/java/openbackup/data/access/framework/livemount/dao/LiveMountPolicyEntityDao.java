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
package openbackup.data.access.framework.livemount.dao;

import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.system.base.common.annotation.DbMangerMapper;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;

import com.baomidou.mybatisplus.core.conditions.Wrapper;
import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.baomidou.mybatisplus.core.metadata.IPage;

import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;

/**
 * Updating Policy Entity Dao
 *
 * @author l00272247
 * @since 2020-09-17
 */
@DbMangerMapper
@CalleeMethods(
        name = "live_mount_policy_dao",
        value = {@CalleeMethod(name = "selectById"), @CalleeMethod(name = "selectBatchIds")})
public interface LiveMountPolicyEntityDao extends BaseMapper<LiveMountPolicyEntity> {
    /**
     * JOIN_SQL
     */
    String JOIN_SQL =
            "select a.*, case when c.total is null then 0 else c.total end as live_mount_count "
                    + "from live_mount_policy as a"
                    + " left join (select policy_id, count(id) as total "
                    + " from LIVE_MOUNT group by policy_id) c on c.policy_id = a.policy_id";

    /**
     * WRAP_SQL
     */
    String WRAP_SQL = "SELECT * from ( " + JOIN_SQL + " ) AS q ${ew.customSqlSegment}";

    /**
     * page query
     *
     * @param page page
     * @param queryWrapper query wrapper
     * @return page
     */
    @Select(WRAP_SQL)
    IPage<LiveMountPolicyEntity> page(
            IPage<LiveMountPolicyEntity> page, @Param("ew") Wrapper<LiveMountPolicyEntity> queryWrapper);

    /**
     * select police by id
     *
     * @param policyId policy id
     * @return policy
     */
    LiveMountPolicyEntity selectPolicy(String policyId);
}

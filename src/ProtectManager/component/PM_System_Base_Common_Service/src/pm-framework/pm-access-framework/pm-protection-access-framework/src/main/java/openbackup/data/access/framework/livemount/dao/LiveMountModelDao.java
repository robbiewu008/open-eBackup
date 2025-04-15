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

import com.baomidou.mybatisplus.core.conditions.Wrapper;
import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.baomidou.mybatisplus.core.metadata.IPage;

import openbackup.data.access.framework.livemount.common.model.LiveMountModel;

import org.apache.ibatis.annotations.Mapper;
import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;
import org.springframework.stereotype.Component;

/**
 * Live Mount Model Dao
 *
 */
@Mapper
@Component
public interface LiveMountModelDao extends BaseMapper<LiveMountModel> {
    /**
     * JOIN_SQL
     */
    String JOIN_SQL = "select a.*, b.name as policy_name, d.cluster_name as cluster_name from live_mount as a "
            + "left join live_mount_policy as b on a.policy_id = b.policy_id "
            + "left join copies as c on a.copy_id = c.uuid "
            + "left join t_cluster_member as d on c.device_esn = d.remote_esn";

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
    IPage<LiveMountModel> page(IPage<LiveMountModel> page, @Param("ew") Wrapper<LiveMountModel> queryWrapper);
}

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

import openbackup.system.base.common.annotation.DbMangerMapper;
import openbackup.system.base.common.model.livemount.LiveMountEntity;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Param;
import org.apache.ibatis.annotations.Select;

import java.util.List;

/**
 * Live Mount Entity Dao
 *
 */
@DbMangerMapper
public interface LiveMountEntityDao extends BaseMapper<LiveMountEntity> {
    /**
     * JOIN_SQL
     */
    String JOIN_SQL = "select a.* from live_mount a left join live_mount_policy b on a.policy_id = b.policy_id "
        + "where resource_id=#{id} and schedule_policy='after_backup_done'";

    /**
     * page query
     *
     * @param resourceId resource id
     * @return page
     */
    @Select(JOIN_SQL)
    List<LiveMountEntity> queryAutoUpdateWhenBackupDoneLiveMounts(@Param("id") String resourceId);
}

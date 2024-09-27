/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.dao;

import openbackup.data.access.framework.core.entity.ReplicationPlanEntity;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.springframework.stereotype.Component;

/**
 * Replication Plan Entity Dao
 *
 * @author l00272247
 * @since 2021-02-08
 */
@Component
public interface ReplicationPlanEntityDao extends BaseMapper<ReplicationPlanEntity> {
    /**
     * find replication plan entity
     *
     * @param resourceId resource id
     * @param targetClusterId target cluster id
     * @return replication plan entity
     */
    default ReplicationPlanEntity find(String resourceId, String targetClusterId) {
        LambdaQueryWrapper<ReplicationPlanEntity> wrapper =
            new LambdaQueryWrapper<ReplicationPlanEntity>().eq(ReplicationPlanEntity::getResourceId, resourceId)
                .eq(ReplicationPlanEntity::getTargetClusterId, targetClusterId);
        return selectOne(wrapper);
    }
}

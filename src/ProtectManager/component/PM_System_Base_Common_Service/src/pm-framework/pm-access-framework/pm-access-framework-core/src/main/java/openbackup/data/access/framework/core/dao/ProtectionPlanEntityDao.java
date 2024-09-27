/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.dao;

import openbackup.data.access.framework.core.entity.ProtectionPlanEntity;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.springframework.stereotype.Component;

/**
 * Protection Plan Entity Dao
 *
 * @author l00557046
 * @version [OceanProtect A8000 1.1.0]
 * @since 2020-08-12
 */
@Component
public interface ProtectionPlanEntityDao extends BaseMapper<ProtectionPlanEntity> {
}

/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.servitization.dao;

import openbackup.data.access.framework.servitization.entity.VpcInfoEntity;
import openbackup.system.base.common.annotation.DbMangerMapper;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

/**
 * VpcEntityDao
 *
 * @author l30044826
 * @since 2023-08-14
 */
@DbMangerMapper
public interface VpcEntityDao extends BaseMapper<VpcInfoEntity> {

}

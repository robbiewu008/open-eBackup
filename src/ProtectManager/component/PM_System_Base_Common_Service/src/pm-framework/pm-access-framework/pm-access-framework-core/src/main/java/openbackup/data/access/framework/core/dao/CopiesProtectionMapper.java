/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.access.framework.core.dao;

import openbackup.data.access.framework.core.entity.CopiesProtectionEntity;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Mapper;
import org.springframework.stereotype.Component;

/**
 * CopiesProtectionMapper
 *
 * @author l30044826
 * @since 2024-01-29
 */
@Mapper
@Component
public interface CopiesProtectionMapper extends BaseMapper<CopiesProtectionEntity> {
}

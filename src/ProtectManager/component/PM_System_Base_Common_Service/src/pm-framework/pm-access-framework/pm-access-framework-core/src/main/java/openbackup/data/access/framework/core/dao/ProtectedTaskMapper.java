/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.core.dao;

import openbackup.data.access.framework.core.entity.ProtectedTaskPo;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.springframework.stereotype.Repository;

/**
 * Protected Task Mapper
 *
 * @author c00631681
 * @since 2024-01-29
 */
@Repository
public interface ProtectedTaskMapper extends BaseMapper<ProtectedTaskPo> {
}
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.access.framework.resource.persistence.dao;

import openbackup.access.framework.resource.persistence.model.ProtectedAgentExtendPo;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Mapper;
import org.springframework.stereotype.Component;

/**
 * agent扩展表查询语句
 *
 * @author z00613137
 * @since 2023-08-09
 */
@Mapper
@Component
public interface ProtectedAgentExtendMapper extends BaseMapper<ProtectedAgentExtendPo> {
}
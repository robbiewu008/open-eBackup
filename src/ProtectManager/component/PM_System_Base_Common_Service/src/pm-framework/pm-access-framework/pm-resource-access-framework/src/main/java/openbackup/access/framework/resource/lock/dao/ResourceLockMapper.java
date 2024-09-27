/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.access.framework.resource.lock.dao;

import openbackup.access.framework.resource.lock.entity.ResourceLock;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Mapper;
import org.springframework.stereotype.Component;

/**
 * 功能描述: LockResourceMapper
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-07-07
 */
@Mapper
@Component
public interface ResourceLockMapper extends BaseMapper<ResourceLock> {
}
/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.access.framework.resource.persistence.dao;

import openbackup.access.framework.resource.persistence.model.ProtectedEnvironmentPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;

import org.apache.ibatis.annotations.Mapper;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * agent主机mybatis Mapper
 *
 * @author z00613137
 * @since 2023-08-08
 */
@Mapper
@Component
public interface ProtectedResourceAgentMapper extends BaseMapper<ProtectedResourcePo> {
    /**
     * 根据条件查询主机资源列表
     *
     * @param map 查询条件
     * @return 资源列表
     */
    List<ProtectedEnvironmentPo> queryAgentResourceList(Map<String, Object> map);

    /**
     * 根据条件查询主机资源的总数
     *
     * @param map 查询条件
     * @return 资源列表
     */
    int queryAgentResourceCount(Map<String, Object> map);

    /**
     * 查询共享Agent
     *
     * @return 资源列表
     */
    List<String> querySharedAgentIds();
}
